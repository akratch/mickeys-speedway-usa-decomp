#!/usr/bin/env python3
"""Digest the part of a reference build that mining actually reads.

tools/verify_reference_builds.sh hands this a checkout and its object roots;
it prints one `<relative path> <sha256>` line per object and, last, the
aggregate sha256 over those lines that tools/reference-builds.lock records.

Why not just hash the `.o` files.  IDO writes an `.mdebug` section carrying the
absolute source path, the build host's name and a build timestamp, so two
builds of the same commit from different directories -- or the same directory
on a different day -- produce different object files while producing a
byte-identical ROM.  Hashing whole objects would therefore report every honest
rebuild as a mismatch, which is a checksum that only ever cries wolf.

So this hashes the object's *mining surface* instead: the bytes of `.text`, and
the name, value and size of every symbol that lives in `.text`.  That is
precisely and only what tools/find_known_objects.py compares against the ROM,
it is stable across rebuilds, and it still changes if the compiler, the flags
or the source change.  A rebuild that matches here is a rebuild that mines to
the same names.

Stdlib only, and it parses ELF directly rather than shelling out to objcopy and
nm: this has to run wherever a farm is, without a MIPS cross-toolchain.
"""

import hashlib
import os
import struct
import sys

SHT_SYMTAB = 2
SHT_NOBITS = 8


class NotAnObject(Exception):
    pass


def _sections(blob):
    """Yield (name, type, offset, size, link, entsize) for each ELF section."""
    if len(blob) < 64 or blob[:4] != b"\x7fELF":
        raise NotAnObject("not an ELF file")
    is64 = blob[4] == 2
    end = ">" if blob[5] == 2 else "<"
    if is64:
        shoff, shentsize, shnum, shstrndx = (
            struct.unpack_from(end + "Q", blob, 0x28)[0],
            struct.unpack_from(end + "H", blob, 0x3A)[0],
            struct.unpack_from(end + "H", blob, 0x3C)[0],
            struct.unpack_from(end + "H", blob, 0x3E)[0],
        )
    else:
        shoff, shentsize, shnum, shstrndx = (
            struct.unpack_from(end + "I", blob, 0x20)[0],
            struct.unpack_from(end + "H", blob, 0x2E)[0],
            struct.unpack_from(end + "H", blob, 0x30)[0],
            struct.unpack_from(end + "H", blob, 0x32)[0],
        )
    if not shoff or not shnum:
        raise NotAnObject("no section headers")

    raw = []
    for i in range(shnum):
        base = shoff + i * shentsize
        if is64:
            nameoff, stype = struct.unpack_from(end + "II", blob, base)
            off, size = struct.unpack_from(end + "QQ", blob, base + 0x18)
            link = struct.unpack_from(end + "I", blob, base + 0x28)[0]
            entsize = struct.unpack_from(end + "Q", blob, base + 0x38)[0]
        else:
            nameoff, stype = struct.unpack_from(end + "II", blob, base)
            off, size = struct.unpack_from(end + "II", blob, base + 0x10)
            link = struct.unpack_from(end + "I", blob, base + 0x18)[0]
            entsize = struct.unpack_from(end + "I", blob, base + 0x24)[0]
        raw.append((nameoff, stype, off, size, link, entsize))

    stroff, strsize = raw[shstrndx][2], raw[shstrndx][3]
    strtab = blob[stroff:stroff + strsize]

    def name_at(offset):
        stop = strtab.find(b"\0", offset)
        return strtab[offset:stop if stop >= 0 else None].decode("ascii", "replace")

    return end, is64, [
        (name_at(nameoff), stype, off, size, link, entsize)
        for nameoff, stype, off, size, link, entsize in raw
    ]


def mining_surface(path):
    """Return the bytes mining reads out of one object: `.text` and its symbols.

    Missing or empty `.text` is not an error -- data-only objects are normal,
    and docs/references.md explains why the mining pass skips them -- but it is
    still hashed, so an object that loses its `.text` in a rebuild is a
    mismatch rather than a silent absence.
    """
    with open(path, "rb") as fh:
        blob = fh.read()
    end, is64, sections = _sections(blob)

    text_index = None
    text = b""
    for i, (name, stype, off, size, _link, _entsize) in enumerate(sections):
        if name == ".text":
            text_index = i
            if stype != SHT_NOBITS:
                text = blob[off:off + size]
            break

    symbols = []
    for name, stype, off, size, link, entsize in sections:
        if stype != SHT_SYMTAB or not entsize:
            continue
        stroff, strsize = sections[link][2], sections[link][3]
        strtab = blob[stroff:stroff + strsize]
        for base in range(off, off + size, entsize):
            if is64:
                nameoff, shndx = (
                    struct.unpack_from(end + "I", blob, base)[0],
                    struct.unpack_from(end + "H", blob, base + 6)[0],
                )
                value, sym_size = struct.unpack_from(end + "QQ", blob, base + 8)
            else:
                nameoff, value, sym_size, shndx = (
                    struct.unpack_from(end + "I", blob, base)[0],
                    struct.unpack_from(end + "I", blob, base + 4)[0],
                    struct.unpack_from(end + "I", blob, base + 8)[0],
                    struct.unpack_from(end + "H", blob, base + 14)[0],
                )
            if shndx != text_index:
                continue
            stop = strtab.find(b"\0", nameoff)
            sym = strtab[nameoff:stop if stop >= 0 else None]
            symbols.append((value, sym_size, sym))

    digest = hashlib.sha256()
    digest.update(b"text %d\n" % len(text))
    digest.update(text)
    for value, sym_size, sym in sorted(symbols):
        digest.update(b"sym %d %d %s\n" % (value, sym_size, sym))
    return digest.hexdigest()


def main(argv):
    per_object = "--per-object" in argv[1:]
    argv = [a for a in argv if a != "--per-object"]
    if len(argv) < 3:
        sys.stderr.write(
            "usage: reference_build_digest.py [--per-object] <checkout> <root>...\n")
        return 2
    checkout, roots = argv[1], argv[2:]

    paths = []
    for root in roots:
        top = os.path.join(checkout, root)
        if not os.path.isdir(top):
            sys.stderr.write("reference-digest: no such object root: %s\n" % top)
            return 2
        for dirpath, dirnames, filenames in os.walk(top):
            dirnames.sort()
            for filename in sorted(filenames):
                if filename.endswith(".o"):
                    full = os.path.join(dirpath, filename)
                    paths.append(os.path.relpath(full, checkout))
    if not paths:
        sys.stderr.write("reference-digest: no objects under %s\n" % " ".join(roots))
        return 2

    aggregate = hashlib.sha256()
    for path in sorted(paths):
        try:
            one = mining_surface(os.path.join(checkout, path))
        except (NotAnObject, struct.error, IndexError) as exc:
            sys.stderr.write("reference-digest: %s: %s\n" % (path, exc))
            return 2
        line = "%s %s\n" % (path, one)
        aggregate.update(line.encode("utf-8"))
        if per_object:
            sys.stdout.write(line)
    print("%s %d" % (aggregate.hexdigest(), len(paths)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
