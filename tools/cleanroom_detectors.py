#!/usr/bin/env python3
"""Clean-room content detectors (see docs/CLEANROOM.md).

This is the engine behind `tools/cleanroom_check.sh`; run that, not this.
The shell script decides *what* to look at -- the worktree, the index, or
every tree in a commit range -- and hands the work list to this script on
stdin.  This script decides *whether any of it looks like ROM content*.

Input: one entry per line, tab-separated, four fields:

    kind <TAB> ident <TAB> label <TAB> path

    kind   "file" (ident is a filesystem path), "blob" (ident is a git blob
           SHA, read via `git cat-file --batch`), or "link" (a submodule
           gitlink -- path checks only, there is no blob here to read).
    ident  what to read the bytes from, per `kind`.
    label  where this came from, for the failure message ("worktree",
           "staged", "commit deadbee").
    path   the repo-relative path.  Last field, so a path containing a tab
           still survives the split.

Content work is deduplicated by `ident`, which is what makes `--range` over
a long history cheap: a file that never changed is one blob no matter how
many commits it appears in.  Path work is deduplicated by `path`.

Every threshold below is measured, not guessed.  The numbers quoted in the
comments are the worst case over *all 183 blobs in this repository's
history* versus the two workbench ledgers that had to be purged from it --
the incident this whole file exists to prevent from recurring.  Those
ledgers are the calibration set for "must fail"; the history is the
calibration set for "must pass".
"""

import re
import subprocess
import sys

# --------------------------------------------------------------------------
# Thresholds.  Each one is a pair -- an absolute count and a rate -- because
# neither half discriminates alone.  A tiny file can be dense by accident (a
# header that names two registers); a large document can mention a handful of
# mnemonics without being a dump.  Both must trip.
# --------------------------------------------------------------------------

# 1. Instruction text.  Distinctive MIPS mnemonics as whole tokens.
#    Case-insensitive: an uppercase listing is still a listing, and matching
#    both spellings costs exactly nothing here -- across every blob in this
#    history the case-insensitive count equals the case-sensitive count.
#    history max: 16 tokens (include/game/runlink.h) -- 2.5x under the limit.
#    purged ledgers: 88 tokens @ 1.298/KiB and 4407 @ 4.558/KiB -- both over.
MNEMONICS = re.compile(r"^(?:addiu|lw|sw|jal|beq|lui|sltu)$", re.IGNORECASE)
MNEMONIC_COUNT_LIMIT = 40
MNEMONIC_RATE_LIMIT = 1.000  # tokens per KiB

# 2. Machine words, bare.  Hex runs of 4/8/16 digits that are NOT 0x-prefixed
#    and not part of an identifier, mixing digits with a-f.  Source and docs
#    write addresses as `0x80001234`; a bare-hex dump writes them naked.
#    history max: 3 tokens in any single blob -- 10x under the limit.
#    purged ledgers: 126 @ 1.86/KiB and 2973 @ 3.07/KiB -- both well over.
#
#    Deliberately bare-only.  Widening it to 0x-prefixed forms was measured and
#    rejected: symbol_addrs.us.txt carries 215 such tokens at 4.917/KiB and
#    docs/modules.md 80 at 2.715/KiB, so any count-and-rate rule covering them
#    either fires on this tree or is too loose to be worth having.  The
#    0x-prefixed case is covered by rules 3 and 4 instead, which key on shape
#    and on value distribution rather than on volume.
BARE_HEX_WORD = re.compile(
    r"(?<![0-9A-Za-z_.$-])(?:[0-9a-fA-F]{4}|[0-9a-fA-F]{8}|[0-9a-fA-F]{16})(?![0-9A-Za-z_.$-])"
)
HEXWORD_COUNT_LIMIT = 32
HEXWORD_RATE_LIMIT = 0.500  # tokens per KiB

