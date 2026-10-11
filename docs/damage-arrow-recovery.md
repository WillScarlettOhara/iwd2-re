# Offscreen party damage locators

2026-10-11. Recover CGameSprite::RenderDamageArrow 708FC0, formerly empty.
No ABI/member/layout change. Existing CGameArea::Render calls it for party
portraits after world rendering. This selects a screen-edge locator, not a
floating damage number.

## Evidence

Full tmp_damage_arrow_ctx.md: 168-line decompile, required calls, full sprite
header and BG2 layout. Read complete reachable sym.py assembly 708FC0-7093CC
(RET 12), jump table 7093D0, and SHORT flash period 85C19C (20). The decompile
loses this/stack arguments and confuses rectangles; assembly is authoritative.

- Read feedback level at CInfGame 43EA+14 before portrait lookup. Nonparty exits.
  Levels outside 2..5 exit. Level 2 needs blood flash, nonzero signed time and
  signed counter%20 >= 10, then falls through. Level 3 requires that flash or
  the area's picked ID. Levels 4/5 bypass the blink/pick condition.
- Require all three active BOOLs, exact stealth byte !=1, derived STATE_DEAD
  clear, same area. These checks precede animation access.
- World view uses nCurrentX/Y and viewport width/height. Copy position, add
  area height offset to Y, calculate FX/reference and animation GC bounds with
  posZ. Any nonempty rectangle intersection suppresses the locator.
- Anchor X uses adjusted position; direction Y uses adjusted Y+posZ. Strict
  outside comparisons preserve equality. Sequences: E0, NE1, N2, NW3, W4,
  SW5, S6, SE7. Corners pin both viewport edges; N/S retain world-relative X;
  E/W retain adjusted Y without posZ. No outside anchor means no render.
- Shared m_vcLocator gets sequence, frame zero, faded damage color only when
  blood flash is truthy and time positive; otherwise marker color. Render uses
  supplied surface, viewport clip, NULL polygon, zero count, 20000 flags, -1
  transparency. Fade uses the global current video mode; passed video is unused.

Existing typed animation wrappers reproduce inlined calls. No raw prototypes,
new dependencies or invented damage/UI behavior.

## Native scenario

damagearrowchecks backs up real software back-surface rows, temporarily gives
the real party actor a small interior viewport, and compares the entire pixel
patch against independent fixed-anchor BAM renders. Cover eight directions,
strict boundaries, Z/height asymmetry, clipped corners, partly visible body,
all feedback levels, signed phase/time, truthy BOOLs, exact stealth byte, death,
party and area guards, plus marker/damage fade. Restore actor, camera, options,
counter, locator selection/tint and sampled pixels before returning. Verify
balanced video locks, no posted messages and unchanged world clock. The save
is disposable and centered before the scenario; all actual HP checks remain.

Native batch .arc/2026-10-11T10-35-38-708217-batch PASS: eight source checks,
zero class findings, 310 compiled member spacings, Win32 build, 49 UI steps,
135 locator assertions, CLEAN 60s and 8972 target hits. Fixture cleanup restored
preserved paths. Real HP regressions remain melee -6, bow/bolt/sling -12 each,
received/direct on-hit source -1/defender -2, effect packets -3/-10, projectiles -3.
Parity YELLOW: 12 binary calls vs 19 source calls (typed getters/rectangle and
animation wrappers). arg_provenance has no operator+ sites. parity_offsets
cannot analyse this header, so static evidence remains incomplete; full assembly
and compiled layout are recorded above. No original/recovered differential claim.
Next: complete HandleEffects 734550, including the status/portrait refresh pass.
