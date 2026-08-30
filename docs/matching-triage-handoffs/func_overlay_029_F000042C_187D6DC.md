<!-- plateau-handoff:func_overlay_029_F000042C_187D6DC:start -->
### `func_overlay_029_F000042C_187D6DC` plateau handoff

- source: `src/overlays/o029/overlay29InitializeObject.c`
- score: 87/102 words
- frame: 0x58
- relocations: 8
- first mismatch: +0x14
- summary: All eight identities align; early volatile init/source-contact load ordering remains after bounded producer, vector, and volatility probes.
- measured shape: target and retained C are both 102 words with frame 0x58; 15 raw and relocation-masked words differ, with mixed structure, schedule, and register effects beginning at the first source-object load.
- relocation evidence: all 8 candidate and target sites agree in function-relative offset, type, and stable runtime identity. Three old resident callee spellings omitted the resident base adjustment; the retained candidate now calls `mathOneFloatRPY`, `trackMakePolylist`, and `func_80010900`.
- bounded attempts: seven current probes covered two source/contact assignment placements, canonical callee identities, exact-sibling vector ordering, aggregate copying, a split source-object producer, and pointer volatility. Regressions ranged from 18 to 93 differing words; aggregate copying exceeded the 0x198 owner by 8 bytes. The prior plateau also exhausted broader typed/load/declaration variants and a bounded permuter run.
- donor evidence: Mickey's exact Overlay 26 initializer corroborates the transform, polylist, and collision-call sequence but its natural grouped-vector ordering regresses this target to 18 differences. The five-reference skeleton search found no JFG candidate in the top ten; the best cross-project score was only 0.076 (Conker), so there is no donor claim.
- JFG utility: no public-ledger-grade JFG function was uncovered. The corrected calls only corroborate JFG's existing `src/hasm/ido/math_util.s::mathOneFloatRPY` and `ver/symbols/symbol_addrs.us.txt::trackMakePolylist` in the track/collision setup pipeline; this Mickey body remains non-matching and has no identified JFG insertion point.
- next lever: retain the volatile field and inspect CFE/ugen statement boundaries for a producer shape that issues `init->object` and its contact load before the position stores without extending the source pointer's lifetime or adding a stack home.
<!-- plateau-handoff:func_overlay_029_F000042C_187D6DC:end -->
