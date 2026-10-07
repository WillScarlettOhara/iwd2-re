# SetCurrAction command-pause marker

Recovered 0x734037..0x734056 in SetCurrAction (0x7338E0), using its
previously read context bundle and PE instructions. The signed SHORT
m_userCommandPause (+0x5576) must be strictly greater than 5*15-2 (73).
Then write the BYTE timescale constant at 0x84EC0C (15) to sprite+0x564C:
m_marker.m_nRecticleForceRender, not the adjacent force-target field.
The gate follows idle-sequence setup and precedes movement feedback.
NO_ACTION has already returned and does not update the marker.

Sixteen typed cases cover WAIT and NO_ACTION with pauses -32768, -1, 0,
72, 73, 74, 75 and 32767. The marker starts at a nonzero sentinel (99),
so threshold and idle cases must retain rather than clear it. Cases also
check unchanged pause, marker type/reference count, force-target byte and
destination marker force byte. Existing target isolation prevents target
release from interfering. Original pause and marker byte are restored.
The earlier 320 assertions remain; the scenario writes no save.

Validation: full batch 2026-10-07T14-17-04-380144-batch passes with one
call-count warning (SetCurrAction 77/41, still partial). Eight shared lints,
class/layout audits and Win32 Debug build pass. All 336 typed assertions
and 11 scenario steps pass; the game holds 60 seconds CLEAN with 39,782
SetCurrAction entries. Fixtures are removed and preserved paths restored
and hashed. Static, observed-runtime and differential evidence remain separate.

Next target: spell-action resource setup beginning at 0x7340BD. This does
not establish full SetCurrAction parity or a visual marker differential.
