# Mickey's Speedway USA

## Clean Room

- No ROMs or extracted assets are committed to this repository.
- Bring your own legally dumped, SHA1-verified ROM.
- See [`docs/CLEANROOM.md`](docs/CLEANROOM.md) for the full policy.

A repository exploring a decompilation of Mickey's Speedway USA.

This is super minimal and likely won't become a full decomp project. It is just being used to explore the Diddy Kong Racing engine as it evolved on the N64.

Grab tools

```sh
git submodule update --init --recursive
```

Install Dependencies
```sh
sudo apt install build-essential pkg-config git python3 wget libcapstone-dev python3-pip binutils-mips-linux-gnu
python3 -m pip install --user colorama watchdog levenshtein cxxfilt
python3 -m pip install --upgrade pycparser
```

Drop in `us` as `baserom.us.z64` (sha1sum: `507341c0a40ca3e9a7cee969b396ee53facfb548`)

```sh
make dependencies
make extract
make
```

