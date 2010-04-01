/* __________________________________________________________________________
	
	FxEngine Framework. 
	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.

	This file is part of FxEngine Editor.
    FxEngine Editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FxEngine Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FxEngine Editor. If not, see <http://www.gnu.org/licenses/>.

   ____________________________________________http://www.SMProcess.com______ */
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/treectrl.h"
#include "wx/progdlg.h"

#include "wx/image.h"
#include "wx/imaglist.h"
#include "wx/math.h"

#include "../Img/icon1.xpm"
#include "../Img/icon2.xpm"
#include "../Img/icon3.xpm"
#include "../Img/icon4.xpm"
#include "../Img/icon5.xpm"

#include "wx/colordlg.h"
#include "wx/numdlg.h"
#include "wx/dnd.h"

#include <wx/dir.h>

#include "Resource.h"

#include "FxEngineEditor.h"
#include "CTreeItemData.h"

#include "EditorDefs.h"

#include "FxEngine.h"
#include "CTreeCtrl.h"

typedef struct  _TREEFXTYPE {
    FX_TYPE FxType;
    std::string str;
 } TREEFXTYPE, *PTREEFXTYPE;

#define TYPES_NUMBER (7)
TREEFXTYPE TreeTypes[] = { {FX_SOURCE,    	"Source"},
                         {FX_RENDERER,      "Renderer"},
                         {FX_ANALYSER,      "Analyser"},
                         {FX_PROCESS,	    "Process"},
                         {FX_SPLITTER,      "Splitter"},
                         {FX_MIXER,	        "Mixer"},
                         {FX_USER,		    "Free user"} };

typedef struct  _TREEFXSCOPE {
    FX_SCOPE FxScope;
    std::string str;
 } TREEFXSCOPE, *PTREEFXSCOPE;

#define SCOPE_NUMBER (7)
TREEFXSCOPE TreeScopes[] = {  {FX_SCOPE_AUDIO,	"Audio"},
                             {FX_SCOPE_VIDEO,    "Video"},
                             {FX_SCOPE_TEXT,    "Text"},
                             {FX_SCOPE_DATA,    "Data"},
                             {FX_SCOPE_NETWORK, "Network"},
                             {FX_SCOPE_ALL,     "All"},
                             {FX_SCOPE_NOT_DEFINED,    "Misc"} };



// verify that the item is ok and insult the user if it is not
#define CHECK_ITEM( item ) if ( !item.IsOk() ) {                                 \
                             wxMessageBox(wxT("Please select some item first!"), \
                                          wxT("Tree sample error"),              \
                                          wxOK | wxICON_EXCLAMATION,             \
                                          this);                                 \
                             return;                                             \
                           }

#if USE_GENERIC_TREECTRL
BEGIN_EVENT_TABLE(CTreeCtrl, wxGenericTreeCtrl)
#else
BEGIN_EVENT_TABLE(CTreeCtrl, wxTreeCtrl)
#endif
    EVT_TREE_BEGIN_DRAG(TreeTest_Ctrl, CTreeCtrl::OnBeginDrag)
    EVT_TREE_END_DRAG(TreeTest_Ctrl, CTreeCtrl::OnEndDrag)
#if 0       // there are so many of those that logging them causes flicker
    EVT_TREE_GET_INFO(TreeTest_Ctrl, CTreeCtrl::OnGetInfo)
