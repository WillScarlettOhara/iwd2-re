# Selecting reverse-engineering targets economically

`scripts/next_targets.py` separates ownership from priority before proposing
work. This avoids spending a recovery arc on code emitted by MFC, the CRT, STL,
the compiler, or an already represented inline template.

Run the normal short view through `arc`:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts\arc.py targets
```

Generate the local vtable evidence once if the command reports it missing:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts\reagent_vtable_map.py
```

Use the JSON audit when a ranking is surprising:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts\next_targets.py `
    --json .ghidra-exports\next_targets.json
```

Every row preserves the exported symbol and records `ownership`, `bucket`,
`classification_reasons`, its probable caller class, unique caller functions,
and raw call-site counts. A probable class is accepted only when at least two
unique recovered functions belong to it and it represents at least 60% of the
classed recovered callers. It is grouping evidence, never a proposed function
name.

The queues have distinct costs:

- **Recover** contains named game-owned functions.
- **Identify** contains placeholders tied to a game class by vtable or caller
  consensus. Assemble context before assigning a name.
- **Triage** has no ownership proof yet. Inspect its digest before a full arc.
- **Support review** contains short high-fan-in placeholders whose callees are
  limited to resource, container, or runtime helpers. Promote a row when the
  disassembly shows game behavior.
- **Excluded** contains named MFC/ATL, CRT, and STL functions. Compiler funclets
  are aggregated because listing roughly 19,600 rows would make the audit less
  useful.

Caller scoring uses unique functions. `call_sites` remains in the output so a
reviewer can see when one function contains many references to the same helper.
After a recovery or a classification change, refresh the tracked snapshot:

```powershell
& .\.venv-reagent\Scripts\python.exe scripts\next_targets.py --write --top 40
```
