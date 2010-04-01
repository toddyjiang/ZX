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
#ifndef _TREE_ITEM_DATA_H
#define _TREE_ITEM_DATA_H

class CTreeItemData : public wxTreeItemData
{
public:
    CTreeItemData(wxString FxName, wxString FxPath, wxString FxVersion, wxString FxAuthor) :
      _FxName(FxName), _FxPath(FxPath), _FxAuthor(FxAuthor), _FxVersion(FxVersion){ }

    const wxString GetFxName() const { return _FxName; }
    const wxString GetFxPath() const { return _FxPath; }
    const wxString GetFxVersion() const { return _FxVersion; }
    const wxString GetFxAuthor() const { return _FxAuthor; }

private:
    wxString _FxName;
    wxString _FxVersion;
    wxString _FxAuthor;
    wxString _FxPath;
};
#endif