#endif
    /*EVT_TREE_SET_INFO(TreeTest_Ctrl, CTreeCtrl::OnSetInfo)
    EVT_TREE_ITEM_EXPANDED(TreeTest_Ctrl, CTreeCtrl::OnItemExpanded)
    EVT_TREE_ITEM_EXPANDING(TreeTest_Ctrl, CTreeCtrl::OnItemExpanding)
    EVT_TREE_ITEM_COLLAPSED(TreeTest_Ctrl, CTreeCtrl::OnItemCollapsed)
    EVT_TREE_ITEM_COLLAPSING(TreeTest_Ctrl, CTreeCtrl::OnItemCollapsing)
    EVT_TREE_BEGIN_LABEL_EDIT(TreeTest_Ctrl, CTreeCtrl::OnBeginLabelEdit)
    EVT_TREE_END_LABEL_EDIT(TreeTest_Ctrl, CTreeCtrl::OnEndLabelEdit)
    EVT_TREE_ITEM_ACTIVATED(TreeTest_Ctrl, CTreeCtrl::OnItemActivated)

    EVT_TREE_SEL_CHANGED(TreeTest_Ctrl, CTreeCtrl::OnSelChanged)
    EVT_TREE_SEL_CHANGING(TreeTest_Ctrl, CTreeCtrl::OnSelChanging)*/
    EVT_TREE_KEY_DOWN(TreeTest_Ctrl, CTreeCtrl::OnTreeKeyDown)

    // so many differents ways to handle right mouse button clicks...
    EVT_CONTEXT_MENU(CTreeCtrl::OnContextMenu)
    // EVT_TREE_ITEM_MENU is the preferred event for creating context menus
    // on a tree control, because it includes the point of the click or item,
    // meaning that no additional placement calculations are required.
    EVT_TREE_ITEM_MENU(TreeTest_Ctrl, CTreeCtrl::OnItemMenu)

END_EVENT_TABLE()

// CTreeCtrl implementation
#if USE_GENERIC_TREECTRL
IMPLEMENT_DYNAMIC_CLASS(CTreeCtrl, wxGenericTreeCtrl)
#else
IMPLEMENT_DYNAMIC_CLASS(CTreeCtrl, wxTreeCtrl)
#endif

CTreeCtrl::CTreeCtrl(wxWindow *parent, const wxWindowID id,
                       const wxPoint& pos, const wxSize& size,
                       long style)
          : wxTreeCtrl(parent, id, pos, size, style)
{
    _reverseSort = false;

	_parent = parent;

    CreateImageList();

    // Add some items to the tree
    InitTree();
}

CTreeCtrl::~CTreeCtrl()
{
    TreeValueScopeMap::iterator Itr = _treeValueScopeMap.begin();
    while(Itr != _treeValueScopeMap.end())
    {
        SAFE_DELETE_OBJECT((*Itr).second);
        Itr++;
    }
    _treeValueScopeMap.clear();
}

Void CTreeCtrl::CreateImageList(int size)
{
    if ( size == -1 )
    {
        SetImageList(NULL);
        return;
    }
    if ( size == 0 )
        size = _imageSize;
    else
        _imageSize = size;

    // Make an image list containing small icons
    wxImageList *images = new wxImageList(size, size, true);

    // should correspond to TreeCtrlIcon_xxx enum
    wxBusyCursor wait;
    wxIcon icons[5];
    icons[0] = wxIcon(icon1_xpm);
    icons[1] = wxIcon(icon2_xpm);
    icons[2] = wxIcon(icon3_xpm);
    icons[3] = wxIcon(icon4_xpm);
    icons[4] = wxIcon(icon5_xpm);

    int sizeOrig = icons[0].GetWidth();
    for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
    {
        if ( size == sizeOrig )
        {
            images->Add(icons[i]);
        }
        else
        {
            images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
        }
    }

    AssignImageList(images);
}

