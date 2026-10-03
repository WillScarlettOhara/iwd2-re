# Sprite script scheduling recovery

2026-10-03. Recovered the normal script scheduler in ProcessAI (0x72B9A0)
and its virtual slot 0xC8 at 0x728B70. Complete context bundles:
tmp_processai_scripts.md and tmp_sprite_script_dispatch.md; unchanged shared
sprite layout reused from prior arcs. PE disassembly is authoritative at
0x72D909..0x72DAC5 and 0x728B70..0x728BBA.

BG2 PDB supplies ProcessPendingTriggers as the sprite virtual method after
ResetAIType; its parameter differs in IWD2. The IWD2 slot at 0x85BE9C points
to 0x728B70 and the method returns with plain ret, without stack parameters.
Its only call targets are CanAct (0x7202E0) and ProcessScripts (0x45CA10).
The cached Ghidra export incorrectly labels the latter ProcessAI: the actual
call operand and existing recovered source establish ProcessScripts.

The wrapper suppresses script selection for derived panic, sprite cutscene,
game cutscene, CanAct failure or base flags bit 0x80000000. Otherwise it calls
the existing CGameAIBase::ProcessScripts selector. No invented base-AI tick or
additional action execution is inserted.

After UpdateBerserkActive/CheckModal, positive command pauses clear triggers.
Eligible scheduling requires unsigned frame%16 == unsigned id%16, nonpositive
pauses, no low three bits in field_3428, and a living sprite or the dead latch
field_558E. The scheduler advances field_54C and arms the latch. Idle action,
pending triggers, signed deferral count >=120 or positive grace count process
scripts immediately. Otherwise it translates the sprite area's viewport by
its view position and tests the sprite position. Visible actors process scripts;
hidden actors increment the deferral count instead. Pending triggers set grace
to 5 before dispatch, then positive grace decrements; processing clears deferral.
Dead script processing clears the latch. Eligible phases zero command pauses.
An idle current action then pulls the next queued action and clears interruption.
Constants 0x8485D0/E0/E2 are BYTE 16 and SHORT 120/5.

The common action-resolution dialogue gate now uses the original evil cutoff
and ignore-dialog-pause flag, matching 0x72DADD onward; the earlier source used
the good cutoff and omitted the flag. ProcessAI still has other incomplete
branches and is not declared fully recovered.

AutoUI scriptschedule installs an in-memory override script with an empty
condition and recognizable WAIT(9876) response, restoring all seven script
pointers before the next engine tick. It observes a real selected response,
not just a clean crash guard. Cases cover wrapper gates, phase, each low flag,
an unrelated flag, visible/offscreen selection, deferral limit, grace, pending
trigger refresh, both command pauses and both dead-latch paths. Visibility
positions exist only during synchronous calls and are restored. The fixture
uses a disposable process: actor fields are reset but not every AI side effect
is snapshotted. No save is written; profile save/config preservation is transactional.

Static parity, runtime branch coverage and original/recovered differential
remain separate evidence. These tests do not establish all script contents or
full ProcessAI equivalence.

Validation: .arc/2026-10-03T22-05-00-398602-batch, PASS with one warning.
All eight source checks, class/layout audits and Win32 build pass. Runtime
CLEAN for the 60-second hold, ProcessPendingTriggers hit 12 times and ProcessAI
87782 times. All 20 script-response/counter checks and eight UI steps pass.
The visible, maximum-deferral, grace, trigger and final-dead cases observe the
fixture response; off-phase, flags, pauses and wrapper guards suppress it.
Profile cleanup restores and hashes preserved save/config paths.

ProcessPendingTriggers parity GREEN. ProcessAI remains YELLOW: 191 original /
157 detected source calls, with its existing floating-point-sensitive-ops
warning. Offset analysis cannot analyze either target; these checks remain
unproven static evidence rather than successful offset audits.

Initial .arc/2026-10-03T21-59-48-919455-batch stopped at three test-only
compile errors: CAITrigger has no default constructor. The fixture now uses
CAITrigger(CAITRIGGER_NO_TRIGGER, 0); the complete validation was rerun.

Next recovery target: common action resolution in ProcessAI, especially its
remaining incapacity/time-stop gates and paused execution paths.
