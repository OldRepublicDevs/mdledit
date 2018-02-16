#include "bead_winctrl.h"
#include "strsafe.h"
#include <stdexcept> // for standard exceptions
//#include <exception> //for std::exception

HTREEITEM TreeView_GetNthChild(HWND hwndTree, HTREEITEM htiParent, int nChild){
    HTREEITEM htiChild = TreeView_GetChild(hwndTree, htiParent);
    if(htiParent == NULL) htiChild = TreeView_GetRoot(hwndTree);
    for(int n = 0; n < nChild; n++){
        htiChild = TreeView_GetNextSibling(hwndTree, htiChild);
    }
    return htiChild;
}

HTREEITEM TreeView_GetChildByText(HWND hwndTree, HTREEITEM htiParent, const std::string & sText){
    HTREEITEM htiChild = TreeView_GetChild(hwndTree, htiParent);
    if(htiParent == NULL) htiChild = TreeView_GetRoot(hwndTree);
    TVITEM tvi;
    tvi.mask = TVIF_TEXT;
    std::string sBuffer ('0', 255);
    while(htiChild != NULL){
        tvi.hItem = htiChild;
        tvi.pszText = &sBuffer.front();
        tvi.cchTextMax = 255;
        TreeView_GetItem(hwndTree, &tvi);
        if(tvi.pszText == sText) return htiChild;
        htiChild = TreeView_GetNextSibling(hwndTree, htiChild);
    }
    return NULL;
}

HTREEITEM TreeView_GetChildByText(HWND hwndTree, HTREEITEM htiParent, const std::wstring & sText){
    HTREEITEM htiChild = TreeView_GetChild(hwndTree, htiParent);
    if(htiParent == NULL) htiChild = TreeView_GetRoot(hwndTree);
    TVITEMW tvi;
    tvi.mask = TVIF_TEXT;
    std::wstring sBuffer (L'0', 255);
    while(htiChild != NULL){
        tvi.hItem = htiChild;
        tvi.pszText = &sBuffer.front();
        tvi.cchTextMax = 255;
        TreeView_GetItem(hwndTree, &tvi);
        if(tvi.pszText == sText) return htiChild;
        htiChild = TreeView_GetNextSibling(hwndTree, htiChild);
    }
    return NULL;
}

int TabCtrl_GetTabIndexByText(HWND hwndTabs, const std::string & sText){
    TCITEM tci;
    tci.mask = TCIF_TEXT;
    std::string sBuffer (255, '\0');
    for(int n = 0; n < TabCtrl_GetItemCount(hwndTabs); n++){
        tci.pszText = &sBuffer.front();
        tci.cchTextMax = 255;
        TabCtrl_GetItem(hwndTabs, n, &tci);
        if(tci.pszText == sText) return n;
    }
    return -1;
}

bool TabCtrl_AppendTab(HWND hTabControl, const std::string & sName){
    int nTabs = TabCtrl_GetItemCount(hTabControl);
    TCITEM tcAdd;
    tcAdd.mask = TCIF_TEXT;
    tcAdd.pszText = const_cast<char*>(&sName.front());
    tcAdd.cchTextMax = strlen(sName.c_str());
    return (TabCtrl_InsertItem(hTabControl, nTabs, &tcAdd) != -1);
}

std::string TabCtrl_GetCurSelName(HWND hTabcontrol){
    std::string sReturn;
    int nSel = TabCtrl_GetCurSel(hTabcontrol);
    if(nSel != -1){
        TCITEM tcitem;
        tcitem.mask = TCIF_TEXT;
        sReturn.resize(255, '\0');
        tcitem.pszText = &sReturn.front();
        tcitem.cchTextMax = 255;
        TabCtrl_GetItem(hTabcontrol, nSel, &tcitem);
        sReturn = sReturn.c_str();
    }
    return sReturn;
}

static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* /*lpelfe*/, NEWTEXTMETRICEX* /*lpntme*/, int /*FontType*/, LPARAM lParam){
    bool & bReturn = * (bool*) lParam;
    bReturn = true;

    return TRUE;
}

bool Font_IsInstalled(const std::string & sFont){
    // Get the screen DC
    HDC hdc = CreateCompatibleDC(NULL);

    LOGFONT lf = { 0 };
    // Any character set will do
    lf.lfCharSet = DEFAULT_CHARSET;
    // Set the facename to check for
    if(sFont.length() > 31) throw std::length_error("The font name '" + sFont + "'is too long!\n");
    StringCchCopy(lf.lfFaceName, 32, sFont.c_str());

    bool bReturn = false;

    // Enumerate fonts
    ::EnumFontFamiliesEx(hdc, &lf,  (FONTENUMPROC) EnumFontFamExProc, (LPARAM) &bReturn, 0);

    return bReturn;
}
