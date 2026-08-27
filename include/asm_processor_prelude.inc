# Assembly prelude for asm-processor's GLOBAL_ASM path.
#
# Splat's nonmatchings output uses this label vocabulary but, unlike the
# whole-file asm/*.s output, deliberately carries no `.include "macro.inc"`
# line. The Makefile passes this file through asm-processor's --asm-prelude
# option so the project does not depend on the submodule's default macro set.
#
# The definitions mirror include/macro.inc, while the register aliases retain
# asm-processor's original public prelude behavior.

.set noat
.set noreorder
.set gp=64

.ifndef _ASM_PROCESSOR_PRELUDE_GUARD
.internal _ASM_PROCESSOR_PRELUDE_GUARD
.set _ASM_PROCESSOR_PRELUDE_GUARD, 1

# A global symbol.
.macro glabel label
    .global \label
    \label:
.endm

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

# Float register aliases (o32 ABI, odd ones are rarely used).
.set $fv0,          $f0
.set $fv0f,         $f1
.set $fv1,          $f2
.set $fv1f,         $f3
.set $ft0,          $f4
.set $ft0f,         $f5
.set $ft1,          $f6
.set $ft1f,         $f7
.set $ft2,          $f8
.set $ft2f,         $f9
.set $ft3,          $f10
.set $ft3f,         $f11
.set $fa0,          $f12
.set $fa0f,         $f13
.set $fa1,          $f14
.set $fa1f,         $f15
.set $ft4,          $f16
.set $ft4f,         $f17
.set $ft5,          $f18
.set $ft5f,         $f19
.set $fs0,          $f20
.set $fs0f,         $f21
.set $fs1,          $f22
.set $fs1f,         $f23
.set $fs2,          $f24
.set $fs2f,         $f25
.set $fs3,          $f26
.set $fs3f,         $f27
.set $fs4,          $f28
.set $fs4f,         $f29
.set $fs5,          $f30
.set $fs5f,         $f31
