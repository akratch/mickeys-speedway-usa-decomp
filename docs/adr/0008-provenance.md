# 0008. Source provenance

Status: Accepted
Date: 2026-08-24

## Context

The project uses published retail-ROM decompilations to identify shared code
and compiler behavior. It needs a clear rule for borrowed library source,
adapted game code, and public projects based on development builds.

## Decision

Official Nintendo 64 library source distributed by the permitted public
decompilation projects may be used. Adapted bodies and borrowed symbol blocks
require a point-of-use `PROVENANCE` note naming their project and file.

The public *Dinosaur Planet* decompilation is based on a development-cartridge
dump, not a retail release. It may be consulted only for the binary overlay
relocation format. Its source, comments, and names remain prohibited. Symbols
from the *Star Fox Adventures* debug build also remain prohibited.

Local comparison services may be used if they receive only material the
contributor is allowed to share. Local tools are preferred when they provide
the same result.

## Consequences

The `n_audio` library and other permitted SDK sources can be reconstructed with
clear provenance. Mickey's bytes remain authoritative. Expanding the permitted
source list requires an explicit clean-room policy change.
