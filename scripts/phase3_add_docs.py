#!/usr/bin/env python3
"""
Phase 3: Add header documentation to key source files.
Generates class documentation comments based on file analysis.
"""
import os, re

SRC = "/home/wills/projects/IWD2-RE/iwd2-re/src"

# Key classes to document (most important for understanding the engine)
KEY_CLASSES = {
    # Core engine
    'CChitin': 'Main application framework - message loop, window management, core timer',
    'CBaldurChitin': 'Baldur\'s Gate / Icewind Dale specific application singleton',
    'CInfinity': '2D world rendering engine - tile maps, sprites, lighting',
    'CInfGame': 'Game state singleton - party, areas, journal, saves',
    
    # Game objects
    'CGameObject': 'Base class for all game world objects',
    'CGameAIBase': 'AI base - action queues, scripts, triggers for interactive objects',
    'CGameSprite': 'Living entities - characters, NPCs, monsters (most complex class)',
    'CGameArea': 'Area/level controller - objects, animations, sounds',
    'CGameDoor': 'Interactive door with open/close animations',
    'CGameTrigger': 'Script trigger regions - OnEnter, OnExit events',
    'CGameContainer': 'Lootable chests, barrels, corpses',
    
    # AI/Scripting
    'CAIAction': 'Script action definition ( MoveToObject, Attack, etc.)',
    'CAIScript': 'Script file - condition=action pairs',
    'CAITrigger': 'Script trigger events (OnCreation, OnDeath, etc.)',
    'CAIObjectType': 'Object filter/matcher for script targeting',
    
    # UI Screens
    'CScreenWorld': 'Main gameplay screen - world exploration and combat',
    'CScreenCharacter': 'Character sheet - stats, inventory, spells',
    'CScreenCreateChar': 'Character creation - race, class, abilities',
    'CScreenInventory': 'Inventory management screen',
    'CScreenSpellbook': 'Spell memorization and casting',
    'CScreenJournal': 'Quest log and player notes',
    'CScreenMap': 'Auto-map and world travel',
    'CScreenConnection': 'Multiplayer connection screen',
    'CScreenOptions': 'Game settings and options',
    
    # Resources
    'CRes': 'Base resource class - .ARE, .CRE, .ITM files',
    'CDimm': 'Resource manager - loads and caches game assets',
    'CResRef': '8-byte resource name reference (filename without extension)',
    
    # Systems
    'CMessage': 'Inter-object message system - effects, damage, movement',
    'CGameEffect': 'Magical/physical effects - buffs, debuffs, damage over time',
    'CDerivedStats': 'Calculated stats - AC, THAC0, saves with modifiers',
    'CGameSpriteSpells': 'Spell lists - memorized, innate, bard songs, shapesifts',
}

def add_documentation(filepath, class_name, description):
    """Add documentation comment after #ifndef guard"""
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Check if already documented
    if 'Address:' in content or 'Key systems:' in content:
        return False, 'already documented'
    
    # Find #ifndef line
    lines = content.split('\n')
    guard_line = -1
    include_line = -1
    
    for i, line in enumerate(lines):
        if line.startswith('#ifndef'):
            guard_line = i
        elif line.startswith('#include'):
            include_line = i
            break
    
    if guard_line == -1:
        return False, 'no #ifndef found'
    
    # Build documentation comment
    doc = f'''// ============================================================================
// {class_name} - {description}
//
// Address: varies by method
// ============================================================================
'''
    
    # Insert before first #include
    if include_line > guard_line:
        lines.insert(include_line, doc.rstrip())
    else:
        lines.insert(guard_line + 1, doc.rstrip())
    
    new_content = '\n'.join(lines)
    
    with open(filepath, 'w') as f:
        f.write(new_content)
    
    return True, 'documented'

def main():
    documented = 0
    skipped = 0
    
    for class_name, desc in KEY_CLASSES.items():
        filename = f"{class_name}.h"
        filepath = os.path.join(SRC, filename)
        
        if not os.path.exists(filepath):
            # Try with lowercase first letter
            filename = class_name[0].lower() + class_name[1:] + '.h'
            filepath = os.path.join(SRC, filename)
        
        if os.path.exists(filepath):
            success, reason = add_documentation(filepath, class_name, desc)
            if success:
                print(f"  ✓ {class_name}.h")
                documented += 1
            else:
                print(f"  ⚠ {class_name}.h ({reason})")
                skipped += 1
        else:
            print(f"  ✗ {class_name}.h not found")
    
    print(f"\nDone: {documented} documented, {skipped} skipped")

if __name__ == '__main__':
    print("=" * 60)
    print("Phase 3: Adding header documentation")
    print("=" * 60)
    main()
