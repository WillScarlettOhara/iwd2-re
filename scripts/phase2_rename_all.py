#!/usr/bin/env python3
"""
Phase 2: Bulk rename all remaining sub_ functions across all files.
Covers IcewindMisc, CInfGame, CGameEffect, CVidCell, and remaining files.
"""
import json, subprocess, sys, time

TOKEN = "iwd2ee-re-tools-2026"
BASE = "http://127.0.0.1:8089"

def gh_post(path, data):
    r = subprocess.run(["curl", "-sf", "-X", "POST", "-m", "30",
        "-H", f"Authorization: Bearer {TOKEN}",
        "-H", "Content-Type: application/json",
        "-d", json.dumps(data), f"{BASE}{path}"],
        capture_output=True, text=True)
    return r.stdout.strip()

ALL_RENAMES = [
    # === IcewindMisc ===
    ("0x00584610", "sub_584610", "DirectionToScreenOffset", "Returns x/y offsets by direction index"),
    ("0x005847B0", "sub_5847B0", "ComputeDistanceVector", "Distance/vector between two points"),
    ("0x00584880", "sub_584880", "RenderAreaEffect", "Renders area effect graphics with stack buffer"),
    ("0x00585210", "sub_585210", "IsCreatureAlive", "Checks if creature is alive (animation < 7)"),
    ("0x00585230", "sub_585230", "AreCreaturesSameType", "Compares animation type of two creatures"),
    ("0x005852A0", "sub_5852A0", "IsOneBehindOther", "Checks relative z-order by animation state"),
    ("0x00585310", "sub_585310", "IsLargeCreature", "Checks if creature is large size category"),
    ("0x00585380", "sub_585380", "ApplyEffectMajor", "Effect application with flag 0x400000"),
    ("0x005853B0", "sub_5853B0", "ApplyEffectMinor", "Effect application with flag 0x80000"),
    ("0x005853E0", "sub_5853E0", "ApplyEffectAuxiliary", "Effect application with flag 0x20000"),
    ("0x00585410", "sub_585410", "ApplyEffectSilent", "Effect application with flag 0 (no feedback)"),
    ("0x00585D90", "sub_585D90", "GetAnimationSlotCount", "Returns constant 0x15 (21 animation slots)"),
    ("0x00585DA0", "sub_585DA0", "CanInitiateCombat", "Checks if combat can be initiated vs target"),
    ("0x005860F0", "sub_5860F0", "ApplyWeaponEffectDamage", "Applies effect damage based on weapon attack type"),

    # === CInfGame ===
    ("0x0059FA00", "sub_59FA00", "CInfGame::WaitForPlayersReady", "Blocks until all players signal ready"),
    ("0x005A0160", "sub_5A0160", "CInfGame::ProcessGameLoop", "Main game loop processing"),
    ("0x005A9780", "sub_5A9780", "CInfGame::MapVirtualKeyToChar", "Maps virtual key code to character"),
    ("0x005A97D0", "sub_5A97D0", "CInfGame::MapKeyToLowercase", "Maps key to lowercase ASCII"),
    ("0x005A9830", "sub_5A9830", "CInfGame::ValidateKeyBindingIndex", "Bounds check key binding index"),
    ("0x005AC0D0", "sub_5AC0D0", "CInfGame::CheckAnyCharacterActive", "Checks if any party character is active"),
    ("0x005AF420", "sub_5AF420", "CInfGame::SetActiveCharacter", "Sets active/selected character"),
    ("0x005BDBA0", "sub_5BDBA0", "CInfGame::SetScrollSpeed", "Sets view scroll speed with bounds"),
    ("0x005C0B30", "sub_5C0B30", "CInfGame::ProcessStringCommand", "Processes string command with formatting"),
    ("0x005C79C0", "sub_5C79C0", "CInfGame::ParseGameDataString", "Parses game data from string buffer"),
    ("0x005C93E0", "sub_5C93E0", "CInfGame::IteratePartyCharacters", "Iterates through party character list"),
    ("0x005CADF0", "sub_5CADF0", "CInfGame::ProcessPartyTurn", "Processes one turn for all party members"),
    ("0x005C3770", "sub_5C3770", "CInfGame::InitializeGameTimer", "Initializes/resets game timer"),
    ("0x005D2DE0", "sub_5D2DE0", "CInfGame::ProcessTileAnimations", "Iterates through animated tiles"),

    # === CGameEffect ===
    ("0x00493400", "sub_493400", "CGameEffect::SetFlagTwo", "Sets/clears flag bit 2 at offset 0x58"),
    ("0x0049DB60", "sub_49DB60", "CGameEffectSummon::SetSummonParam", "Summon parameter setter at offset 400"),
    ("0x004A7750", "sub_4A7750", "CGameEffectDamage::RemoveHoldEffect", "Clears disease/hold flags and effects"),
    ("0x004B2680", "sub_4B2680", "CGameEffectStun::CanStunTarget", "Checks if stun can be applied to target"),
    ("0x004B5BF0", "sub_4B5BF0", "CGameEffectDisease::ApplyDisease", "Applies disease effect to target"),
    ("0x004B5D90", "sub_4B5D90", "CGameEffectDisease::RemoveDisease", "Removes disease effect"),
    ("0x004B5E50", "sub_4B5E50", "CGameEffectDisease::ProcessDiseaseTick", "Processes disease damage tick"),
    ("0x004B5FF0", "sub_4B5FF0", "CGameEffectDisease::CanSpreadDisease", "Checks if disease can spread to nearby"),
    ("0x004B8730", "sub_4B8730", "CGameEffect::ApplyToTarget", "Applies effect to a target creature"),
    ("0x004C3F30", "sub_4C3F30", "CGameEffect::ClampMaxValue", "Clamps value at offset 0x928"),

    # === CVidCell (rendering functions) ===
    ("0x007AF1C0", "sub_7AF1C0", "CVidCell::BlitOpaque", "Opaque pixel blit with palette"),
    ("0x007AF8A0", "sub_7AF8A0", "CVidCell::BlitAlphaKey", "Alpha-keyed blit (transparency)"),
    ("0x007AFB50", "sub_7AFB50", "CVidCell::BlitShaded", "Shaded/tinted blit with color modulation"),
    ("0x007AFED0", "sub_7AFED0", "CVidCell::BlitWithHighlight", "Highlight blit with secondary color"),
    ("0x007B49D0", "sub_7B49D0", "CVidCell::BlitMirrored", "Mirrored/flipped blit"),
    ("0x007CECD0", "sub_7CECD0", "CVidCell::BlitScaledX", "Horizontal scaled blit"),
    ("0x007CF3F0", "sub_7CF3F0", "CVidCell::BlitAdditive", "Additive blend blit"),
    ("0x007CF650", "sub_7CF650", "CVidCell::BlitSubtractive", "Subtractive/darken blit"),
    ("0x007CF8D0", "sub_7CF8D0", "CVidCell::BlitStencil", "Stencil/masked blit"),
    ("0x007D0950", "sub_7D0950", "CVidCell::BlitGradient", "Gradient fill blit"),
    ("0x007D0C60", "sub_7D0C60", "CVidCell::BlitOutline", "Outlined/drop-shadow blit"),
    ("0x007D1190", "sub_7D1190", "CVidCell::BlitComposite", "Composite/multi-layer blit"),

    # === Remaining sub_ functions ===
    # CAIObjectType
    ("0x0040CB20", "sub_40CB20", "CAIObjectType::FindTarget", "Finds target matching object type criteria"),
    ("0x0040CCA0", "sub_40CCA0", "CAIObjectType::GetBestTarget", "Selects best target among candidates"),
    ("0x0040CD80", "sub_40CD80", "CAIObjectType::EvaluateTarget", "Evaluates target suitability score"),
    ("0x0040CED0", "sub_40CED0", "CAIObjectType::GetTargetRange", "Computes targeting range"),
    ("0x0040D050", "sub_40D050", "CAIObjectType::IsValidTarget", "Validates target meets type requirements"),
    ("0x0040D0F0", "sub_40D0F0", "CAIObjectType::SortTargetsByPriority", "Prioritizes target list"),
    ("0x0040D900", "sub_40D900", "CAIObjectType::GetTypeBitmask", "Returns object type bitmask"),
    ("0x0040D920", "sub_40D920", "CAIObjectType::CheckTypeConflict", "Checks for type conflict/incompatibility"),
    
    # CAIResponse
    ("0x0040D8A0", "sub_40D8A0", "CAIResponse::ExecuteResponse", "Executes AI response action"),
    
    # CBaldurProjector
    ("0x0043E300", "sub_43E300", "CBaldurProjector::RenderFrame", "Renders one frame to projector surface"),
    
    # CImm
    ("0x004D89E0", "sub_4D89E0", "CImm::SetInputMode", "Sets IMM32 input mode"),
    ("0x007C2680", "sub_7C2680", "CImm::ProcessComposition", "Processes IME composition string"),
    ("0x007C27D0", "sub_7C27D0", "CImm::GetCandidateList", "Gets IME candidate list"),
    ("0x007C2CC0", "sub_7C2CC0", "CImm::SelectCandidate", "Selects IME candidate"),
    ("0x007C2E10", "sub_7C2E10", "CImm::CommitComposition", "Commits IME composition to text"),
    ("0x007C2ED0", "sub_7C2ED0", "CImm::TranslateCharMessage", "Translates WM_CHAR for IME"),
    ("0x007C3020", "sub_7C3020", "CImm::HandleImmMessage", "Handles IME-related window messages"),
    ("0x007C3140", "sub_7C3140", "CImm::InitializeImmContext", "Initializes IME context for window"),
    
    # CMultiplayerSettings
    ("0x00518560", "sub_518560", "CMultiplayerSettings::GetPermissionLevel", "Gets player permission level"),
    ("0x00518580", "sub_518580", "CMultiplayerSettings::SetPermissionLevel", "Sets player permission"),
    ("0x00518650", "sub_518650", "CMultiplayerSettings::CanPause", "Checks if player can pause"),
    ("0x00518660", "sub_518660", "CMultiplayerSettings::CanSave", "Checks save permission"),
    ("0x00518690", "sub_518690", "CMultiplayerSettings::CanControlCharacter", "Checks character control rights"),
    ("0x005186A0", "sub_5186A0", "CMultiplayerSettings::IsHostPlayer", "Checks if player is host"),
    
    # CScreenCharacter
    ("0x005D8B50", "sub_5D8B50", "CScreenCharacter::DrawCharacterScreen", "Renders character screen UI"),
    ("0x005E4560", "sub_5E4560", "CScreenCharacter::OnCreateCharButton", "Handles create character button"),
    ("0x005E4A20", "sub_5E4A20", "CScreenCharacter::OnAcceptCharacter", "Confirms character creation"),
    ("0x005E4B30", "sub_5E4B30", "CScreenCharacter::OnRejectCharacter", "Cancels character creation"),
    ("0x005F89B0", "sub_5F89B0", "CScreenCharacter::UpdatePortrait", "Updates character portrait"),
    ("0x005F8C90", "sub_5F8C90", "CScreenCharacter::DrawStatPanel", "Renders attribute/skill panel"),
    ("0x005F91F0", "sub_5F91F0", "CScreenCharacter::ValidateCharacter", "Validates character before save"),
    
    # CScreenStore
    ("0x006734F0", "sub_6734F0", "CScreenStore::ProcessBuyRequest", "Processes item purchase"),
    ("0x00673740", "sub_673740", "CScreenStore::ProcessSellRequest", "Processes item sale"),
    ("0x0067A130", "sub_67A130", "CScreenStore::DrawStoreInventory", "Renders store inventory list"),
    ("0x0067A5B0", "sub_67A5B0", "CScreenStore::DrawPartyInventory", "Renders party inventory list"),
    ("0x0067ABF0", "sub_67ABF0", "CScreenStore::HandleItemTransfer", "Transfers item between inventories"),
    ("0x0067B030", "sub_67B030", "CScreenStore::UpdateGoldDisplay", "Updates gold amount display"),
    
    # CScreenWorld
    ("0x00453780", "sub_453780", "CScreenWorld::GetAreaFlag", "Gets area configuration flag"),
    ("0x00453790", "sub_453790", "CScreenWorld::SetAreaFlag", "Sets area configuration flag"),
    ("0x00693710", "sub_693710", "CScreenWorld::ProcessWorldClick", "Processes mouse click on world"),
    ("0x00695570", "sub_695570", "CScreenWorld::UpdateFogOfWar", "Updates fog of war visibility"),
    ("0x00695650", "sub_695650", "CScreenWorld::DrawAreaMap", "Renders area map overlay"),
    
    # CScreenKeymaps
    ("0x00639E70", "sub_639E70", "CScreenKeymaps::MapKeyToAction", "Maps key stroke to game action"),
    ("0x0063A660", "sub_63A660", "CScreenKeymaps::SaveKeyBindings", "Saves key bindings to config"),
    ("0x0063A7A0", "sub_63A7A0", "CScreenKeymaps::LoadKeyBindings", "Loads key bindings from config"),
    
    # CScreenLoad
    ("0x0063E150", "sub_63E150", "CScreenLoad::EnumerateSaves", "Lists available save games"),
    ("0x0063E440", "sub_63E440", "CScreenLoad::LoadSaveGame", "Loads selected save game"),
    
    # CScreenSpellbook
    ("0x00669830", "sub_669830", "CScreenSpellbook::DrawSpellPage", "Renders spellbook page"),
    ("0x0066A010", "sub_66A010", "CScreenSpellbook::OnMemorizeSpell", "Handles spell memorization click"),
    
    # CScreenStart
    ("0x0066F990", "sub_66F990", "CScreenStart::DrawMainMenu", "Renders main menu screen"),
    ("0x006702C0", "sub_6702C0", "CScreenStart::OnStartGame", "Initiates new game start"),
    
    # CScreenConnection
    ("0x005FEB70", "sub_5FEB70", "CScreenConnection::ConnectToServer", "Establishes multiplayer connection"),
    
    # CScreenCreateChar
    ("0x00617D80", "sub_617D80", "CScreenCreateChar::DrawCreationScreen", "Renders character creation UI"),
    
    # CScreenWorldMap
    ("0x0069CB40", "sub_69CB40", "CScreenWorldMap::DrawTravelMap", "Renders world travel map"),
    
    # CGameArea
    ("0x0046DAE0", "sub_46DAE0", "CGameArea::LoadAreaResources", "Loads area map/resources from file"),
    ("0x0047A060", "sub_47A060", "CGameArea::ProcessAreaObjects", "Processes area object updates"),
    ("0x0047A190", "sub_47A190", "CGameArea::GetAreaObjectCount", "Returns count of area objects"),
    
    # CGameAreaNotes
    ("0x0047B140", "sub_47B140", "CGameAreaNotes::GetNoteText", "Gets area note text by index (was Getfield_8D)"),
    ("0x0047B150", "sub_47B150", "CGameAreaNotes::AddAreaNote", "Adds annotation to area map"),
    
    # CGameContainer
    ("0x00480480", "sub_480480", "CGameContainer::TransferItemToParty", "Moves item from container to party"),
    
    # CGameAIBase
    ("0x0045B6D0", "sub_45B6D0", "CGameAIBase::GetScriptIndex", "Gets current script index (was Getfield_58C)"),
    ("0x0045B6E0", "sub_45B6E0", "CGameAIBase::UnimplementedStub1", "Empty stub function"),
    ("0x00467970", "sub_467970", "CGameAIBase::ExecuteScript", "Executes AI script"),
    ("0x00481890", "sub_481890", "CGameAIBase::EvaluateCondition", "Evaluates AI trigger condition"),
    
    # CGameDoor
    ("0x0045B700", "sub_45B700", "CGameDoor::GetOpenState", "Checks door open/closed state"),
    ("0x0045B950", "sub_45B950", "CGameDoor::ToggleDoor", "Opens or closes door"),
    
    # CGameTrigger
    ("0x0045B930", "sub_45B930", "CGameTrigger::ActivateTrigger", "Activates area trigger"),
    
    # CGameJournal
    ("0x004C7600", "sub_4C7600", "CGameJournal::AddJournalEntry", "Adds entry to player journal"),
    
    # CGameSpy
    ("0x004D1EC0", "sub_4D1EC0", "CGameSpy::AdvertiseGame", "Advertises game on GameSpy lobby"),
    ("0x004D2060", "sub_4D2060", "CGameSpy::ListAvailableGames", "Queries GameSpy for game list"),
    ("0x004D2210", "sub_4D2210", "CGameSpy::JoinGame", "Joins advertised multiplayer game"),
    ("0x004D2310", "sub_4D2310", "CGameSpy::LeaveLobby", "Disconnects from GameSpy lobby"),
    
    # CUIPanel
    ("0x004D2D20", "sub_4D2D20", "CUIPanel::LoadPanelLayout", "Loads UI panel layout from resource"),
    ("0x004D2D50", "sub_4D2D50", "CUIPanel::UnloadPanel", "Unloads UI panel resources"),
    
    # CItem
    ("0x004EA580", "sub_4EA580", "CItem::GetItemAbility", "Gets item ability by index"),
    
    # CRuleTables
    ("0x00542560", "sub_542560", "CRuleTables::GetClassTable", "Looks up class progression table"),
    ("0x00546B60", "sub_546B60", "CRuleTables::GetRaceTable", "Looks up racial ability table"),
    
    # CVisibility
    ("0x005518A0", "sub_5518A0", "CVisibilityMap::ClearVisibility", "Clears visibility map"),
    ("0x00553440", "sub_553440", "CVisibilityMap::ComputeLineOfSight", "Ray-traces line of sight"),
    ("0x005535D0", "sub_5535D0", "CVisibilityMap::UpdateVisibility", "Updates visibility for all creatures"),
    ("0x00553800", "sub_553800", "CVisibilityMap::IsVisible", "Checks if point is visible"),
    ("0x00553990", "sub_553990", "CVisibilityMap::GetVisibleArea", "Computes visible area radius"),
    
    # CWorldMap
    ("0x0055A3A0", "sub_55A3A0", "CWorldMap::LoadMapImage", "Loads world map bitmap"),
    ("0x0055A450", "sub_55A450", "CWorldMap::GetAreaPosition", "Gets area position on world map"),
    
    # IcewindCGameEffect
    ("0x0055ED90", "sub_55ED90", "CGameEffectSavingThrowChange::ApplyEffect", "Applies saving throw modifier"),
    ("0x0055EE70", "sub_55EE70", "CGameEffectSavingThrowChange::RemoveEffect", "Removes saving throw modifier"),
    ("0x0055EFB0", "sub_55EFB0", "CGameEffectSavingThrowChange::GetSaveModifier", "Gets saving throw modification value"),
    
    # IcewindCVisualEffect
    ("0x00586A60", "sub_586A60", "IcewindCVisualEffect::PlayEffect", "Triggers visual effect animation"),
    ("0x00586A90", "sub_586A90", "IcewindCVisualEffect::StopEffect", "Stops visual effect"),
    ("0x00586AC0", "sub_586AC0", "IcewindCVisualEffect::IsEffectPlaying", "Checks if effect is active"),
    
    # Icewind586B70
    ("0x00586CA0", "sub_586CA0", "Icewind586B70::InitializeTracker", "Initializes observer tracker"),
    ("0x00586CF0", "sub_586CF0", "Icewind586B70::AddObserver", "Registers observer"),
    ("0x00586D60", "sub_586D60", "Icewind586B70::RemoveObserver", "Removes observer"),
    ("0x00586F20", "sub_586F20", "Icewind586B70::NotifyObservers", "Notifies all registered observers"),
    ("0x00586FC0", "sub_586FC0", "Icewind586B70::ProcessObservationChange", "Processes state change"),
    ("0x00587190", "sub_587190", "Icewind586B70::GetObservationState", "Gets current observation state"),
    
    # CDerivedStats
    ("0x00447B00", "sub_447B00", "CDerivedStats::CalculateStats", "Calculates derived creature stats"),
    
    # CNetwork
    ("0x007A61D0", "sub_7A61D0", "CNetwork::SendPacket", "Sends network packet"),
    ("0x007A73D0", "sub_7A73D0", "CNetwork::ReceivePacket", "Receives network packet"),
    
    # CUnknown2
    ("0x00799C90", "sub_799C90", "CUnknown2::EmptyMethod", "Empty/placeholder method"),
    ("0x007C25B0", "sub_7C25B0", "CUnknown2::SubInitialize", "Second initializer method"),
    
    # CSoundMixer
    ("0x007ACA10", "sub_7ACA10", "CSoundMixer::SetVolume", "Sets channel volume"),
    ("0x007ACA30", "sub_7ACA30", "CSoundMixer::GetChannelCount", "Returns active channel count"),
    
    # CGameSprite remaining
    ("0x0054C1D0", "sub_54C1D0", "CGameSprite::SaveSpriteData", "Serializes sprite data"),
    ("0x0054C230", "sub_54C230", "CGameSprite::LoadSpriteData", "Deserializes sprite data"),
    ("0x0054C460", "sub_54C460", "CGameSprite::SaveCreatureFile", "Saves CRE file"),
    ("0x0054C5B0", "sub_54C5B0", "CGameSprite::EmptySaveStub", "Empty save-related stub"),
    ("0x0054C5F0", "sub_54C5F0", "CGameSprite::LoadCreatureFile", "Loads CRE file"),
    ("0x0054C770", "sub_54C770", "CGameSprite::ApplyTemplate", "Applies creature template"),
    ("0x0054CA80", "sub_54CA80", "CGameSprite::GetCreatureName", "Gets creature display name"),
    ("0x0054CAF0", "sub_54CAF0", "CGameSprite::SetCreatureName", "Sets creature display name"),
    ("0x0054CCB0", "sub_54CCB0", "CGameSprite::GenerateCreature", "Generates creature from template"),
    ("0x0054CF80", "sub_54CF80", "CGameSprite::GetCreatureAlignment", "Gets creature alignment"),
    ("0x0054CFE0", "sub_54CFE0", "CGameSprite::SetCreatureAlignment", "Sets creature alignment"),
    ("0x0054D130", "sub_54D130", "CGameSprite::GetCreatureClass", "Gets creature class info"),

    # CVariableHash
    ("0x00550B30", "sub_550B30", "CVariableHash::InsertEntry", "Inserts hash entry"),
    ("0x00550CF0", "sub_550CF0", "CVariableHash::LookupEntry", "Looks up entry by key"),
    
    # CUIControlButtonMapAreaMap
    ("0x00642C90", "sub_642C90", "CUIControlButtonMapAreaMap::DrawButton", "Renders map area button"),
    
    # CScreenMap
    ("0x005CA160", "sub_5CA160", "CScreenMap::GetMapMarker", "Gets map marker by identifier"),
    
    # CScreenMovies
    ("0x0062E150", "sub_62E150", "CScreenMovies::PlayMovie", "Plays cinematic movie"),
    
    # CScreenOptions
    ("0x0063B040", "sub_63B040", "CScreenOptions::ApplySettings", "Applies option changes"),
    
    # CScreenSave
    ("0x0063D240", "sub_63D240", "CScreenSave::SaveGame", "Performs save game operation"),
]

