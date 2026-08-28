#!/usr/bin/env python3
"""Report-and-skip for the digest-guarded POSTPROCESS passes.

Every per-file POSTPROCESS rule encodes the *matching* object's exact layout:
a section trimmed to the ownership row's size, a relocation set asserted by a
`.text` prefix hash, an instruction normalization asserted by a digest. That is
the right behaviour for the build, whose whole job is to fail loudly when an
object stops being the object the rule was written against.

It is the wrong behaviour for `tools/promotion_trial.py`. A promoted candidate
whose codegen is a different *size* trips the guard at compile time, so the
build dies before the link and the trial learns only "build-error" -- 53 of the
279 overlay candidates ended there, all of them saying nothing more useful than
that a guard fired. What the trial wants is the measurement: how far off the
size is, and then a linked ROM to diff for everything else.

So when `PROMOTION_TRIAL` is set in the environment, a guard prints a
machine-readable marker on stderr and skips its pass instead of aborting.  The
object is left un-normalized, so the resulting ROM is not a valid build -- that
is fine and expected, because `promotion_trial.py` classifies from the marker,
not from those bytes.  The normal build never sets the variable, so nothing
about `gmake` / `gmake verify` changes.

    tools/promotion_trial.py       sets PROMOTION_TRIAL=1 for its own builds
    gmake PROMOTION_TRIAL=1        the same, by hand, for one experiment
"""

import os
import sys

ENV = "PROMOTION_TRIAL"
MARKER = "PROMOTION-TRIAL"


def enabled():
    return os.environ.get(ENV, "") not in ("", "0")


def report(kind, message):
    """Emit one marker line and skip the pass. Returns only if not enabled."""
    if not enabled():
        return False
    print(f"{MARKER}: {kind}: {message}", file=sys.stderr)
    sys.stderr.flush()
    return True


def fail(message, kind="guard-tripped"):
    """Abort, or -- under PROMOTION_TRIAL -- report the reason and skip.

    A usage error is never skipped: it means the *invocation* is wrong, not
    that the object moved, and silently skipping one would hide a real bug in
    the trial harness behind a green build.
    """
    if not str(message).startswith("usage:") and report(kind, message):
        raise SystemExit(0)
    raise SystemExit(message)