# 3. Word arrays.  Machine words -- in ANY encoding, 0x-prefixed included --
#    written adjacently, separated only by commas and whitespace.  That is the
#    shape of a C array of ROM words, of `.word` directives, and of a column
#    dump; nothing in a source tree writes four 32-bit constants in a row.
#    Keying on adjacency rather than volume is what lets this cover the
#    0x-prefixed forms rule 2 must leave alone.
#    history max: 0 tokens in adjacency runs, in every blob ever.
#    0x-prefixed ROM word array fixture: 400 -- 25x over the limit.
WORD_RUN = re.compile(
    r"(?:(?:0[xX][0-9a-fA-F]{8}|0[xX][0-9a-fA-F]{16}"
    r"|(?<![0-9A-Za-z_.$-])[0-9a-fA-F]{8}|(?<![0-9A-Za-z_.$-])[0-9a-fA-F]{16})"
    r"(?![0-9A-Za-z_.$-])(?:[ \t]*,[ \t]*|\s+)){3,}"
    r"(?:0[xX][0-9a-fA-F]{8}|0[xX][0-9a-fA-F]{16}"
    r"|(?<![0-9A-Za-z_.$-])[0-9a-fA-F]{8}|(?<![0-9A-Za-z_.$-])[0-9a-fA-F]{16})"
    r"(?![0-9A-Za-z_.$-])"
)
WORD_RUN_TOKEN = re.compile(r"(?:0[xX])?[0-9a-fA-F]{8,16}")
WORD_ARRAY_LIMIT = 16  # tokens appearing inside such runs

# 4. Word tables.  The single-line JSON case, where adjacency never happens
#    because every word is wrapped in its own key.  What still separates a dump
#    of the ROM's instructions from this tree's hex is the *value distribution*:
#    addresses cluster (everything here is 0x8xxxxxxx VRAM or 0x0xxxxxxx ROM
#    offsets), while instruction words spread across the whole 32-bit space.
#    So: count 32-bit-valued tokens in any encoding -- bare hex, 0x-prefixed or
#    decimal -- and require BOTH a real population AND a wide spread of their
#    high bytes.  Neither gate alone survives contact with this tree.
#    history max: symbol_addrs.us.txt 217 values but only 2 distinct high bytes
#    (8x under the spread limit); tools/n64crc.c 12 distinct high bytes but
#    only 16 values (4x under the count limit).  Every blob in history fails at
#    least one gate by 4x or more.
#    purged ledgers: 157/18 and 3895/35.  0x-prefixed ledger fixture: 400/46.
#    decimal-encoded fixture: 369/55.
WORD_HEX = re.compile(
    r"(?<![0-9A-Za-z_.$-])(?:0[xX])?([0-9a-fA-F]{8})(?![0-9A-Za-z_.$-])"
)
WORD_DEC = re.compile(r"(?<![0-9A-Za-z_.$-])([0-9]{8,10})(?![0-9A-Za-z_.$-])")
WORD_TABLE_COUNT_LIMIT = 64
WORD_TABLE_SPREAD_LIMIT = 16  # distinct high bytes among those values

# 5. Hexdump layout.  `xxd`/`hexdump -C`/`od`, and C byte arrays of ROM bytes,
#    share a shape: most tokens on the line are short hex.
#    history max: 0 such lines in any blob, ever -- so these thresholds are set
#    by what they must CATCH, not by what they must let through.  4 tokens per
#    line covers narrow `od -w5`-style output; the 5% share stops a dump from
#    hiding inside a large file of prose.
HEX_TOKEN = re.compile(
    r"^(?:0[xX][0-9a-fA-F]{2}|0[xX][0-9a-fA-F]{4}"
    r"|[0-9a-fA-F]{2}|[0-9a-fA-F]{4}|[0-9a-fA-F]{8}|[0-9a-fA-F]{16})$"
)
HEXDUMP_TOKENS_PER_LINE = 4  # hex tokens needed before a line counts
HEXDUMP_LINE_LIMIT = 8  # such lines needed before the file counts
HEXDUMP_LINE_SHARE = 0.05  # ...and they must be this share of nonblank lines

