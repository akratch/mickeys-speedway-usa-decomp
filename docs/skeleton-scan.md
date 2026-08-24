# skeleton_scan

`tools/skeleton_scan.py` is the project-tool version of the review's scratch
prototypes (`scratchpad/fingerprint.py`, `kinship.py`, `resident.py`,
`selfsim.py` -- see `docs/acceleration-survey.md` sections 2-3 for the
methodology and the original numbers). It finds masked-instruction-shape
("skeleton") matches between the five permitted reference decomps and
Mickey's ROM, and prints offsets, sizes and donor candidates. It never writes
ROM bytes, disassembly text, or hexdumps to any file -- it reads the baserom
and reference *built objects* at runtime and prints only derived numbers and
symbol names, the same discipline `tools/find_known_objects.py` already
follows. `gmake cleanroom` covers it like any other tracked file.

Nothing here adopts a name on its own. It prints candidates; a human still
adds a `PROVENANCE` note and pastes the line into `symbol_addrs.us.txt`
following the tiers in `docs/modules.md` section 1.

## What "skeleton" matching means

Every instruction word is masked down to the fields that survive a recompile
of the same source with different register allocation, addresses and
constants: the primary opcode, plus (for `SPECIAL`) the funct field, (for
`REGIMM`) the rt sub-opcode field, and (for `COP1`) the fmt field and, for
arithmetic, funct too. Two functions with identical masked bytes are, with
very high confidence, the same source compiled by the same compiler. This is
pure bit arithmetic on opcode fields (`mask_word` in the tool) -- it is not
disassembly and never produces mnemonic text, which is what keeps the tool
itself clean-room-safe to track.

## Requirements

Stdlib only. Reads:

- `baseroms/mickey.us.z64` (gitignored, must exist -- see the build
  quickstart in `CLAUDE.md`)
- `config/overlays.us.json` (the generated overlay atlas)
- `symbol_addrs.us.txt` (for the "already named" checks)
- the five reference decomps' built `.o` files, default location
  `~/Desktop/dev/decomp-refs/{diddy-kong-racing,jfg,perfect_dark,banjo-kazooie,conker}`,
  overridable with `--refs DIR [DIR...]` or `$MICKEY_DECOMP_REFS`

It does **not** need `build/mickey.us.elf`. Mickey-side function boundaries
come from `config/overlays.us.json`'s `text_ownership` (overlays) or
`symbol_addrs.us.txt`'s `size:0x..` comments (resident, for `similar` only);
`scan` itself uses the same greedy longest-match-first, non-overlapping
search the prototype used, since most of the resident segment and nearly all
overlay text has no boundary information yet.

Runtime: 2-4 seconds per subcommand on this tree (five reference projects,
~21,000 functions >= 10 words indexed).

## `scan`

```sh
python3 tools/skeleton_scan.py scan --region resident --unnamed-only
python3 tools/skeleton_scan.py scan --region overlays --unnamed-only --json
python3 tools/skeleton_scan.py scan --region rom:0x1000-0x2000
python3 tools/skeleton_scan.py scan --region resident --unnamed-only --emit-symbols
```

- `--region resident` -- the resident static segment's `.text`, ROM
  `0x1000`-`0x76D10`. (This is `mickey.us.yaml`'s `main` segment text; the
  `- [0x76D10, bin]` / `- [0x76E60, data]` subsegments there are where
  `.data` starts, so this is *not* the same as the review prototype's
  "resident tail 0x76D10-0x86990" region -- that range is `.data`/`.rodata`,
  not code, and scanning it as instruction words would not be a legitimate
  skeleton match.)
- `--region overlays` -- all (currently 106 non-empty) overlay `.text`
  sections from `config/overlays.us.json`.
- `--region rom:START-END` -- an arbitrary raw ROM byte range in hex, for
  probing anything not covered by the two named regions.
- `--refs DIR [DIR...]` -- override the reference project list. Each
  directory is searched recursively for `*.o`; the project tag is inferred
  from a known directory name or the directory's own basename.
- `--min-words N` (default 10) -- functions shorter than this are not
  indexed on the reference side.
- `--unnamed-only` -- drop hits that start at a location already recorded as
  named/matched: an exact `symbol_addrs.us.txt` `type:func` VRAM (resident)
  or a `text_ownership` entry with `matched: true` covering the hit's start
  (overlays).
