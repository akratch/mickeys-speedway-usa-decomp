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
comments are the worst case over *all 224 blobs in this repository's
history* versus the two workbench ledgers that had to be purged from it --
the incident this whole file exists to prevent from recurring.  Those
ledgers are the calibration set for "must fail"; the history is the
calibration set for "must pass".

Re-measure when you change a threshold OR a decoder.  Two rounds of false
positives here came from decoders that turned ordinary text into "machine
words", not from thresholds being too tight, and a decoder change moves every
number in this file.
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
#    FALSE POSITIVES ARE THE LIVE RISK HERE, not evasion.  A gate that blocks
#    sanctioned work gets bypassed with --no-verify, and then none of this
#    exists.  docs/CLEANROOM.md positively encourages adapting function bodies
#    from published decomps, so source files are *expected* to carry hex
#    constants, struct offsets and small instruction citations.
#
#    A round of review found src/main/runlink.c at 162 words / spread 28
#    against limits of 192 / 32 -- 1.19x and 1.14x, about thirty words from
#    firing on the repository's own worked example of permitted work.  The
#    cause turned out not to be a threshold problem at all: 116 of those 162
#    words were base64 FALSE DECODES of `#pragma GLOBAL_ASM("asm/.../
#    func_80031A30.s")` paths, and being decoded garbage they were uniformly
#    distributed, so they inflated `spread` far faster than they inflated
#    `count`.  Fixing the decoder (see _decode_base_n) rather than raising the
#    limit is what bought the headroom back, and it is the honest fix: the
#    number was wrong, not the threshold.
#
#    A second false-decode source turned up in the same measurement and is
#    fixed in _words_from_hex: `_`-stripping treated `D_80081898` and
#    `func_10003920` -- symbol names, which decomp source and documentation
#    are made of -- as digit-grouped literals.
#
#    Measured after both fixes, over all 224 blobs in this repository's
#    history.  The protecting gate is named, because for every file in this
#    tree it is `spread` -- addresses cluster, and volume alone was never the
#    signal:
#      docs/modules.md       106 words, spread 13 -- 2.46x under spread
#      symbol_addrs.us.txt   448 words, spread 13 -- 2.46x under spread, and
#                            note it is already 2.3x OVER the count limit;
#                            431 of those words share the high byte 0x80,
#                            which is the whole reason the rule is a pair
#      src/main/runlink.c     47 words, spread  4 -- 8.0x under spread
#      the two manifests      17 and 12 words after their schema-validated
#                            digests are accounted for
#    Tightest margin anywhere in history: 2.46x.  Nothing in history fires.
#    The fixture families (400 real ROM words each) land at 343-1146 words and
#    spread 40-167.
#
#    Per-context thresholds were considered and NOT adopted.  They would have
#    to be loosest for `src/**.c` and `include/**.h` -- which is exactly where
#    an accidental paste of a disassembly listing would land, since that is
#    where someone works with reference material open.  Loosening the rule on
#    the files most likely to carry a real accident, to fix a number that was
#    wrong for an unrelated reason, is the wrong trade.  The release valve is
#    CONTENT_EXEMPTIONS below instead: narrower than --no-verify, per file and
#    per detector, and it leaves a reason in the diff.
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
#    (Whitelisted manifests do not rely on this: their digests are identified
#    by the schema check, which knows which leaves are supposed to be hashes,
#    and blanked before these statistics run.  See check_content.)
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
#    Measured: every one of the 224 blobs in history contributes 0, and the
#    whole worktree contributes 0.  The pass side is not close to the line; it
#    is not on the board.
#
#    Known and NOT fixed: the rate floor is a per-file rate, so padding a leak
#    with prose drops it under 12 words/KiB and it stops contributing.  A
#    fixture of 400 ROM words diluted into a sentence apiece contributes 0 to
#    this budget (it is still caught per-file, by machine-word-dump and
#    word-table, but the aggregate is not what catches it).  Lowering the floor
#    is not available: docs/modules.md already runs at 3.3 words/KiB, so a floor
#    low enough to catch padded text is a floor that counts documentation.
#    Stated in docs/CLEANROOM.md rather than papered over.
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

