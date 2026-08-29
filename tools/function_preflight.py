#!/usr/bin/env python3
"""One fail-closed evidence preflight for a matching-decomp function.

The report contains geometry and relocation metadata, never instruction text
or ROM bytes.  It deliberately runs before a flag sweep so a worker starts
from the right symbol identity, translation unit, build mode, owned range,
call surface, and current full-TU score.

Usage:
    tools/function_preflight.py overlay16ApplyGradient
    tools/function_preflight.py func_overlay_016_F00001E0_1873678 --json
    tools/function_preflight.py overlay16ApplyGradient --no-build
"""

from __future__ import annotations

import argparse
import dataclasses
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Iterable


REPO = Path(__file__).resolve().parent.parent
TOOLS = REPO / "tools"
ROM = REPO / "baseroms" / "mickey.us.z64"
ATLAS = REPO / "config" / "overlays.us.json"
ALIASES = REPO / "overlay_undefined_syms.us.txt"
TARGET_ELF = REPO / "build" / "mickey.us.elf"
WB_COMPARE = TOOLS / "wb_compare.sh"
TYPE_NAMES = {2: "R_MIPS_32", 4: "R_MIPS_26", 5: "R_MIPS_HI16", 6: "R_MIPS_LO16"}

sys.path.insert(0, str(TOOLS))
import overlay_tables as ot  # noqa: E402
import proof_provenance as pp  # noqa: E402
import reloc_surface as rs  # noqa: E402


class PreflightError(RuntimeError):
    """The requested identity or evidence surface is not uniquely provable."""


@dataclasses.dataclass(frozen=True)
class Resolution:
    requested_symbol: str
    target_symbol: str
    candidate_symbol: str
    source: Path
    translation_unit: str
    candidate_build_dir: str
    candidate_object: Path
    target_asm: Path
    selection: str


def _relative(path: Path) -> str:
    try:
        return path.resolve().relative_to(REPO.resolve()).as_posix()
    except ValueError:
        return path.as_posix()


def _aliases(path: Path = ALIASES) -> tuple[dict[str, str], dict[str, list[str]]]:
    forward: dict[str, str] = {}
    reverse: dict[str, list[str]] = {}
    if not path.is_file():
        raise PreflightError(f"missing generated alias surface: {_relative(path)}")
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        match = re.match(
            r"^\s*(func_overlay_\d{3}_F[0-9A-Fa-f]{7}_[0-9A-Fa-f]+)\s*=\s*"
            r"([A-Za-z_]\w*)\s*;",
            line,
        )
        if not match:
            continue
        generated, friendly = match.groups()
        old = forward.setdefault(generated, friendly)
        if old != friendly:
            raise PreflightError(
                f"generated symbol {generated} has conflicting aliases {old} and {friendly}"
            )
        reverse.setdefault(friendly, []).append(generated)
    return forward, reverse


def _resolve_names(symbol: str, alias_path: Path = ALIASES) -> tuple[str, str]:
    forward, reverse = _aliases(alias_path)
    if symbol in forward:
        return symbol, forward[symbol]
    generated = reverse.get(symbol, [])
    if len(generated) > 1:
        raise PreflightError(
            f"friendly symbol {symbol} maps to {len(generated)} generated identities: "
            + ", ".join(sorted(generated))
        )
    if len(generated) == 1:
        return generated[0], symbol
    return symbol, symbol


def _unique(paths: list[Path], description: str) -> Path:
    paths = sorted({path.resolve() for path in paths})
    if len(paths) != 1:
        rendered = ", ".join(_relative(path) for path in paths) or "none"
        raise PreflightError(
            f"expected one {description}, found {len(paths)} ({rendered})"
        )
    return paths[0]


def _source_for(target_symbol: str, candidate_symbol: str, root: Path = REPO) -> tuple[Path, Path]:
    root = root.resolve()
    asm_matches = list((root / "asm" / "nonmatchings").rglob(f"{target_symbol}.s"))
    target_asm = _unique(asm_matches, f"fallback assembly for {target_symbol}")

    relative_parent = target_asm.parent.relative_to(root / "asm" / "nonmatchings")
    direct = root / "src" / relative_parent.with_suffix(".c")
    candidates: list[Path] = []
    if direct.is_file():
        candidates.append(direct)
    if not candidates:
        for source in (root / "src").rglob("*.c"):
            text = source.read_text(encoding="utf-8", errors="replace")
            candidate_facts = pp.source_facts(text, candidate_symbol)
            target_facts = pp.source_facts(text, target_symbol)
            if candidate_facts.definitions and (
                target_facts.pragmas or target_symbol == candidate_symbol
            ):
                candidates.append(source)
    return _unique(candidates, f"canonical source for {candidate_symbol}"), target_asm