#if USE_GENERIC_TREECTRL || !defined(__WXMSW__)
Void CTreeCtrl::CreateButtonsImageList(int size)
{
    if ( size == -1 )
    {
        SetButtonsImageList(NULL);
        return;
    }

    // Make an image list containing small icons
    wxImageList *images = new wxImageList(size, size, true);

    // should correspond to TreeCtrlIcon_xxx enum
    wxBusyCursor wait;
    wxIcon icons[4];
    icons[0] = wxIcon(icon3_xpm);   // closed
    icons[1] = wxIcon(icon3_xpm);   // closed, selected
    icons[2] = wxIcon(icon5_xpm);   // open
    icons[3] = wxIcon(icon5_xpm);   // open, selected

    for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
    {
        int sizeOrig = icons[i].GetWidth();
        if ( size == sizeOrig )
        {
            images->Add(icons[i]);
        }
        else
        {
            images->Add(wxBitmap(wxBitmap(icons[i]).ConvertToImage().Rescale(size, size)));
        }
    }

    AssignButtonsImageList(images);
#else
Void CTreeCtrl::CreateButtonsImageList(int WXUNUSED(size))
{
#endif
}

int CTreeCtrl::OnCompareItems(const wxTreeItemId& item1,
                               const wxTreeItemId& item2)
{
    if ( _reverseSort )
    {
        // just exchange 1st and 2nd items
        return wxTreeCtrl::OnCompareItems(item2, item1);
    }
    else
    {
        return wxTreeCtrl::OnCompareItems(item1, item2);
    }
}

wxTreeItemId CTreeCtrl::AddFxItem(int image, const wxTreeItemId& idParent, const wxString& strName, CTreeItemData *pItem)
{
    // here we pass to AppendItem() normal and selected item images (we
    // suppose that selected image follows the normal one in the enum)
    int imageSel;
    imageSel = image + 1;

    wxTreeItemId id = AppendItem(idParent, strName, image, imageSel,
                                 pItem);

    return id;
}

Void CTreeCtrl::InitTreeValues(wxArrayString* pstrDirectories)
{
    /*! For each directory, get all Fx and put them in tree */
    Int32 hr;
    const FX_DESCRIPTOR* pFxDescriptor = NULL;

    wxArrayString files;
    wxDir dir;
    wxArrayString::iterator Itr, ItrFile;

    TreeValueMap::iterator ItrTree;

    FX_HANDLE hFxEngine = NULL;
    FX_HANDLE hFx;
	hr = FEF_CreateFxEngine(&hFxEngine);
	if(FEF_FAILED(hr))
        return;

    wxProgressDialog dialog(_T("Loading Fx ....                                                            "),
                            _T("Please Wait                                                                "),
                            100,    // range
                            this,   // parent
                            wxPD_AUTO_HIDE |
                            wxPD_APP_MODAL |
                            wxPD_ELAPSED_TIME | wxPD_SMOOTH);

	wxMilliSleep(100); //!< just to see progress bar !!

    /*! Delete last Items */
    TreeValueScopeMap::iterator ItrScope = _treeValueScopeMap.begin();
    while(ItrScope != _treeValueScopeMap.end())
    {
        for(ItrTree = (*ItrScope).second->begin(); ItrTree != (*ItrScope).second->end(); ItrTree++)
        {
            DeleteChildren((*ItrTree).second);
        }
        ItrScope++;
    }

	for(Itr = pstrDirectories->begin(); Itr != pstrDirectories->end(); Itr++)
    {

        files.clear();
        //const wxString& dirname
#ifdef WIN32
        dir.GetAllFiles((*Itr), &files, wxT("*.dll"), wxDIR_FILES);
#else
        dir.GetAllFiles((*Itr), &files, wxT("*.so"), wxDIR_FILES);
#endif
        dialog.Update(0, (*Itr));

        float fCount = 0;
        for(ItrFile = files.begin(); ItrFile != files.end(); ItrFile++)
        {
            dialog.Update((fCount++/(float)files.size())*100);
            
            //std::string strFx = (*ItrFile).mb_str();
            std::string strFx = ws2s(*ItrFile);
            hr = FEF_AddFx(hFxEngine, strFx, &hFx);
            if(FEF_FAILED(hr))
                continue;

            hr = FEF_GetFxInfo(hFxEngine, hFx, &pFxDescriptor);
	        if(FEF_FAILED(hr))
                continue;

            /*! Get scope first */
            ItrScope = _treeValueScopeMap.find(pFxDescriptor->FxScope);
            if(ItrScope != _treeValueScopeMap.end())
            {
                ItrTree = (*ItrScope).second->find(pFxDescriptor->FxType);
                if(ItrTree != (*ItrScope).second->end())
                {
                    CTreeItemData* pItem = new CTreeItemData(s2ws(pFxDescriptor->strName), (*ItrFile), s2ws(pFxDescriptor->strVersion), s2ws(pFxDescriptor->strAuthor));
                    AddFxItem(CTreeCtrl::TreeCtrlIcon_File, (*ItrTree).second , s2ws(pFxDescriptor->strName), pItem);
                }
            }

            FEF_RemoveFx(hFxEngine, hFx);
        }

    }
    FEF_ReleaseFxEngine(hFxEngine);

    _parent->SetFocus();
    return;
}

Void CTreeCtrl::InitTree()
{
    int image = CTreeCtrl::TreeCtrlIcon_Folder;
    wxTreeItemId rootId = AddRoot(wxT("Fx Scopes"),
                                  image, image,
                                  new CTreeItemData(wxT("NULL"), wxT("NULL"), wxT("NULL"), wxT("NULL")));
    if ( image != -1 )
    {
        SetItemImage(rootId, TreeCtrlIcon_FolderOpened, wxTreeItemIcon_Expanded);
    }

    TreeValueMap *ptreeValuesMap;
    wxTreeItemId FxScopeId;
    for(int j = 0; j < SCOPE_NUMBER; j++)
    {
        /*! create FxScopes */
        FxScopeId = AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, rootId, s2ws(TreeScopes[j].str), NULL);

        ptreeValuesMap = new TreeValueMap;
        for(int i = 0; i < TYPES_NUMBER; i++)
        {
            ptreeValuesMap->insert(make_pair(TreeTypes[i].FxType, AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, FxScopeId, s2ws(TreeTypes[i].str), NULL)));
        }
        _treeValueScopeMap.insert(make_pair(TreeScopes[j].FxScope, ptreeValuesMap));
    }

    ExpandAll();

    /*wxTreeItemId Id = AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Source"), NULL);


    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Renderer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Analyser"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Process"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Splitter"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Mixer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Free user"), NULL);

    Id = AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, rootId, wxT("Video"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Source"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Renderer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Analyser"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Process"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Splitter"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Mixer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Free user"), NULL);

    Id = AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, rootId, wxT("Text"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Source"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Renderer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Analyser"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Process"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Splitter"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Mixer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Free user"), NULL);

    Id = AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, rootId, wxT("Data"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Source"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Renderer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Analyser"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Process"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Splitter"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Mixer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Free user"), NULL);

    Id = AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, rootId, wxT("User"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Source"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Renderer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Analyser"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Process"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Splitter"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Mixer"), NULL);
    AddFxItem(CTreeCtrl::TreeCtrlIcon_Folder, Id, wxT("Free user"), NULL);*/

	_parent->SetFocus();
}

