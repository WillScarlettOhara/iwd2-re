// Native thrown-leaf/factory and DWORD immunity capture; base/resources isolated.
// MD5: 25cb3d8aa2ec648e94e328d8c467973e
#ifndef THROWN_WEAPON_GOLDEN_H_
#define THROWN_WEAPON_GOLDEN_H_
struct ThrownWeaponFactoryGolden { USHORT type; BYTE delay; const char* cell; SHORT speed; BOOL colored; };
static const ThrownWeaponFactoryGolden THROWN_WEAPON_FACTORY[] = {
    {7,0,"AXE",40,0},
    {7,255,"AXE",40,0},
    {10,0,"AXE",40,0},
    {10,255,"AXE",40,0},
    {11,0,"AXE",40,0},
    {11,255,"AXE",40,0},
    {27,0,"DAGGER",40,1},
    {27,255,"DAGGER",40,1},
    {30,0,"DAGGER",40,1},
    {30,255,"DAGGER",40,1},
    {31,0,"DAGGER",40,1},
    {31,255,"DAGGER",40,1},
    {56,0,"SPEAR",40,0},
    {56,255,"SPEAR",40,0},
    {59,0,"SPEAR",40,0},
    {59,255,"SPEAR",40,0},
    {60,0,"SPEAR",40,0},
    {60,255,"SPEAR",40,0},
    {375,0,"BOULDER",30,0},
    {375,255,"BOULDER",30,0},
};
struct ProjectileImmunityGolden { DWORD values[4]; INT count; DWORD query; BOOL found; };
static const ProjectileImmunityGolden PROJECTILE_IMMUNITY[] = {
    {{0u,0u,0u,0u},0,0u,0},
    {{0u,0u,0u,0u},1,0u,1},
    {{7u,0u,0u,0u},1,7u,1},
    {{7u,0u,0u,0u},1,8u,0},
    {{1u,0u,4294967295u,65536u},4,0u,1},
    {{1u,0u,4294967295u,65536u},4,4294967295u,1},
    {{1u,0u,4294967295u,65536u},4,65536u,1},
    {{1u,0u,4294967295u,65536u},4,1u,1},
    {{7u,7u,8u,0u},3,8u,1},
    {{7u,7u,8u,0u},3,9u,0},
    {{65543u,0u,0u,0u},1,7u,0},
    {{65543u,0u,0u,0u},1,65543u,1},
};
#endif
