# 0006. Overlay source layout

Status: Accepted
Date: 2026-08-24

## Context

`docs/acceleration-survey.md` §9 measured the current overlay layout: 747
files, one per function, each with its own private struct typedefs (e.g.
`Overlay1OwnerState { u8 pad000[0x37C]; s16 recordIndex; u8 selector; }`),
its own externs, and in 107 cases a second extern name for the same address
(`D_1DA0Read`) or a `volatile` cast purely to force a reload — workarounds
for the fact that no shared header exists to declare the real type once.
447 files carry pad-structs, 59 use `register`. Only nine shared headers
exist under `include/overlays/`. Because the atlas records text ownership
only, the overlay's 61,312 initialised and 77,680 BSS bytes have no owning
object at all, and the relocation model needs filter/rebind spec files
because addends can't otherwise be expressed against a per-function object.

§9 also measured where consolidation pays: overlays 50, 52, and 54 share
about 40% of their code shapes with each other (a shared record/offset
family), and overlay 101's unmatched remainder shares 26% with code already
matched elsewhere.

§13.5 confirmed the target layout by surveying the reference projects: DKR
is one `.c` per original translation unit; JFG's overlays are one
`src/overlays/oN/overlay_N.c` each over shared
`include/{common,functions,structs,variables}.h`; dp64 is one directory per
DLL with a shared `dll.h`. There is no per-function-file layout in any of
them.

## Decision

Adopt JFG's layout: **one translation unit per overlay**,
`src/overlays/oNNN/overlay_NNN.c`, over shared headers. Per-overlay TUs own
their data, rodata, and BSS, not just text — the atlas's `(overlay,
section, offset)` identity becomes the ordinary splat subsegment list for
that TU rather than a text-only record layered on top of per-function
files.

The per-function-TU workarounds are consolidated away as each overlay is
folded: private per-file struct typedefs collapse into shared structs in
the overlay's header (or `include/overlays/` where genuinely shared across
overlays); `volatile` reload hacks and `D_xxxxRead`-style duplicate extern
names are removed once the real type is declared once, correctly, in one
place.

Do this for each overlay once it has more than about 5 matched functions —
below that the per-function scaffolding isn't yet costing anything. Overlay
50, 52, 54, and 101 go first, per the n-gram measurement above. Build the
consolidated overlay as a relocatable object through a JFG/dp64-style
`elf2dll`-equivalent step (structural/format tooling, permitted under
`docs/CLEANROOM.md`), and compare it with objdiff per section.

## Consequences

- Relocation filter/rebind spec files and section-trim steps, which ADR
  0002 tolerates only as scaffolding for the per-function-TU model, are
  retired overlay-by-overlay as each one consolidates. Once every overlay
  above the threshold has moved, that scaffolding has no remaining
  consumer and can be removed outright.
- `config/overlays.us.json` and the splat yaml gain real data/rodata/BSS
  subsegment ownership per overlay, closing the gap where 139,000+ bytes
  currently have no owning object.
- This is consolidation of existing matched/attempted work, not a
  re-matching effort: the underlying C bodies and their byte-identity
  status (ADR 0001) don't change, only which file and which shared
  declarations they live in.
- Per-function-file layout is not banned outright for a brand-new overlay's
  very first function — it's still the fastest way to bank a first match —
  but it is expected to be temporary scaffolding retired at the ~5-function
  mark, not the permanent shape.