# Per-file, per-detector exemptions.  THE RELEASE VALVE, and the reason it
# exists: the failure mode that ends this whole system is a gate firing on
# legitimate, policy-sanctioned work, because the next thing that happens is
# `git commit --no-verify` and after that nothing is checked at all.  This is
# the narrower escape hatch -- one file, one detector, a reason recorded in the
# source, and a diff someone has to review -- so the answer to a false positive
# is never "turn the hooks off".
#
# Rules for using it: name the exact path and the exact detector, never a
# pattern; write the reason as the value; and prefer fixing the detector, since
# every entry here is a permanent hole.  Empty today, and that is the goal --
# the tightest margin in this repository's whole history is 2.46x, so nothing
# needs one.  If this set is not empty, the count belongs in the report.
CONTENT_EXEMPTIONS: "dict[tuple[str, str], str]" = {}

# 8. (The aggregate budget now lives with rule 4 -- it is measured on the
#    normalized stream, not on bare-hex tokens.)


# 9. Whitelisted workbench manifests get a structural check IN ADDITION to
#    every rule above -- see check_manifest_schema() and its block comment.
#    (MANIFEST_MAX_WORD_VALUES / MANIFEST_MAX_MNEMONICS lived here and were
#    dead: nothing read them once the schema check replaced sampling.)

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
# Two whole-line classes, because they are two different alphabets and using
# one for both was a bug.  `[!-u]` is ascii85's range; base64's alphabet runs
# to `z`, so ANY wrapped base64 line containing v, w, x, y or z failed to match
# and the block join -- the entire mechanism for "wrapped base64 is still
# caught" -- silently did nothing on it.  What actually caught wrapped blobs
# was BASE_RUN, whose 40-character floor is wider than a 31-column wrap, which
# is why a 31-column wrap passed everything while 64- and 76-column wraps were
# caught.  Separate classes, and a per-line floor low enough that a narrow wrap
# has nowhere to sit.
PURE_B64_LINE = re.compile(r"^[A-Za-z0-9+/=_-]+$")
PURE_B64_LINE_MIN = 16
PURE_BASE_LINE = re.compile(r"^[!-u]+$")
PURE_BASE_LINE_MIN = 32
BASE_RUN = re.compile(r"[A-Za-z0-9+/=_-]{40,}")
# ascii85 uses most of printable ASCII, so it needs its own class -- the base64
# one does not contain its punctuation and the encoding sailed straight past.
# The class is necessarily broad, so it is only tried on a run that is either
# <~ ~>-delimited or occupies a whole line: that is what an ascii85 payload
# looks like, and it stops the rule matching long regex literals in source.
# (It did, at first -- this file detected itself.)
A85_RUN = re.compile(r"(?:<~)?[!-uz]{64,}(?:~>)?")
DECODE_BUDGET = 4 << 20

