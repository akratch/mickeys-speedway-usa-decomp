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

import base64
import json
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

# 4. Word tables -- the normalized-stream rule.
#
#    Rules 1-3 all key on a token *shape*, with a strict boundary class.  A
#    review demonstrated 28 fixtures carrying 400 real ROM words that scored
#    literally zero against them, and most needed no evasion intent at all: a
#    `u` suffix on a C literal, a sentence ending in a period, `xxd -p` output,
#    a hex range written with a dash.  Shape-matching loses that game by
#    construction -- every fix is one more regex, and the next encoding walks
#    straight past it.
#
#    So: normalize first, measure second.  normalize_words() turns a file into
#    a canonical stream of candidate 32-bit values from any plausible encoding,
#    and the metrics run on that stream.  One normalizer retires whole families
#    of evasion at once instead of one regex per fixture.
#
#    Two gates, because neither separates alone:
#      count  -- how many machine words the file yields
#      spread -- how many distinct high bytes those values take.  Addresses
#                cluster (this tree is 0x8xxxxxxx VRAM and 0x0xxxxxxx ROM
#                offsets); ROM instruction words do not.
#
#    Measured across all of history, excluding the whitelisted manifests, which
#    get a structural rule instead (see check_manifest_schema -- a hash-dense
#    file is not distinguishable from ROM data by content, and measuring harder
#    would only produce a number that looks like a guarantee):
#      symbol_addrs.us.txt          431 words, spread 14 -- 2.3x under spread
#      src/main/runlink.c            68 words, spread 26 -- 2.8x under count
#      tools/cleanroom_detectors.py 119 words, spread 17
#    Every non-manifest blob in history fails at least one gate.
#    The fixture families (400 real ROM words each) land at 392-5986 words and
#    spread 40-117.
WORD_TABLE_COUNT_LIMIT = 192
WORD_TABLE_SPREAD_LIMIT = 32

#    Digest-shaped strings are the awkward case: a SHA-256 normalizes to eight
#    uniformly-distributed words, so a file that legitimately records hashes
#    looks like a file carrying ROM.  This tree does record them (34 in the
#    busiest manifest).  They are exempt -- but only where they sit inside a
#    line beside other content, which is what a recorded hash looks like
#    ("sha256": "..."").  A digest-shaped token alone on its line, repeatedly,
#    is a dump and is not exempt.  The exemption is capped: past
#    DIGEST_EXEMPTION tokens in one file, they all count.  This is a
#    deliberate, bounded hole -- see the limits section in docs/CLEANROOM.md.
DIGEST_TOKEN = re.compile(
    r"(?<![0-9A-Za-z_.$-])"
    r"(?:[0-9a-fA-F]{32}|[0-9a-fA-F]{40}|[0-9a-fA-F]{64}|[0-9a-fA-F]{128})"
    r"(?![0-9A-Za-z_.$-])"
)
DIGEST_EXEMPTION = 64

#    Aggregate.  Per-file rules are per-file, so a leak spread across files
#    passes all of them: 7 files of 60 words is 400 ROM words with nothing
#    firing.  Summing raw counts does not work either -- this tree's own total
#    is 1191, mostly symbol_addrs.us.txt, so a budget above it is useless and a
#    budget below it fires on a clean checkout.
#
#    A file therefore contributes only if it is BOTH uniform-looking and
#    unusually word-dense for its size.  Real instruction words cluster by
#    opcode, so spread saturates near 20 for a small sample -- which is why the
#    spread floor here is much lower than the per-file rule's, and why density
#    has to carry the other half.
#    Measured: every blob in history contributes 0, the whole worktree
#    contributes 0, the 7-file spread attack 432, the 8-part split base64 blob
#    606.  The pass side is not close to the line; it is not on the board.
AGGREGATE_SPREAD_FLOOR = 16
AGGREGATE_RATE_FLOOR = 12.0  # words per KiB
AGGREGATE_WORD_BUDGET = 96
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

# 8. (The aggregate budget now lives with rule 4 -- it is measured on the
#    normalized stream, not on bare-hex tokens.)


