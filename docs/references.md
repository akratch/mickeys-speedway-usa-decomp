# Reference builds

Level-A names come from relocation-aware comparisons between Mickey's ROM and
objects built by published decompilation projects. The reference checkouts,
ROMs, and objects are not part of this repository.

`tools/reference-builds.lock` records each repository, commit, ROM checksum,
build result, object count, and aggregate object digest. The digest covers each
object's `.text` bytes and text-symbol records. It excludes debug sections,
which contain paths, host names, and timestamps.

## Terms

- **Adopted**: a whole-object match added a named translation unit and symbol
  block to this repository.
- **Re-confirmed**: a match agreed with a translation unit already identified
  from another reference. This count comes from the original scan log and
  cannot be recomputed from this tree.
- **Corroborated**: a translation unit adopted from one title also matched an
  object from another title. This is recorded in `symbol_addrs.us.txt` and is
  recomputed by `gmake check-docs`.

Do not combine these three counts.

## Summary

| Title | Commit | ROM checksum checked | Build result | Objects | Objects scanned | Adopted TUs | Names adopted | Re-confirmed | Corroborations |
|---|---|---|---|---:|---:|---:|---:|---:|---:|
| Diddy Kong Racing | `38d7f9ba` | yes | full US v77 match | 243 | 223 | — | — | — | — |
| Jet Force Gemini | `c82affff` | yes | full match | 772 | 391 | **84** | **192** | 84 | — |
| Perfect Dark | `169ed48b` | MD5 from upstream; local SHA-1 recorded | code links; compressed assets differ | 2546 | 467 | **3** | **3** | 52 | 25 |
| Banjo-Kazooie | `6eaae281` | yes | full match | 1232 | 1105 | **0** | **0** | 73 | 2 |
| Conker's Bad Fur Day | `3adf2291` | yes | all source objects build; final link incomplete | 1446 | 705 | **0** | **0** | 65 | 8 |
| **Totals for adopted pass** | | | | | **2891** | **87** | **195** | — | **26 TUs** |

**26 of the 87 adopted translation units were matched by more than one title**.
Each adopted unit is credited once, to the object named in its symbol block.

## Diddy Kong Racing

- Repository: <https://github.com/akratch/Diddy-Kong-Racing.git>, based on the
  published <https://github.com/DavidSM64/Diddy-Kong-Racing> project
- Commit: `38d7f9ba39642e2b5311a76e0b83fb3fe2733262`
- Version: US v77
- ROM SHA-1: `0cb115d8716dbbc2922fda38e533b9fe63bb9670`
- Build: complete byte match
- Object count: 243
- Object digest:
  `917ba733782e07382dd753b50b496c9f8647caec8695f7bca0359a19f0cd763b`

DKR supplied the initial libultra and shared game-code map. Overlay scans use
both US v77 and a secondary US v80 build. Exact overlay reuse is limited, but
DKR remains the first structural reference for shared game systems.

## Jet Force Gemini

- Repository: <https://github.com/Ryan-Myers/Jet-Force-Gemini>
- Commit: `c82affffe8f11cb5b440cfa918f4582ad8573279`
- ROM SHA-1: `493ced9008dbe932d6e91179b68e8630cf23a023`
- Build: complete byte match
- Object count: 772
- Object digest:
  `8c4036b7e2404e989e010ba899331a1dfe972b0d570e8c9535d7fde53788585d`

JFG supplied 84 adopted translation units and 192 names. These include
libultra, `n_audio`, the runtime linker, and several shared engine units. JFG
objects that expose only generated placeholder names do not supply adopted
names.

## Perfect Dark

- Repository: <https://github.com/n64decomp/perfect_dark>
- Commit: `169ed48bdcbfb3b568b028bd5bebb27680073514`
- ROM MD5 published upstream: `e03b088b6ac9e0080440efed07c1e40f`
- ROM SHA-1 recorded for this build:
  `af8788ac4d1a57260eae9c53ffe851fcf2a3319b`
- Build: code links; some host-compressed asset bytes differ
- Object count: 2546
- Object digest:
  `7314cac7eb7e75186963df28d7e3aecd7df95ebf837f5eb05381b1c246b3ca22`

Perfect Dark supplied three Transfer Pak translation units and three names. It
also **corroborates 25** of the 87 adopted translation units and **re-confirms
52** subsegments.

## Banjo-Kazooie

- Repository: <https://github.com/n64decomp/banjo-kazooie>
- Commit: `6eaae281481c9e4b367dc161faabfc3c79fe8733`
- ROM SHA-1: `1fe1632098865f639e22c11b9a81ee8f29c75d7a`
- Build: complete byte match
- Object count: 1232
- Object digest:
  `da2fb7fe970a1627cc6059d1eff3f22a51b3142e585b619a6ac440da67f98e89`

Banjo-Kazooie supplied no new adopted names. It **corroborates 2** of the 87
adopted translation units and **re-confirms 73** subsegments.

## Conker's Bad Fur Day

- Repository: <https://github.com/mkst/conker>
- Commit: `3adf229175c037c771f251f169f9dd80ca306924`
- ROM SHA-1: `4cbadd3c4e0729dec46af64ad018050eada4f47a`
- Build: all source files compile; the final link is incomplete
- Object count: 1446
- Object digest:
  `1a49d7ddc080789752d493f447ab300c7d12c55e77005b41f751a08ee4f61bc6`

Conker supplied no new adopted names. It **corroborates 8** of the 87 adopted
translation units and **re-confirms 65** subsegments.

## Rebuild and check

Provide the required reference ROMs outside this repository, then run:

```sh
gmake reference-builds
gmake reference-builds REFS_ARGS=jfg
gmake check-reference-builds
```

`tools/setup_reference_builds.sh` checks each ROM before building and applies
the documented host fixes. `tools/verify_reference_builds.sh` compares the
resulting object digest with the lock file.

For direct searches:

```sh
tools/find_known_objects.py <object-root> --start 0x1000 --end 0x86640 \
    --sections --rom-occ
tools/find_known_objects.py <object-root> --start 0x1000 --end 0x86640 \
    --rom-occ
gmake overlay-donors-scan-check
```

The first command finds whole-object boundaries. The second finds individual
functions. `--rom-occ` checks uniqueness across the whole Mickey ROM.

Empty `.text` objects are excluded. Several reference builds contain valid
data-only objects, and a failed preprocessing pipe can also leave an empty ELF
object. Neither can contribute code evidence.

A matching digest proves that the local objects expose the same comparison
data as the locked build. It does not prove that a partial upstream project has
a complete final link, and it does not replace Mickey-side boundary,
relocation, and ROM checks.