# Every base-N decoder here will happily decode text that was never encoded.
# `asm/nonmatchings/main/runlink/func_80031A30` is 43 characters of the base64
# alphabet, so b64decode returns 32 bytes of uniformly-distributed garbage --
# eight "machine words" whose high bytes are, by construction, spread across
# the whole space.  That is a false decode, and it was the dominant term in
# this tree's own numbers: it alone put src/main/runlink.c at 162 words /
# spread 28 against limits of 192 / 32, and both whitelisted manifests over the
# spread limit.  Every #pragma GLOBAL_ASM path, every long cache key, every
# hyphenated slug in a document was being counted as ROM.
#
# The discriminator is on the INPUT side, because decoding garbage produces
# garbage either way: an encoder fed high-entropy bytes emits its alphabet
# near-uniformly, and ordinary text does not.  Over random input base64 is
# 26/64 uppercase, 26/64 lowercase, 10/64 digits; base32 is 26/32 uppercase and
# 6/32 digits; ascii85 spreads over 85 symbols.  The floors below sit far under
# those expectations -- for the shortest run we accept (40 characters) an
# uppercase count of 6 against an expected 16 is more than three standard
# deviations low, so a real payload essentially never fails, and a 40-character
# run decodes to 7 words in any case.  Text fails by an order of magnitude:
# that path is 1/43 = 2% uppercase.
#
# This is not an anti-evasion rule and it does not need to be robust to one.
# It removes decodes that were never data.  Attacker-supplied base64 of ROM
# bytes passes these floors by construction -- see the fixture table in the
# report -- because the attacker does not control the encoder's alphabet
# statistics without changing encoding, and each encoding has its own floors.
B64_MIN_UPPER = 0.15
B64_MIN_LOWER = 0.15
B64_MIN_DIGIT = 0.04
B32_MIN_UPPER = 0.60
B32_MIN_DIGIT = 0.04
A85_MIN_UPPER = 0.10
A85_MIN_LOWER = 0.10
A85_MIN_DIGIT = 0.03


def _shares(run):
    """(uppercase, lowercase, digit) share of one run."""
    size = len(run) or 1
    upper = lower = digit = 0
    for char in run:
        if char.isupper():
            upper += 1
        elif char.islower():
            lower += 1
        elif char.isdigit():
            digit += 1
    return upper / size, lower / size, digit / size


def _words_from_bytes(raw, out):
    for i in range(0, len(raw) - 3, 4):
        out.append(int.from_bytes(raw[i : i + 4], "big"))


def _words_from_hex(run, out, halves):
    """Words from one hex run.

    Type suffixes, adjacent punctuation and underscore separators are all
    irrelevant here, because nothing about a boundary character is required --
    which is the whole point of normalizing instead of shape-matching.

    Alignment is the subtle part.  A run whose length is a multiple of 8 has
    exactly one reading.  A run that is not -- `080031a30`, a zero-padded word;
    `c_80031a30`, the tail of a symbol name -- has two plausible ones, and
    reading only from the left is what let a 9-digit zero-padded dump through:
    every value came out shifted by a nibble, so the high bytes all collapsed
    onto one another and `spread` fell from 41 to 8.  Both alignments are
    emitted when the length is ragged.  It costs a duplicate word per ragged
    run on the pass side (measured: +9 words across the whole worktree) and it
    closes an alignment hole that a one-character change could open.
    """
    prefixed = run[:2] in ("0x", "0X")
    if prefixed:
        run = run[2:]
    if "_" in run:
        # Underscores mean two different things and only one of them is a
        # numeric separator.  `0x8008_1898` is digit grouping and must be
        # joined; `D_80081898` and `func_10003920` are SYMBOL NAMES, and
        # joining them fabricates a word from an identifier prefix plus an
        # address -- `D80081898`, whose left-aligned reading is `0xd8008189`,
        # a value that appears nowhere and lands on a high byte nothing else
        # in the file uses.  Decomp source and documentation are made of such
        # names, so this was a steady drip of uniformly-distributed noise into
        # `spread`, which is the metric protecting every file in this tree.
        #
        # Digit grouping is written in even, regular chunks.  Accept the join
        # only when every group is 2, 4 or 8 digits; otherwise take each group
        # on its own merits, which still yields the real address.
        groups = run.split("_")
        if prefixed or all(len(g) in (2, 4, 8) for g in groups if g):
            run = "".join(groups)
        else:
            for group in groups:
                if group:
                    _words_from_hex(group, out, halves)
            return
    if not run or not any(c.isdigit() for c in run):
        # All-letter runs are English words ("decade", "faced"), not data.
        return
    if len(run) >= 8:
        for i in range(0, len(run) - 7, 8):
            out.append(int(run[i : i + 8], 16))
        if len(run) % 8:
            # Right-aligned reading of the same run.
            tail = run[len(run) % 8 :]
            for i in range(0, len(tail) - 7, 8):
                out.append(int(tail[i : i + 8], 16))
    elif len(run) == 4:
        halves.append(int(run, 16))


