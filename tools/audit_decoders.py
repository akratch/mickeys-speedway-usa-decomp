#!/usr/bin/env python3
"""Assert that no clean-room decoder is inventing data.

    gmake audit-decoders            # tracked files (fast, the default)
    gmake audit-decoders AUDIT_ARGS=--all
    python3 tools/audit_decoders.py [--all] [--verbose] [--by-file STAGE]

WHY THIS EXISTS
---------------
`tools/cleanroom_detectors.py` turns a file into a stream of candidate 32-bit
machine words and then measures that stream.  Five separate times, the thing it
measured was not in the file:

  * `#pragma GLOBAL_ASM("asm/.../func_80031A30.s")` -- a PATH, base64-decoded
    into 8 uniformly-distributed "words";
  * `D_80081898` and `func_10003920` -- SYMBOL NAMES, `_`-joined into words;
  * `0x1B74, 0x27A0` in a comment and `| 1232 | 1105 |` in a markdown table --
    unrelated numbers fused as 16-bit halves;
  * `OBJDUMP=tools/binutils/mips64-elf-objdump` -- a shell assignment out of
    this repository's own build scripts, decoded first as base64 and then,
    after that was blocked, as ascii85;
  * ragged hex runs read from one end only, so a zero-padded dump decoded to
    garbage.

Every one of those inflated `spread` -- the metric that decides whether a file
looks like ROM -- because decoded garbage is uniformly distributed by
construction.  Two of them took the repository's own files to within 1.2x of
failing the gate on legitimate, policy-sanctioned work, which is the failure
that ends this system: a gate that blocks real work gets bypassed with
`--no-verify`, and then nothing is checked at all.

Each was found by asking one question of one file -- *where did these words
actually come from?* -- which is sampling.  This tool asks it of every file,
every time.

THE RULE THIS ENFORCES, AND THE ONE IT ASKS YOU TO FOLLOW
---------------------------------------------------------
**A decoder change must be re-audited, never argued.**  Twice now a fix for one
false decode simply re-routed it: blocking base64 on that shell assignment
handed it to the ascii85 branch, and requiring halves to be adjacent still left
comment lists pairing.  Both were caught by re-running this audit and neither
would have been caught by reasoning about the diff.  Run `gmake audit-decoders`
after touching anything in `normalize_words_by_stage` or any decoder it calls.

WHAT THIS DOES **NOT** COVER
----------------------------
This audit only looks in the false-POSITIVE direction: it catches a decoder
that starts producing words out of text that was never encoded.  It says
nothing about the opposite failure -- a decoder that *stops* producing words it
should.  Both defects found in one review round were of that second kind: a
base64 run whose length was 1 mod 4 raised inside the decoder and was silently
swallowed, and a per-line floor of 32 meant narrow-wrapped ascii85 was never
joined at all.  Neither would move a single number here, because a decoder
going quiet looks exactly like a decoder behaving.

False-negative coverage comes from the FIXTURE SUITE -- real ROM words in every
encoding, wrapped at every width, checked to still be caught -- not from this
tool.  Run both.  Neither replaces the other, and the ceilings below are not
evidence that anything still works.

One more gap worth naming: a phantom emitted into the `hex-run` bucket, which
legitimately carries thousands of words, is under a tripwire rather than a zero
and would need to be large to show up.  The eight zero-ceiling stages have no
such slack.

WHAT IT ASSERTS
---------------
1. **Stage totals reconstruct the real output.**  The per-stage buckets must
   sum to exactly what `normalize_words` returns, as a multiset.  This is a
   self-check on the split, not a drift check -- there is deliberately only one
   implementation (see `normalize_words`'s docstring for why the two-copy
   version was worse).
2. **The stage set is closed, in all three directions.**  Every stage in
   `DECODER_STAGES` needs a ledger entry below; every ledger entry needs a
   stage; and every bucket `normalize_words_by_stage` actually *produces* must
   be one of them.  That last one is not redundant -- a decoder emitting into
   an undeclared bucket still has its words returned by `normalize_words`, so
   without it a phantom could ship while this tool reported success.  A new
   decoder cannot be added without recording a measured ceiling for it.
3. **Synthetic decoders contribute nothing.**  Eight of the ten produce zero
   words across this repository.  Zero is the ceiling, and it is not a budget
   to spend: a nonzero count means that decoder is reading text that was never
   encoded.
4. **Real decoders stay under a tripwire.**  `hex-run` and `dec-token` carry
   genuine content, so their ceilings are generous and exist to catch a
   *flood*, not to budget growth.  Raising one is a reviewed one-line diff --
   and it should follow reading the `--verbose` output, not precede it.
"""