def resolve(symbol: str, root: Path = REPO, alias_path: Path = ALIASES) -> Resolution:
    root = root.resolve()
    target_symbol, candidate_symbol = _resolve_names(symbol, alias_path)
    source, target_asm = _source_for(target_symbol, candidate_symbol, root)
    text = source.read_text(encoding="utf-8", errors="replace")

    ordinary, ordinary_reason = pp.classify_source_selection(
        text,
        candidate_symbol=candidate_symbol,
        target_symbol=target_symbol,
        defines=(),
    )
    nonmatching, nonmatching_reason = pp.classify_source_selection(
        text,
        candidate_symbol=candidate_symbol,
        target_symbol=target_symbol,
        defines=("NON_MATCHING",),
    )
    if ordinary == pp.ORDINARY_C:
        build_dir = "build"
        selection = ordinary_reason
    elif nonmatching == pp.NON_MATCHING_C:
        build_dir = "build_non_matching"
        selection = nonmatching_reason
    else:
        raise PreflightError(
            f"{_relative(source)} does not select one C definition for "
            f"{candidate_symbol}; ordinary={ordinary}, NON_MATCHING={nonmatching}"
        )

    rel_source = source.relative_to(root).as_posix()
    candidate_object = root / build_dir / f"{rel_source}.o"
    return Resolution(
        requested_symbol=symbol,
        target_symbol=target_symbol,
        candidate_symbol=candidate_symbol,
        source=source,
        translation_unit=rel_source.removeprefix("src/").removesuffix(".c"),
        candidate_build_dir=build_dir,
        candidate_object=candidate_object,
        target_asm=target_asm,
        selection=selection,
    )


def _run(argv: list[str], *, capture: bool = False) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        argv,
        cwd=REPO,
        check=False,
        text=True,
        stdout=subprocess.PIPE if capture else None,
        stderr=subprocess.PIPE if capture else None,
    )


def _build_logic_inputs(root: Path = REPO) -> tuple[Path, ...]:
    """Return build recipes whose edits Make cannot age against an object.

    Normal prerequisites (the source, every header, the splat inputs and the
    generated linker inputs) are covered by ``gmake -q`` below.  GNU Make does
    not rebuild a target merely because the recipe that produced it changed,
    so the root Makefile and its checked-in normalization fragments need an
    explicit timestamp check.
    """

    inputs = [root / "Makefile"]
    inputs.extend(sorted((root / "config" / "normalizations").glob("*.mk")))
    return tuple(path for path in inputs if path.is_file())


def _newer_inputs(artifact: Path, inputs: Iterable[Path]) -> tuple[Path, ...]:
    if not artifact.is_file():
        return ()
    artifact_mtime = artifact.stat().st_mtime_ns
    return tuple(
        path for path in inputs if path.is_file() and path.stat().st_mtime_ns > artifact_mtime
    )


def _make_freshness_command(target: Path, *, non_matching: bool) -> list[str]:
    command = ["gmake", "--no-print-directory", "-q"]
    if non_matching:
        command.append("NON_MATCHING=1")
    command.append(_relative(target))
    return command


def _require_fresh_target(
    target: Path,
    *,
    label: str,
    non_matching: bool,
    build_logic_inputs: Iterable[Path],
) -> None:
    if not target.is_file():
        raise PreflightError(f"missing {label}: {_relative(target)}")

    newer = _newer_inputs(target, build_logic_inputs)
    if newer:
        rendered = ", ".join(_relative(path) for path in newer[:3])
        suffix = "" if len(newer) <= 3 else f" (+{len(newer) - 3} more)"
        raise PreflightError(
            f"stale {label} {_relative(target)}: newer build recipe/config input "
            f"{rendered}{suffix}"
        )

    query = _run(_make_freshness_command(target, non_matching=non_matching), capture=True)
    if query.returncode == 0:
        return
    if query.returncode == 1:
        mode = " NON_MATCHING=1" if non_matching else ""
        raise PreflightError(
            f"stale {label} {_relative(target)} according to the Make dependency graph; "
            f"run `nice -n 10 gmake -j2{mode} {_relative(target)}`"
        )
    detail = (query.stderr or query.stdout or "gmake -q failed").strip().splitlines()
    raise PreflightError(
        f"could not prove {label} freshness for {_relative(target)}: "
        f"{detail[-1] if detail else f'gmake -q exited {query.returncode}'}"
    )