def _decode_base_n(run, out):
    if len(run) < 40:
        return
    upper, lower, digit = _shares(run)
    body = run.rstrip("=")
    for decoder, pad, ok in (
        (base64.b64decode, 4, upper >= B64_MIN_UPPER and lower >= B64_MIN_LOWER and digit >= B64_MIN_DIGIT),
        (base64.urlsafe_b64decode, 4, upper >= B64_MIN_UPPER and lower >= B64_MIN_LOWER and digit >= B64_MIN_DIGIT),
        (base64.b32decode, 8, upper >= B32_MIN_UPPER and digit >= B32_MIN_DIGIT),
        (base64.a85decode, 0, upper >= A85_MIN_UPPER and lower >= A85_MIN_LOWER and digit >= A85_MIN_DIGIT),
    ):
        if not ok:
            continue
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
        matched = False
        for pattern, floor in (
            (PURE_B64_LINE, PURE_B64_LINE_MIN),
            (PURE_BASE_LINE, PURE_BASE_LINE_MIN),
        ):
            if not (pattern.match(stripped) and len(stripped) >= floor):
                continue
            end = index
            block = []
            while end < len(lines):
                candidate = lines[end].strip()
                if not (pattern.match(candidate) and len(candidate) >= floor):
                    break
                block.append(candidate)
                end += 1
            if len(block) > 1:
                _decode_base_n("".join(block), out)
            index = end
            matched = True
            break
        if not matched:
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
            upper, lower, digit = _shares(run)
            if not (
                upper >= A85_MIN_UPPER
                and lower >= A85_MIN_LOWER
                and digit >= A85_MIN_DIGIT
            ):
                continue
            try:
                raw = base64.a85decode(run.strip("<~>"))
            except Exception:
                continue
            if raw and len(raw) >= 16:
                _words_from_bytes(raw[:DECODE_BUDGET], out)

    return out


# --------------------------------------------------------------------------
# The campaign manifest schema.
#
# `.decomp-workbench/campaigns/*/manifest.json` is the one file allowed under a
# directory that is otherwise ROM-derived by construction, so it gets checked
# by structure rather than by statistics.  The previous version of this check
# was a flat set of permitted key *names* with a loose value rule, and a review
# walked around it five ways: the real schema's own `sources: [{cache_key,
# path}]` shape, an uncapped 32-128-hex-digit digest exemption, `"/" in value`
# skipping validation entirely, integers below 2^24, and -- a plain bug -- a
# `walk` with an `int` branch and no `float` branch, so `16777217.0` was never
# looked at while `16777217` was.
#
# What replaces it is a **path-typed** allow-list.  Every leaf the workbench
# writes is enumerated below by its canonical path, with the JSON types it may
# take and a checker for its value's shape.  A path that is not listed is a
# finding; a listed path with the wrong type is a finding; a value that fails
# its checker is a finding.  There is no fall-through branch: `_check_leaf`
# ends in an unconditional rejection, which is what the float bug was.
#
# The schema is `decomp_workbench.campaign_state.write_manifest` /
# `finish_manifest` as of workbench 0.4.0.  A legitimate schema change fails
# this check until someone updates the table, and that is the intended cost:
# the file is a hole in a whitelist, and widening a hole should be a reviewed
# diff, not a silent pass.  The error message says so.
#
# HONEST BOUND, because the doc used to claim more than this delivers: a
# sha256 is 32 arbitrary bytes, and a manifest legitimately carries several.
# Typing cannot make a hash field carry less than a hash.  What the table does
# is *count and cap* them -- at most MANIFEST_MAX_SOURCES source records, so at
# most 4 + 2*MANIFEST_MAX_SOURCES digest-typed leaves -- and force every other
# leaf into a shape with no room.  The residual capacity is therefore bounded
# and stated in docs/CLEANROOM.md rather than claimed away.
# --------------------------------------------------------------------------