Void CTreeCtrl::OnBeginLabelEdit(wxTreeEvent& event)
{
    event.Veto();
}

Void CTreeCtrl::OnEndLabelEdit(wxTreeEvent& event)
{
   event.Veto();
}

Void CTreeCtrl::GetItemsRecursively(const wxTreeItemId& idParent,
                                     wxTreeItemIdValue cookie)
{
    wxTreeItemId id;

    if ( !cookie )
        id = GetFirstChild(idParent, cookie);
    else
        id = GetNextChild(idParent, cookie);

    if ( !id.IsOk() )
        return;

    wxString text = GetItemText(id);
    wxLogMessage(text);

    if (ItemHasChildren(id))
        GetItemsRecursively(id);

    GetItemsRecursively(idParent, cookie);
}

Void CTreeCtrl::LogEvent(const wxChar *name, const wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    wxString text;
    if ( item.IsOk() )
        text << wxT('"') << GetItemText(item).c_str() << wxT('"');
    else
        text = wxT("invalid Fx");
    wxLogMessage(wxT("%s(%s)"), name, text.c_str());
}

// avoid repetition
#define TREE_EVENT_HANDLER(name)                                 \
Void CTreeCtrl::name(wxTreeEvent& event)                        \
{                                                                \
    SetLastItem(wxTreeItemId());                                 \
    event.Skip();                                                \
}

TREE_EVENT_HANDLER(OnGetInfo)
TREE_EVENT_HANDLER(OnSetInfo)
TREE_EVENT_HANDLER(OnItemExpanded)
TREE_EVENT_HANDLER(OnItemExpanding)
TREE_EVENT_HANDLER(OnItemCollapsed)
TREE_EVENT_HANDLER(OnSelChanged)
TREE_EVENT_HANDLER(OnSelChanging)

#undef TREE_EVENT_HANDLER

