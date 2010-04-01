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
#ifndef _FX_PROPERTY_SHEET_H
#define _FX_PROPERTY_SHEET_H

#include <wx/propdlg.h>
#include <wx/generic/propdlg.h>

#include "FxEngine.h"
using namespace FEF; //!< Using FxEngineFramework

class CFxPropertySheet: public wxPropertySheetDialog
{
DECLARE_CLASS(CFxPropertySheet)
public:
    CFxPropertySheet(wxWindow* parent, FX_HANDLE hFxEngine, FX_HANDLE hFx);
    virtual ~CFxPropertySheet();

    wxPanel* CreateInfoPage(wxWindow* parent);
    wxPanel* CreateInPinPage(wxWindow* parent);
    wxPanel* CreateOutPinPage(wxWindow* parent);
    wxPanel* CreateParameterPage(wxWindow* parent);

private:
    wxListBox *_lboxInfo;
    wxListBox *_lboxOutPin;
    wxListBox *_lboxInPin;
    wxListBox *_lboxParam;
    FX_HANDLE _hFxEngine;
    FX_HANDLE _hFx;

protected:

    enum {
        ID_SHOW_TOOLTIPS = 100,
        ID_AUTO_SAVE,
        ID_AUTO_SAVE_MINS,
        ID_LOAD_LAST_PROJECT,

        ID_APPLY_SETTINGS_TO,
        ID_BACKGROUND_STYLE,
        ID_FONT_SIZE
    };

    wxImageList*    _imageList;

DECLARE_EVENT_TABLE()
};
#endif