MANIFEST_SCHEMA_ID = "decomp-workbench-campaign-manifest-v1"
MANIFEST_STATUSES = frozenset({"running", "complete", "exact", "interrupted"})
MANIFEST_MAX_SOURCES = 64  # real manifests carry 6-15
MANIFEST_MAX_ENVIRONMENT = 32  # real manifests carry 0
MANIFEST_MAX_PATH = 256  # real maximum is 113 characters
MANIFEST_MAX_NAME = 64
# A path is the only leaf with meaningful free-form room, so it gets an
# explicit word budget of its own rather than being waved through on "/".
# Real manifests: 2 words per path (a campaign directory carries a 12-hex
# identity prefix).
MANIFEST_PATH_WORD_BUDGET = 4

MANIFEST_SHA256 = re.compile(r"^[0-9a-f]{64}$")
MANIFEST_PATH_CHARS = re.compile(r"^[A-Za-z0-9 ._/@+~-]+$")
MANIFEST_TEMPLATE_CHARS = re.compile(r"^[A-Za-z0-9 ._/@+~{}<>=:-]+$")
MANIFEST_NAME = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
MANIFEST_SECTION = re.compile(r"^\.[A-Za-z0-9_.]{1,31}$")
MANIFEST_EXPERIMENT = re.compile(r"^[A-Za-z0-9][A-Za-z0-9 ._-]*$")
# Unix seconds.  2001-09-09 .. 2096-10-02: wide enough never to argue about,
# narrow enough that a 32-bit ROM word cannot be parked here -- the range holds
# only 3e9 of the 4.3e9 possible word values and none below 1e9.
MANIFEST_TIME_MIN = 1_000_000_000
MANIFEST_TIME_MAX = 4_000_000_000


def _m_sha256(value, ctx):
    return None if MANIFEST_SHA256.match(value) else "not a lowercase sha256 hex digest"


def _hex_runs_are_accounted_for(value, ctx):
    """Reject hex runs in a free-form string that the schema cannot explain.

    A path is the one place in this schema with room to write, and a word
    budget alone is too blunt: `/a/27bdffc0/afbf0014/8c820010/10400005` yields
    exactly four words, which any budget generous enough for the real files
    also admits.

    So do not budget -- *account*.  The only long hex a real manifest path
    contains is the campaign directory, which the workbench names
    `{symbol}-{identity[:12]}`, plus the digests validated elsewhere.  Any hex
    run of eight or more digits that is not a prefix of this document's own
    `identity` is unexplained, and unexplained is a finding.
    """
    identity = ctx.get("identity") or ""
    for match in re.finditer(r"(?<![0-9A-Za-z])[0-9a-fA-F]{8,}(?![0-9A-Za-z])", value):
        run = match.group(0).lower()
        if not (identity and identity.startswith(run)):
            return f"unexplained {len(run)}-digit hex run {run!r}"
    words = normalize_words(value)
    if len(words) > MANIFEST_PATH_WORD_BUDGET:
        return f"yields {len(words)} machine words (budget {MANIFEST_PATH_WORD_BUDGET})"
    return None


def _m_path(value, ctx):
    if len(value) > MANIFEST_MAX_PATH:
        return f"path of {len(value)} characters (limit {MANIFEST_MAX_PATH})"
    if not MANIFEST_PATH_CHARS.match(value):
        return "characters outside the permitted path set"
    if "/" not in value:
        return "not a path"
    detail = _hex_runs_are_accounted_for(value, ctx)
    return None if detail is None else f"path {detail}"


def _m_template(value, ctx):
    if len(value) > MANIFEST_MAX_PATH:
        return f"template of {len(value)} characters"
    if not MANIFEST_TEMPLATE_CHARS.match(value):
        return "characters outside the permitted template set"
    detail = _hex_runs_are_accounted_for(value, ctx)
    return None if detail is None else f"template {detail}"


def _m_name(value, ctx):
    if len(value) > MANIFEST_MAX_NAME or not MANIFEST_NAME.match(value):
        return "not a C identifier of 64 characters or fewer"
    return None