import argparse
import collections
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from cleanroom_detectors import (  # noqa: E402
    CONTENT_EXEMPTIONS,
    DECODER_STAGES,
    normalize_words,
    normalize_words_by_stage,
)

# Per-stage ceilings on the number of words the stage may emit, summed over
# every text scanned.  Measured, not guessed; `--verbose` prints the current
# totals so a ceiling is never set from memory.
#
# `None` means "no ceiling in this scope" and is used only where a count grows
# with the decomp itself.
CEILINGS = {
    # ---- synthetic: these decode things.  Real content in this tree is not
    # ---- encoded, so every one of them must stay at zero, forever.
    "hexline-block": {"tracked": 0, "all": 0},
    "base-block": {"tracked": 0, "all": 0},
    "halves-pair": {"tracked": 0, "all": 0},
    "oct-token": {"tracked": 0, "all": 0},
    "dotted-quad": {"tracked": 0, "all": 0},
    "escaped-bytes": {"tracked": 0, "all": 0},
    "base-run": {"tracked": 0, "all": 0},
    "a85-run": {"tracked": 0, "all": 0},
    # ---- real: these read numbers that are genuinely written in the tree.
    #
    # hex-run is THE signal -- every `0x8xxxxxxx` address in source, docs and
    # symbol_addrs.us.txt.  It grows with the decomp, so a tight ceiling here
    # would be a false-positive generator, which is the specific failure this
    # whole file exists to prevent. The tripwire catches a decoder flooding the
    # stage, not a session adding symbols. Exact content exemptions remove the canonical
    # symbol map and generated overlay relocation surface from this total
    # without exempting either file from the clean-room detector itself.
    "hex-run": {"tracked": 4000, "all": 20000},
    # dec-token catches 8-10 digit decimals inside the 32-bit range; in
    # practice, unix timestamps and one fixed-point audio timing table. Small
    # and not expected to grow much -- if it does, look at what appeared.
    "dec-token": {"tracked": 64, "all": 256},
}

#: Stages whose ceiling is zero -- named separately so the failure message can
#: say *why* this is different from exceeding a budget.
SYNTHETIC = frozenset(
    stage for stage, limits in CEILINGS.items() if limits["tracked"] == 0
)
REAL_STAGES = frozenset(DECODER_STAGES) - SYNTHETIC


def texts(scope):
    """Yield ``(label, text)`` for tracked files, or for all history."""
    if scope == "tracked":
        listing = subprocess.run(
            ["git", "ls-files", "-z"], capture_output=True, check=True
        ).stdout
        for path in listing.split(b"\0"):
            if not path:
                continue
            name = path.decode("utf-8", "replace")
            try:
                with open(name, "rb") as handle:
                    data = handle.read()
            except OSError:
                continue
            yield name, data
        return

    revs = subprocess.run(
        ["git", "rev-list", "--all"], capture_output=True, text=True, check=True
    ).stdout.split()
    seen = {}
    for rev in revs:
        listing = subprocess.run(
            ["git", "ls-tree", "-r", rev], capture_output=True, text=True, check=True
        ).stdout
        for line in listing.split("\n"):
            if not line:
                continue
            meta, path = line.split("\t", 1)
            _mode, kind, blob = meta.split()
            if kind == "blob":
                seen.setdefault(blob, path)
    for blob, path in seen.items():
        data = subprocess.run(
            ["git", "cat-file", "blob", blob], capture_output=True, check=True
        ).stdout
        yield f"{path} @{blob[:8]}", data


