// AutoUI-only borrowed cells. The game's arrays keep their original ownership.
namespace Iwd2AutoUI {
struct BeginLevelUpTableFixture {
    C2DArray& table;
    CString *oldX,*oldY,*oldCells,fallback;
    SHORT width,height;
    std::vector<CString> x,y,cells;
    BeginLevelUpTableFixture(C2DArray& t,SHORT w,SHORT h) : table(t),
        oldX(t.m_pNamesX),oldY(t.m_pNamesY),oldCells(t.m_pArray),fallback(t.m_default),
        width(t.m_nSizeX),height(t.m_nSizeY),x(w),y(h),cells(w*h) {
        table.m_pNamesX=x.data(); table.m_pNamesY=y.data(); table.m_pArray=cells.data();
        table.m_nSizeX=w; table.m_nSizeY=h; table.m_default="0";
    }
    ~BeginLevelUpTableFixture() {
        table.m_pNamesX=oldX; table.m_pNamesY=oldY; table.m_pArray=oldCells;
        table.m_nSizeX=width; table.m_nSizeY=height; table.m_default=fallback;
    }
};
}
