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
#    history max: 16 tokens (include/game/runlink.h) -- 2.5x under the limit.
#    purged ledgers: 88 tokens @ 1.298/KiB and 4407 @ 4.558/KiB -- both over.
MNEMONICS = re.compile(
    r"^(?:addiu|lw|sw|jal|beq|lui|sltu)$"
)
MNEMONIC_COUNT_LIMIT = 40
MNEMONIC_RATE_LIMIT = 1.000  # tokens per KiB

# 2. Machine words.  Bare hex runs of 4/8/16 digits that are not 0x-prefixed
#    and not part of an identifier, and that mix digits with a-f.  Source and
#    docs write addresses as `0x80001234`; a dump of the ROM's words writes
#    them bare.  That distinction is what makes this detector sharp.
#    history max: 3 tokens in any single blob -- 10x under the limit.
#    purged ledgers: 126 @ 1.86/KiB and 2973 @ 3.07/KiB -- both well over.
BARE_HEX_WORD = re.compile(
    r"(?<![0-9A-Za-z_.$-])(?:[0-9a-fA-F]{4}|[0-9a-fA-F]{8}|[0-9a-fA-F]{16})(?![0-9A-Za-z_.$-])"
)
HEXWORD_COUNT_LIMIT = 32
HEXWORD_RATE_LIMIT = 0.500  # tokens per KiB

# 3. Hexdump layout.  `xxd`/`hexdump -C`/`od` output, and C byte arrays of ROM
#    bytes, all share a shape: most of the tokens on the line are short hex.
#    Detected per line, then required to be a real share of the file.
#    history max: 0 such lines in any blob, ever.  Any threshold passes the
#    tree; these are set low enough to catch a dump of a single function.
HEX_TOKEN = re.compile(
    r"^(?:0[xX][0-9a-fA-F]{2}|0[xX][0-9a-fA-F]{4}"
    r"|[0-9a-fA-F]{2}|[0-9a-fA-F]{4}|[0-9a-fA-F]{8}|[0-9a-fA-F]{16})$"
)
HEXDUMP_TOKENS_PER_LINE = 6  # hex tokens needed before a line counts
HEXDUMP_LINE_LIMIT = 8  # such lines needed before the file counts
HEXDUMP_LINE_SHARE = 0.10  # ...and they must be this share of nonblank lines

# 4. Base64 blobs.  A long unbroken alnum+/ run that mixes case and digits is
#    not prose, not code, and not a hash used in this tree.
#    history max: 58 characters (a cache key in a workbench manifest).
#    2.2x under the limit.
BASE64_RUN = re.compile(r"[A-Za-z0-9+/]{64,}")
BASE64_RUN_LIMIT = 128

# 5. Size.  Nothing legitimate in a decomp source tree is this big; extracted
#    data is.  history max: 44,779 B (symbol_addrs.us.txt) -- 5.9x under.
SIZE_LIMIT = 256 * 1024
# Paths permitted to exceed SIZE_LIMIT or to be non-text.  Empty by design:
# add an entry only with a written reason, and expect to justify it at review.
SIZE_ALLOWLIST: "set[str]" = set()
BINARY_ALLOWLIST: "set[str]" = set()

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


def check_content(path, data):
    """Content rules.  Returns [(detector, [detail lines]), ...]."""
    out = []
    size = len(data)
    if size == 0:
        return out

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
            out.append(("binary-blob", ["not valid UTF-8 text; every tracked file must be text"]))
        return out

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
    for lineno, line in enumerate(text.split("\n"), 1):
        for m in BASE64_RUN.finditer(line):
            run = m.group(0)
            if (
                len(run) > worst
                and any(c.islower() for c in run)
                and any(c.isupper() for c in run)
                and any(c.isdigit() for c in run)
            ):
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

    return out


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
        for detector, lines in check_content(path, data):
            findings.add(detector, path, label, lines)

    return findings.report()


if __name__ == "__main__":
    sys.exit(main())