def _m_section(value, ctx):
    return None if MANIFEST_SECTION.match(value) else "not an ELF section name"


def _m_status(value, ctx):
    return (
        None
        if value in MANIFEST_STATUSES
        else f"not one of {sorted(MANIFEST_STATUSES)}"
    )


def _m_schema(value, ctx):
    return None if value == MANIFEST_SCHEMA_ID else f"not {MANIFEST_SCHEMA_ID!r}"


def _m_experiment(value, ctx):
    if len(value) > MANIFEST_MAX_NAME or not MANIFEST_EXPERIMENT.match(value):
        return "not a short experiment label"
    if normalize_words(value):
        return "experiment label yields machine words"
    return None


def _m_time(value, ctx):
    if not (MANIFEST_TIME_MIN <= value <= MANIFEST_TIME_MAX):
        return f"{value} is not a plausible unix timestamp"
    return None


def _m_seconds(value, ctx):
    return None if 0 < value <= 86400 else f"{value} is not a timeout in seconds"


def _m_count(value, ctx):
    return None if 0 <= value <= 65536 else f"{value} is not a small count"


#: path -> (permitted JSON types, value checker or None).  `str` here never
#: includes `bool`; `int` never includes `bool` either (see _json_type).
MANIFEST_SCHEMA = {
    ".schema": (("str",), _m_schema),
    ".identity": (("str",), _m_sha256),
    ".created_at_unix": (("number",), _m_time),
    ".updated_at_unix": (("number",), _m_time),
    ".status": (("str",), _m_status),
    ".state_directory": (("str",), _m_path),
    ".ledger": (("str",), _m_path),
    ".cache_directory": (("str",), _m_path),
    ".artifact_directory": (("str", "null"), _m_path),
    ".experiment": (("str", "null"), _m_experiment),
    ".execution.jobs": (("int",), _m_count),
    ".execution.timeout_seconds": (("number",), _m_seconds),
    ".execution.stop_on_exact": (("bool",), None),
    ".identity_inputs.symbol": (("str",), _m_name),
    ".identity_inputs.section": (("str",), _m_section),
    ".identity_inputs.target.path": (("str",), _m_path),
    ".identity_inputs.target.sha256": (("str",), _m_sha256),
    ".identity_inputs.objdump.requested": (("str",), _m_path),
    ".identity_inputs.objdump.resolved": (("str",), _m_path),
    ".identity_inputs.objdump.sha256": (("str",), _m_sha256),
    ".identity_inputs.compile.template": (("str",), _m_template),
    ".identity_inputs.compile.working_directory": (("str",), _m_path),
    ".identity_inputs.compile.compiler.requested": (("str",), _m_path),
    ".identity_inputs.compile.compiler.resolved": (("str",), _m_path),
    ".identity_inputs.compile.compiler.sha256": (("str",), _m_sha256),
    ".sources[].path": (("str",), _m_path),
    ".sources[].sha256": (("str",), _m_sha256),
    ".sources[].cache_key": (("str",), _m_sha256),
    ".last_run.results": (("int",), _m_count),
    ".last_run.prepared": (("int",), _m_count),
    ".last_run.exact": (("bool",), None),
    ".last_run.interrupted": (("bool",), None),
}
#: Paths whose value is a JSON object with further declared children.
MANIFEST_OBJECTS = frozenset(
    {
        "",
        ".execution",
        ".identity_inputs",
        ".identity_inputs.target",
        ".identity_inputs.objdump",
        ".identity_inputs.compile",
        ".identity_inputs.compile.compiler",
        ".sources[]",
        ".last_run",
    }
)
#: Paths whose value is a JSON array, and the cap on its length.
MANIFEST_ARRAYS = {".sources": MANIFEST_MAX_SOURCES}
#: Paths whose value is an open-ended string map, and the cap on its size.
#: `compile.environment` is `Mapping[str, str]` in the workbench, so its keys
#: cannot be enumerated -- it is the one genuinely free-form region of the
#: schema.  Keys are required to be environment-variable names and values to
#: pass the same shape and word-budget checks a path does, and the whole map is
#: capped.  Both real manifests carry `{}`.
MANIFEST_MAPS = {".identity_inputs.compile.environment": MANIFEST_MAX_ENVIRONMENT}
#: A manifest is nine objects deep at most; anything deeper is not this schema.
MANIFEST_MAX_DEPTH = 12