print(f"Applying {len(ALL_RENAMES)} renames to Ghidra...\n")

success = 0
failed_list = []
mapping = {}

for i, (addr_hex, old_name, new_name, rationale) in enumerate(ALL_RENAMES):
    sys.stdout.write(f"\r  [{i+1}/{len(ALL_RENAMES)}] {old_name} -> {new_name.split('::')[-1]}  ")
    sys.stdout.flush()
    
    resp = gh_post("/rename_function_by_address", {
        "function_address": addr_hex,
        "new_name": new_name
    })
    
    try:
        data = json.loads(resp) if resp.startswith('{') else resp
        if isinstance(data, dict) and (data.get("success") or "result" in str(data)):
            success += 1
            mapping[old_name] = new_name
        else:
            failed_list.append((addr_hex, old_name, str(resp)[:100]))
    except:
        failed_list.append((addr_hex, old_name, resp[:100]))
    
    time.sleep(0.1)

print(f"\n\nDone! Applied: {success}, Failed: {len(failed_list)}, Total: {len(ALL_RENAMES)}")

if failed_list:
    print("\nFailed renames:")
    for addr, name, reason in failed_list:
        print(f"  {addr} {name}: {reason}")

# Save mapping
with open('/tmp/all_rename_mapping.json', 'w') as f:
    json.dump({'success': success, 'failed': len(failed_list), 'total': len(ALL_RENAMES), 'mapping': mapping}, f, indent=2)

# Category stats
cats = {}
for _, old, new, rationale in ALL_RENAMES:
    if '::' in new:
        cls = new.split('::')[0]
    else:
        cls = new.split('::')[0]
    cats[cls] = cats.get(cls, 0) + 1

print(f"\n--- Per-class rename counts ---")
for cls, count in sorted(cats.items(), key=lambda x: -x[1]):
    print(f"  {cls}: {count}")

print(f"\nMapping saved to /tmp/all_rename_mapping.json")
