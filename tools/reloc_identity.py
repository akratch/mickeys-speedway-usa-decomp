#!/usr/bin/env python3
"""Shared parsing and canonical identity helpers for MIPS relocations.

The matching tools need to answer one question consistently: which stable
runtime identity does a relocation name after linker aliases, objcopy renames,
and the relocation's own addend are applied?  This module contains no ROM or
ELF policy.  It only parses the two textual surfaces involved and propagates
already-authenticated identities without guessing through ambiguity.
"""

from __future__ import annotations

import collections
import dataclasses
import re
import shlex
from collections.abc import Iterable, Mapping


Identity = tuple[int, int]
_SYMBOL_RE = re.compile(r"^[A-Za-z_.$][A-Za-z0-9_.$]*$")
_SECTION_RE = re.compile(r"^RELOCATION RECORDS FOR \[(.+)\]:?$")
_RELOCATION_RE = re.compile(
    r"^\s*([0-9A-Fa-f]+)\s+(R_[A-Za-z0-9_]+)\s+(.+?)\s*$"
)
_ALIAS_ASSIGNMENT_RE = re.compile(
    r"^\s*([A-Za-z_.$][A-Za-z0-9_.$]*)\s*=\s*"
    r"([A-Za-z_.$][A-Za-z0-9_.$]*)\s*;\s*(?://.*)?$"
)
# The Make database deliberately retains ``$(OBJCOPY)`` in target-specific
# POSTPROCESS assignments, while direct commands contain a concrete tool path.
# Both spellings have "objcopy" as an unambiguous token substring.
_OBJCOPY_RE = re.compile(r"objcopy", re.IGNORECASE)


class RelocationIdentityError(ValueError):
    """A relocation or alias surface cannot be interpreted uniquely."""


@dataclasses.dataclass(frozen=True)
class ObjdumpRelocation:
    """One canonical tuple parsed from ``objdump -r`` output."""

    section: str
    offset: int
    rtype: str
    symbol: str
    addend: int = 0


@dataclasses.dataclass(frozen=True)
class RelocationRecord:
    """One function-relative relocation and its stable runtime identity."""

    offset: int
    rtype: int
    identity: Identity | None = None
    link_addend: int | None = dataclasses.field(default=None, compare=False)


@dataclasses.dataclass
class AliasClosure:
    """Canonical provenance for destinations in a directed rename graph.

    ``resolved[destination]`` is the unique original source that reaches the
    destination.  Cycles and destinations reached from multiple independent
    sources are listed in ``ambiguous`` and deliberately omitted from
    ``resolved``.  Repeated identical pairs are harmless and recorded in
    ``duplicates`` so callers can diagnose them without changing identity.
    """

    resolved: dict[str, str]
    ambiguous: frozenset[str]
    duplicates: tuple[tuple[str, str], ...]
    conflicts: tuple[tuple[str, tuple[str, ...]], ...]
    cycles: tuple[tuple[str, ...], ...]


@dataclasses.dataclass
class IdentityResolution:
    """Unique and ambiguous names after equality and rename propagation."""

    resolved: dict[str, Identity]
    ambiguous: frozenset[str]


def _parse_symbol_expression(value: str) -> tuple[str, int]:
    """Split objdump's ``symbol[+|-]addend`` spelling, fail closed."""
    value = value.strip()
    match = re.fullmatch(
        r"([A-Za-z_.$][A-Za-z0-9_.$]*)(?:\s*([+-])\s*(0[xX][0-9A-Fa-f]+|\d+))?",
        value,
    )
    if not match:
        raise RelocationIdentityError(
            f"unsupported objdump relocation value {value!r}"
        )
    symbol, sign, raw_addend = match.groups()
    addend = int(raw_addend, 0) if raw_addend is not None else 0
    if sign == "-":
        addend = -addend
    return symbol, addend


def parse_objdump_relocations(
    text: str, *, section: str | None = None
) -> list[ObjdumpRelocation]:
    """Parse GNU objdump relocation records into stable, typed tuples.

    Header rows and blank lines are ignored.  A data-looking row in a selected
    relocation section that cannot be parsed is rejected rather than silently
    disappearing from an identity proof.
    """
    current: str | None = None
    output: list[ObjdumpRelocation] = []
    for raw_line in text.splitlines():
        stripped = raw_line.strip()
        header = _SECTION_RE.fullmatch(stripped)
        if header:
            current = header.group(1)
            continue
        if current is None or (section is not None and current != section):
            continue
        if not stripped or stripped.startswith("OFFSET"):
            continue
        match = _RELOCATION_RE.fullmatch(raw_line)
        if not match:
            if re.match(r"^\s*[0-9A-Fa-f]+\s+", raw_line):
                raise RelocationIdentityError(
                    f"cannot parse objdump relocation row {raw_line!r}"
                )
            continue
        raw_offset, rtype, raw_value = match.groups()
        symbol, addend = _parse_symbol_expression(raw_value)
        output.append(
            ObjdumpRelocation(current, int(raw_offset, 16), rtype, symbol, addend)
        )
    return output