def require_fresh_evidence(resolution: Resolution) -> None:
    """Fail before comparison unless both linked and full-TU artifacts are current."""

    build_logic = _build_logic_inputs()
    _require_fresh_target(
        TARGET_ELF,
        label="canonical linked ELF",
        non_matching=False,
        build_logic_inputs=build_logic,
    )
    _require_fresh_target(
        resolution.candidate_object,
        label="candidate object",
        non_matching=resolution.candidate_build_dir == "build_non_matching",
        build_logic_inputs=build_logic,
    )


def _build_target(target: Path, *, non_matching: bool, label: str) -> None:
    """Run the Makefile's required split phase before one evidence target."""

    command = ["nice", "-n", "10", "gmake", "-j2"]
    if non_matching:
        command.append("NON_MATCHING=1")
    build_dir = "build_non_matching" if non_matching else "build"
    split = _run([*command, f"{build_dir}/.splat-stamp"], capture=True)
    if split.returncode:
        raise PreflightError(f"{label} split phase failed with exit {split.returncode}")
    result = _run([*command, _relative(target)], capture=True)
    if result.returncode:
        raise PreflightError(f"{label} build failed with exit {result.returncode}")


def _build(resolution: Resolution) -> None:
    _build_target(TARGET_ELF, non_matching=False, label="canonical")
    _build_target(
        resolution.candidate_object,
        non_matching=resolution.candidate_build_dir == "build_non_matching",
        label="candidate",
    )


def _symbol_geometry(elf: rs.Elf, names: tuple[str, ...]) -> tuple[str, int, int, str]:
    found: list[tuple[str, int, int, str]] = []
    for name in dict.fromkeys(names):
        for symbol, value, size, _info, shndx in elf.symbols():
            if symbol != name or shndx == rs.SHN_UNDEF or size == 0:
                continue
            section = elf.names[shndx] if shndx < len(elf.names) else ""
            found.append((name, value, size, section))
    geometries: dict[tuple[int, int, str], list[str]] = {}
    for name, value, size, section in found:
        geometries.setdefault((value, size, section), []).append(name)
    if len(geometries) != 1:
        summary = ", ".join(
            f"{'/'.join(symbols)}@{value:#x}+{size:#x}"
            for (value, size, _section), symbols in geometries.items()
        )
        raise PreflightError(
            f"linked geometry is ambiguous for {names}: found {len(geometries)} "
            f"nonzero definitions ({summary or 'none'})"
        )
    (value, size, section), symbols = next(iter(geometries.items()))
    selected = next((name for name in names if name in symbols), symbols[0])
    return selected, value, size, section


def _source_signature(source: Path, symbol: str) -> str:
    original = source.read_text(encoding="utf-8", errors="replace")
    facts = pp.source_facts(original, symbol)
    if len(facts.definitions) != 1:
        raise PreflightError(
            f"expected one C definition signature for {symbol}, found "
            f"{len(facts.definitions)}"
        )
    text = pp._mask_c(original)
    starts = [0]
    starts.extend(match.end() for match in re.finditer("\n", text))
    definition_start = starts[facts.definitions[0].line - 1]
    match = re.search(rf"\b{re.escape(symbol)}\s*\(", text[definition_start:])
    if not match:
        raise PreflightError(f"cannot find C definition signature for {symbol}")
    symbol_start = definition_start + match.start()
    symbol_end = definition_start + match.end()
    opening = text.find("(", symbol_start, symbol_end)
    closing = pp._matching_paren(text, opening)
    if closing is None:
        raise PreflightError(f"unbalanced candidate declaration for {symbol}")
    brace = closing + 1
    while brace < len(text) and text[brace].isspace():
        brace += 1
    if brace >= len(text) or text[brace] != "{":
        raise PreflightError(f"{symbol} occurrence is not a function definition")
    line_start = text.rfind("\n", 0, symbol_start) + 1
    signature = " ".join(text[line_start:brace].split())
    if not signature:
        raise PreflightError(f"empty candidate declaration for {symbol}")
    return signature


