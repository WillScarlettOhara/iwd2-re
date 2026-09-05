# Economical original/reconstruction trace comparison

`scripts/frida_trace_diff.py` replaces one-off diff scripts when both captures
are JSONL. It compares three independent signals: calls per tag, call order and
arguments/fields for the same occurrence of a tag. Its output is deliberately
short; use normalized files only when a mismatch needs inspection.

```powershell
$py = '.\.venv-reagent\Scripts\python.exe'
& $py scripts/frida_trace_diff.py compare tmp_orig.jsonl tmp_ours.jsonl
& $py scripts/frida_trace_diff.py normalize tmp_orig.jsonl -o tmp_orig.normalized.jsonl
```

Linux uses `.venv-reagent/bin/python` with identical arguments. Exit code `0`
means all enabled signals match, `1` means a behavioral difference, and `2`
means malformed or empty input. A captured `crash`, `exception`, `assert` or
`ERROR` event can therefore never produce a successful parity gate.

The normalizer accepts raw events and Frida envelopes:

```json
{"tag":"Apply","this":"0x12345678","a0_s32":7}
{"type":"send","payload":{"tag":"Apply","this":"0x76543210","a0_s32":7}}
```

Harness events (`ready`, `sites`, `drive`, `loaded`, `HITS`), timestamps,
thread/process ids and backtraces are removed by default. Process-local pointer
fields preserve null versus non-null but normalize their addresses to `<null>`
or `<ptr>`. Use `--raw-pointers` only when the actual pointer value is the fact
being tested.

Useful controls:

- `--ignore-field 'TAG.tick'` or `--ignore-field tick` removes known noise;
- `--tag-map aliases.json` maps names from two differently instrumented sides;
- `--include-tag TAG` limits a large capture to the question being tested;
- `--ignore-order` keeps frequency and argument checks when Frida delivery order
  is known to be asynchronous;
- `--float-tolerance 0.001` handles an explicitly justified numeric tolerance;
- `--json` emits one compact machine-readable result.

Keep tolerances in the command recorded with the investigation. Do not add a
global tolerance to hide an unexplained difference. Logged events are capped by
each hook's `max`; because periodic `HITS` records are harness metadata, choose a
large enough `max` when exact frequency is part of the question.

For a cheap runtime campaign, use the smallest scenario that exercises the
batch once:

```powershell
& $py scripts/native.py smoke --new-game --party 1 --hold 30 `
    --ui scripts/scenarios/prologue-world-stable.txt --hit 'CClass::Method'
```

Use `scripts/scenarios/world-dump.txt` for a loaded-world UI/layout check and
`scripts/scenarios/newgame-reload.txt` only for save, ownership and lifecycle
changes. This keeps ordinary function batches short while retaining a deeper
stateful gate for the code that needs it.