# 9. Whitelisted workbench manifests are checked structurally rather than
#    statistically -- see check_manifest_schema().
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


# Encodings the normalizer decodes.  Work is bounded: at most DECODE_BUDGET
# bytes of decoded output per blob, so a pathological input cannot turn the
# gate into a denial of service.
HEX_RUN_ANY = re.compile(r"(?:0[xX])?[0-9a-fA-F][0-9a-fA-F_]*")
DEC_TOKEN = re.compile(r"(?<![0-9A-Za-z_.$-])([0-9]{8,10})(?![0-9A-Za-z_.$-])")
OCT_TOKEN = re.compile(r"(?<![0-9A-Za-z_.$-])0[oO]?([0-7]{9,12})(?![0-9A-Za-z_.$-])")
ESCAPED_BYTES = re.compile(r"(?:\\x[0-9a-fA-F]{2}){4,}")
ESCAPED_BYTE = re.compile(r"\\x([0-9a-fA-F]{2})")
DOTTED_QUAD = re.compile(
    r"(?<![0-9.])(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})(?![0-9.])"
)
PURE_HEX_LINE = re.compile(r"^[0-9a-fA-F]+$")
PURE_BASE_LINE = re.compile(r"^[!-u]+$")
BASE_RUN = re.compile(r"[A-Za-z0-9+/=_-]{40,}")
# ascii85 uses most of printable ASCII, so it needs its own class -- the base64
# one does not contain its punctuation and the encoding sailed straight past.
# The class is necessarily broad, so it is only tried on a run that is either
# <~ ~>-delimited or occupies a whole line: that is what an ascii85 payload
# looks like, and it stops the rule matching long regex literals in source.
# (It did, at first -- this file detected itself.)
A85_RUN = re.compile(r"(?:<~)?[!-uz]{64,}(?:~>)?")
DECODE_BUDGET = 4 << 20


def _words_from_bytes(raw, out):
    for i in range(0, len(raw) - 3, 4):
        out.append(int.from_bytes(raw[i : i + 4], "big"))


def _words_from_hex(run, out, halves):
    """Words from one hex run.

    Type suffixes, adjacent punctuation and underscore separators are all
    irrelevant here, because nothing about a boundary character is required --
    which is the whole point of normalizing instead of shape-matching.
    """
    run = run.replace("_", "")
    if run[:2] in ("0x", "0X"):
        run = run[2:]
    if not run or not any(c.isdigit() for c in run):
        # All-letter runs are English words ("decade", "faced"), not data.
        return
    if len(run) >= 8:
        for i in range(0, len(run) - 7, 8):
            out.append(int(run[i : i + 8], 16))
    elif len(run) == 4:
        halves.append(int(run, 16))


def _decode_base_n(run, out):
    if len(run) < 40:
        return
    body = run.rstrip("=")
    for decoder, pad in (
        (base64.b64decode, 4),
        (base64.urlsafe_b64decode, 4),
        (base64.b32decode, 8),
        (base64.a85decode, 0),
    ):
        try:
            raw = decoder(body + "=" * (-len(body) % pad) if pad else run)
        except Exception:
            continue
        if raw and len(raw) >= 16:
            _words_from_bytes(raw[:DECODE_BUDGET], out)
            return


