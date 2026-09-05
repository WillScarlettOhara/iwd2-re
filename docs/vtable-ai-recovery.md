# CGameAIBase vtable recovery — 2026-09-05

The three findings from the first economic batch were source discrepancies,
not auditor false positives.

| Slot | Original | Correction |
|---|---|---|
| 0x28 | inherited `CGameObject::CanSaveGame`, 0x44CBC0 | Remove the permissive base override |
| 0x8C | scheduler at 0x45C730 | Recover virtual `ProcessAI`; name the direct selector at 0x45CA10 `ProcessScripts` |
| 0xA4 | decoder at 0x45D050 | Recover `QuickDecode(CAITrigger&, CGameSprite*&)` |

The scheduler checks local ownership, dequeues actions, updates randomness,
handles paused instant actions, staggers timers every 16 updates and gates script
selection using pending triggers and counters. It then executes actions while
the following actions are instant. The area-specific action fallback in the old
selector is removed because the scheduler now performs that work. The sprite's
explicit call to the selector remains direct, via `ProcessScripts`.

Binary evidence:

- 0x45C730–0x45CA0D: scheduler, including the direct call at 0x45C961 to 0x45CA10.
- 0x45D050–0x45D124: decode the trigger cause at offset 6, request object type
  0x31, compare its AI type with `NOT_SPRITE` at 0x8C76C8, release the share and
  clear the output if that sentinel matches. Return FALSE on all branches.
- 0x44CBC0: write error string 16502 and return FALSE.
- 0x4CC2DB: the tiled-object constructor installs vtable 0x84C700; its slot
  0x28 is 0x44CBC0. This resolves the uncertainty cited by the old save hack.

`vtable_audit.py CGameAIBase` is clean (22/22 anchors), as is CGameSprite
(33/33). The static batch for 0x45C730 and 0x45D050 passes with call-count
warnings (inlined getters/list operations appear as source calls). Its automatic
layout step is skipped because it does not recognize this header's offset
annotation style; no data members were added or changed.

Static evidence: `.arc/2026-09-05T02-44-37-batch/`.

Full VS2019 Win32 rebuild passed. `newgame-reload.txt` passed all 20 steps,
with 16,101 calls to the recovered base scheduler and 13,370 to the selector;
the 40-second smoke returned CLEAN. Saves and configuration were restored from
backup. The compiled CGameSprite layout audit passed all 310 annotated members.
QuickDecode's branches were not dynamically exercised by this scenario; its
validation here is static assembly/slot evidence. Existing incomplete sprite
combat branches and script-trigger cases remain outside this change.
