// AutoUI-only borrowed rule cells; original arrays retain their ownership.
namespace Iwd2AutoUI {
struct LevelUpTableFixture {
    C2DArray& table;
    CString *oldX,*oldY,*oldCells,fallback;
    SHORT width,height;
    CString x[9],y[10],cells[90];
    explicit LevelUpTableFixture(C2DArray& t) : table(t),oldX(t.m_pNamesX),oldY(t.m_pNamesY),
        oldCells(t.m_pArray),fallback(t.m_default),width(t.m_nSizeX),height(t.m_nSizeY) {
        for (INT i=0;i<9;++i) x[i].Format(_T("%d"),i+1);
        for (INT i=0;i<10;++i) y[i].Format(_T("%d"),i+1);
        table.m_pNamesX=x; table.m_pNamesY=y; table.m_pArray=cells;
        table.m_nSizeX=9; table.m_nSizeY=10; table.m_default="0";
    }
    void Set(INT previous,INT current,BOOL domain=FALSE) {
        for (INT row=0;row<10;++row) for (INT col=0;col<9;++col)
            cells[row*9+col].Format(_T("%d"),domain?(col==0?1:0):(row==9?current:previous));
    }
    ~LevelUpTableFixture() {
        table.m_pNamesX=oldX; table.m_pNamesY=oldY; table.m_pArray=oldCells;
        table.m_default=fallback; table.m_nSizeX=width; table.m_nSizeY=height;
    }
};
}
