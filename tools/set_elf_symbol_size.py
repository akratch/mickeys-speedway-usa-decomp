#!/usr/bin/env python3
"""Set one asserted symbol size in a big-endian ELF32 object."""
import pathlib
import struct
import sys

if len(sys.argv) != 5:
    raise SystemExit("usage: set_symbol_size.py OBJECT SYMBOL OLD_SIZE NEW_SIZE")
path = pathlib.Path(sys.argv[1])
name_wanted = sys.argv[2]
old_size = int(sys.argv[3], 0)
new_size = int(sys.argv[4], 0)
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit("expected big-endian ELF32")
shoff = struct.unpack_from(">I", data, 0x20)[0]
shentsize = struct.unpack_from(">H", data, 0x2E)[0]
shnum = struct.unpack_from(">H", data, 0x30)[0]
headers = [struct.unpack_from(">10I", data, shoff + i * shentsize) for i in range(shnum)]
matches = 0
for header in headers:
    if header[1] != 2:
        continue
    strings_header = headers[header[6]]
    strings = data[strings_header[4]:strings_header[4] + strings_header[5]]
    entsize = header[9] or 16
    for offset in range(header[4], header[4] + header[5], entsize):
        name_offset, _, size = struct.unpack_from(">III", data, offset)
        end = strings.find(b"\0", name_offset)
        name = strings[name_offset:end].decode("ascii")
        if name == name_wanted:
            if size != old_size:
                raise SystemExit(f"old symbol size drift: expected {old_size:#x}, got {size:#x}")
            struct.pack_into(">I", data, offset + 8, new_size)
            matches += 1
if matches != 1:
    raise SystemExit(f"expected one matching symbol, found {matches}")
path.write_bytes(data)
