# Tooling: decomp-permuter, objdiff, and the smoke test

This is reference documentation for the project's matching tools. See
`docs/reference-findings.md` section 6 and ADR 0007 for the findings and
decisions that motivated them. For how to *use* each tool day-to-day, see
`skills/tools/permuter.md` and `skills/tools/objdiff.md` -- this file covers
setup, what's committed vs. gitignored, and why each piece is shaped the way
it is.

## decomp-permuter

Not a pip package: `~/Desktop/dev/decomp-permuter` (or wherever it's
checked out) has a `pyproject.toml` with no `[build-system]` table, only
`[tool.pyright]`/`[tool.black]` config, so `pip install -e` has nothing to
build. Instead:

```sh
ln -sfn /path/to/decomp-permuter tools/permuter   # gitignored, machine-specific
.venv/bin/pip install toml pynacl                 # its only real deps beyond stdlib
```

`toml` reads `tools/permuter_settings.toml`; `pynacl` is only used by
permuter@home (the distributed-compute mode, unused here) but `import.py`
imports it unconditionally at startup regardless. Both are pinned in
`requirements.txt`, along with the exact decomp-permuter commit this was
proven against (`requirements.txt`'s comment, since the checkout itself
isn't a package with its own version string).

### permuter_settings.toml: why the compiler command is hardcoded

`import.py`'s default `build_system = "make"` mode runs `gmake
--always-make --dry-run --debug=j PERMUTER=1 <target>` and parses the one
debug-trace line containing the source file to recover the compiler
invocation (`fixup_build_command()` in decomp-permuter's `import.py`). That
assumes the recipe is one line. This project's C rule is a shell line
*continuation*:

```make
$(BUILD_DIR)/%.c.o: %.c ... | $(ALL_DIRS) $(SPLAT_STAMP)
	$(ASM_PROCESSOR) $(CC) -- $(AS) $(ASM_PROC_ASFLAGS) -- \
		-c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<
```

`make --debug=j` echoes this as two physical lines (the trailing `\`
survives, and the tab-indented continuation prints separately). Only the
second line contains the `.c` path, so `import.py`'s line-scan finds a
"compiler" of just `-c ...` with `tools/ido/cc` missing, and fails with
`-c: command not found`. Verified by hand:

```sh
gmake --always-make --dry-run --debug=j PERMUTER=1 build/src/libultra/<f>.c.o
```

The fix, matching what `tools/permuter_settings.toml`'s own comment
documents in full: set `compiler_command`/`assembler_command` directly,
bypassing `find_build_command_line` entirely. Candidates the permuter
mutates are plain C with no `#pragma GLOBAL_ASM`, so asm-processor's
split-and-recombine step is a no-op for them -- calling `tools/ido/cc`
directly, with the same flags asm-processor would have passed through, is
equivalent. The `assembler_command` also deliberately omits
`include/asm_processor_prelude.inc`: that file and decomp-permuter's own
`prelude.inc` (always prepended to `target.s` by `import.py`) both define
`glabel`/`endlabel`/etc. as `.macro`, and assembling both together is a
"Macro already defined" error.

The hardcoded flags are the libultra-default group (`-O2 -mips1 -32`); the
toml's header comment lists the other three groups this Makefile defines
(overlay/`src/main` game code at `-O2 -mips2 -32`; two other libultra
sub-groups) and how to retarget for them.

### Proof: `__osContRamRead`

Ran end-to-end against `src/libultra/contramread.c` /
`asm/nonmatchings/libultra/contramread/__osContRamRead.s` (still
`#pragma GLOBAL_ASM`; a from-scratch candidate C body was written for the
test, adapted from Jet Force Gemini's published `libultra/src/io/contramread.c`
with a `PROVENANCE` note, per `docs/CLEANROOM.md` -- the same pattern
`src/libultra/pfsgetstatus.c` already uses for a sibling function -- then
reverted afterward; this was a tooling proof, not a matching attempt):

```
base score = 1330
best score (3-minute cap, -j 12, --stop-on-zero): 1055
```

No exact match (not required -- the point was a working loop). The winning
mutation joined a `for` loop's body onto one line, which is exactly the
kind of `perm_sameline`-shaped move IDO's scheduler is sensitive to and a
human would otherwise have to guess at.

## objdiff

`tools/setup_objdiff.sh` fetches the `objdiff-cli` binary from
[encounter/objdiff releases](https://github.com/encounter/objdiff/releases)
(macOS arm64 asset `objdiff-cli-macos-arm64`) into `tools/objdiff/`
(gitignored, like `tools/ido/`/`tools/binutils/`) and records the resolved
version in `tools/objdiff/VERSION`. Proven against v3.8.0.

objdiff diffs *object files*, base vs. target. The target ("expected") side
here is a snapshot of a previously `gmake verify`-clean `build/` --
following DKR's/dp64's `expected/build/...` convention (dp64's own
`objdiff.json`, read for schema reference, uses exactly this shape:
`target_path`/`base_path` pairs per unit) -- not the baserom directly, since
objdiff needs linked, sectioned object files, not a ROM binary blob.
`tools/make_expected.sh` runs `gmake verify` and then `cp -R build
expected/build`; re-run it whenever `build/` changes, or the report compares
against a stale target.

`objdiff.json` (committed) lists one `unit` per built object:
`tools/gen_objdiff_config.py` regenerates it from whatever's currently under
`build/` (`*.o`, excluding `build/permuter/` and `build/wb/` scratch), and
`tools/objdiff_report.sh` calls it automatically when `build/` looks newer
than the existing config.

### The trimmed-object exclusion list

686 of this project's ~832 C objects carry a Makefile `POSTPROCESS`
override -- overwhelmingly `trim_elf_section.py` (IDO aligns standalone
`.text` to 16 bytes; many of these reviewed overlay functions continue at a
4-byte boundary inside a larger module, so the trimmer shortens the section
header after the fact) or `normalize_elf_instructions.py` (patches
individual instruction words post-compile; see the Makefile's own comments
on both). objdiff-cli's `report generate` aborts its *entire* batch on the
first object it can't parse as a result -- `Section symbol without section`,
or for some objects, an unattributed `Symbol data out of bounds` with no
file name in the error at all, `-L debug` included.

Given that, per-object bisection to find every offender isn't worth it:
`tools/objdiff_report.sh` regenerates `tools/objdiff_exclude.txt` fresh on
every run with the full `POSTPROCESS`-override list:

```sh
grep -oE '^\$\(BUILD_DIR\)/\$\(SRC_DIR\)/[A-Za-z0-9_/]+\.c\.o: POSTPROCESS' \
    Makefile | sed -E 's#\$\(BUILD_DIR\)/\$\(SRC_DIR\)/#src/#; s/: POSTPROCESS$//' \
    | sort -u > tools/objdiff_exclude.txt
```

Deliberately gitignored rather than committed, unlike `objdiff.json`: it is
~700 lines of nothing but object-file basenames, no punctuation, no other
structure -- which `tools/cleanroom_detectors.py`'s base64-volume heuristic
reads as high-entropy text (834.5 base64-shaped chars/KiB against its
400/KiB threshold) even though every byte in it is a filename copied from
the Makefile, not ROM content. A real false positive on this specific file
shape, not a loophole worth routing around with a `CONTENT_EXEMPTIONS`
entry when "don't track the derived file, regenerate it" is simpler and
correct on its own merits regardless of the detector.

`tools/objdiff_report.sh` also retries with a newly-discovered offender
excluded on any *attributable* failure (`Failed to open ... .o`), as a
defensive fallback -- but the unattributed failure mode means this can't be
fully automatic. **This is a known scope limit**: objdiff currently reports
on the un-postprocessed objects only (719 of 1405 units as of this writing),
which is still most non-overlay code plus the overlay functions that don't
need trimming/normalization. Extending coverage to the rest would mean
teaching objdiff-cli's ELF reader about this project's post-linked object
shapes, which is out of scope for this lane.

### Proof: current build

```sh
gmake -j8 && gmake verify && ./tools/make_expected.sh && ./tools/objdiff_report.sh
```

719 units, 414 with nonzero code size, 711156/711156 bytes matched (100%) --
expected, since `expected/build/` was snapshotted from the same `build/`
being diffed. The report becomes informative once `expected/build/` is
refreshed from an *older* verified build (regressions), or a future
alternate target (a donor object, a different flag group) is compared
against the current one via a second `-2 <base>` pointed elsewhere with
`objdiff-cli diff` directly.

## mapfile_parser

`pip install mapfile_parser` (pinned `2.13.2` in `requirements.txt`).
Underlies decomp.dev-style progress reporting elsewhere in the splat/objdiff
ecosystem; not itself wired into a script here, but smoke-tested by
`tools/check_tools.sh` since it's a `gmake setup` dependency going forward.

## tools/check_tools.sh

Runs each tool's `--version`/`--help` and prints one line per tool:

```sh
./tools/check_tools.sh
```

Covers splat, spimdisasm, asm-differ, m2c, mapfile_parser, toml,
decomp-permuter (skipped with a note if `tools/permuter` isn't linked),
objdiff-cli (skipped if not fetched), and the gitignored IDO/binutils
binaries (skipped if `gmake setup` hasn't run). Exits nonzero if anything
that *is* present fails to start.

When the workbench, baserom, and an existing build are present, the smoke test
also runs `tools/wb_compare.sh --rom` on a matched overlay function; the
wrapper maps resident and overlay symbols to ROM offsets from their ELF
section VMA/LMA pairs and writes its retained dumps only under ignored
`build/wb/`. The default candidate is `build/`; set `WB_ROM_BUILD_DIR` to an
alternate build directory to compare a linked `NON_MATCHING=1` diagnostic ROM
without replacing the verified build tree.

## Map: the rest of the toolbox

The tools above (decomp-permuter, objdiff, mapfile_parser, check_tools.sh)
have their own detailed sections because this file started as their setup
doc. Everything below just points at where its own documentation lives —
this file is a map, not a manual, for the rest.

| Tool | What it does | Documented in |
|---|---|---|
| `tools/skeleton_scan.py` | Masked-instruction-shape ("skeleton") matching against the reference farm: finds a donor whose bytes changed but whose structure didn't, which the exact-match `find_known_objects.py` cannot do (ADR 0007). Prints candidates only; never writes ROM bytes to a file. | [`docs/skeleton-scan.md`](skeleton-scan.md) |
| `tools/flag_sweep.py` | Compiles one candidate under the full known compiler-flag lattice and ranks by objdiff score, before any hand permutation is attempted (ADR 0007). | [`docs/flag-sweep.md`](flag-sweep.md) |
| `tools/overlay_graph_match.py` | Structural overlay-to-module matching against Jet Force Gemini by size, function count, and call graph, since byte identity mostly returns nothing against a differently-revised source tree. Writes `config/overlay-graph.us.json`. | [`docs/overlay-graph.md`](overlay-graph.md) |
| `tools/permute.sh` | One bounded decomp-permuter run for one function: locates its C file and target `.s` (regenerating the target from the baserom via a temporary `GLOBAL_ASM` swap if the function already has a C body), imports both, and runs `permuter.py` under a wall-clock cap. Batch-only per ADR 0007, separate from the interactive matching loop. | this file, `## decomp-permuter` above |
| `tools/new_lane.sh`, `tools/merge_lane.sh` | Create and integrate an isolated lane worktree. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Lane helpers` |
| `tools/fix_stale_externs.py`, `tools/refresh_atlas_digest.py`, `tools/resolve_modules_split.py` | Post-merge/integration housekeeping: stale `func_<VRAM>` externs, a stale atlas digest, and the `docs/modules.md`/`docs/overlays.md` split conflict. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Integration housekeeping` and `## docs/modules.md / docs/overlays.md split` |
| `tools/postprocess_audit.py` | Classifies every object's `POSTPROCESS` build step as `altered` (forbidden, ADR 0002) or `metadata` (permitted); the mechanical check behind the scoreboard's decompiled line. | [`docs/CONTRIBUTING.md`](CONTRIBUTING.md) `## Auditing post-compile steps` |
