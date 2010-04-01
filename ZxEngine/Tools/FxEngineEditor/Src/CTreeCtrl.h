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
#ifndef _TREE_CTRL_H
#define _TREE_CTRL_H

#define USE_GENERIC_TREECTRL 0

#if USE_GENERIC_TREECTRL
#include "wx/generic/treectlg.h"
#ifndef wxTreeCtrl
#define wxTreeCtrl wxGenericTreeCtrl 
#define s_classwxTreeCtrl s_classwxGenericTreeCtrl
#endif
#endif

#include "wx/treectrl.h"

#include <map>

using namespace std;

class CTreeItemData;

typedef map<FX_TYPE, wxTreeItemId> TreeValueMap;
typedef map<FX_SCOPE, TreeValueMap*> TreeValueScopeMap;

class CTreeCtrl : public wxTreeCtrl
{
public:
    enum
    {
        TreeCtrlIcon_File,
        TreeCtrlIcon_FileSelected,
        TreeCtrlIcon_Folder,
        TreeCtrlIcon_FolderSelected,
        TreeCtrlIcon_FolderOpened
    };

    CTreeCtrl() { }
    CTreeCtrl(wxWindow *parent, const wxWindowID id,
               const wxPoint& pos, const wxSize& size,
               long style);
    virtual ~CTreeCtrl();

	Void OnBeginDrag(wxTreeEvent& event);
    Void OnEndDrag(wxTreeEvent& event);
    Void OnBeginLabelEdit(wxTreeEvent& event);
    Void OnEndLabelEdit(wxTreeEvent& event);
    Void OnItemActivated(wxTreeEvent& event);
    Void OnContextMenu(wxContextMenuEvent& event);
    Void OnItemMenu(wxTreeEvent& event);
    Void OnGetInfo(wxTreeEvent& event);
    Void OnSetInfo(wxTreeEvent& event);
    Void OnItemExpanded(wxTreeEvent& event);
    Void OnItemExpanding(wxTreeEvent& event);
    Void OnItemCollapsed(wxTreeEvent& event);
    Void OnItemCollapsing(wxTreeEvent& event);
    Void OnSelChanged(wxTreeEvent& event);
    Void OnSelChanging(wxTreeEvent& event);
    Void OnTreeKeyDown(wxTreeEvent& event);
   
    Void GetItemsRecursively(const wxTreeItemId& idParent,
                             wxTreeItemIdValue cookie = 0);

    Void CreateImageList(int size = 16);
    Void CreateButtonsImageList(int size = 11);

    Void InitTreeValues(wxArrayString* pstrDirectories);
    Void InitTree();

    Void DoSortChildren(const wxTreeItemId& item, bool reverse = false)
        { _reverseSort = reverse; wxTreeCtrl::SortChildren(item); }
    Void DoEnsureVisible() { if (_lastItem.IsOk()) EnsureVisible(_lastItem); }

    Void ShowMenu(wxTreeItemId id, const wxPoint& pt);

    int ImageSize(void) const { return _imageSize; }

    Void SetLastItem(wxTreeItemId id) { _lastItem = id; }

protected:
    virtual int OnCompareItems(const wxTreeItemId& i1, const wxTreeItemId& i2);

    // is this the test item which we use in several event handlers?
    bool IsTestItem(const wxTreeItemId& item)
    {
        // the test item is the first child folder
        return GetItemParent(item) == GetRootItem() && !GetPrevSibling(item);
    }

private:
    wxTreeItemId AddFxItem(int image, const wxTreeItemId& idParent, const wxString& strName, CTreeItemData *pAudio);

    Void LogEvent(const wxChar *name, const wxTreeEvent& event);
	wxWindow *_parent;

    TreeValueScopeMap _treeValueScopeMap;

    int          _imageSize;               // current size of images
    bool         _reverseSort;             // flag for OnCompareItems
    wxTreeItemId _lastItem,                // for OnEnsureVisible()
                 _draggedItem;             // item being dragged right now

    // NB: due to an ugly wxMSW hack you _must_ use DECLARE_DYNAMIC_CLASS()
    //     if you want your overloaded OnCompareItems() to be called.
    //     OTOH, if you don't want it you may omit the next line - this will
    //     make default (alphabetical) sorting much faster under wxMSW.
    DECLARE_DYNAMIC_CLASS(CTreeCtrl)
    DECLARE_EVENT_TABLE()
};
#endif