Void LogKeyEvent(const wxChar *name, const wxKeyEvent& event)
{
    wxString key;
    long keycode = event.GetKeyCode();
    {
        switch ( keycode )
        {
            case WXK_BACK: key = wxT("BACK"); break;
            case WXK_TAB: key = wxT("TAB"); break;
            case WXK_RETURN: key = wxT("RETURN"); break;
            case WXK_ESCAPE: key = wxT("ESCAPE"); break;
            case WXK_SPACE: key = wxT("SPACE"); break;
            case WXK_DELETE: key = wxT("DELETE"); break;
            case WXK_START: key = wxT("START"); break;
            case WXK_LBUTTON: key = wxT("LBUTTON"); break;
            case WXK_RBUTTON: key = wxT("RBUTTON"); break;
            case WXK_CANCEL: key = wxT("CANCEL"); break;
            case WXK_MBUTTON: key = wxT("MBUTTON"); break;
            case WXK_CLEAR: key = wxT("CLEAR"); break;
            case WXK_SHIFT: key = wxT("SHIFT"); break;
            case WXK_ALT: key = wxT("ALT"); break;
            case WXK_CONTROL: key = wxT("CONTROL"); break;
            case WXK_MENU: key = wxT("MENU"); break;
            case WXK_PAUSE: key = wxT("PAUSE"); break;
            case WXK_CAPITAL: key = wxT("CAPITAL"); break;
            case WXK_END: key = wxT("END"); break;
            case WXK_HOME: key = wxT("HOME"); break;
            case WXK_LEFT: key = wxT("LEFT"); break;
            case WXK_UP: key = wxT("UP"); break;
            case WXK_RIGHT: key = wxT("RIGHT"); break;
            case WXK_DOWN: key = wxT("DOWN"); break;
            case WXK_SELECT: key = wxT("SELECT"); break;
            case WXK_PRINT: key = wxT("PRINT"); break;
            case WXK_EXECUTE: key = wxT("EXECUTE"); break;
            case WXK_SNAPSHOT: key = wxT("SNAPSHOT"); break;
            case WXK_INSERT: key = wxT("INSERT"); break;
            case WXK_HELP: key = wxT("HELP"); break;
            case WXK_NUMPAD0: key = wxT("NUMPAD0"); break;
            case WXK_NUMPAD1: key = wxT("NUMPAD1"); break;
            case WXK_NUMPAD2: key = wxT("NUMPAD2"); break;
            case WXK_NUMPAD3: key = wxT("NUMPAD3"); break;
            case WXK_NUMPAD4: key = wxT("NUMPAD4"); break;
            case WXK_NUMPAD5: key = wxT("NUMPAD5"); break;
            case WXK_NUMPAD6: key = wxT("NUMPAD6"); break;
            case WXK_NUMPAD7: key = wxT("NUMPAD7"); break;
            case WXK_NUMPAD8: key = wxT("NUMPAD8"); break;
            case WXK_NUMPAD9: key = wxT("NUMPAD9"); break;
            case WXK_MULTIPLY: key = wxT("MULTIPLY"); break;
            case WXK_ADD: key = wxT("ADD"); break;
            case WXK_SEPARATOR: key = wxT("SEPARATOR"); break;
            case WXK_SUBTRACT: key = wxT("SUBTRACT"); break;
            case WXK_DECIMAL: key = wxT("DECIMAL"); break;
            case WXK_DIVIDE: key = wxT("DIVIDE"); break;
            case WXK_F1: key = wxT("F1"); break;
            case WXK_F2: key = wxT("F2"); break;
            case WXK_F3: key = wxT("F3"); break;
            case WXK_F4: key = wxT("F4"); break;
            case WXK_F5: key = wxT("F5"); break;
            case WXK_F6: key = wxT("F6"); break;
            case WXK_F7: key = wxT("F7"); break;
            case WXK_F8: key = wxT("F8"); break;
            case WXK_F9: key = wxT("F9"); break;
            case WXK_F10: key = wxT("F10"); break;
            case WXK_F11: key = wxT("F11"); break;
            case WXK_F12: key = wxT("F12"); break;
            case WXK_F13: key = wxT("F13"); break;
            case WXK_F14: key = wxT("F14"); break;
            case WXK_F15: key = wxT("F15"); break;
            case WXK_F16: key = wxT("F16"); break;
            case WXK_F17: key = wxT("F17"); break;
            case WXK_F18: key = wxT("F18"); break;
            case WXK_F19: key = wxT("F19"); break;
            case WXK_F20: key = wxT("F20"); break;
            case WXK_F21: key = wxT("F21"); break;
            case WXK_F22: key = wxT("F22"); break;
            case WXK_F23: key = wxT("F23"); break;
            case WXK_F24: key = wxT("F24"); break;
            case WXK_NUMLOCK: key = wxT("NUMLOCK"); break;
            case WXK_SCROLL: key = wxT("SCROLL"); break;
            case WXK_PAGEUP: key = wxT("PAGEUP"); break;
            case WXK_PAGEDOWN: key = wxT("PAGEDOWN"); break;
            case WXK_NUMPAD_SPACE: key = wxT("NUMPAD_SPACE"); break;
            case WXK_NUMPAD_TAB: key = wxT("NUMPAD_TAB"); break;
            case WXK_NUMPAD_ENTER: key = wxT("NUMPAD_ENTER"); break;
            case WXK_NUMPAD_F1: key = wxT("NUMPAD_F1"); break;
            case WXK_NUMPAD_F2: key = wxT("NUMPAD_F2"); break;
            case WXK_NUMPAD_F3: key = wxT("NUMPAD_F3"); break;
            case WXK_NUMPAD_F4: key = wxT("NUMPAD_F4"); break;
            case WXK_NUMPAD_HOME: key = wxT("NUMPAD_HOME"); break;
            case WXK_NUMPAD_LEFT: key = wxT("NUMPAD_LEFT"); break;
            case WXK_NUMPAD_UP: key = wxT("NUMPAD_UP"); break;
            case WXK_NUMPAD_RIGHT: key = wxT("NUMPAD_RIGHT"); break;
            case WXK_NUMPAD_DOWN: key = wxT("NUMPAD_DOWN"); break;
            case WXK_NUMPAD_PAGEUP: key = wxT("NUMPAD_PAGEUP"); break;
            case WXK_NUMPAD_PAGEDOWN: key = wxT("NUMPAD_PAGEDOWN"); break;
            case WXK_NUMPAD_END: key = wxT("NUMPAD_END"); break;
            case WXK_NUMPAD_BEGIN: key = wxT("NUMPAD_BEGIN"); break;
            case WXK_NUMPAD_INSERT: key = wxT("NUMPAD_INSERT"); break;
            case WXK_NUMPAD_DELETE: key = wxT("NUMPAD_DELETE"); break;
            case WXK_NUMPAD_EQUAL: key = wxT("NUMPAD_EQUAL"); break;
            case WXK_NUMPAD_MULTIPLY: key = wxT("NUMPAD_MULTIPLY"); break;
            case WXK_NUMPAD_ADD: key = wxT("NUMPAD_ADD"); break;
            case WXK_NUMPAD_SEPARATOR: key = wxT("NUMPAD_SEPARATOR"); break;
            case WXK_NUMPAD_SUBTRACT: key = wxT("NUMPAD_SUBTRACT"); break;
            case WXK_NUMPAD_DECIMAL: key = wxT("NUMPAD_DECIMAL"); break;

            default:
            {
               if ( keycode < 128 && wxIsprint((int)keycode) )
                   key.Printf(wxT("'%c'"), (char)keycode);
               else if ( keycode > 0 && keycode < 27 )
                   key.Printf(_("Ctrl-%c"), wxT('A') + keycode - 1);
               else
                   key.Printf(wxT("unknown (%ld)"), keycode);
            }
        }
    }

    /*wxLogMessage( wxT("%s event: %s (flags = %c%c%c%c)"),
                  name,
                  key.c_str(),
                  event.ControlDown() ? wxT('C') : wxT('-'),
                  event.AltDown() ? wxT('A') : wxT('-'),
                  event.ShiftDown() ? wxT('S') : wxT('-'),
                  event.MetaDown() ? wxT('M') : wxT('-'));*/
}

