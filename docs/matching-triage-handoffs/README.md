# Per-symbol plateau handoffs

Each generated Markdown file in this directory records the bounded plateau
for exactly one symbol. The filename is the exact C symbol, and the embedded
`source` field identifies its owning translation unit.

Create or refresh a shard with `tools/finalize_plateau.py`; do not add a
shared generated index. The fixed one-symbol paths let parallel lanes commit
different plateau handoffs without editing the same ledger file. Historical
records remain in [`../matching-triage.md`](../matching-triage.md), which is
still supported as an explicit `--handoff-doc` ledger.

`tools/plateau_handoff_audit.py --check` compares the structured markers in
tracked C sources with these shards. Its explicit `--write` mode atomically
writes only missing or stale exact-symbol shards from validated marker fields;
it preserves valid detail lines and never extracts metrics from prose-only
comments. Use `--json` for the deterministic machine-readable report. Review
generated shard changes separately from tooling changes.