def _identity_text(identity: tuple[int, int] | None) -> str:
    if identity is None or len(identity) != 2:
        raise PreflightError(f"runtime relocation identity is ambiguous: {identity!r}")
    overlay, offset = identity
    return f"resident:+0x{offset:X}" if overlay == 0 else f"overlay:{overlay}:+0x{offset:X}"


def _relocation_rows(records: list[rs.SurfaceRecord]) -> list[dict[str, object]]:
    return [
        {
            "offset": f"+0x{record.offset:X}",
            "type": TYPE_NAMES.get(record.rtype, str(record.rtype)),
            "identity": _identity_text(record.identity),
            "addend": record.link_addend,
        }
        for record in records
    ]


def _relocation_evidence(
    resolution: Resolution,
    context: dict[str, object],
    runtime_records: list[rs.SurfaceRecord],
    target_elf: rs.Elf,
    linked_name: str,
    target_value: int,
    target_size: int,
    target_section: str,
) -> dict[str, list[dict[str, object]]]:
    """Keep overlay runtime records distinct from resident static tuples."""
    if context["kind"] == "overlay":
        return {
            "target_static_relocations": [],
            "runtime_overlay_records": _relocation_rows(runtime_records),
            "resident_runtime_records": [],
        }

    static_records = rs._resident_target_records(
        resolution.candidate_object,
        resolution.translation_unit,
        target_elf,
        linked_name,
        target_value,
        target_size,
        target_section,
        ALIASES,
        runtime_records,
    )
    return {
        "target_static_relocations": _relocation_rows(static_records),
        "runtime_overlay_records": [],
        "resident_runtime_records": _relocation_rows(runtime_records),
    }


def _target_context(
    resolution: Resolution,
    target_value: int,
    target_size: int,
    atlas: dict[str, object],
    rom: bytes,
) -> tuple[dict[str, object], list[rs.SurfaceRecord]]:
    generated = rs.GEN_NAME_RE.match(resolution.target_symbol)
    if generated:
        overlay = int(generated.group(1))
        offset = int(generated.group(2), 16)
        if target_value != rs.SYNTHETIC_VMA + offset:
            raise PreflightError(
                f"linked target value {target_value:#x} disagrees with generated offset {offset:#x}"
            )
        modules = [row for row in atlas["modules"] if row["overlay"] == overlay]
        if len(modules) != 1:
            raise PreflightError(f"atlas has {len(modules)} rows for overlay {overlay}")
        module = modules[0]
        owners = [
            row
            for row in module["text_ownership"]
            if int(row["offset"], 16) <= offset
            and offset + target_size <= int(row["end_offset"], 16)
        ]
        if len(owners) != 1:
            raise PreflightError(
                f"target range overlay:{overlay}:+0x{offset:X}..+0x{offset + target_size:X} "
                f"has {len(owners)} atlas owners"
            )
        owner = owners[0]
        owner_end = int(owner["end_offset"], 16)
        following = [
            row for row in module["text_ownership"] if int(row["offset"], 16) >= offset + target_size
        ]
        following.sort(key=lambda row: int(row["offset"], 16))
        next_row = following[0] if following else None
        module_runtime = ot.build_modules(ot.read_headers(rom))[overlay - 1]
        records = rs._target_runtime_records(
            rom,
            {"kind": "overlay", "overlay": overlay, "module": module_runtime},
            offset,
            target_size,
        )
        exports = [
            row for row in module["exports"] if int(row["offset"], 16) == offset
        ]
        context = {
            "kind": "overlay",
            "overlay": overlay,
            "offset": offset,
            "rom_start": module_runtime["rom_start"] + offset,
            "rom_end": module_runtime["rom_start"] + offset + target_size,
            "owner": owner,
            "owner_end": owner_end,
            "next_owner": next_row,
            "exports": exports,
        }
        return context, records

    if not (ot.RESIDENT_VRAM_BASE <= target_value < 0x90000000):
        raise PreflightError(f"resident target value is out of range: {target_value:#x}")
    offset = target_value - ot.RESIDENT_VRAM_BASE
    records = rs._target_runtime_records(
        rom, {"kind": "resident"}, offset, target_size
    )
    return {
        "kind": "resident",
        "offset": offset,
        "vram_start": target_value,
        "vram_end": target_value + target_size,
        "rom_start": target_value - ot.VRAM_ROM_DELTA,
        "rom_end": target_value - ot.VRAM_ROM_DELTA + target_size,
        "exports": [],
    }, records


