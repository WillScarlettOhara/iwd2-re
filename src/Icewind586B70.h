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

        int m_targetId;
        int m_controllerId;
        bool m_bPermanent;
    };

    static Icewind586B70* Instance();
    static void Destroy();

    Icewind586B70();
    bool IsPartyNotFull(CGameSprite* sprite);
    bool CanAddSprite(CGameSprite* sprite1, CGameSprite* sprite2);
    void AddTemporary(CGameSprite* sprite1, CGameSprite* sprite2);
    void AddPermanently(CGameSprite* sprite1, CGameSprite* sprite2);
    void Remove(CGameSprite* sprite);
    void Reinstate(CGameSprite* sprite);
    void SyncToSprites();
    void Clear();
    int GetCount();

    static Icewind586B70* mInstance;
    std::vector<Entry> mEntries;
};

#endif /* ICEWIND586B70_H_ */