Void CTreeCtrl::OnTreeKeyDown(wxTreeEvent& event)
{
    //LogKeyEvent(wxT("Tree key down "), event.GetKeyEvent());

    event.Veto();
	//wxGetApp().
}

Void CTreeCtrl::OnBeginDrag(wxTreeEvent& event)
{
    // need to explicitly allow drag
    if ( event.GetItem() != GetRootItem() )
    {
        _draggedItem = event.GetItem();

        CTreeItemData *item = _draggedItem.IsOk() ? (CTreeItemData *)GetItemData(_draggedItem)
                                         : NULL;
        if(item)
        {
            wxTextDataObject my_data(item->GetFxPath());

            wxDropSource dragSource( this );
	        dragSource.SetData( my_data );
	        dragSource.DoDragDrop( wxDrag_DefaultMove  );
        }
        event.Veto();
    }
    else
    {
        //wxLogMessage(wxT("OnBeginDrag: this item can't be dragged."));
    }
}

Void CTreeCtrl::OnEndDrag(wxTreeEvent& event)
{
    wxTreeItemId itemSrc = _draggedItem,
                 itemDst = event.GetItem();

	event.Veto();

    //_draggedItem = (wxTreeItemId)0l;

    // where to copy the item?
    /*if ( itemDst.IsOk() && !ItemHasChildren(itemDst) )
    {
         copy to the parent then
        itemDst = GetItemParent(itemDst);
    }*/

    //if ( !itemDst.IsOk() )
    //{
    //    wxLogMessage(wxT("OnEndDrag: can't drop here."));

    //    return;
    //}

    //wxString text = GetItemText(itemSrc);
    //wxLogMessage(wxT("OnEndDrag: '%s' copied to '%s'."),
    //             text.c_str(), GetItemText(itemDst).c_str());

    //// just do append here - we could also insert it just before/after the item
    //// on which it was dropped, but this requires slightly more work... we also
    //// completely ignore the client data and icon of the old item but could
    //// copy them as well.
    ////
    //// Finally, we only copy one item here but we might copy the entire tree if
    //// we were dragging a folder.
    //int image = wxGetApp().ShowImages() ? TreeCtrlIcon_File : -1;
    //AppendItem(itemDst, text, image);
}