def parse_linker_aliases(text: str) -> list[tuple[str, str]]:
    """Return identifier-to-identifier assignments from a linker symbol file."""
    output = []
    for line in text.splitlines():
        match = _ALIAS_ASSIGNMENT_RE.fullmatch(line)
        if match:
            output.append(match.groups())
    return output


def parse_numeric_assignments(text: str) -> dict[str, int]:
    """Parse simple numeric linker assignments, rejecting conflicting values."""
    values: dict[str, int] = {}
    pattern = re.compile(
        r"^\s*([A-Za-z_.$][A-Za-z0-9_.$]*)\s*=\s*"
        r"(0[xX][0-9A-Fa-f]+|\d+)\s*;\s*(?://.*)?$"
    )
    for line in text.splitlines():
        match = pattern.fullmatch(line)
        if not match:
            continue
        name, raw_value = match.groups()
        value = int(raw_value, 0)
        previous = values.setdefault(name, value)
        if previous != value:
            raise RelocationIdentityError(
                f"numeric symbol {name} has conflicting values "
                f"{previous:#x} and {value:#x}"
            )
    return values


def parse_objcopy_redefine_pairs(command: str) -> list[tuple[str, str]]:
    """Return ordered ``(source, destination)`` pairs from objcopy commands.

    Shell command separators are respected so transitive renames performed by
    successive objcopy invocations remain in order.  Malformed redefine
    options fail closed.
    """
    lexer = shlex.shlex(command, posix=True, punctuation_chars=";&|")
    lexer.whitespace_split = True
    lexer.commenters = ""
    segments: list[list[str]] = []
    segment: list[str] = []
    for token in lexer:
        if token in {"&&", "||", ";", "&", "|"}:
            if segment:
                segments.append(segment)
                segment = []
        else:
            segment.append(token)
    if segment:
        segments.append(segment)

    output: list[tuple[str, str]] = []
    for tokens in segments:
        if not any(_OBJCOPY_RE.search(token) for token in tokens):
            continue
        index = 0
        while index < len(tokens):
            token = tokens[index]
            spec = None
            if token == "--redefine-sym":
                index += 1
                if index >= len(tokens):
                    raise RelocationIdentityError(
                        "objcopy --redefine-sym has no argument"
                    )
                spec = tokens[index]
            elif token.startswith("--redefine-sym="):
                spec = token[len("--redefine-sym=") :]
            if spec is not None:
                if spec.count("=") != 1:
                    raise RelocationIdentityError(
                        f"invalid objcopy redefine specification {spec!r}"
                    )
                source, destination = spec.split("=", 1)
                if not _SYMBOL_RE.fullmatch(source) or not _SYMBOL_RE.fullmatch(
                    destination
                ):
                    raise RelocationIdentityError(
                        f"invalid objcopy redefine specification {spec!r}"
                    )
                output.append((source, destination))
            index += 1
    return output


def _canonical_cycle(nodes: list[str]) -> tuple[str, ...]:
    body = nodes[:-1]
    rotations = [tuple(body[index:] + body[:index]) for index in range(len(body))]
    return min(rotations)


def canonicalize_redefine_aliases(
    pairs: Iterable[tuple[str, str]],
) -> AliasClosure:
    """Collapse transitive rename chains without guessing through ambiguity."""
    incoming: dict[str, set[str]] = collections.defaultdict(set)
    seen: set[tuple[str, str]] = set()
    duplicates: list[tuple[str, str]] = []
    destinations: set[str] = set()
    self_aliases: set[str] = set()
    for source, destination in pairs:
        if not _SYMBOL_RE.fullmatch(source) or not _SYMBOL_RE.fullmatch(destination):
            raise RelocationIdentityError(
                f"invalid symbol rename {source!r} -> {destination!r}"
            )
        pair = (source, destination)
        if pair in seen:
            duplicates.append(pair)
        seen.add(pair)
        destinations.add(destination)
        if source == destination:
            self_aliases.add(destination)
        else:
            incoming[destination].add(source)

    cycles: set[tuple[str, ...]] = set()

    def provenance(name: str, path: list[str]) -> tuple[set[str], bool]:
        if name in path:
            start = path.index(name)
            cycle = path[start:] + [name]
            cycles.add(_canonical_cycle(cycle))
            return set(), True
        sources = incoming.get(name)
        if not sources:
            return {name}, False
        roots: set[str] = set()
        cyclic = False
        for source in sorted(sources):
            found, has_cycle = provenance(source, path + [name])
            roots.update(found)
            cyclic = cyclic or has_cycle
        return roots, cyclic

    resolved: dict[str, str] = {}
    ambiguous: set[str] = set()
    conflicts: list[tuple[str, tuple[str, ...]]] = []
    for destination in sorted(destinations):
        roots, cyclic = provenance(destination, [])
        # ``old=old`` is an idempotent spelling emitted by a few generated
        # rules. It preserves identity. If another source also targets that
        # existing name, retain both roots so the collision fails closed.
        if destination in self_aliases and incoming.get(destination):
            roots.add(destination)
        if cyclic or len(roots) != 1:
            ambiguous.add(destination)
            if len(roots) > 1:
                conflicts.append((destination, tuple(sorted(roots))))
            continue
        resolved[destination] = next(iter(roots))
    return AliasClosure(
        resolved=resolved,
        ambiguous=frozenset(ambiguous),
        duplicates=tuple(duplicates),
        conflicts=tuple(conflicts),
        cycles=tuple(sorted(cycles)),
    )