# 6. Base64 blobs.  Two rules, because base64 is routinely line-wrapped -- MIME
#    at 76 columns, `openssl base64` at 64 -- and a rule keyed on the longest
#    unbroken run sees a wrapped 4KB ROM blob as a series of harmless
#    76-character strings.  So: the longest run AND the total volume.
#    history max: 58-character longest run (a cache key in a workbench
#    manifest), 2.2x under; 918 aggregate characters at 160.3/KiB in that same
#    manifest, 2.2x and 2.5x under.
#    wrapped-ROM fixture: 76-character runs, invisible to the run rule, but
#    5386 characters at 996.3/KiB -- 2.6x and 2.5x over.
BASE64_RUN = re.compile(r"[A-Za-z0-9+/]{32,}")
BASE64_RUN_LIMIT = 128  # longest single run
BASE64_TOTAL_LIMIT = 2048  # aggregate base64-shaped characters...
BASE64_RATE_LIMIT = 400.0  # ...at this many per KiB

# 7. Size.  Nothing legitimate in a decomp source tree is this big; extracted
#    data is.  history max: 44,779 B (symbol_addrs.us.txt) -- 5.9x under.
SIZE_LIMIT = 256 * 1024
# Paths permitted to exceed SIZE_LIMIT or to be non-text.  Empty by design:
# add an entry only with a written reason, and expect to justify it at review.
SIZE_ALLOWLIST: "set[str]" = set()
BINARY_ALLOWLIST: "set[str]" = set()

# 8. Aggregate budget.  Every rule above is per-file, so a leak spread thinly
#    across many files passes all of them: 13 files carrying 31 bare machine
#    words each is 403 ROM words with no single file breaking a rule.  This is
#    the backstop -- the total across one scan unit (the worktree, the index,
#    or one commit's tree).
#    Per-tree rather than per-scan on purpose: a whole-history scan must not
#    accumulate an ever-growing total and eventually fail on its own success.
#    measured: current worktree 8, and no historical tree exceeds it -- 16x
#    under the budget.
AGGREGATE_BARE_WORD_BUDGET = 128

# 9. Whitelisted workbench manifests get a far tighter budget than an ordinary
#    file.  The path whitelist says campaigns/*/manifest.json may be tracked;
#    that is a statement about paths and hashes, not a licence to carry ROM
#    words under an approved filename.  A manifest that trips these is either a
#    bug in the workbench or a ledger wearing a manifest's name.
#    both tracked manifests measure 0 on all three.
MANIFEST_MAX_BARE_WORDS = 4
MANIFEST_MAX_WORD_VALUES = 4
MANIFEST_MAX_MNEMONICS = 4

# --------------------------------------------------------------------------
# Path rules.
# --------------------------------------------------------------------------

# Paths/extensions the clean-room policy says must never be tracked: ROM
# images, and anything extracted or derived from one.  Mirrors the clean-room
# section of .gitignore.
ROM_PATH = re.compile(
    r"\.(?:z64|n64|v64|bin)$|(?:^|/)baseroms/|(?:^|/)asm/|(?:^|/)assets/|(?:^|/)expected/",
    re.IGNORECASE,
)

# The workbench keeps per-campaign state under .decomp-workbench/.  Exactly
# one kind of file there is ever allowed to be tracked: the manifest, which is
# paths and hashes only.  Everything else -- ledgers above all -- is ROM-
# derived by construction.  This is a whitelist, not a blacklist, because the
# incident happened with a filename nobody had thought to blacklist.
WORKBENCH_ROOT = ".decomp-workbench/"
WORKBENCH_ALLOWED = re.compile(r"^\.decomp-workbench/campaigns/[^/]+/manifest\.json$")

IDENT_TOKEN = re.compile(r"[A-Za-z0-9_]+")


class Findings:
    def __init__(self) -> None:
        self.items: "list[tuple[str, str, str, list[str]]]" = []

    def add(self, detector, path, label, lines):
        self.items.append((detector, path, label, lines))

    def report(self) -> int:
        if not self.items:
            return 0
        # Group by detector so a broad failure reads as one problem.
        self.items.sort(key=lambda i: (i[0], i[1]))
        for detector, path, label, lines in self.items:
            print(f"cleanroom: FAIL [{detector}] {path}  ({label})", file=sys.stderr)
            for line in lines:
                print(f"    {line}", file=sys.stderr)
        return 1


# --------------------------------------------------------------------------
# Detectors.  Each returns a list of detail lines, empty if clean.
# --------------------------------------------------------------------------


