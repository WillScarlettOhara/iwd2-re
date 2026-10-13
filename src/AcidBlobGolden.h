// Canonical acid-leaf/factory/range capture; resource/base callbacks isolated.
// MD5: 25cb3d8aa2ec648e94e328d8c467973e
#ifndef ACID_BLOB_GOLDEN_H_
#define ACID_BLOB_GOLDEN_H_
struct AcidBlobFactoryGolden { USHORT type; BYTE delay, color, cached; };
static const AcidBlobFactoryGolden ACID_BLOB_FACTORY[] = {
    {108,0,73,73},
    {108,255,73,73},
    {192,0,86,73},
    {192,255,86,73},
    {193,0,110,73},
    {193,255,110,73},
    {194,0,26,73},
    {194,255,26,73},
};
struct AcidBlobRangeGolden { DWORD range,value; BYTE color,cached; USHORT row; };
static const AcidBlobRangeGolden ACID_BLOB_RANGE[] = {
    {0u,86u,86,73,86},
    {1u,110u,110,0,110},
    {6u,26u,26,0,26},
    {65539u,65609u,73,0,73},
    {4294901760u,4294901870u,110,73,110},
};
#endif
