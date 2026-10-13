// Canonical native factory descriptors and Explode dispatch; services isolated.
// MD5: 25cb3d8aa2ec648e94e328d8c467973e
#ifndef SPARKLE_EXPLOSION_GOLDEN_H_
#define SPARKLE_EXPLOSION_GOLDEN_H_
struct SparkleExplosionFactoryGolden { USHORT type; BOOL sparkle; const char *cell, *palette, *fire, *arrival; SHORT color, child; };
static const SparkleExplosionFactoryGolden SPARKLE_EXPLOSION_FACTORY[] = {
    {149,TRUE,"TRAVEL","STTRAVL1","TRA_04A","CAS_M08",2,46},
    {150,TRUE,"TRAVEL","STTRAVL2","TRA_04C","EFF_M05",4,47},
    {151,TRUE,"TRAVEL","STTRAVL5","TRA_04B","CAS_P03",6,48},
    {152,TRUE,"TRAVEL","STTRAVL7","","",9,49},
    {153,TRUE,"TRAVEL","","","",10,50},
    {154,TRUE,"TRAVEL","","","",1,51},
    {155,TRUE,"TRAVEL","","","",3,52},
    {156,TRUE,"TRAVEL","STTRAVL6","","",7,53},
    {157,TRUE,"TRAVEL","STTRAVL3","TRA_04A","CAS_P02",5,54},
    {182,TRUE,"TRAVEL","STTRAVL4","","",11,54},
    {183,TRUE,"TRAVEL","STTRAVL8","TRA_03","EFF_M04",12,184},
    {196,FALSE,"SHAREA","","","",0,0},
    {197,FALSE,"SHAREA","SHAREA1","","",0,0},
    {198,FALSE,"SHAREA","","","",0,0},
    {199,FALSE,"SHAREA","SHAREA3","","",0,0},
    {200,FALSE,"SHAREA","SHAREA4","","",0,0},
    {201,FALSE,"SHAREA","SHAREA5","","",0,0},
    {202,FALSE,"SHAREA","SHAREA6","","",0,0},
    {203,FALSE,"SHAREA","SHAREA2","","",0,0},
};
struct SparkleExplosionDispatchGolden { SHORT color; USHORT type; };
static const SparkleExplosionDispatchGolden SPARKLE_EXPLOSION_DISPATCH[] = {
    {-32768,0},
    {-1,0},
    {0,0},
    {1,196},
    {2,197},
    {3,200},
    {4,198},
    {5,199},
    {6,201},
    {7,202},
    {8,202},
    {9,196},
    {10,196},
    {11,200},
    {12,203},
    {13,0},
    {32767,0},
};
#endif