Void CTreeCtrl::OnItemCollapsing(wxTreeEvent& event)
{
  event.Skip();
}

Void CTreeCtrl::OnItemActivated(wxTreeEvent& event)
{
    // show some info about this item
    wxTreeItemId itemId = event.GetItem();
    CTreeItemData *item = itemId.IsOk() ? (CTreeItemData *)GetItemData(itemId)
                                         : NULL;
    wxPoint pt;

    if ( item != NULL )
    {
        // attempt to guess where to show the menu
        // if an item was clicked, show menu to the right of it
        wxRect rect;
        GetBoundingRect(item, rect, true /* only the label */);
        pt = wxPoint(rect.GetRight(), rect.GetTop());
        ShowMenu(item, pt);
    }
}

Void CTreeCtrl::OnItemMenu(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();

    event.Skip();
}

Void CTreeCtrl::OnContextMenu(wxContextMenuEvent& event)
{
    wxPoint pt = event.GetPosition();
    wxTreeItemId item;

    // check if event was generated by keyboard (MSW-specific?)
    if ( pt.x == -1 && pt.y == -1 ) //(this is how MSW indicates it)
    {
        if ( !HasFlag(wxTR_MULTIPLE) )
            item = GetSelection();

        // attempt to guess where to show the menu
        if ( item.IsOk() )
        {
            // if an item was clicked, show menu to the right of it
            wxRect rect;
            GetBoundingRect(item, rect, true /* only the label */);
            pt = wxPoint(rect.GetRight(), rect.GetTop());
        }
        else
        {
            pt = wxPoint(0, 0);
        }
    }
    else // event was generated by mouse, use supplied coords
    {
        pt = ScreenToClient(pt);
        item = HitTest(pt);
    }

    ShowMenu(item, pt);
}

Void CTreeCtrl::ShowMenu(wxTreeItemId id, const wxPoint& pt)
{
    wxString title;
    CTreeItemData *item = id.IsOk() ? (CTreeItemData *)GetItemData(id)
                                         : NULL;
        if(item)
            title << item->GetFxName();// + endl + item->GetFxVersion() + "\n" + item->GetFxAuthor() + "\n" + item->GetFxPath() ;
        else
            return;

#if wxUSE_MENUS
    wxMenu menu(title);
    menu.Append(0, wxT("Version: ") + item->GetFxVersion());
    menu.Append(1, wxT("Author: ") + item->GetFxAuthor());
    menu.Append(2, wxT("Path: ") + item->GetFxPath());
    //menu.AppendSeparator();
    //menu.Append(TreeTest_Highlight, wxT("&Highlight item"));
    //menu.Append(TreeTest_Dump, wxT("&Dump"));

    PopupMenu(&menu, pt);
#endif // wxUSE_MENUS
}