def normalize_words(text):
    """Return candidate 32-bit machine words in any plausible encoding.

    Handles C literals with type suffixes, underscore separators, prose
    punctuation, hex ranges, 16-bit halves, whole-line and wrapped hex, escaped
    byte strings, octal, decimal, dotted quads, and base64 / base64url /
    base32 / ascii85 -- including blobs split across lines.
    """
    out = []
    halves = []

    digests = []
    for line in text.split("\n"):
        stripped = line.strip()
        for match in DIGEST_TOKEN.finditer(line):
            if match.group(0) != stripped:
                digests.append(match.group(0))
    if 0 < len(digests) <= DIGEST_EXEMPTION:
        for digest in set(digests):
            text = text.replace(digest, " " * len(digest))

    lines = text.split("\n")

    # Blocks of consecutive pure-hex or pure-base-N lines are one datum split
    # across lines; join before extracting, so wrapping is not a hiding place.
    index = 0
    while index < len(lines):
        stripped = lines[index].strip()
        if PURE_HEX_LINE.match(stripped) and len(stripped) >= 16:
            end = index
            block = []
            while end < len(lines):
                candidate = lines[end].strip()
                if not (PURE_HEX_LINE.match(candidate) and len(candidate) >= 16):
                    break
                block.append(candidate)
                end += 1
            _words_from_hex("".join(block), out, halves)
            index = end
            continue
        if PURE_BASE_LINE.match(stripped) and len(stripped) >= 32:
            end = index
            block = []
            while end < len(lines):
                candidate = lines[end].strip()
                if not (PURE_BASE_LINE.match(candidate) and len(candidate) >= 32):
                    break
                block.append(candidate)
                end += 1
            _decode_base_n("".join(block), out)
            index = end
            continue
        index += 1

    for match in HEX_RUN_ANY.finditer(text):
        _words_from_hex(match.group(0), out, halves)
    for i in range(0, len(halves) - 1, 2):
        out.append((halves[i] << 16) | halves[i + 1])

    for match in DEC_TOKEN.finditer(text):
        value = int(match.group(1))
        if (1 << 24) <= value < (1 << 32):
            out.append(value)
    for match in OCT_TOKEN.finditer(text):
        try:
            value = int(match.group(1), 8)
        except ValueError:
            continue
        if (1 << 24) <= value < (1 << 32):
            out.append(value)
    for match in DOTTED_QUAD.finditer(text):
        parts = [int(part) for part in match.groups()]
        if all(part < 256 for part in parts):
            out.append((parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3])
    for match in ESCAPED_BYTES.finditer(text):
        _words_from_bytes(
            bytes(int(h, 16) for h in ESCAPED_BYTE.findall(match.group(0))), out
        )
    for match in BASE_RUN.finditer(text):
        _decode_base_n(match.group(0), out)
    for line in text.split("\n"):
        stripped = line.strip()
        for match in A85_RUN.finditer(line):
            run = match.group(0)
            delimited = run.startswith("<~") and run.endswith("~>")
            if not delimited and run != stripped:
                continue
            try:
                raw = base64.a85decode(run.strip("<~>"))
            except Exception:
                continue
            if raw and len(raw) >= 16:
                _words_from_bytes(raw[:DECODE_BUDGET], out)

    return out


# The campaign manifest's schema, as the workbench actually writes it.  A
# default-deny key list: an unknown key fails rather than being sampled for
# suspicious content, because "unexpected content in a whitelisted file" is
# exactly the shape the original incident took.  If the workbench legitimately
# adds a field this is the one place to update, and failing closed until
# someone does is the correct behaviour.
MANIFEST_KEYS = frozenset(
    {
        "artifact_directory",
        "cache_directory",
        "cache_key",
        "compile",
        "compiler",
        "created_at_unix",
        "environment",
        "exact",
        "execution",
        "experiment",
        "identity",
        "identity_inputs",
        "interrupted",
        "jobs",
        "last_run",
        "ledger",
        "objdump",
        "path",
        "prepared",
        "region",
        "requested",
        "resolved",
        "results",
        "schema",
        "section",
        "sha256",
        "sources",
        "state_directory",
        "status",
        "stop_on_exact",
        "symbol",
        "target",
        "template",
        "timeout_seconds",
        "updated_at_unix",
        "working_directory",
    }
)
MANIFEST_DIGEST = re.compile(r"^[0-9a-fA-F]{32,128}$")
MANIFEST_MAX_STRING = 512


