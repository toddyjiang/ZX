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

#include "FxEngine.h"
using namespace FEF; //!< Using FxEngineFramework

#include "Resource.h"

#include "wx/spinctrl.h"
#include "wx/image.h"
#include "wx/bookctrl.h"




#include "EditorPropertySheet.h"

IMPLEMENT_CLASS(CEditorPropertySheet, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(CEditorPropertySheet, wxPropertySheetDialog)
    EVT_BUTTON(ID_ADD_DIRECTORIES, CEditorPropertySheet::OnButtonAdd)
    EVT_BUTTON(ID_DEL_DIRECTORIES, CEditorPropertySheet::OnButtonDel)
    EVT_CHECKBOX(ID_CHECKBOX_ISLOCKED, CEditorPropertySheet::OnCheckBoxLock)

END_EVENT_TABLE()

CEditorPropertySheet::CEditorPropertySheet(wxWindow* win, wxArrayString* pstrDirectories, Bool* LockOnStart, wxSize* pSize)
{
    _pstrDirectories = pstrDirectories;
    _checkbox = NULL;
	_LockOnStart = LockOnStart;
	_pSize = pSize;

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

    int tabImage1 = -1;
    int tabImage2 = -1;

    bool useToolBook = FALSE;
    int resizeBorder = wxRESIZE_BORDER;

    _imageList = NULL;

    Create(win, wxID_ANY, _("FxEngine Options"), wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE| (int)wxPlatform::IfNot(wxOS_WINDOWS_CE, resizeBorder)
    );

    // If using a toolbook, also follow Mac style and don't create buttons
    if (!useToolBook)
        CreateButtons(wxOK);

    wxBookCtrlBase* notebook = GetBookCtrl();
    notebook->SetImageList(_imageList);

    wxPanel* DirectiesPage =        CreateDirectiesPage(notebook);
    //wxPanel* SizePage =        CreateSizePage(notebook);
    wxPanel* OptionsPage =        CreateOptionPage(notebook);

    notebook->AddPage(DirectiesPage, _("Fx Directories"), true, tabImage1);
    notebook->AddPage(OptionsPage, _("Editor Options"), false, tabImage2);

    LayoutDialog();
}

CEditorPropertySheet::~CEditorPropertySheet()
{
    delete _lboxDirectries;
}

wxPanel* CEditorPropertySheet::CreateDirectiesPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

    int flags = wxLB_ALWAYS_SB | wxLB_HSCROLL;

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );

    _lboxDirectries = new wxListBox(panel, INFO_SHEET,
                           wxDefaultPosition, wxDefaultSize,
                           0, NULL,
                           flags);

    topSizer->Add(_lboxDirectries, 1, wxGROW | wxLEFT, 5);

    wxButton *btnAdd = new wxButton(panel, ID_ADD_DIRECTORIES, wxT("&Add"));
    topSizer->Add(btnAdd, 0, wxALIGN_RIGHT, 5);

    wxButton *btnDel = new wxButton(panel, ID_DEL_DIRECTORIES, wxT("&Delete"));
    topSizer->Add(btnDel, 0, wxALIGN_RIGHT, 5);

    _lboxDirectries->InsertItems(*_pstrDirectories, 0);

    topSizer->Layout();

    panel->SetSizer(topSizer);
    topSizer->Fit(panel);
    return panel;
}
wxPanel* CEditorPropertySheet::CreateSizePage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );

    wxStaticBox *box2 = new wxStaticBox(panel, wxID_ANY, wxT("&FxEditor Working Size"));
    wxSizer *sizerMiddle = new wxStaticBoxSizer(box2, wxVERTICAL);

    wxSizer *sizerRow = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *Text1 = new wxStaticText(panel, wxID_ANY, wxT("&Width"));
    _textH = new wxTextCtrl(panel, 5, wxT("test item 0"));
    sizerRow->Add(Text1, 1, wxLEFT, 5);
    sizerRow->Add(_textH, 1, wxLEFT, 5);

    wxSizer *sizerRow2 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *Text2 = new wxStaticText(panel, wxID_ANY, wxT("&Heigth"));
    _textW = new wxTextCtrl(panel, 5, wxT("test item 0"));
    sizerRow2->Add(Text2, 1, wxLEFT, 5);
    sizerRow2->Add(_textW, 1, wxLEFT, 5);

    sizerMiddle->Add(sizerRow, 0, wxALL | wxGROW, 5);
    sizerMiddle->Add(sizerRow2, 0, wxALL | wxGROW, 5);
    topSizer->Add(sizerMiddle, 0, wxALL | wxGROW, 5);

    topSizer->Layout();

    panel->SetSizer(topSizer);
    topSizer->Fit(panel);
    return panel;
}

wxPanel* CEditorPropertySheet::CreateOptionPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );

    _checkbox = new wxCheckBox( panel, ID_CHECKBOX_ISLOCKED, wxT("&Lock Design on start"), wxPoint(-1,10));
    _checkbox->SetValue(*_LockOnStart);

    topSizer->Layout();

    panel->SetSizer(topSizer);
    topSizer->Fit(panel);
    return panel;
}

Void CEditorPropertySheet::OnButtonAdd(wxCommandEvent& event)
{
    wxString dirHome;
    wxGetHomeDir(&dirHome);

    wxDirDialog dialog(this, wxT("Choose new directory"), dirHome, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dialog.ShowModal() == wxID_OK)
    {
        _pstrDirectories->Add(dialog.GetPath().c_str());
        _lboxDirectries->Clear();
        _lboxDirectries->InsertItems(*_pstrDirectories, 0);
    }

	event.Skip();

    return;
}

Void CEditorPropertySheet::OnButtonDel(wxCommandEvent& event)
{
    wxArrayInt selections;
    int n = _lboxDirectries->GetSelections(selections);
    while ( n > 0 )
    {
        _pstrDirectories->RemoveAt(selections[--n]);
    }

    _lboxDirectries->Clear();
    _lboxDirectries->InsertItems(*_pstrDirectories, 0);

	event.Skip();
    return;
}

Void CEditorPropertySheet::OnCheckBoxLock(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case ID_CHECKBOX_ISLOCKED:
            {
               wxCheckBox *cb = (wxCheckBox*)event.GetEventObject();
                if (event.GetInt())
                    *_LockOnStart = TRUE;
                else
                    *_LockOnStart = FALSE;


                break;
            }
    }
    return;
}
