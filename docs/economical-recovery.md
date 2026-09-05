# Sustainable recovery campaigns

Work in small, reviewable batches over months. There is no weekly completion
target. Optimize for validated behavior per investigation, and preserve enough
evidence to resume without rereading a long conversation.

## One campaign

1. Run `scripts/arc.py targets --top 10`. Review ownership reasons before picking
   2–5 related functions. Caller class affinity is a hypothesis, not proof of
   ownership; support-review entries remain visible and can be promoted.
2. Keep one short context note per batch: addresses, question, relevant fields,
   callers/callees, binary evidence, save/scenario, unresolved points and next
   command. Aim for 4 KiB; keep full exports and logs on disk.
3. Use `src_find.py`, `fn_digest.py`, `bg2_find.py` and graph queries first.
   Request full decompilation or assembly only for the unresolved branch.
4. Edit and validate the batch with `arc batch`. It shares source/class audits,
   compiles once and runs one smoke requiring all selected symbols to fire.
5. When checking behavior, capture the same inputs on both executables and run
   `frida_trace_diff.py compare`. Record capture limits and normalization options.
6. Record the verdict and remaining uncertainty, update the graph, and commit
   the validated change. Start the next batch with the short note.

```powershell
$py = '.\.venv-reagent\Scripts\python.exe'
& $py scripts/arc.py targets --top 10
& $py scripts/arc.py batch 0x453840 0x6F5FF0 --new-game --party 1 --hold 30 `
    --ui scripts/scenarios/prologue-world-stable.txt
& $py scripts/frida_trace_diff.py compare tmp_orig.jsonl tmp_ours.jsonl
& '.\.venv-reagent\Scripts\code-review-graph.exe' update --repo . --brief
```

Select an isolated test save/configuration for runtime work. New-game and reload
scenarios can modify game files. The batch command does not sandbox saves.
An explicit `--hit` list replaces the default list; `--expect` replaces symbol
gates with a log gate. These overrides prove only the paths they actually observe.
Overloaded symbol counters can cover more than one overload: use precise binary
instrumentation when overload identity is part of the claim.

## Validation tiers

| Change | Smallest useful runtime gate |
|---|---|
| Prologue AI/effects | `prologue-world-stable.txt` plus relevant symbol hooks |
| Loaded-world UI | `world-dump.txt` or the specific existing UI scenario |
| CRE/GAM, ownership, lifecycle | `newgame-reload.txt` and cross-load in original |
| Inventory, spells, store | Existing specialized scenario and a suitable fixture |

`--static` skips build/runtime explicitly. Use it while investigating, then run
the full batch before claiming runtime validation. Ghidra parity still runs
serially per address; this implementation shares build/smoke and class audits,
not a persistent Ghidra process. Linux uses the existing VM adapter; runtime
validation on Windows does not establish that a Linux VM is configured correctly.

## Spending and context policy

Use one agent by default. Delegate only independent bounded work with a compact
input and result. Escalate reasoning effort for specific unresolved ABI,
assembly or runtime questions, then return to ordinary effort for mechanical work.
Do not infer subscription savings from source-graph token estimates.

Build, tests, graph, parity and this trace comparator do not need an LLM API.
Do not invoke `re-agent reverse`, `ds_batch` or another external LLM provider
without explicit authorization. The optional provider configuration is separate
from the subscription-backed coding session. Do not redeem usage resets
automatically. Preserve a continuation note if limits interrupt a campaign.

Track validated functions/behaviors, regressions, repeated investigations and
gate duration. A source address marker, a classification hypothesis and a short
crash-free interval are different evidence levels, not completion guarantees.

## First Windows campaign, 2026-09-05

The example batch above completed in 112 seconds with one build (1.1 s) and one
smoke (40.4 s). AutoUI passed all eight steps. The trace observed
`CGameAIBase::EvaluateStatusTrigger` 8,178 times and `CGameSprite::AIUpdate`
18,410 times. Test saves and configuration were backed up and restored.

The overall batch correctly returned FAIL: the class audit reported three
existing discrepancies in the CGameAIBase/CGameObject hierarchy (slots 0x28,
0x8c and 0xa4). Parity remained YELLOW, and one layout check was skipped.
This campaign validates the batch harness, not full parity of those functions.
Evidence: `.arc/2026-09-05T02-33-20-batch/`. Investigate those slots in a separate
game-code recovery arc. The trace comparator is covered by synthetic regression
tests; no new matched original/reconstruction capture pair was produced here.
