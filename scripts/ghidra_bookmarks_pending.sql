-- Pending bookmarks for speculative analysis results
-- Run when GhidraSQL is restarted fresh

INSERT INTO bookmarks (address, type, category, comment) VALUES (0x405090, 'NOTE', 'ANALYSIS', 'Copies fields between structs — likely CAIAction copy/assign operator.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x406170, 'NOTE', 'ANALYSIS', 'Iterates list, calls GetCharacterPortraitNum, inserts in sorted order — likely CAIGroup::AddMember.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x40A8F0, 'NOTE', 'ANALYSIS', 'Calls CAIIdList::LoadList, CDimm::GetResObject — loads AI ID list from resource.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x40FCD0, 'NOTE', 'ANALYSIS', 'Allocates objects, creates CAIScript — initialization/constructor pattern.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x40AC60, 'NOTE', 'ANALYSIS', 'Calls FUN_0040ad00 with many params — small wrapper/init.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x45C030, 'NOTE', 'ANALYSIS', 'Gets object, checks immunity types — likely CGameAIBase::GetValidTarget.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x45C290, 'NOTE', 'ANALYSIS', 'Gets object, simple immunity check — likely CGameAIBase::GetTargetObject.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x45B890, 'NOTE', 'ANALYSIS', 'Initializes struct with params — looks like CAIActionMsg constructor.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x44DAC0, 'NOTE', 'ANALYSIS', 'Checks area, LOS, distance to target — likely CGameAIBase::IsTargetReachable.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x405370, 'NOTE', 'ANALYSIS', 'Allocates array, iterates group members, gets positions — possible CAIGroup::GetFormationPositions.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x4088C0, 'NOTE', 'ANALYSIS', 'Huge stack frame, calls multiple sub-funcs — possible CAIGroup::FormationMove.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x405A10, 'NOTE', 'ANALYSIS', 'Processes group target/action — possible CAIGroup::ProcessGroupTarget.');
INSERT INTO bookmarks (address, type, category, comment) VALUES (0x766EF0, 'NOTE', 'ANALYSIS', 'Sets up recticle desc, calls DrawRecticle — likely CMarker::Render destination overload.');

SELECT save_database();
