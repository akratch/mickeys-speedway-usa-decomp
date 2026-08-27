# Overlay graph comparison

`tools/overlay_graph_match.py` compares Mickey overlays with Jet Force Gemini
modules using sizes, function counts, dependency shape, and named resident
callees. The output is `config/overlay-graph.us.json`.

This is structural evidence. It does not prove byte identity or an original
module boundary.

## Inputs

Mickey features come from `config/overlays.us.json`, relocation tables, and
`symbol_addrs.us.txt`:

- text, initialized-data, and BSS sizes;
- current source-range count;
- incoming and outgoing overlay dependencies; and
- named resident functions reached through external-call relocations.

JFG features come from its published symbol list, map, dependency list, and
overlay relocation tables. Set `JFG_ROOT` when its checkout is not at the
script's default location.

## Scoring

When either side has named resident calls, the score combines:

| Feature | Weight |
|---|---:|
| Weighted overlap of named resident callees | 0.55 |
| Section-size similarity | 0.25 |
| Function-count similarity | 0.12 |
| Incoming and outgoing degree similarity | 0.08 |

Common resident functions receive less weight than rare callees. When neither
side has named resident calls, the script falls back to section size, function
count, and graph degree.

A pair is marked confident only when its score and lead over the next candidate
pass the thresholds stored by the script. Even then, proposed function names
must be checked against Mickey's callers and data flow. A size-only proposal is
level D at most.

Overlay 32 is empty and is never accepted as a structural match. Overlay
numbers are not compared directly because the two games number modules
differently.

## Commands

```sh
.venv/bin/python tools/overlay_graph_match.py --write
.venv/bin/python tools/overlay_graph_match.py --check
.venv/bin/python tools/overlay_graph_match.py --overlay 15
```

`--write` refreshes the JSON report. `--check` fails when the committed report
does not match current inputs. `--overlay` limits printed detail.

## Interpretation

The comparison is strongest when several uncommon resident callees, section
sizes, and function order agree. It is weak when a module has no named calls or
when one game's module boundary moved. An exact function match inside two
differently sized modules does not make the modules equivalent.

Treat the output as a list of places to inspect. Adopt a name only after the
evidence level in [The module map](modules.md) is satisfied.