- `--emit-symbols` -- after the hit list, print `name = 0xVRAM; //
  type:func size:0x.. tier:B skeleton:<project>:<name>` lines for hits that
  are both unnamed and unambiguous (exactly one donor). Only meaningful for
  `--region resident`, since `symbol_addrs.us.txt` only names resident VRAM
  symbols -- overlay functions are named through the atlas's
  `text_ownership.source`, not this file, so overlay hits print without a
  usable VRAM. The tool never writes to `symbol_addrs.us.txt`; paste
  candidates in yourself, with a `PROVENANCE` note.
- `--json` -- machine-readable hit list.

Ambiguity in the output is the number of distinct `(project, name)` pairs
that share the hit's masked skeleton -- `amb=1` is an unambiguous donor,
higher counts mean the shape is common enough (small/generic functions,
mostly) that several reference names collide on it.

## `kinship`

```sh
python3 tools/skeleton_scan.py kinship
python3 tools/skeleton_scan.py kinship --ngram 8 --refs DIR [DIR...]
```

For each region (resident text, all overlay text, and each overlay
`>= 8000` bytes), prints the fraction of the region's masked N-word n-grams
(default N=8) that occur anywhere in each reference project's masked
n-gram set, plus a reference-vs-reference calibration block computed the
same way. High kinship to one project with the calibration rows as a
baseline is what makes "Mickey's resident segment is JFG's engine, not
DKR's" (`docs/acceleration-survey.md` section 2.1) a measured claim rather
than an impression.

### Calibration numbers measured on this tree (8-gram, `--min-words 10`)

Reference-vs-reference (asymmetric: row's grams checked against column):

| ref \ vs | dkr | jfg | pd | bk | conker |
|---|---:|---:|---:|---:|---:|
| dkr | | 17.4% | 5.0% | 8.8% | 5.7% |
| jfg | 11.1% | | 8.2% | 4.1% | 6.5% |
| pd | 2.4% | 6.1% | | 4.9% | 8.4% |
| bk | 6.2% | 4.6% | 7.2% | | 6.6% |
| conker | 2.5% | 4.6% | 7.9% | 4.2% | |

Mickey regions vs each reference project (`any` = union of all five):

| Region | dkr | jfg | pd | bk | conker | any |
|---|---:|---:|---:|---:|---:|---:|
| resident text 0x1000-0x76D10 | 10.4% | **34.2%** | 21.7% | 9.9% | 16.1% | 41.9% |
| all overlay text | 3.1% | 7.9% | 5.7% | 4.5% | 5.2% | 15.6% |

The resident segment sits well above every reference-vs-reference baseline
against JFG (34.2% vs. the 11-17% DKR/JFG cross-family baseline); the
overlays sit inside the unrelated-engine range. This matches
`docs/acceleration-survey.md` section 2.1's finding with Conker added as a
fifth project (the survey's original run used four).

## `similar`

```sh
python3 tools/skeleton_scan.py similar --target 0x8006E820 --top 10
python3 tools/skeleton_scan.py similar --target "49:+0x354" --top 10
```

Finds the nearest reference functions to one target Mickey function, for use
as in-context examples when hand-matching it (m2c prompt, permuter seed,
manual comparison). `--target` is either:

- `0xVRAM` -- a resident function. It must already have a `size:0x..`
  comment in `symbol_addrs.us.txt` (the tool has no other way to bound it).
- `N:+0xOFF` -- overlay `N` at text offset `OFF`. `OFF` must be an existing
  `text_ownership` entry's start (i.e. a function boundary splat already
  knows).

Candidates are filtered to reference functions within +/-30% of the
target's size, then ranked by masked n-gram (default 4-word) Jaccard
similarity. A short n-gram than `kinship`'s default is used here because
single functions are often well under 8 words long by the time you're
looking for an example to match against.

## Not yet done

`docs/acceleration-survey.md` section 3 names three real tools for this job
(coddog, objdiff, and this prototype). `skeleton_scan.py` is the
`fingerprint.py` promotion; coddog setup and findings are covered separately
(`tools/setup_coddog.sh`, section below).

## coddog

[coddog](https://github.com/ethteck/coddog) (ethteck, Rust) computes three
per-function hashes (exact bytes, opcodes+some operands, opcodes only) plus
bounded-Levenshtein opcode-sequence comparison, and offers `compare-raw
<binary> <yamls...>` for comparing a raw binary directly against one or more
`decomp.yaml`-described projects, and a Postgres-backed indexer/`match`
workflow for cross-project search at scale.

`tools/setup_coddog.sh` clones and builds it (`cargo build --release`) into
`~/Desktop/dev/coddog`, outside this repo. See that script's own comments and
the status note below for what was and wasn't runnable in the time budget.
