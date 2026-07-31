"""False-negative coverage for the clean-room content detectors.

    gmake check-fixtures
    python3 tools/false_negative_fixtures.py [--verbose] [--list]

WHY THIS EXISTS, AND WHY IT IS NOT `audit-decoders`
---------------------------------------------------
`tools/audit_decoders.py` asks *is a decoder inventing words that are not
really there?*  That is the false-POSITIVE direction, and it is the direction
that makes a gate fire on legitimate work and get bypassed with `--no-verify`.

It is structurally blind to the opposite failure: a decoder that quietly STOPS
producing words it should.  A decoder going silent looks exactly like a decoder
behaving -- every number in the audit stays green, every file in the tree stays
green, and the gate is now a no-op.  Both defects found in one review round
were of that kind (a base64 run whose length was 1 mod 4 raised inside the
decoder and was swallowed; a per-line floor meant narrow-wrapped payloads were
never joined).  Neither moved a single number in the audit.

This tool is the other direction: real ROM bytes, in every encoding the
normalizer claims to implement, at every wrap width, asserted to still be
caught.  Run both.  Neither replaces the other.

WHY THE FIXTURES ARE GENERATED AND NEVER WRITTEN TO THE TREE
------------------------------------------------------------
A fixture that proves the detectors catch ROM data *is* ROM data.  Committing
one would be the exact thing this repository forbids, and `gmake cleanroom`
would (correctly) reject it.  So the suite has no on-disk form: it reads
`baseroms/mickey.us.z64` -- which is gitignored, present only on a maintainer's
machine, and already required by `gmake verify` -- synthesizes each fixture in
memory, scores it, and drops it.  Nothing is written anywhere.

That is also why this is not in CI and not in the git hooks: there is no
baserom on a CI runner, and there never will be.  Like `audit-decoders`, it is
aimed at whoever edits `tools/cleanroom_detectors.py`, and it must be run there.

WHAT IT ASSERTS
---------------
Two directions, because a suite that only checks one of them drifts:

1. MUST CATCH -- every fixture family below carries real ROM words and must
   produce at least one finding.  A miss is a hole in the detectors.
2. MUST PASS -- the families corresponding to holes `docs/CLEANROOM.md`
   documents by name must NOT be caught.  If one of them starts being caught,
   that is good news, but the document is now wrong and says the tree is weaker
   than it is.  Either direction fails the run.
"""

import argparse
import base64
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
sys.path.insert(0, HERE)

import cleanroom_detectors as D  # noqa: E402

BASEROM = os.path.join(REPO, "baseroms", "mickey.us.z64")

#: Offset into the ROM the fixtures are cut from.  Any code-bearing region
#: works; this one is the one every prior measurement in docs/CLEANROOM.md used,
#: so figures quoted there are reproducible against this suite.
FIXTURE_OFFSET = 0x32630
FIXTURE_WORDS = 400


def hardwrap(text, width):
    """Faithful column wrap.

    `textwrap.wrap` breaks on hyphens and reflows, which silently mangles
    base-N payloads -- that produced phantom "misses" in an earlier round.
    """
    return "\n".join(text[i:i + width] for i in range(0, len(text), width))


def load_rom():
    if not os.path.exists(BASEROM):
        sys.stderr.write(
            f"false_negative_fixtures: {BASEROM} not found.\n"
            "  This suite synthesizes its fixtures from the baserom at run time\n"
            "  (they are ROM data and must never be committed).  Without one\n"
            "  there is nothing to test, and reporting success would be a lie,\n"
            "  so this is a failure rather than a skip.  Put the US baserom in\n"
            "  baseroms/ -- the same one `gmake verify` needs.\n")
        raise SystemExit(2)
    with open(BASEROM, "rb") as fh:
        fh.seek(FIXTURE_OFFSET)
        return fh.read(FIXTURE_WORDS * 4)


