<!-- plateau-handoff:overlay58FinalizePackedStatus:start -->
### `overlay58FinalizePackedStatus` plateau handoff

- source: `src/overlays/o058/overlay58FinalizePackedStatus.c`
- score: 127/304 words
- frame: 0x48
- relocations: 48
- first mismatch: +0x18
- summary: Exact geometry remains blocked by mixed control-flow and register allocation after the full flag lattice.
- assignment base: `ccbd4a78b29afb17ad817dd9228f774012b7d9ac`
- owned range: overlay 58 `+0x5554..+0x5A14`, 1,216 bytes / 304 words; the following `+0x5A14..+0x5A20` range is separately owned padding
- baseline: exact 304-word geometry and `0x48` frame, with 178 raw differences, 177 relocation-masked differences, first raw mismatch `+0x8`, and first masked mismatch `+0x18`
- retained score: 127/304 relocation-masked words match; evaluating the desired-rank condition before the loop bound lowers the workbench normalized structural distance from 177 to 176 without changing size, frame, calls, or semantics
- relocation proof: target and candidate each emit 48 records; 42 offsets/types align, but all 48 candidate identities remain unresolved and zero stable identities align, so `function_preflight.py` correctly returns partial and fails promotion closed
- mismatch proof: the retained aligned view classifies 125 register, 62 structural, seven schedule, and nine constant sites; the residual is broad rather than a near-exact allocator-only tail
- attempts: ten bounded source hypotheses covered loop-condition order, explicit pointer/rank carriers, extended-mode nesting, declaration order, captured-player reuse, stack-home ordering, next-count lifetime, and `for`/`do` loop forms; the complete 119-row flag lattice found no exact result and tied canonical `-O2 -mips2 -32` for best at 177 masked differences
- retained improvement: preserve the desired-first first-loop condition; every other source form regressed geometry, frame, or difference count, and no generic permuter was run
- next action: reopen only with new evidence for the stack-home/declaration web and the mask/shift carrier allocation, together with authenticated candidate relocation identities; do not repeat these ten forms or the completed flag lattice
- JFG: no credible insertion point was found. The pinned JFG overlay donor row is `none`; the nearest JFG skeleton, `func_overlay_3_00304968_1ED9E48`, has only 0.032 similarity and supplies neither a source analogue nor reusable code
<!-- plateau-handoff:overlay58FinalizePackedStatus:end -->