def check_manifest_schema(text):
    """Validate a whitelisted campaign manifest structurally.

    Content statistics cannot help here -- the real manifests normalize to 868
    words at spread 206, because they are full of hashes, which is exactly what
    ROM data looks like.  What is available instead is structure: the file is
    machine-generated with a small fixed schema, so every key can be required
    to be known and every value required to be a shape the schema actually
    uses.  Then there is nowhere in the file to put a ROM word.
    """
    problems = []
    try:
        document = json.loads(text)
    except ValueError as error:
        return [[f"not parseable as JSON ({error}); a manifest is machine-written"]]
    if not isinstance(document, dict):
        return [["top level is not a JSON object"]]

    def walk(node, where):
        if isinstance(node, dict):
            for key, value in node.items():
                if key not in MANIFEST_KEYS:
                    problems.append(
                        [
                            f"unknown key {where}.{key}",
                            "a manifest records paths, hashes and run state; an"
                            " unrecognised key is either a schema change (update"
                            " MANIFEST_KEYS) or content that does not belong",
                        ]
                    )
                    continue
                walk(value, f"{where}.{key}")
        elif isinstance(node, list):
            for i, value in enumerate(node):
                walk(value, f"{where}[{i}]")
        elif isinstance(node, str):
            if len(node) > MANIFEST_MAX_STRING:
                problems.append([f"{where}: string of {len(node)} characters"])
            elif not (MANIFEST_DIGEST.match(node) or "/" in node):
                leaked = normalize_words(node)
                if leaked:
                    problems.append(
                        [
                            f"{where}: string yields {len(leaked)} machine word(s)",
                            "manifest strings are identifiers, paths and hashes",
                        ]
                    )
        elif isinstance(node, bool) or node is None:
            return
        elif isinstance(node, int) and (1 << 24) <= node < (1 << 32):
            problems.append([f"{where}: integer {node} is a 32-bit word value"])

    walk(document, "")
    return problems


def check_content(path, data):
    """Content rules.

    Returns ``([(detector, [detail lines]), ...], metrics)``.  The metrics feed
    the aggregate budget in :func:`main`, which is the one rule that cannot be
    decided by looking at a single file.
    """
    out = []
    metrics = {"words": 0, "spread": 0, "scored": 0}
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
    values = normalize_words(text)
    spread = len({(value >> 24) & 0xFF for value in values})
    metrics["words"] = len(values)
    metrics["spread"] = spread
    # A whitelisted manifest is hash-dense by design: it normalizes to hundreds
    # of uniformly-distributed words and would trip every statistical rule
    # here, forever, on a clean checkout.  It is covered by the structural
    # check below instead, which is strictly stronger for a file whose schema
    # is known.  Exempting it from the statistics is what lets those
    # statistics keep a usable threshold for every other file.
    is_manifest = bool(WORKBENCH_ALLOWED.match(path))
    # A file feeds the aggregate only if its own values look uniform.  This
    # tree's word volume is real but clustered (addresses); a leak's is not.
    # Without the weighting the aggregate is useless -- the clean worktree's
    # raw total is 1191.
    word_rate = len(values) * 1024.0 / size
    metrics["scored"] = (
        len(values)
        if not is_manifest
        and spread >= AGGREGATE_SPREAD_FLOOR
        and word_rate >= AGGREGATE_RATE_FLOOR
        else 0
    )
    if (
        not is_manifest
        and len(values) >= WORD_TABLE_COUNT_LIMIT
        and spread >= WORD_TABLE_SPREAD_LIMIT
    ):
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

    # -- whitelisted workbench manifests: structure, not statistics ---------
    if WORKBENCH_ALLOWED.match(path):
        for problem in check_manifest_schema(text):
            out.append(("workbench-manifest-schema", problem))

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
        totals[label] = totals.get(label, 0) + metrics["scored"]

    for label, total in sorted(totals.items()):
        if total >= AGGREGATE_WORD_BUDGET:
            findings.add(
                "aggregate-word-budget",
                "(all files in this scan unit)",
                label,
                [
                    f"{total} machine words from files whose values look"
                    f" uniform and word-dense (budget: <{AGGREGATE_WORD_BUDGET};"
                    f" counted per file at spread >={AGGREGATE_SPREAD_FLOOR} and"
                    f" >={AGGREGATE_RATE_FLOOR:.0f} words/KiB)",
                    "no single file has to break a per-file rule for a tree to"
                    " carry a function's worth of ROM words between them",
                ],
            )

    return findings.report()


if __name__ == "__main__":
    sys.exit(main())
