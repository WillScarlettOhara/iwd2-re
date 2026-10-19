# Cost-conscious recovery

The default is to recover a useful behavior, run the existing checks, and leave
one concise result. A new test framework, probe pair, evidence page or token
report is not a deliverable of every recovery. CLAUDE.md is the short entry point;
[technical details](recovery-reference.md) are read only as needed.

## Spend reasoning on the unresolved part

- Batch related functions around a playable outcome. Check dependencies before
  starting; avoid repeatedly investigating a large caller whose prerequisite is
  still missing. Continue beyond tiny edits when the authorized task allows it.
- Read complete target bodies and relevant layout/call evidence once. Use the
  context assembler's repeated `--address` and shared blocks. Re-read only what
  changed or what a concrete failure calls into question.
- Reuse AutoUI commands, loaded-world fixtures and the generic Frida hook-table
  driver before writing custom Python. Extend a common fixture with a few inputs
  instead of copying a reference driver and comparator for every batch.
- Generate repetitive golden data with local tools; do not ask the model to
  enumerate it. Do not read entire generated headers/JSONL logs for a verdict.
- Keep raw logs and captures under `.arc/` or ignored temporary paths. Use the
  generated continuation or a 5-10 line checkpoint; retain only unusual binary
  discoveries in durable notes. No mandatory new Markdown file per lot.
- Use a fresh chat with that short checkpoint when context becomes unwieldy;
  do not automatically create a new task or lose the current authorized work.
- Run token audits only to answer a cost question or evaluate a workflow change.
  Cached input is part of input, reasoning part of output; neither token counts
  nor graph savings estimates give a subscription percentage.

## Validation proportional to the change

Always preserve the binary behavior, compile changed C++, and exercise the
recovered path. Prefer one `arc batch` using a suitable existing profile; a
selected profile keeps all its hit/build/parity/cleanup gates. Never weaken an
existing combat regression to get a new lot through.

Simple UI wiring and display code normally need ASM review and an existing
scenario that observes the new behavior. No synthetic hundreds-case matrix by
default. Combat calculations, RNG, signed overflow, packet formats and ownership
need targeted boundary cases and an original oracle when uncertain. Reuse a
valid existing reference; add only the missing evidence. Do not claim paired
conformity for a path tested only on the reconstruction.

Keep static results, observed runtime and differential scope separate. A missing
symbol, skipped tool, failed load or timeout stays unproven. Retry only the
failed part when possible, keep its original record, and check that relevant
inputs did not change. Inspect detailed logs only when the compact verdict does
not explain a problem. A slow local tool is not a reason for repeated model-led
log reads. The constructor lint uses cppcheck's incremental build directory so
an unchanged ARC/pre-commit rerun can reuse analysis without caching a PASS.

## Ideas retained from thierbig/iwd2r

Reviewed at `b95c46e723f256c06a1332ce754f9eb2b19a213e`:

- [Parity runner](https://github.com/thierbig/iwd2r/blob/b95c46e723f256c06a1332ce754f9eb2b19a213e/tools/parity/run.py): scope evidence to changed functions, share batch work and
  return one compact table. Apply this through our existing ARC, not a second runner.
- [Reusable scenario checkpoints](https://github.com/thierbig/iwd2r/blob/b95c46e723f256c06a1332ce754f9eb2b19a213e/scenarios/first_fight.txt): validate a player-visible path with stable checkpoints. Prefer extending
  our scenarios to inventing a new private diagnostic for each function.
- [Evidence categories](https://github.com/thierbig/iwd2r/blob/b95c46e723f256c06a1332ce754f9eb2b19a213e/tools/check_evidence.py): distinguish original oracle, replay, static reading and trace. Keep our
  existing evidence fields; do not reproduce their per-function Markdown requirement.
- [Shared oracle cases](https://github.com/thierbig/iwd2r/tree/b95c46e723f256c06a1332ce754f9eb2b19a213e/oracle/cases): collect cases by subsystem behind common helpers. Reuse this principle
  with our current fixtures; porting their native PE loader is a separate project.

That repository also has extensive documentation and testing: it is not evidence
of a cheaper subscription workflow. Its cross-recovery imports functions from
this repository, so check provenance before treating a matching body as an
independent implementation. Match original addresses before names. Never execute
external recovery scripts or copy an engine body as proof of binary correctness.
Do not introduce its SDL/platform port, multi-agent wave orchestration, native
oracle loader or evidence bureaucracy merely to reduce token use here.
