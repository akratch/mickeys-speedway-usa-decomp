<!-- plateau-handoff:func_overlay_022_F0000A7C_1878B84:start -->
### `func_overlay_022_F0000A7C_1878B84` plateau handoff

- source: `src/overlays/o022/overlay22ResolvePlane.c`
- score: 170/173 words
- frame: 0x88
- relocations: 3
- first mismatch: +0x74
- summary: Three commutative operand-order encodings remain; lifetime reuse fixed both call-stack homes; all 3/3 relocation identities authenticate.

Evidence and blocker:

- The candidate and target are both 692 bytes (173 words), with 170 words exact and an exact `0x88` frame.
- The remaining differences are three commutative multiply operand-order encodings; the first is at `+0x74`.
- Reusing `crossZ` as the reflection scale across mutually exclusive branches moved both call-stack homes to their target offsets without changing the frame.
- Static relocation authentication resolves all three identities: the overlay-local data pair binds to overlay 22 offset `+0xEF0`, and the resident call binds to offset `+0x6EC00` (`sqrtf`).
- Ten coherent attempts were bounded and checkpointed. Direct operand reversals canonicalized to the same output, while compound-expression spellings escaped the owned function boundary. The 12/173 regression was rejected and is not preserved.

Next action: seek a source-level lifetime or scheduling spelling that reverses only the three commutative operand encodings without introducing assignment temporaries; re-authenticate function ownership and all relocation identities after every experiment.
<!-- plateau-handoff:func_overlay_022_F0000A7C_1878B84:end -->
