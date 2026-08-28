#!/usr/bin/env python3
"""Reduce one big-endian ELF32 section to a proved byte size.

IDO pads each translation unit's .text to a 16-byte object boundary. Overlay
functions can end on a four-byte boundary inside a larger runtime module, so
that object-only padding must not displace the following subsegment. Refuse to
discard anything but zero bytes unless the caller supplies the complete exact
discarded payload as hex. That explicit form is reserved for reviewed compiler-
only material moved past the function symbol by a guarded normalization.
"""

import pathlib
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import postprocess_guard as guard


if len(sys.argv) not in (4, 5):
    raise SystemExit(
        "usage: trim_elf_section.py OBJECT SECTION NEW_SIZE "
        "[EXPECTED_DISCARDED_HEX]"
    )

path = pathlib.Path(sys.argv[1])
section_name = sys.argv[2]
new_size = int(sys.argv[3], 0)
expected_discarded = bytes.fromhex(sys.argv[4]) if len(sys.argv) == 5 else None
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit(f"{path}: expected a big-endian ELF32 object")

section_header_offset = struct.unpack_from(">I", data, 0x20)[0]
section_header_size = struct.unpack_from(">H", data, 0x2E)[0]
section_count = struct.unpack_from(">H", data, 0x30)[0]
string_table_index = struct.unpack_from(">H", data, 0x32)[0]

def header(index):
    offset = section_header_offset + index * section_header_size
    return offset, struct.unpack_from(">10I", data, offset)

_, string_header = header(string_table_index)
string_offset, string_size = string_header[4], string_header[5]
strings = data[string_offset : string_offset + string_size]

for index in range(section_count):
    header_offset, values = header(index)
    name_offset, file_offset, old_size = values[0], values[4], values[5]
    end = strings.find(b"\0", name_offset)
    name = strings[name_offset:end].decode("ascii")
    if name != section_name:
        continue
    # The requested size is the ownership row's extent: what the module says
    # this translation unit occupies. A candidate whose codegen is a different
    # size therefore trips one of the three guards below, and the delta is the
    # measurement the promotion trial wants -- not a build failure with no
    # number in it. `text-size-differs (+N)` means the candidate's section is N
    # bytes longer than the module owns; negative, N bytes shorter.
    delta = old_size - new_size
    if new_size > old_size:
        guard.fail(
            f"{path}: cannot grow {name} from {old_size:#x} to {new_size:#x}",
            kind=f"{name.lstrip('.')}-size-differs ({delta:+d} bytes)",
        )
    discarded = data[file_offset + new_size : file_offset + old_size]
    if expected_discarded is not None and discarded != expected_discarded:
        guard.fail(
            f"{path}: discarded {name} payload changed: expected "
            f"{expected_discarded.hex()}, got {discarded.hex()}",
            kind=f"{name.lstrip('.')}-payload-differs ({delta:+d} bytes)",
        )
    if expected_discarded is None and any(discarded):
        guard.fail(
            f"{path}: refusing to trim nonzero bytes from {name}",
            kind=f"{name.lstrip('.')}-size-differs ({delta:+d} bytes)",
        )
    struct.pack_into(">I", data, header_offset + 0x14, new_size)
    path.write_bytes(data)
    break
else:
    guard.fail(f"{path}: section {section_name!r} not found",
               kind="section-missing")