def check_path(path):
    """Path-only rules.  Cheap, and they run even on submodule gitlinks."""
    out = []
    if ROM_PATH.search(path):
        out.append(("rom-path", ["tracked path matches a ROM/extracted-asset pattern"]))
    if path.startswith(WORKBENCH_ROOT) and not WORKBENCH_ALLOWED.match(path):
        out.append(
            (
                "workbench-path",
                [
                    "only .decomp-workbench/campaigns/*/manifest.json may be tracked",
                    "under .decomp-workbench/; everything else there is ROM-derived",
                ],
            )
        )
    return out


def word_values(text):
    """Return the 32-bit values written in this text, in any encoding.

    Hex (bare or 0x-prefixed) and decimal alike, because the encoding is the
    author's free choice and the value distribution is not.
    """
    values = []
    for match in WORD_HEX.finditer(text):
        values.append(int(match.group(1), 16))
    for match in WORD_DEC.finditer(text):
        value = int(match.group(1))
        # Below 2^24 it is a plausible ordinary number -- a size, a line count,
        # a byte offset.  At or above 2^32 it is not a machine word at all.
        if (1 << 24) <= value < (1 << 32):
            values.append(value)
    return values


def check_content(path, data):
    """Content rules.

    Returns ``([(detector, [detail lines]), ...], metrics)``.  The metrics feed
    the aggregate budget in :func:`main`, which is the one rule that cannot be
    decided by looking at a single file.
    """
    out = []
    metrics = {"bare_words": 0}
    size = len(data)
    if size == 0:
        return out, metrics

    if size > SIZE_LIMIT and path not in SIZE_ALLOWLIST:
        out.append(
            (
                "oversize",
                [
                    f"{size} bytes exceeds the {SIZE_LIMIT}-byte tracked-file limit",
                    "large tracked files are extracted data until proven otherwise;"
                    " allowlist in tools/cleanroom_detectors.py with a reason",
                ],
            )
        )

    if b"\0" in data[:8192]:
        text = None
    else:
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            text = None

    if text is None:
        if path not in BINARY_ALLOWLIST:
            out.append(
                ("binary-blob", ["not valid UTF-8 text; every tracked file must be text"])
            )
        return out, metrics

    # -- instruction text ---------------------------------------------------
    count = 0
    for token in IDENT_TOKEN.finditer(text):
        if MNEMONICS.match(token.group(0)):
            count += 1
    if count >= MNEMONIC_COUNT_LIMIT:
        rate = count * 1024.0 / size
        if rate >= MNEMONIC_RATE_LIMIT:
            out.append(
                (
                    "instruction-dump",
                    [
                        f"{count} MIPS mnemonics in {size}B = {rate:.3f} per KiB"
                        f" (limit: >={MNEMONIC_COUNT_LIMIT} and >={MNEMONIC_RATE_LIMIT:.3f}/KiB)",
                        f"first at line {_first_line(text, lambda l: any(MNEMONICS.match(t.group(0)) for t in IDENT_TOKEN.finditer(l)))}",
                    ],
                )
            )

    # -- machine words ------------------------------------------------------
    words = [
        w
        for w in BARE_HEX_WORD.findall(text)
        if any(c.isdigit() for c in w) and any(c.lower() in "abcdef" for c in w)
    ]
    metrics["bare_words"] = len(words)
    if len(words) >= HEXWORD_COUNT_LIMIT:
        rate = len(words) * 1024.0 / size
        if rate >= HEXWORD_RATE_LIMIT:
            out.append(
                (
                    "machine-word-dump",
                    [
                        f"{len(words)} bare hex machine words in {size}B = {rate:.3f} per KiB"
                        f" (limit: >={HEXWORD_COUNT_LIMIT} and >={HEXWORD_RATE_LIMIT:.3f}/KiB)",
                        "source and docs write addresses as 0x...; bare hex words are a dump",
                        f"first at line {_first_line(text, lambda l: bool(BARE_HEX_WORD.search(l)))}",
                    ],
                )
            )

    # -- word arrays (adjacency) --------------------------------------------
    run_tokens = 0
    first_run_line = 0
    for match in WORD_RUN.finditer(text):
        run_tokens += len(WORD_RUN_TOKEN.findall(match.group(0)))
        if not first_run_line:
            first_run_line = text.count("\n", 0, match.start()) + 1
    if run_tokens >= WORD_ARRAY_LIMIT:
        out.append(
            (
                "word-array",
                [
                    f"{run_tokens} machine words written adjacently"
                    f" (limit: >={WORD_ARRAY_LIMIT})",
                    "four 32-bit constants in a row, separated only by commas or"
                    " whitespace, is an array of ROM words in any encoding",
                    f"first at line {first_run_line}",
                ],
            )
        )

    # -- word tables (value distribution) -----------------------------------
    values = word_values(text)
    spread = len({(value >> 24) & 0xFF for value in values})
    if len(values) >= WORD_TABLE_COUNT_LIMIT and spread >= WORD_TABLE_SPREAD_LIMIT:
        out.append(
            (
                "word-table",
                [
                    f"{len(values)} 32-bit word values spread over {spread} distinct"
                    f" high bytes (limit: >={WORD_TABLE_COUNT_LIMIT} values and"
                    f" >={WORD_TABLE_SPREAD_LIMIT} high bytes)",
                    "addresses cluster (0x8xxxxxxx, 0x0xxxxxxx); instruction words"
                    " spread across the whole 32-bit space",
                ],
            )
        )

    # -- hexdump layout -----------------------------------------------------
    nonblank = 0
    hexlines = 0
    first_hexline = 0
    for lineno, line in enumerate(text.split("\n"), 1):
        tokens = [t.strip(",;:()[]{}'\"<>") for t in line.split()]
        tokens = [t for t in tokens if t]
        if tokens:
            nonblank += 1
        hits = sum(1 for t in tokens if HEX_TOKEN.match(t))
        if hits >= HEXDUMP_TOKENS_PER_LINE and hits * 2 >= len(tokens):
            hexlines += 1
            if not first_hexline:
                first_hexline = lineno
    if hexlines >= HEXDUMP_LINE_LIMIT and nonblank and hexlines >= nonblank * HEXDUMP_LINE_SHARE:
        out.append(
            (
                "hexdump",
                [
                    f"{hexlines} of {nonblank} nonblank lines are hexdump-shaped"
                    f" (limit: >={HEXDUMP_LINE_LIMIT} lines and >={HEXDUMP_LINE_SHARE:.0%} of them)",
                    f"first at line {first_hexline}",
                ],
            )
        )

    # -- base64 blobs -------------------------------------------------------
    worst = 0
    worst_line = 0
    total_b64 = 0
    for lineno, line in enumerate(text.split("\n"), 1):
        for m in BASE64_RUN.finditer(line):
            run = m.group(0)
            if not (
                any(c.islower() for c in run)
                and any(c.isupper() for c in run)
                and any(c.isdigit() for c in run)
            ):
                continue
            total_b64 += len(run)
            if len(run) > worst:
                worst = len(run)
                worst_line = lineno
    if worst >= BASE64_RUN_LIMIT:
        out.append(
            (
                "base64-blob",
                [
                    f"{worst}-character base64-shaped run (limit: >={BASE64_RUN_LIMIT})",
                    f"first at line {worst_line}",
                ],
            )
        )
    b64_rate = total_b64 * 1024.0 / size
    if total_b64 >= BASE64_TOTAL_LIMIT and b64_rate >= BASE64_RATE_LIMIT:
        out.append(
            (
                "base64-volume",
                [
                    f"{total_b64} base64-shaped characters in {size}B ="
                    f" {b64_rate:.1f} per KiB (limit: >={BASE64_TOTAL_LIMIT} and"
                    f" >={BASE64_RATE_LIMIT:.0f}/KiB)",
                    "line-wrapped base64 hides from a longest-run rule; total"
                    " volume is what a wrapped blob cannot hide",
                ],
            )
        )

    # -- whitelisted workbench manifests: a much tighter budget --------------
    if WORKBENCH_ALLOWED.match(path):
        for name, measured, limit in (
            ("bare machine words", len(words), MANIFEST_MAX_BARE_WORDS),
            ("32-bit word values", len(values), MANIFEST_MAX_WORD_VALUES),
            ("MIPS mnemonics", count, MANIFEST_MAX_MNEMONICS),
        ):
            if measured > limit:
                out.append(
                    (
                        "workbench-manifest-content",
                        [
                            f"{measured} {name} in a whitelisted manifest"
                            f" (limit: {limit})",
                            "the path whitelist covers paths and hashes, not ROM"
                            " content wearing an approved filename",
                        ],
                    )
                )

    return out, metrics


