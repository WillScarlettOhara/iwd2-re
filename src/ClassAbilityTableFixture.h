// AutoUI-only borrowed table cells; never owns or reloads game resources.
namespace Iwd2AutoUI {
struct ClassAbilityTableFixture {
    C2DArray& table;
    CString* oldArray;
    CString oldDefault;
    SHORT width,height;
    CString cells[30];
    explicit ClassAbilityTableFixture(C2DArray& value) : table(value),oldArray(value.m_pArray),
        oldDefault(value.m_default),width(value.m_nSizeX),height(value.m_nSizeY) {
        for (auto& cell : cells) cell="*";
        table.m_pArray=cells; table.m_default="*"; table.m_nSizeX=30; table.m_nSizeY=1;
    }
    ~ClassAbilityTableFixture() {
        table.m_pArray=oldArray; table.m_default=oldDefault; table.m_nSizeX=width; table.m_nSizeY=height;
    }
};
}