def _json_type(node):
    """JSON type name.  `bool` is checked before `int` -- in Python it is one."""
    if node is None:
        return "null"
    if isinstance(node, bool):
        return "bool"
    if isinstance(node, int):
        return "int"
    if isinstance(node, float):
        return "float"
    if isinstance(node, str):
        return "str"
    if isinstance(node, list):
        return "list"
    if isinstance(node, dict):
        return "dict"
    return type(node).__name__


def check_manifest_schema(text, digests=None):
    """Validate a whitelisted campaign manifest against its declared schema.

    Returns a list of problems, each a list of message lines; empty means the
    document is exactly the shape ``decomp_workbench.campaign_state`` writes.

    Strict in every direction: unknown paths, unexpected types (including
    floats, which the previous version never examined), out-of-range numbers,
    over-long or wrongly-shaped strings, over-long arrays and unexpected
    nesting are all rejected.  See the block comment above for the bound this
    does and does not deliver.

    If ``digests`` is a set it collects every value accepted as a sha256, so
    the caller can blank exactly those and run the ordinary content statistics
    over what is left.  That is what lets a manifest be covered by BOTH layers
    without the hashes it exists to record tripping the statistical one.
    """
    problems = []
    try:
        document = json.loads(text)
    except ValueError as error:
        return [[f"not parseable as JSON ({error}); a manifest is machine-written"]]

    # `.identity` is read before the walk because path values are checked
    # against it -- the campaign directory embeds its first 12 digits.  It is
    # still validated as a sha256 in its own right by the walk below, so a
    # forged identity cannot launder a path: whatever is put here has to be 64
    # lowercase hex digits, which is the same bounded digest capacity already
    # accounted for, not a free-form escape.
    identity = document.get("identity") if isinstance(document, dict) else None
    ctx = {
        "identity": identity.lower() if isinstance(identity, str) else "",
    }

    def reject(where, message, hint=None):
        lines = [f"{where or '<root>'}: {message}"]
        lines.append(
            hint
            or "a manifest is machine-written to a fixed schema; update"
            " MANIFEST_SCHEMA in tools/cleanroom_detectors.py if the workbench"
            " legitimately changed, and expect to justify it at review"
        )
        problems.append(lines)

    def walk(node, where, depth):
        if depth > MANIFEST_MAX_DEPTH:
            reject(where, f"nested deeper than {MANIFEST_MAX_DEPTH}")
            return
        kind = _json_type(node)

        if where in MANIFEST_ARRAYS:
            if kind != "list":
                reject(where, f"expected an array, found {kind}")
                return
            limit = MANIFEST_ARRAYS[where]
            if len(node) > limit:
                reject(where, f"{len(node)} entries exceeds the cap of {limit}")
                return
            for item in node:
                walk(item, where + "[]", depth + 1)
            return

        if where in MANIFEST_MAPS:
            if kind != "dict":
                reject(where, f"expected an object, found {kind}")
                return
            limit = MANIFEST_MAPS[where]
            if len(node) > limit:
                reject(where, f"{len(node)} entries exceeds the cap of {limit}")
                return
            for key, value in node.items():
                if not isinstance(key, str) or not MANIFEST_NAME.match(key):
                    reject(where, f"key {key!r} is not an environment name")
                    continue
                if _json_type(value) != "str":
                    reject(f"{where}.{key}", f"expected str, found {_json_type(value)}")
                    continue
                detail = _m_template(value, ctx)
                if detail is not None:
                    reject(f"{where}.{key}", detail)
            return

        if where in MANIFEST_OBJECTS:
            if kind != "dict":
                reject(where, f"expected an object, found {kind}")
                return
            for key, value in node.items():
                if not isinstance(key, str) or not MANIFEST_NAME.match(key):
                    reject(where, f"key {key!r} is not a schema key name")
                    continue
                child = f"{where}.{key}"
                if (
                    child not in MANIFEST_SCHEMA
                    and child not in MANIFEST_OBJECTS
                    and child not in MANIFEST_ARRAYS
                    and child not in MANIFEST_MAPS
                ):
                    reject(child, "not a key this schema declares")
                    continue
                walk(value, child, depth + 1)
            return

        spec = MANIFEST_SCHEMA.get(where)
        if spec is None:
            reject(where, "not a leaf this schema declares")
            return
        types, checker = spec
        if kind == "float" and "number" in types:
            pass
        elif kind == "int" and ("int" in types or "number" in types):
            pass
        elif kind in types:
            pass
        else:
            reject(where, f"expected {'/'.join(types)}, found {kind}")
            return
        if node is None or checker is None:
            return
        # No fall-through: every remaining kind is handed to its checker, and a
        # kind with no checker cannot reach here because it was not permitted.
        detail = checker(node, ctx)
        if detail is not None:
            reject(where, detail)
        elif checker is _m_sha256 and digests is not None:
            digests.add(node)

    walk(document, "", 0)
    return problems