def _first_line(text, pred):
    for lineno, line in enumerate(text.split("\n"), 1):
        if pred(line):
            return lineno
    return 0


# --------------------------------------------------------------------------
# Driver.
# --------------------------------------------------------------------------


def read_blobs(shas):
    """Read many blobs through one `git cat-file --batch` process."""
    contents = {}
    if not shas:
        return contents
    proc = subprocess.Popen(
        ["git", "cat-file", "--batch"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
    )
    try:
        for sha in shas:
            proc.stdin.write((sha + "\n").encode())
            proc.stdin.flush()
            header = proc.stdout.readline().decode().split()
            if len(header) < 3:
                contents[sha] = None  # missing object; nothing to inspect
                continue
            size = int(header[2])
            payload = proc.stdout.read(size)
            proc.stdout.read(1)  # trailing newline
            contents[sha] = payload if header[1] == "blob" else None
    finally:
        proc.stdin.close()
        proc.wait()
    return contents


def main():
    entries = []
    for raw in sys.stdin.read().split("\n"):
        if not raw:
            continue
        parts = raw.split("\t", 3)
        if len(parts) != 4:
            print(f"cleanroom: malformed work item: {raw!r}", file=sys.stderr)
            return 2
        entries.append(tuple(parts))  # kind, ident, label, path

    findings = Findings()

    # Path rules: once per distinct path.
    seen_paths = {}
    for _kind, _ident, label, path in entries:
        if path not in seen_paths:
            seen_paths[path] = label
    for path, label in seen_paths.items():
        for detector, lines in check_path(path):
            findings.add(detector, path, label, lines)

    # Content rules: once per distinct source of bytes.  This dedup is why a
    # full-history scan costs about the same as a single-commit scan.
    seen_idents = {}
    for kind, ident, label, path in entries:
        if kind == "link":
            continue
        key = (kind, ident)
        if key not in seen_idents:
            seen_idents[key] = (label, path)

    blob_shas = [ident for (kind, ident) in seen_idents if kind == "blob"]
    blobs = read_blobs(blob_shas)

    metrics_by_ident = {}
    for (kind, ident), (label, path) in seen_idents.items():
        if kind == "blob":
            data = blobs.get(ident)
        else:
            try:
                with open(ident, "rb") as fh:
                    data = fh.read()
            except OSError:
                data = None
        if data is None:
            continue
        found, metrics = check_content(path, data)
        metrics_by_ident[(kind, ident)] = metrics
        for detector, lines in found:
            findings.add(detector, path, label, lines)

    # Aggregate budget, per scan unit.  Every rule above is per-file, so a leak
    # spread thinly across many files passes all of them.  Totals are summed
    # per label -- one worktree, one index, or one commit's tree -- and NOT
    # across the whole scan, so that scanning more history does not accumulate
    # a total that eventually fails on its own.
    totals = {}
    counted = {}
    for kind, ident, label, _path in entries:
        if kind == "link":
            continue
        metrics = metrics_by_ident.get((kind, ident))
        if metrics is None:
            continue
        # A blob appearing at two paths in one tree counts once.
        seen = counted.setdefault(label, set())
        if ident in seen:
            continue
        seen.add(ident)
        totals[label] = totals.get(label, 0) + metrics["bare_words"]

    for label, total in sorted(totals.items()):
        if total >= AGGREGATE_BARE_WORD_BUDGET:
            findings.add(
                "aggregate-word-budget",
                "(all files in this scan unit)",
                label,
                [
                    f"{total} bare machine words in total"
                    f" (budget: <{AGGREGATE_BARE_WORD_BUDGET})",
                    "no single file has to break a per-file rule for a tree to"
                    " carry a function's worth of ROM words between them",
                ],
            )

    return findings.report()


if __name__ == "__main__":
    sys.exit(main())
