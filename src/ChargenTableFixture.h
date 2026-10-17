// AutoUI-only table defaults: original cells and labels remain owned by the game.
namespace Iwd2AutoUI {
struct ChargenTableFixture {
    C2DArray& table;
    SHORT width,height;
    CString fallback;
    CString *oldX,*oldY,*oldCells;
    CString x[5],y[1],cells[5];
    explicit ChargenTableFixture(C2DArray& value) : table(value),width(value.m_nSizeX),
        height(value.m_nSizeY),fallback(value.m_default),oldX(value.m_pNamesX),
        oldY(value.m_pNamesY),oldCells(value.m_pArray) {
        table.m_nSizeX=table.m_nSizeY=0;
    }
    void Set(const CString& value) { table.m_default=value; }
    void Gold(const CString& row) {
        x[0]=CRuleTables::SIDES; x[1]=CRuleTables::ROLLS; x[2]=CRuleTables::MODIFIER;
        x[3]=CRuleTables::MULTIPLIER; x[4]=CRuleTables::BONUS_PER_LEVEL;
        y[0]=row; cells[0]="1"; cells[1]="0"; cells[2]="0"; cells[3]="1"; cells[4]="37";
        table.m_pNamesX=x; table.m_pNamesY=y; table.m_pArray=cells;
        table.m_nSizeX=5; table.m_nSizeY=1;
    }
    ~ChargenTableFixture() {
        table.m_nSizeX=width; table.m_nSizeY=height; table.m_default=fallback;
        table.m_pNamesX=oldX; table.m_pNamesY=oldY; table.m_pArray=oldCells;
    }
};
}
