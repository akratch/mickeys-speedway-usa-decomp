"""asm-differ configuration.

`./diff.sh <symbol>` disassembles <symbol> out of the baserom and out of the
freshly built ROM and shows the two side by side; a matched function shows no
differing lines. Useful flags: -m (rebuild first), -o (compare objects rather
than the whole ROM), -s (interleave C source), and -3 (three-way against
expected/). Do not use -w watch mode on the occupied workstation.
"""


def apply(config, args):
    config["baseimg"] = "baseroms/mickey.us.z64"
    config["myimg"] = "build/mickey.us.z64"
    config["mapfile"] = "build/mickey.us.map"
    config["source_directories"] = ["src", "include"]
    config["show_line_numbers_default"] = True
    config["expected_dir"] = "expected/"

    # The build directory the map file's paths are relative to; asm-differ needs
    # this to find build/src/**/*.o for -o (object) mode.
    config["build_dir"] = "build/"

    # Cross toolchain: the vendored binutils, not whatever `mips-linux-gnu-` the
    # host happens to have. Kept in sync with $(OBJDUMP) in the Makefile.
    config["objdump_executable"] = "tools/binutils/mips64-elf-objdump"
    config["objdump_flags"] = ["-Mreg-names=32"]

    # -m rebuilds before diffing. `gmake` (not `make`): the build relies on GNU
    # make's two-phase recursive `all` target, and macOS ships make 3.81 as
    # `make`. Keep agent-triggered rebuilds low priority and capped at two jobs
    # so an interactive workstation remains responsive.
    config["make_command"] = [
        "nice", "-n", "10", "gmake", *config.get("makeflags", []), "-j2"
    ]
