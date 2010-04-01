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
#ifndef _EDITOR_PROPERTY_SHEET_H
#define _EDITOR_PROPERTY_SHEET_H

#include <wx/propdlg.h> 
#include <wx/generic/propdlg.h>
class CEditorPropertySheet: public wxPropertySheetDialog
{
DECLARE_CLASS(CEditorPropertySheet)
public:
    CEditorPropertySheet(wxWindow* parent, wxArrayString* strDirectories, Bool* LockOnStart, wxSize* Size);
    virtual ~CEditorPropertySheet();

    Void OnButtonAdd(wxCommandEvent& event);
    Void OnButtonDel(wxCommandEvent& event);
    Void OnCheckBoxLock(wxCommandEvent& event);
	Void OnCheckBoxRxTx(wxCommandEvent& event);

    wxPanel* CreateDirectiesPage(wxWindow* parent);
    wxPanel* CreateSizePage(wxWindow* parent);
    wxPanel* CreateOptionPage(wxWindow* parent);

    wxArrayString* _pstrDirectories;
    wxSize* _pSize;

    wxCheckBox    *_checkbox;
    Bool          *_LockOnStart;
	
	wxTextCtrl *_textH;
    wxTextCtrl *_textW;
    
private:
    wxListBox *_lboxDirectries;
    
protected:

    enum {
        ID_ADD_DIRECTORIES = 100,
        ID_DEL_DIRECTORIES,
        ID_CHECKBOX_ISLOCKED
    };

    wxImageList*    _imageList;

DECLARE_EVENT_TABLE()
};
#endif