def _resident_boundary(
    elf: rs.Elf, target_value: int, target_size: int, section: str
) -> dict[str, object]:
    end = target_value + target_size
    interior: list[tuple[str, int]] = []
    following: dict[int, list[str]] = {}
    for name, value, size, info, shndx in elf.symbols():
        if not name or size == 0 or shndx == rs.SHN_UNDEF or (info & 0xF) != 2:
            continue
        symbol_section = elf.names[shndx] if shndx < len(elf.names) else ""
        if symbol_section != section:
            continue
        if target_value < value < end:
            interior.append((name, value))
        elif value >= end:
            following.setdefault(value, []).append(name)
    if interior:
        rendered = ", ".join(f"{name}@0x{value:X}" for name, value in interior)
        raise PreflightError(f"resident owned range contains another function: {rendered}")
    if not following:
        return {"next_symbol": None, "next_start": None, "padding_size": None}
    next_start = min(following)
    return {
        "next_symbol": "/".join(sorted(set(following[next_start]))),
        "next_start": next_start,
        "padding_size": next_start - end,
    }


def _inbound_references(
    context: dict[str, object], rom: bytes
) -> list[dict[str, object]]:
    target_identity = (
        int(context.get("overlay", 0)),
        int(context["offset"]),
    )
    rom_table = ot.read_rom_table(rom)
    modules = ot.build_modules(ot.read_headers(rom))
    refs: list[dict[str, object]] = []

    count, resident = ot.read_reloc_table(rom)
    if count != len(resident):
        raise PreflightError("resident relocation table count disagrees")
    resident_records = []
    for row in resident:
        resident_records.append(
            dict(
                row,
                mode=row["flags"] >> 4,
                op=row["flags"] & 0xF,
                symbol_index=row["rom_table_index"],
                relative_offset=row["call_site_offset"],
            )
        )
    attached = rs._attach_runtime_identities(
        resident_records,
        rom,
        0,
        rom_table,
        lambda row: ot.RESIDENT_VRAM_BASE + row["call_site_offset"] - ot.VRAM_ROM_DELTA,
    )
    for source, record in zip(resident_records, attached):
        if record.identity == target_identity:
            refs.append(
                {
                    "source": "resident",
                    "site": f"vram:0x{ot.RESIDENT_VRAM_BASE + source['call_site_offset']:X}",
                    "type": TYPE_NAMES.get(record.rtype, str(record.rtype)),
                }
            )

    for module in modules:
        raw = ot.read_module_relocations(rom, module, rom_table)
        prepared = [dict(row, relative_offset=row["target_offset"]) for row in raw]
        attached = rs._attach_runtime_identities(
            prepared,
            rom,
            module["overlay"],
            rom_table,
            lambda row, module=module: module["rom_start"] + row["target_offset"],
        )
        for source, record in zip(prepared, attached):
            if record.identity == target_identity:
                refs.append(
                    {
                        "source": f"overlay:{module['overlay']}",
                        "site": f"+0x{source['target_offset']:X}",
                        "type": TYPE_NAMES.get(record.rtype, str(record.rtype)),
                    }
                )
    return refs


def _workbench(resolution: Resolution) -> dict[str, object]:
    command = [str(WB_COMPARE), resolution.requested_symbol, "--json", "--color", "never"]
    result = _run(command, capture=True)
    if result.returncode:
        raise PreflightError(f"wb_compare failed with exit {result.returncode}")
    try:
        payload = json.loads(result.stdout)
    except json.JSONDecodeError as error:
        raise PreflightError("wb_compare did not return one JSON report") from error
    if payload.get("schema") != "decomp-workbench-comparison-v1":
        raise PreflightError(f"unexpected workbench report schema: {payload.get('schema')!r}")
    required = ("words", "target_instructions", "candidate_instructions", "first_divergent_row")
    missing = [key for key in required if key not in payload]
    if missing:
        raise PreflightError("workbench report lacks: " + ", ".join(missing))
    return {
        "differing_words": payload["words"],
        "target_words": payload["target_instructions"],
        "candidate_words": payload["candidate_instructions"],
        "matched_words": (
            payload["target_instructions"] - payload["words"]
            if payload["target_instructions"] == payload["candidate_instructions"]
            else None
        ),
        "first_mismatch": (
            None
            if payload["first_divergent_row"] is None
            else f"+0x{4 * payload['first_divergent_row']:X}"
        ),
        "verdict": payload.get("verdict"),
        "target_frame": (
            abs(payload["target_frame_size"])
            if isinstance(payload.get("target_frame_size"), int)
            else None
        ),
        "candidate_frame": (
            abs(payload["candidate_frame_size"])
            if isinstance(payload.get("candidate_frame_size"), int)
            else None
        ),
    }


