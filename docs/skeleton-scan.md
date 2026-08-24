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
`~/Desktop/dev/coddog`, outside this repo. Only the `coddog-cli` crate is
built; `coddog-api`/`coddog-db` need Postgres (via `sqlx`) and are not needed
for any of the CLI-only comparisons below -- `SQLX_OFFLINE=true` plus the
committed `.sqlx` query cache in coddog's own repo lets the workspace build
without a live database, but building just `-p coddog-cli` sidesteps the
question entirely.

### Status: built and working via `compare-raw`; no Postgres involved

Build: `cargo build --release -p coddog-cli`, 34 s cold, clean. `coddog
compare-raw` and the other CLI subcommands (`match`, `cluster`, `submatch`,
`compare2`, `compare-n`) need no database -- only the Postgres-backed
`coddog-db` indexer (out of scope here, and not attempted) does.

**`decomp.yaml`**: only JFG's reference checkout already ships one
(`~/Desktop/dev/decomp-refs/jfg/decomp.yaml`), and even that one lists a
`kiosk` version whose `.elf` was never built in this checkout, which makes
`compare-raw` abort outright (it reads every listed version). DKR, Perfect
Dark, Banjo-Kazooie and Conker have none. Per the task, none of the four
reference repos were modified; instead minimal `decomp.yaml` files (and a
trimmed single-version copy of JFG's) were written to a scratch directory,
`~/Desktop/dev/coddog/mickey-ref-configs/{jfg,dkr,bk,pd,conker}/decomp.yaml`,
pointing at each project's existing build output with absolute paths:

| Project | Symbol source used | Notes |
|---|---|---|
| JFG | `build/jfg.us.elf` | trimmed to the `us` version only |
| DKR | `build/dkr.us.v77.elf` | |
| Banjo-Kazooie | `build/us.v10/banjo.us.v10.elf` | |
| Conker | `conker/build/conker.us.map` + `baserom.us.z64` as `target` | no `.elf` was ever produced in this checkout, so map-based ingestion is used instead (`decomp_settings`' other supported symbol source) |
| Perfect Dark | `build/ntsc-final/pd.map` + `pd.ntsc-final.z64` | **does not work** -- see below |

**Perfect Dark is blocked.** Its build only produces a single-stage
`build/ntsc-final/stage1.elf`, not a whole-ROM ELF, so the `elf` path was
left unset and the map-based fallback (`paths.target` + `paths.map`) was
tried instead. That panics inside coddog itself:
`crates/core/src/ingest.rs:136`, `range end index 33554516 out of range for
slice of length 33554432` while reading `pd.map` against the 32 MiB ROM --
a symbol's claimed vROM range runs past the end of the target file coddog
was given. This reads as a real bug in coddog's PD/`objdiff`-based map
ingestion, not a Mickey-side or reference-side data problem (the file sizes
and map are exactly what PD's own build produced), and fixing it is out of
scope for this task. **PD was dropped from the `compare-raw` runs below**;
everywhere else in this repo (`docs/acceleration-survey.md`'s counts,
`tools/find_known_objects.py`, `skeleton_scan.py` itself) PD is indexed
successfully from its per-object `.o` files instead, which don't hit this
map-parsing path.

### Findings: `compare-raw` against Mickey's ROM

```sh
python3 - <<'EOF'   # writes a scratch .bin outside the repo, never committed
rom = open('baseroms/mickey.us.z64','rb').read()
open('/tmp/resident.bin','wb').write(rom[0x1000:0x76D10])
EOF
~/Desktop/dev/coddog/target/release/coddog compare-raw /tmp/resident.bin \
  ~/Desktop/dev/coddog/mickey-ref-configs/jfg/decomp.yaml \
  ~/Desktop/dev/coddog/mickey-ref-configs/dkr/decomp.yaml \
  ~/Desktop/dev/coddog/mickey-ref-configs/bk/decomp.yaml \
  ~/Desktop/dev/coddog/mickey-ref-configs/conker/decomp.yaml
```

- **Resident text (ROM 0x1000-0x76D10, 482,576 B), vs JFG+DKR+BK+Conker**:
  260 lines, `0xOFF - Project Version: name (decompiled)`, offsets relative
  to the sliced file (add `0x1000` for ROM). Runtime well under a second.
  Sample hits agree with independently-derived evidence already in the
  tree: `0x2957C -> Diddy Kong Racing US v77: rand_range` is exactly the ROM
  0x2A57C / DKR `rand_range` correspondence `symbol_addrs.us.txt` already
  records (0x2957C + 0x1000 = 0x2A57C). The bulk of the hits are JFG names
  already seen in `skeleton_scan.py scan --region resident`
  (`shadowBoxPolyOverlap`, the `light*`/`matrix*`/`math*` family,
  `frontDrawRectangle`, `diCpuTraceInit`, etc.), which is the expected
  overlap between two tools measuring the same underlying fact.
- **Overlay 101 (52,960 B) and overlay 49 (896 B), same four projects**:
  **zero hits**, both. Matches `skeleton_scan.py kinship`'s finding that
  overlay text carries almost no reference-project skeleton content (0.3%
  region coverage, `docs/acceleration-survey.md` section 2.1) -- overlay 49
  is the one place `skeleton_scan.py scan --region overlays` finds anything
  at all in the whole atlas (a 44-byte, 6-way-ambiguous hit at `+0x354`),
  and even that is invisible to `compare-raw`, for a structural reason (see
  below).

### `compare-raw` vs `skeleton_scan.py`: what each catches that the other doesn't

- **Window size.** `compare-raw` hard-codes a 20-*instruction* (80-byte)
  sliding window (`window_size` in `crates/cli/src/main.rs`'s `CompareRaw`
  arm) and only reports a match when the *first* hash of that window lands
  on a *first* hash already seen in a reference function, then verifies the
  following instructions agree exactly. `skeleton_scan.py`'s `--min-words
  10` (40-byte) floor is half that, and it matches whole functions, not
  windows -- the overlay 49 hit above (44 bytes = 11 words) is exactly the
  size class `compare-raw` cannot see with its default window, and no CLI
  flag changes it (would need a coddog patch).
- **Match unit.** `skeleton_scan.py scan` reports whole-function boundaries
  greedily and non-overlapping, with an ambiguity count, and a
  `--emit-symbols` line ready to paste. `compare-raw` reports one line per
  window-start match, not de-duplicated to function boundaries, and no
  ambiguity count -- multiple reference projects each print their own line
  for the same Mickey offset instead of grouping into one hit with several
  candidate names (visible above: DKR, JFG and BK all appear as separate
  lines rather than one row with three donors).
- **Similarity, not just identity.** `coddog`'s `match`/`cluster`/`compare2`
  compute a graded score (edit distance over opcode sequences, plus an
  "equivalent" hash that keeps some operands), which is closer to
  `skeleton_scan.py similar`'s Jaccard ranking than to `scan`'s hard
  in/out matching -- but those subcommands operate on one project's own
  indexed symbol set (`match`/`cluster`) or two full projects
  (`compare2`/`compare-n`), not a raw unnamed binary slice, so they were not
  usable directly against Mickey's overlay ROM without first getting
  Mickey's own build indexed as a `decomp.yaml` project (not attempted --
  out of scope, since the goal here was reference-vs-Mickey matching, not
  Mickey-vs-Mickey).
- **What's stronger in coddog:** real instruction decoding via
  `objdiff-core`/`rabbitizer` rather than `skeleton_scan.py`'s hand-rolled
  MIPS field masking, so it is very unlikely to have a decode bug
  `skeleton_scan.py` might; and its three-hash design (exact / equivalent /
  opcode-only) gives a similarity gradient `scan`'s single masked-skeleton
  key does not.

Net: for this task's purpose -- finding donor names for unnamed resident
functions -- the two tools agree everywhere they overlap, `skeleton_scan.py`
catches more (smaller functions, function-granularity ambiguity), and
`compare-raw` needed real setup work (the PD blocker, the missing
`decomp.yaml`s) that `skeleton_scan.py` does not, because the latter reads
reference `.o` files directly rather than going through `decomp_settings`.
coddog's `match`/`cluster` (Mickey-vs-Mickey duplicate/near-duplicate
finding within the overlay set) remains an unexplored, plausibly useful
follow-up that would need Mickey indexed as its own `decomp.yaml` project.