def check_content(path, data):
    """Content rules.

    Returns ``([(detector, [detail lines]), ...], metrics)``.  The metrics feed
    the aggregate budget in :func:`main`, which is the one rule that cannot be
    decided by looking at a single file.

    Findings named in :data:`CONTENT_EXEMPTIONS` for this path are dropped at
    the end, with the recorded reason reported so an exemption is visible in
    every run rather than silently absorbed.
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

    # -- whitelisted workbench manifests: structure first --------------------
    # A manifest is the one file allowed under a directory that is otherwise
    # ROM-derived by construction, so it is validated against its schema.  The
    # digest values the schema accepts are then blanked before the statistics
    # run, which is what lets the SAME file be covered by both layers: a
    # sha256 is indistinguishable from ROM data by any content metric, but a
    # sha256 that the schema has already placed at a declared digest leaf is
    # accounted for, and everything else in the file still has to answer to
    # the ordinary rules.  Previously the manifest was exempted from the
    # statistics wholesale, which made the schema check the only thing
    # standing between `.decomp-workbench/` and a tracked file.
    is_manifest = bool(WORKBENCH_ALLOWED.match(path))
    measured = text
    if is_manifest:
        manifest_digests = set()
        for problem in check_manifest_schema(text, manifest_digests):
            out.append(("workbench-manifest-schema", problem))
        for digest in manifest_digests:
            measured = measured.replace(digest, " " * len(digest))

    # -- word tables (value distribution) -----------------------------------
    values = normalize_words(measured)
    spread = len({(value >> 24) & 0xFF for value in values})
    metrics["words"] = len(values)
    metrics["spread"] = spread
    # A file feeds the aggregate only if its own values look uniform.  This
    # tree's word volume is real but clustered (addresses); a leak's is not.
    # Without the weighting the aggregate is useless -- the clean worktree's
    # raw total is 1191.
    word_rate = len(values) * 1024.0 / size
    metrics["scored"] = (
        len(values)
        if spread >= AGGREGATE_SPREAD_FLOOR and word_rate >= AGGREGATE_RATE_FLOOR
        else 0
    )
    if (
        len(values) >= WORD_TABLE_COUNT_LIMIT
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

    if CONTENT_EXEMPTIONS:
        kept = []
        for detector, lines in out:
            reason = CONTENT_EXEMPTIONS.get((path, detector))
            if reason is None:
                kept.append((detector, lines))
            else:
                print(
                    f"cleanroom: exempt [{detector}] {path} -- {reason}",
                    file=sys.stderr,
                )
        out = kept

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
