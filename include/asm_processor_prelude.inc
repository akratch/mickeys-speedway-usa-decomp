# Extra assembly macros for asm-processor's GLOBAL_ASM path.
#
# asm-processor assembles each `#pragma GLOBAL_ASM` block by concatenating its
# own tools/asm-processor/prelude.inc with the .s file, and that prelude only
# defines `glabel` -- but splat's nonmatchings output (asm/nonmatchings/**.s)
# also uses `endlabel` and the other label macros, and unlike the whole-file
# asm/*.s output it deliberately carries no `.include "macro.inc"` line of its
# own (asm-processor is supposed to be the one supplying the prelude).
#
# Rather than patch the asm-processor submodule or weaken splat's macro
# options, the Makefile passes this file to the assembler as an additional
# input ahead of asm-processor's temporary .s -- gas assembles several input
# files as one stream, so the macros are in scope by the time the generated
# code uses them.
#
# Deliberately does NOT define `glabel`: asm-processor's own prelude defines
# that one, and redefining a macro in gas is at best noisy. The definitions
# below mirror include/macro.inc, which serves the same purpose for the
# hand-assembled asm/*.s files.

.ifndef _ASM_PROCESSOR_PRELUDE_GUARD
.internal _ASM_PROCESSOR_PRELUDE_GUARD
.set _ASM_PROCESSOR_PRELUDE_GUARD, 1

# The end of a function symbol.
.macro endlabel label
    .size \label, . - \label
.endm

# An alternative entry to a function.
.macro alabel label, visibility=global
    .\visibility \label
    .type \label, @function
    \label:
.endm

# A label referenced by an error handler table.
.macro ehlabel label, visibility=global
    .\visibility \label
    \label:
.endm

# A label referenced by a jumptable.
.macro jlabel label, visibility=global
    .\visibility \label
    \label:
.endm

# A data symbol.
.macro dlabel label, visibility=global
    .\visibility \label
    .type \label, @object
    \label:
.endm

# End of a data symbol.
.macro enddlabel label
    .size \label, . - \label
.endm

.endif
