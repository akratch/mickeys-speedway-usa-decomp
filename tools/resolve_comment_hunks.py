#!/usr/bin/env python3
r"""Resolve merge-conflict hunks that carry no code.

  mickey.us.yaml   hunks whose both sides are `#` comment lines: take ours.
  *.c / *.h        hunks whose both sides are C comment lines only (plateau
                   notes above a `#ifdef NON_MATCHING`): take theirs, the lane
                   that just worked the function.
  docs/*.md        hunks whose both sides are ledger table rows for the same
                   function(s) (`| \`name\` | ...`): take theirs.

Any hunk with a code line on either side is left for a human. Usage: FILE...
Exit 1 if any hunk is left.
"""
from pathlib import Path
import re, sys

pat = re.compile(r"<<<<<<< [^\n]*\n(.*?)=======\n(.*?)>>>>>>> [^\n]*\n", re.S)


def yaml_comments(block):
    return all(l.strip() == "" or l.lstrip().startswith("#") for l in block.split("\n"))


def c_comments(block):
    inside = False
    for l in block.split("\n"):
        t = l.strip()
        if t == "":
            continue
        if inside:
            if "*/" in t:
                inside = False
                if t.split("*/", 1)[1].strip():
                    return False
            continue
        if t.startswith("//"):
            continue
        if t.startswith("/*"):
            if "*/" in t:
                if t.split("*/", 1)[1].strip():
                    return False
            else:
                inside = True
            continue
        if t.startswith("*"):  # continuation line of a block comment
            continue
        return False
    return not inside


plateau_block = re.compile(
    r"(/\* PLATEAU-HANDOFF:([^:\n]+):start\n.*?"
    r"\n \* PLATEAU-HANDOFF:\2:end)",
    re.S,
)


def plateau_sequence(block):
    """Parse adjacent plateau blocks and whether the shared close is external."""
    parsed = []
    position = 0
    for match in plateau_block.finditer(block):
        separator = block[position:match.start()]
        if parsed:
            if separator.strip() != "*/":
                return None
        elif separator.strip():
            return None
        parsed.append((match.group(2), match.group(1)))
        position = match.end()
    if not parsed:
        return None
    tail = block[position:].strip()
    if tail == "":
        return parsed, True
    if tail == "*/":
        return parsed, False
    return None


def merge_plateau_sequences(ours, theirs):
    ours_sequence = plateau_sequence(ours)
    theirs_sequence = plateau_sequence(theirs)
    if not ours_sequence or not theirs_sequence:
        return None
    ours_blocks, ours_open = ours_sequence
    theirs_blocks, theirs_open = theirs_sequence
    if ours_open != theirs_open:
        return None

    incoming = {name for name, _body in theirs_blocks}
    merged = [block for block in ours_blocks if block[0] not in incoming]
    merged.extend(theirs_blocks)
    rendered = "\n */\n\n".join(body for _name, body in merged)
    if not ours_open:
        rendered += "\n */\n"
    return rendered


def row_names(block):
    names = []
    for l in block.split("\n"):
        if l.strip() == "":
            continue
        m = re.match(r"\|\s*`([^`]+)`\s*\|", l)
        if not m:
            return None
        names.append(m.group(1))
    return names


def resolve(path):
    source_path = Path(path)
    s = source_path.read_text()
    left = 0

    def sub(m):
        nonlocal left
        ours, theirs = m.group(1), m.group(2)
        if path.endswith((".yaml", ".yml")):
            if yaml_comments(ours) and yaml_comments(theirs):
                return ours
        elif path.endswith((".c", ".h")):
            merged = merge_plateau_sequences(ours, theirs)
            if merged is not None:
                return merged
            if c_comments(ours) and c_comments(theirs):
                return theirs
        elif path.endswith(".md"):
            a, b = row_names(ours), row_names(theirs)
            if a and b and set(a) == set(b):
                return theirs
        left += 1
        return m.group(0)

    s = pat.sub(sub, s)
    source_path.write_text(s)
    print(f"{path}: {'resolved' if left == 0 else f'{left} hunk(s) left'}")
    return left


if __name__ == "__main__":
    sys.exit(1 if sum(resolve(p) for p in sys.argv[1:]) else 0)