def collect(resolution: Resolution) -> dict[str, object]:
    for path, label in (
        (TARGET_ELF, "canonical linked ELF"),
        (resolution.candidate_object, "candidate object"),
        (ROM, "baserom"),
        (ATLAS, "overlay atlas"),
    ):
        if not path.is_file():
            raise PreflightError(f"missing {label}: {_relative(path)}")

    target_elf = rs.Elf(TARGET_ELF)
    linked_name, target_value, target_size, section = _symbol_geometry(
        target_elf, (resolution.candidate_symbol, resolution.target_symbol)
    )
    atlas = json.loads(ATLAS.read_text(encoding="utf-8"))
    rom = ROM.read_bytes()
    context, runtime_records = _target_context(
        resolution, target_value, target_size, atlas, rom
    )
    if context["kind"] == "resident":
        context.update(
            _resident_boundary(target_elf, target_value, target_size, section)
        )
    comparison = rs.function_surface_comparison(
        resolution.requested_symbol,
        resolution.candidate_object,
        TARGET_ELF,
        rom_path=ROM,
        atlas_path=ATLAS,
        values_path=ALIASES,
        candidate_symbol=resolution.candidate_symbol,
        target_symbol=linked_name,
        source=resolution.translation_unit,
    )
    relocation_evidence = _relocation_evidence(
        resolution,
        context,
        runtime_records,
        target_elf,
        linked_name,
        target_value,
        target_size,
        section,
    )
    if comparison["candidate_identity_resolved_count"] != comparison["candidate_record_count"]:
        raise PreflightError(
            "candidate static relocation identity is unresolved at "
            f"{comparison['candidate_record_count'] - comparison['candidate_identity_resolved_count']} "
            "site(s)"
        )
    inbound = _inbound_references(context, rom)
    result: dict[str, object] = {
        "schema": "mickey-function-evidence-preflight-v1",
        "requested_symbol": resolution.requested_symbol,
        "target_symbol": resolution.target_symbol,
        "candidate_symbol": resolution.candidate_symbol,
        "source": _relative(resolution.source),
        "translation_unit": resolution.translation_unit,
        "candidate_build_dir": resolution.candidate_build_dir,
        "candidate_object": _relative(resolution.candidate_object),
        "candidate_selection": resolution.selection,
        "candidate_signature": _source_signature(
            resolution.source, resolution.candidate_symbol
        ),
        "linked_symbol": linked_name,
        "linked_section": section,
        "owned_size": target_size,
        "context": context,
        "exports": context["exports"],
        "callers": [row for row in inbound if row["type"] == "R_MIPS_26"],
        "inbound_references": inbound,
        # Compatibility for existing JSON consumers. New consumers should use
        # the explicitly named static/runtime fields below.
        "runtime_relocations": _relocation_rows(runtime_records),
        **relocation_evidence,
        "relocation_comparison": comparison,
        "workbench": _workbench(resolution),
    }
    return result


