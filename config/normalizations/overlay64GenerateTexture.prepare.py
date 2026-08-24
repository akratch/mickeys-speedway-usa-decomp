#!/usr/bin/env python3
"""Lower O64's four redundant forms and ten complete representation family.

This stage is target-independent.  It accepts only the immutable natural
compiler stream, removes the two separate LO16 address-forming ADDIUs and two
rematerialized ``li 2`` values, and establishes the target opcode families for
the complete ten-instruction representation census.  Scheduling, registers,
and immediates remain owned by ``normalization.ops``.
"""

import hashlib
import pathlib
import struct
import sys

NATURAL_SIZE = 0x6A0
OUTPUT_SIZE = 0x690
NATURAL_SHA = "a860bf8275e0a0cd07ef9e98b5ed41068de79e76b4512685fd855ae96294dd3d"
DROP_OFFSETS = (0x01C, 0x1A4, 0x1B0, 0x294)


def fail(message):
    raise SystemExit(message)


if len(sys.argv) != 3:
    fail("usage: prepare_o64_intermediate.py NATURAL_TEXT.bin OUTPUT.bin")

input_path = pathlib.Path(sys.argv[1])
output_path = pathlib.Path(sys.argv[2])
data = input_path.read_bytes()
if len(data) != NATURAL_SIZE:
    fail(f"natural size drift: expected {NATURAL_SIZE:#x}, got {len(data):#x}")
if hashlib.sha256(data).hexdigest() != NATURAL_SHA:
    fail("natural compiler stream digest drift")

words = list(struct.unpack(">424I", data))
kept = [word for index, word in enumerate(words) if index * 4 not in DROP_OFFSETS]
if len(kept) != 420:
    fail("drop census did not yield exactly 420 instructions")

# Post-drop sites and the asserted old/new opcode representation.  The whole
# natural digest above guards all other decoded fields.
conversions = (
    (0x088, 5, 21, "bnez to bnezl"),
    (0x0AC, 0, 0, "move to sll"),
    (0x0C0, 0, 0, "move to sll"),
    (0x0E4, 0, 0, "move to sll"),
    (0x0EC, 40, 10, "sb to slti"),
    (0x124, 0, 15, "move to lui"),
    (0x17C, 0, 0, "move to nop"),
    (0x204, 4, 4, "beq to beqz"),
    (0x254, 1, 1, "bgezl to bgez"),
    (0x2A4, 4, 4, "beq to beqz"),
)
for offset, old_opcode, new_opcode, description in conversions:
    index = offset // 4
    word = kept[index]
    if word >> 26 != old_opcode:
        fail(f"{description} opcode drift at {offset:#x}")
    word = (word & 0x03FFFFFF) | (new_opcode << 26)
    if description.startswith("move to sll") or description == "move to nop":
        if (kept[index] & 0x3F) != 0x25:
            fail(f"{description} function drift at {offset:#x}")
        word &= ~0x3F
    if description == "bgezl to bgez":
        if ((kept[index] >> 16) & 0x1F) != 3:
            fail(f"{description} rt drift at {offset:#x}")
        word = (word & ~(0x1F << 16)) | (1 << 16)
    kept[index] = word

output = struct.pack(">420I", *kept)
if len(output) != OUTPUT_SIZE:
    fail("intermediate size drift")
output_path.write_bytes(output)
print(hashlib.sha256(output).hexdigest())
