#ifndef ICEWIND586B70_H_
#define ICEWIND586B70_H_

#include <vector>

class CGameSprite;

class Icewind586B70 {
public:
    class Entry {
    public:
        Entry();
        Entry(int a1, int a2);
        void Clear();

        int field_0;
        int field_4;
        bool field_8;
    };

    static Icewind586B70* Instance();
    static void Destroy();

    Icewind586B70();
    bool CanJoinParty(CGameSprite* sprite);
    bool CanAddSummoned(CGameSprite* sprite1, CGameSprite* sprite2);
    void AddSummoned(CGameSprite* sprite1, CGameSprite* sprite2);
    void AddDismissedSummoned(CGameSprite* sprite1, CGameSprite* sprite2);
    void Remove(CGameSprite* sprite);
    void RestoreSummonLinks(CGameSprite* sprite);
    void SaveSummonLinks();
    void Clear();
    int GetCount();

    static Icewind586B70* mInstance;
    std::vector<Entry> mEntries;
};

#endif /* ICEWIND586B70_H_ */