def fixtures(raw):
    """Return ``{name: (text, must_be_caught)}``.

    Each family is one way the same 400 ROM words can reach a text file.
    """
    words = [int.from_bytes(raw[i:i + 4], "big") for i in range(0, len(raw), 4)]
    b64 = base64.b64encode(raw).decode()
    b32 = base64.b32encode(raw).decode()
    a85 = base64.a85encode(raw).decode()
    f = {}

    def add(name, text, must_catch=True):
        f[name] = (text, must_catch)

    # -- plain layouts ----------------------------------------------------
    add("hex_bare", "\n".join(f"{w:08x}" for w in words))
    add("hex_upper", "\n".join(f"0X{w:08X}" for w in words))
    add("c_array", "static const u32 t[] = {\n" + "\n".join(
        "    " + ", ".join(f"0x{w:08x}" for w in words[i:i + 4]) + ","
        for i in range(0, len(words), 4)) + "\n};\n")
    add("c_array_u_suffix",
        "static const u32 t[] = {" + ",".join(f"0x{w:08x}u" for w in words) + "};")
    add("c_array_underscored",
        "static const u32 t[] = {" + ",".join(
            f"0x{w >> 16:04x}_{w & 0xffff:04x}" for w in words) + "};")
    add("disasm", "\n".join(
        f"  {0x80000000 + i * 4:08x}:\t{w:08x}\taddiu\t$sp, $sp, -0x{w & 0xff:02x}"
        for i, w in enumerate(words)))
    add("hexdump_xxd", "\n".join(
        f"{i * 16:08x}: " + " ".join(
            raw[i * 16 + j:i * 16 + j + 2].hex() for j in range(0, 16, 2))
        for i in range(len(raw) // 16)))
    add("escaped_bytes", '"' + "".join(f"\\x{b:02x}" for b in raw) + '"')
    add("dotted_quad", "\n".join(
        ".".join(str((w >> s) & 0xFF) for s in (24, 16, 8, 0)) for w in words))
    add("octal", "\n".join(f"0o{w:o}" for w in words))
    add("decimal_json",
        json.dumps([{"i": i, "w": w} for i, w in enumerate(words)]))
    add("jsonl_ledger", "\n".join(
        json.dumps({"index": i, "target_word": f"{w:08x}", "class": "operand"})
        for i, w in enumerate(words)))
    add("hex_range", "\n".join(
        f"0x{w:08x}-0x{(w + 4) & 0xffffffff:08x}" for w in words))

    # -- 16-bit halves ----------------------------------------------------
    # Volume, not adjacency, is the discriminator (see HALF_PAIR_MIN).  Every
    # one of these defeated an earlier, narrower rule.
    add("halves_spaced", " ".join(
        f"{w >> 16:04x} {w & 0xffff:04x}" for w in words))
    add("halves_comma", ",".join(
        f"{w >> 16:04x},{w & 0xffff:04x}" for w in words))
    add("halves_column", "\n".join(
        f"{w >> 16:04x}\n{w & 0xffff:04x}" for w in words))
    add("halves_json_array", "{\n" + "\n".join(
        f'  "w{i}": ["0x{w >> 16:04x}", "0x{w & 0xffff:04x}"],'
        for i, w in enumerate(words)) + "\n}")
    add("halves_split_keys", "{\n" + "\n".join(
        f'  "w{i}_hi": "0x{w >> 16:04x}",\n  "w{i}_lo": "0x{w & 0xffff:04x}",'
        for i, w in enumerate(words)) + "\n}")
    add("halves_unicode_escape", '"' + "".join(
        f"\\u{w >> 16:04x}\\u{w & 0xffff:04x}" for w in words) + '"')
    add("halves_md_table", "| id | hi | lo |\n|---|---|---|\n" + "\n".join(
        f"| w{i} | {w >> 16:04x} | {w & 0xffff:04x} |"
        for i, w in enumerate(words)))

    # -- base-N, unwrapped ------------------------------------------------
    add("b64_plain", b64)
    add("b64url", base64.urlsafe_b64encode(raw).decode())
    add("b32_plain", b32)
    add("a85_plain", a85)

    # -- base-N, wrapped.  THIS IS THE REGRESSION SET. --------------------
    # A per-line floor that is too high means narrow-wrapped payloads are never
    # joined, and nothing else in the tree notices.  One such floor shipped.
    for width in (4, 6, 8, 10, 12, 14, 15, 17, 20, 21, 25, 27, 31, 40, 64, 76):
        add(f"b64_wrap{width:02d}", hardwrap(b64, width))
    for width in (4, 8, 12, 16, 20, 24, 28, 31, 40, 64, 72, 76):
        add(f"a85_wrap{width:02d}", hardwrap(a85, width))
    for width in (8, 16, 32, 64, 76):
        add(f"b32_wrap{width:02d}", hardwrap(b32, width))

    # -- base-N, undecodable trailing length ------------------------------
    # A run whose length is 1 mod 4 (base64) cannot be decoded as-is.  Raising
    # inside the decoder swallowed the whole run; the fix trims to the longest
    # decodable prefix.  Reverting that trim leaves audit-decoders green.
    def truncate_to(text, modulus, want):
        n = len(text.rstrip("="))
        while n % modulus != want:
            n -= 1
        return text[:n]

    add("b64_len_mod4_is_1", truncate_to(b64, 4, 1))
    add("b32_len_mod8_is_1", truncate_to(b32, 8, 1))
    add("a85_len_mod5_is_1", truncate_to(a85, 5, 1))

    # -- documented holes: these MUST still pass --------------------------
    # Each is named in docs/CLEANROOM.md "What it does not catch".  If one
    # starts being caught, that document is overstating the hole and must be
    # corrected -- so it fails the run in that direction too.
    add("HOLE_decimal_halves", " ".join(
        f"{w >> 16} {w & 0xffff}" for w in words), must_catch=False)
    add("HOLE_prose_dilution", "\n".join(
        f"The word at index {i} of the routine under study is 0x{w:08x}, and it "
        f"was observed during the comparison campaign recorded here."
        for i, w in enumerate(words[:D.WORD_TABLE_COUNT_LIMIT - 1])),
        must_catch=False)
    add("HOLE_high_byte_stripped", " ".join(
        f"{w & 0xFFFFFF:06x}" for w in words), must_catch=False)
    add("HOLE_digests_65", "| id | digest | state |\n|---|---|---|\n" + "\n".join(
        f"| ref {i} | `{raw[i * 32:i * 32 + 32].hex()}` | verified |"
        for i in range(65)), must_catch=False)
    return f


def run(verbose=False):
    raw = load_rom()
    if len(raw) != FIXTURE_WORDS * 4:
        sys.stderr.write("false_negative_fixtures: short read from baserom\n")
        return 2

    misses, regressions, ok = [], [], 0
    for name, (text, must_catch) in sorted(fixtures(raw).items()):
        # Scored under a path with no exemptions and no manifest handling, so
        # the result is the content rules alone.
        found, metrics = D.check_content("docs/fixture.md", text.encode())
        detectors = sorted({d for d, _ in found})
        if must_catch and not detectors:
            misses.append((name, metrics, len(text)))
        elif not must_catch and detectors:
            regressions.append((name, detectors))
        else:
            ok += 1
        if verbose:
            print(f"  {name:26s} {len(text):7d}B  words={metrics['words']:4d} "
                  f"spread={metrics['spread']:3d}  "
                  f"{','.join(detectors) or '(passes)'}")

    if misses:
        sys.stderr.write("false-negative fixtures: FAILED -- ROM data not caught\n")
        for name, metrics, size in misses:
            sys.stderr.write(
                f"  {name}: {size} bytes of real ROM produced NO finding "
                f"(words={metrics['words']}, spread={metrics['spread']})\n")
        sys.stderr.write(
            "\nA decoder has gone silent.  `gmake audit-decoders` cannot see "
            "this -- it only\nlooks for decoders inventing words, and a decoder "
            "that stops producing them\nlooks identical to one behaving.  Fix "
            "the decoder; do not adjust a threshold.\n")
    if regressions:
        sys.stderr.write(
            "false-negative fixtures: FAILED -- a documented hole is now closed\n")
        for name, detectors in regressions:
            sys.stderr.write(
                f"  {name}: expected to pass (docs/CLEANROOM.md lists it under "
                f"\"What it does not catch\"), but fired {','.join(detectors)}\n")
        sys.stderr.write(
            "\nThis is good news about the detectors and bad news about the "
            "document:\ndocs/CLEANROOM.md is now claiming a weakness that no "
            "longer exists.  Update it\nand this suite together.\n")
    if misses or regressions:
        return 1

    print(f"false-negative fixtures OK -- {ok} fixture families, "
          f"every encoding caught, every documented hole still open")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--verbose", action="store_true",
                        help="print every fixture with its metrics")
    parser.add_argument("--list", action="store_true",
                        help="list fixture family names and exit")
    args = parser.parse_args()
    if args.list:
        for name, (_t, must) in sorted(fixtures(load_rom()).items()):
            print(f"{'catch' if must else 'pass '}  {name}")
        return 0
    return run(args.verbose)


if __name__ == "__main__":
    sys.exit(main())
