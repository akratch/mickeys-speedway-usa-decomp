#!/usr/bin/env python3
"""Set the big-endian ELF32 e_flags word on a generated object."""

import pathlib
import struct
import sys

path = pathlib.Path(sys.argv[1])
flags = int(sys.argv[2], 0)
data = bytearray(path.read_bytes())
if data[:6] != b"\x7fELF\x01\x02":
    raise SystemExit(f"{path}: expected a big-endian ELF32 object")
struct.pack_into(">I", data, 0x24, flags)
path.write_bytes(data)