def _render_human(report: dict[str, object]) -> None:
    def print_relocation(row: dict[str, object]) -> None:
        addend = row["addend"]
        suffix = "" if addend is None else f" addend={addend:+#x}"
        print(
            f"  {row['offset']} {row['type']} identity={row['identity']}"
            f"{suffix}"
        )

    context = report["context"]
    print(f"identity: {report['target_symbol']} -> {report['candidate_symbol']}")
    print(
        f"candidate: {report['source']} [{report['candidate_build_dir']}] "
        f"{report['candidate_signature']}"
    )
    if context["kind"] == "overlay":
        print(
            f"owned range: overlay:{context['overlay']}:+0x{context['offset']:X}.."
            f"+0x{context['offset'] + report['owned_size']:X} "
            f"(ROM 0x{context['rom_start']:X}..0x{context['rom_end']:X})"
        )
        owner = context["owner"]
        next_owner = context["next_owner"]
        if next_owner:
            print(
                f"boundary: owner ends +0x{context['owner_end']:X}; next "
                f"{next_owner['type']} range {next_owner['source']} starts "
                f"+0x{int(next_owner['offset'], 16):X}"
            )
        else:
            print(f"boundary: owner ends +0x{context['owner_end']:X}; end of overlay text")
    else:
        print(
            f"owned range: resident 0x{context['vram_start']:X}..0x{context['vram_end']:X} "
            f"(ROM 0x{context['rom_start']:X}..0x{context['rom_end']:X})"
        )
        if context["next_symbol"] is None:
            print("boundary: no following resident function in the linked section")
        else:
            print(
                f"boundary: next {context['next_symbol']} starts "
                f"0x{context['next_start']:X}; padding=0x{context['padding_size']:X}"
            )

    exports = report["exports"]
    print("exports: " + (", ".join(str(row["rom_table_index"]) for row in exports) or "none"))
    callers = report["callers"]
    print(f"callers: {len(callers)}")
    for row in callers:
        print(f"  {row['source']} {row['site']} {row['type']}")
    noncall = [row for row in report["inbound_references"] if row["type"] != "R_MIPS_26"]
    if noncall:
        print(f"other inbound references: {len(noncall)}")
        for row in noncall:
            print(f"  {row['source']} {row['site']} {row['type']}")
    if context["kind"] == "overlay":
        tuples = report["runtime_overlay_records"]
        print(f"runtime overlay records: {len(tuples)}")
        for row in tuples:
            print_relocation(row)
    else:
        tuples = report["target_static_relocations"]
        print(f"target static relocations: {len(tuples)}")
        for row in tuples:
            print_relocation(row)
        resident_runtime = report["resident_runtime_records"]
        print(
            f"resident runtime records: {len(resident_runtime)} "
            "(sparse startup table; zero is valid)"
        )
        print("runtime overlay records: not applicable (resident function)")
    reloc = report["relocation_comparison"]
    print(
        "candidate static relocation surface: "
        f"target={reloc['target_surface_source']} "
        f"offset/type={reloc['offset_type_alignment_count']}/"
        f"{reloc['target_record_count']} identity="
        f"{reloc['stable_identity_alignment_count']}/"
        f"{reloc['target_record_count']}"
    )
    wb = report["workbench"]
    matched = "n/a" if wb["matched_words"] is None else f"{wb['matched_words']}/{wb['target_words']}"
    target_frame = "n/a" if wb["target_frame"] is None else f"0x{wb['target_frame']:X}"
    candidate_frame = (
        "n/a" if wb["candidate_frame"] is None else f"0x{wb['candidate_frame']:X}"
    )
    print(
        f"workbench: matched={matched} differing={wb['differing_words']} "
        f"target={wb['target_words']} candidate={wb['candidate_words']} "
        f"first={wb['first_mismatch'] or 'none'} verdict={wb['verdict']} "
        f"frame={target_frame}/{candidate_frame}"
    )


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("symbol")
    parser.add_argument("--json", action="store_true")
    parser.add_argument(
        "--no-build",
        action="store_true",
        help="require existing canonical and candidate artifacts instead of updating them",
    )
    parser.add_argument(
        "--resolve-wb",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    args = parser.parse_args(argv)
    try:
        resolution = resolve(args.symbol)
        if args.resolve_wb:
            require_fresh_evidence(resolution)
            fields = (
                resolution.target_symbol,
                resolution.candidate_symbol,
                _relative(resolution.source),
                resolution.translation_unit,
                resolution.candidate_build_dir,
                _relative(resolution.target_asm),
            )
            if any("\t" in field or "\n" in field for field in fields):
                raise PreflightError("resolved paths cannot be represented safely")
            print("\t".join(fields))
            return 0
        if not args.no_build:
            _build(resolution)
        require_fresh_evidence(resolution)
        report = collect(resolution)
    except (OSError, ValueError, rs.SurfaceComparisonError, PreflightError) as error:
        parser.error(str(error))
    if args.json:
        print(json.dumps(report, indent=2, sort_keys=True))
    else:
        _render_human(report)
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