def audit(scope, verbose=False, by_file=()):
    problems = []

    # -- assertion 2: the stage set is closed ------------------------------
    missing = set(DECODER_STAGES) - set(CEILINGS)
    extra = set(CEILINGS) - set(DECODER_STAGES)
    if missing:
        problems.append(
            f"decoder stage(s) {sorted(missing)} have no ceiling in "
            f"tools/audit_decoders.py. A new decoder needs a MEASURED ceiling "
            f"before it ships: run with --verbose, read what it actually "
            f"emits, and record it. Do not set one from memory."
        )
    if extra:
        problems.append(
            f"ceiling(s) recorded for {sorted(extra)}, which are not in "
            f"DECODER_STAGES. Remove them, or restore the decoder."
        )

    totals = collections.Counter({stage: 0 for stage in DECODER_STAGES})
    exempted_totals = collections.Counter({stage: 0 for stage in DECODER_STAGES})
    contributors = {stage: [] for stage in DECODER_STAGES}
    exempted_contributors = {stage: [] for stage in DECODER_STAGES}
    # Keep one worked example per stage so a failure is actionable.
    example = {}
    scanned = 0

    for label, data in texts(scope):
        if not data or b"\0" in data[:8192]:
            continue
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            continue
        scanned += 1
        by = normalize_words_by_stage(text)
        path = label.rsplit(" @", 1)[0] if scope == "all" else label
        word_table_exempt = (path, "word-table") in CONTENT_EXEMPTIONS

        # -- assertion 2b: the buckets PRODUCED match the declared set -------
        # DECODER_STAGES <-> CEILINGS was checked above, but that is only two
        # of the three sides. A decoder emitting into a bucket that is in
        # neither list was invisible: its words still reach normalize_words
        # (which flattens whatever it is handed), so the phantom shipped while
        # the audit reported success.
        produced = set(by)
        if produced != set(DECODER_STAGES):
            unknown = sorted(produced - set(DECODER_STAGES))
            absent = sorted(set(DECODER_STAGES) - produced)
            problems.append(
                f"{label}: normalize_words_by_stage produced buckets "
                f"{unknown or '[]'} that are not declared in DECODER_STAGES"
                f"{f', and omitted {absent}' if absent else ''}. Words in an "
                f"undeclared bucket are still returned by normalize_words, so "
                f"they reach the detectors unaudited. Declare the stage and "
                f"give it a measured ceiling."
            )

        # -- assertion 1: the split reconstructs the real output -----------
        split = collections.Counter()
        for words in by.values():
            split.update(words)
        if split != collections.Counter(normalize_words(text)):
            problems.append(
                f"{label}: per-stage words do not reconstruct normalize_words(). "
                f"normalize_words must stay the flattening of "
                f"normalize_words_by_stage -- one of them has been edited alone."
            )

        for stage, words in by.items():
            if words:
                # A word-table exemption documents that this exact file
                # legitimately carries numeric word-shaped content.  Apply it
                # only to the two real numeric stages: synthetic decoders must
                # remain zero even inside an exempt file, or an exemption
                # would hide the false decodes this audit exists to expose.
                stage_exempt = (
                    path,
                    f"decoder-audit:{stage}",
                ) in CONTENT_EXEMPTIONS
                if stage in REAL_STAGES and (word_table_exempt or stage_exempt):
                    exempted_totals[stage] += len(words)
                    exempted_contributors[stage].append(
                        (len(words), label, words[:4])
                    )
                    continue
                totals[stage] += len(words)
                contributors[stage].append((len(words), label, words[:4]))
                if stage not in example:
                    example[stage] = (label, words[:4])

    # -- assertions 3 and 4: ceilings --------------------------------------
    for stage in DECODER_STAGES:
        limit = CEILINGS.get(stage, {}).get(scope)
        if limit is None:
            continue
        count = totals[stage]
        if count <= limit:
            continue
        label, sample = example.get(stage, ("?", []))
        shown = ", ".join(f"0x{word:08x}" for word in sample)
        if stage in SYNTHETIC:
            problems.append(
                f"[{stage}] emitted {count} word(s); the ceiling is 0.\n"
                f"    first seen in: {label}\n"
                f"    sample: {shown}\n"
                f"    Nothing in this tree is encoded, so this decoder is "
                f"reading text that was never encoded -- a FALSE DECODE, not a "
                f"budget overrun. Those words are uniformly distributed and go "
                f"straight into `spread`, which is what decides whether a file "
                f"looks like ROM.\n"
                f"    Find what it matched before changing any threshold, and "
                f"re-run this audit after the fix rather than reasoning about "
                f"it -- twice, a fix here simply re-routed the same phantom to "
                f"another decoder."
            )
        else:
            problems.append(
                f"[{stage}] emitted {count} word(s), over its tripwire of "
                f"{limit}.\n"
                f"    first seen in: {label}\n"
                f"    sample: {shown}\n"
                f"    This stage carries real content, so this is not "
                f"automatically a defect -- but confirm the growth is genuine "
                f"before raising the number. Run with --verbose and read what "
                f"it emitted."
            )

    if verbose or problems:
        print(f"decoder audit: {scanned} texts ({scope})", file=sys.stderr)
        width = max(len(stage) for stage in DECODER_STAGES)
        for stage in DECODER_STAGES:
            limit = CEILINGS.get(stage, {}).get(scope)
            mark = "" if limit is None or totals[stage] <= limit else "  <-- OVER"
            shown = "none" if limit is None else str(limit)
            exempt = (
                f"   +{exempted_totals[stage]} exempt"
                if exempted_totals[stage]
                else ""
            )
            print(
                f"  {stage:{width}}  {totals[stage]:7d}   ceiling "
                f"{shown}{exempt}{mark}",
                file=sys.stderr,
            )

    for stage in by_file:
        print(f"decoder contributors [{stage}]", file=sys.stderr)
        rows = sorted(contributors[stage], key=lambda row: (-row[0], row[1]))
        if not rows:
            print("  (none)", file=sys.stderr)
        for count, label, sample in rows:
            shown = ", ".join(f"0x{word:08x}" for word in sample)
            print(f"  {count:7d}  {label}  {shown}", file=sys.stderr)
        exempt_rows = sorted(
            exempted_contributors[stage], key=lambda row: (-row[0], row[1])
        )
        for count, label, sample in exempt_rows:
            shown = ", ".join(f"0x{word:08x}" for word in sample)
            print(
                f"  {count:7d}  {label}  {shown}  (content exempt)",
                file=sys.stderr,
            )

    return problems, scanned


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument(
        "--all",
        action="store_const",
        const="all",
        dest="scope",
        default="tracked",
        help="audit every blob in history, not just tracked files",
    )
    parser.add_argument("--verbose", action="store_true", help="always print totals")
    parser.add_argument(
        "--by-file",
        action="append",
        choices=DECODER_STAGES,
        default=[],
        metavar="STAGE",
        help="print each contributing file for one decoder stage (repeatable)",
    )
    args = parser.parse_args()

    try:
        problems, scanned = audit(args.scope, args.verbose, args.by_file)
    except subprocess.CalledProcessError as error:
        # Fail closed, like every other check here: "I could not tell" is not
        # an answer this repository accepts from a gate.
        print(f"audit-decoders: git failed ({error}); refusing to pass", file=sys.stderr)
        return 2

    if problems:
        print("", file=sys.stderr)
        for problem in problems:
            print(f"audit-decoders: FAIL {problem}", file=sys.stderr)
        print(
            "\nSee the header of tools/audit_decoders.py for what this means.",
            file=sys.stderr,
        )
        return 1

    print(f"decoder audit OK -- {scanned} texts, no decoder inventing words ({args.scope})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