def resolve_identities(
    proposed: Mapping[str, Iterable[Identity]],
    *,
    equality_aliases: Iterable[tuple[str, str]] = (),
    redefine_aliases: Iterable[tuple[str, str]] = (),
) -> IdentityResolution:
    """Propagate identities through linker equality and objcopy rename edges.

    Linker aliases are equality components.  Objcopy aliases are directed
    provenance chains.  Any component, destination, cycle, or downstream name
    reached by more than one identity remains ambiguous.
    """
    identities: dict[str, set[Identity]] = collections.defaultdict(set)
    for name, values in proposed.items():
        identities[name].update(values)

    equality_graph: dict[str, set[str]] = collections.defaultdict(set)
    for left, right in equality_aliases:
        equality_graph[left].add(right)
        equality_graph[right].add(left)
    visited: set[str] = set()
    for name in sorted(equality_graph):
        if name in visited:
            continue
        component: set[str] = set()
        stack = [name]
        while stack:
            current = stack.pop()
            if current in component:
                continue
            component.add(current)
            stack.extend(equality_graph[current] - component)
        visited.update(component)
        merged: set[Identity] = set()
        for member in component:
            merged.update(identities.get(member, ()))
        for member in component:
            identities[member].update(merged)

    closure = canonicalize_redefine_aliases(redefine_aliases)
    # Canonical sources may themselves be linker aliases.  Carry all known
    # identities, then mark every ambiguous rename and its descendants below.
    for destination, source in closure.resolved.items():
        identities[destination].update(identities.get(source, ()))

    ambiguous = {name for name, values in identities.items() if len(values) > 1}
    ambiguous.update(closure.ambiguous)
    # A unique graph root that is itself ambiguous makes its destination
    # ambiguous too; repeat because chains can be arbitrarily long.
    changed = True
    while changed:
        changed = False
        for destination, source in closure.resolved.items():
            if source in ambiguous and destination not in ambiguous:
                ambiguous.add(destination)
                changed = True
    resolved = {
        name: next(iter(values))
        for name, values in identities.items()
        if len(values) == 1 and name not in ambiguous
    }
    return IdentityResolution(resolved, frozenset(ambiguous))


def effective_identity(identity: Identity, addend: int) -> Identity:
    """Apply a relocation addend in the identity's runtime address space."""
    return identity[0], identity[1] + addend


def format_identity(identity: Identity | None) -> str:
    """Render the stable identity spelling shared by evidence reports."""
    if identity is None or len(identity) != 2:
        raise RelocationIdentityError(
            f"runtime relocation identity is ambiguous: {identity!r}"
        )
    overlay, offset = identity
    return f"resident:+0x{offset:X}" if overlay == 0 else f"overlay:{overlay}:+0x{offset:X}"


def compare_records(target, candidate) -> dict[str, object]:
    """Return the compatibility-stable exactness census for two surfaces."""
    target_shape = collections.Counter((row.offset, row.rtype) for row in target)
    candidate_shape = collections.Counter(
        (row.offset, row.rtype) for row in candidate
    )
    target_identity = collections.Counter(
        (row.offset, row.rtype, row.identity)
        for row in target
        if row.identity is not None
    )
    candidate_identity = collections.Counter(
        (row.offset, row.rtype, row.identity)
        for row in candidate
        if row.identity is not None
    )
    return {
        "target_record_count": len(target),
        "target_runtime_record_count": len(target),
        "candidate_record_count": len(candidate),
        "offset_type_alignment_count": sum(
            (target_shape & candidate_shape).values()
        ),
        "stable_identity_alignment_count": sum(
            (target_identity & candidate_identity).values()
        ),
        "candidate_identity_resolved_count": sum(
            row.identity is not None for row in candidate
        ),
        "offset_type_exact": target_shape == candidate_shape,
        "stable_identity_exact": (
            target_shape == candidate_shape
            and len(target_identity) == len(target)
            and target_identity == candidate_identity
        ),
    }


def augment_effective_identity(
    comparison: Mapping[str, object], *, linked_exact: bool
) -> dict[str, object]:
    """Combine static and authenticated linked/runtime identity evidence."""
    result = dict(comparison)
    target_count = int(result["target_record_count"])
    static_count = int(result["stable_identity_alignment_count"])
    linked_count = target_count - static_count if linked_exact else 0
    effective_count = static_count + linked_count
    result.update(
        {
            "linked_runtime_identity_alignment_count": linked_count,
            "effective_identity_alignment_count": effective_count,
            "effective_identity_exact": effective_count == target_count,
            "identity_proof_mode": (
                "static"
                if result.get("stable_identity_exact") is True
                else "static-plus-runtime-table-and-linked-rom"
                if linked_exact
                else "partial-static"
            ),
        }
    )
    return result
