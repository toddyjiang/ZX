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

#include "Resource.h"

#include "wx/spinctrl.h"
#include "wx/image.h"
#include "wx/bookctrl.h"
#include <wx/propdlg.h>


#include "FxPropertySheet.h"

#include "EditorDefs.h"

IMPLEMENT_CLASS(CFxPropertySheet, wxPropertySheetDialog)

BEGIN_EVENT_TABLE(CFxPropertySheet, wxPropertySheetDialog)
END_EVENT_TABLE()

CFxPropertySheet::CFxPropertySheet(wxWindow* win, FX_HANDLE hFxEngine, FX_HANDLE hFx)
{
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);

    _hFxEngine = hFxEngine;
    _hFx = hFx;

    int tabImage1 = -1;
    int tabImage2 = -1;
    int tabImage3 = -1;
    int tabImage4 = -1;

    Bool useToolBook = FALSE;
    int resizeBorder = wxRESIZE_BORDER;

    _imageList = NULL;

    Create(win, wxID_ANY, _("Fx Properties"), wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE| (int)wxPlatform::IfNot(wxOS_WINDOWS_CE, resizeBorder)
    );

    // If using a toolbook, also follow Mac style and don't create buttons
    if (!useToolBook)
        CreateButtons(wxOK);

    wxBookCtrlBase* notebook = GetBookCtrl();
    notebook->SetImageList(_imageList);

    wxPanel* infos =        CreateInfoPage(notebook);
    wxPanel* InPin =        CreateInPinPage(notebook);
    wxPanel* OutPin =       CreateOutPinPage(notebook);
    wxPanel* Parameter =    CreateParameterPage(notebook);

    notebook->AddPage(infos, _("Infos"), true, tabImage1);
    notebook->AddPage(InPin, _("In Pins"), false, tabImage2);
    notebook->AddPage(OutPin, _("Out Pins"), false, tabImage3);
    notebook->AddPage(Parameter, _("Parameters"), false, tabImage4);

    LayoutDialog();
}

CFxPropertySheet::~CFxPropertySheet()
{
    delete _imageList;
    delete _lboxInfo;
    delete _lboxOutPin;
    delete _lboxInPin;
    delete _lboxParam;
}

wxPanel* CFxPropertySheet::CreateInfoPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

    int flags = wxLB_ALWAYS_SB | wxLB_HSCROLL;

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );

    _lboxInfo = new wxListBox(panel, INFO_SHEET,
                           wxDefaultPosition, wxDefaultSize,
                           0, NULL,
                           flags);

    Int32 hr;

	const FX_DESCRIPTOR* pFxDescriptor = NULL;
	hr = FEF_GetFxInfo(_hFxEngine, _hFx, &pFxDescriptor);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_GetFxInfo failed !!"), wxT("FxEngine Error"), wxICON_ERROR);
	}
    else
    {
        //Char str[1024];
        wxArrayString items;
        std::string strFxType;
	    FEF_GetConstToString(FX_TYPE_CONST, pFxDescriptor->FxType, strFxType);
        std::string strFxScope;
	    FEF_GetConstToString(FX_SCOPE_CONST, pFxDescriptor->FxScope, strFxScope);
	    //wsprintf(str,"Fx Name: %s", pFxDescriptor->strName.c_str());
	    items.Add(wxT("Fx Name: ") + s2ws(pFxDescriptor->strName));
	    //wsprintf(str,"Fx Version: %s", pFxDescriptor->strVersion.c_str());
	    items.Add(wxT("Fx Version: ") + s2ws(pFxDescriptor->strVersion));
	    //wsprintf(str,"Fx Type: %s", strFxType.c_str());
	    items.Add(wxT("Fx Type: ") + s2ws(strFxType));
        //wsprintf(str,"Fx Scope: %s", strFxScope.c_str());
	    items.Add(wxT("Fx Scope: ") + s2ws(strFxScope));
        items.Add(wxT("Fx Author: ") + s2ws(pFxDescriptor->strAuthor));
        items.Add(wxT("Fx Copyright: ") + s2ws(pFxDescriptor->strCopyright));

        _lboxInfo->InsertItems(items, 0);
    }


    topSizer->Add(_lboxInfo, 1, wxGROW | wxALL, 5);
    topSizer->Layout();

    panel->SetSizer(topSizer);
    topSizer->Fit(panel);
    return panel;
}
wxPanel* CFxPropertySheet::CreateInPinPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

    int flags = wxLB_ALWAYS_SB | wxLB_HSCROLL;

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );

    _lboxInPin = new wxListBox(panel, INPIN_SHEET,
                           wxDefaultPosition, wxDefaultSize,
                           0, NULL,
                           flags);

    Int32 hr;

	Uint16 wPinCount = 0;
	hr = FEF_GetFxPinCount(_hFxEngine, _hFx, &wPinCount);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_GetFxPinCount failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
	}
    else
    {

        wxArrayString items;
	    FX_PIN_STATE PinState;
	    FX_PIN_TYPE PinType;

	    wxString strPinInfo;
        std::string strPinName;
	    std::string strMainMedia;
	    std::string strSubMedia;
	    FX_MEDIA_TYPE FxMediaType;
	    Uint16 wMediaCount;
	    for(Int16 Idx = 0; Idx < wPinCount; Idx++)
	    {
		    IFxPin* pIFxpin;
		    hr = FEF_QueryFxPinInterface(_hFxEngine, _hFx, &pIFxpin, Idx);
		    if(FEF_FAILED(hr))
		    {
			    wxMessageBox(wxT("FEF_QueryFxPinInterface failed !!"), wxT("FxEngine Error"), wxICON_ERROR);
		    }
            else
            {
		        if(pIFxpin)
		        {
			        pIFxpin->GetPinType(&PinType);
			        if(PinType == PIN_IN)
			        {
				        pIFxpin->GetPinName(strPinName);

				        pIFxpin->GetPinState(&PinState);
				        if(PinState == PIN_CONNECTED)
				        {
					        pIFxpin->GetConnectionMediaType(&FxMediaType);

					        FEF_GetConstToString(FX_MAINMEDIATYPE_CONST, FxMediaType.MainMediaType, strMainMedia);

					        FEF_GetConstToString(FX_SUBMEDIATYPE_CONST, FxMediaType.SubMediaType, strSubMedia);

					        //wsprintf(strPinInfo, "%d: Name: %s, Main Media: %s, Sub Media: %s", Idx, strPinName.c_str(), strMainMedia.c_str(), strSubMedia.c_str());
                            strPinInfo = wxT("");
					        strPinInfo << Idx; strPinInfo += wxT(": Name: ") + s2ws(strPinName) + wxT(", Main Media: ") + s2ws(strMainMedia) + wxT(", Sub Media: ") + s2ws(strSubMedia);
					        items.Add(strPinInfo);
				        }
				        else 
				        {
					        //wsprintf(strPinInfo, "%d: Name: %s", Idx, strPinName.c_str());
                            strPinInfo = wxT("");
					        strPinInfo << Idx; strPinInfo += wxT(": Name: ") + s2ws(strPinName);
					        items.Add(strPinInfo);
					        pIFxpin->GetMediaTypeCount(&wMediaCount);
					        for(Int16 MediaIdx = 0; MediaIdx < wMediaCount; MediaIdx++)
					        {
						        pIFxpin->GetMediaType(&FxMediaType, MediaIdx);
						        FEF_GetConstToString(FX_MAINMEDIATYPE_CONST, FxMediaType.MainMediaType, strMainMedia);

						        FEF_GetConstToString(FX_SUBMEDIATYPE_CONST, FxMediaType.SubMediaType, strSubMedia);

						        //wsprintf(strPinInfo, "    Main Media: %s, Sub Media: %s", strMainMedia.c_str(), strSubMedia.c_str());
						        strPinInfo = wxT("    Main Media: ") + s2ws(strMainMedia) + wxT(", Sub Media: ") + s2ws(strSubMedia);
						        items.Add(strPinInfo);
					        }
				        }
			        }
                }
		    }
        }
		_lboxInPin->InsertItems(items, 0);
        //SAFE_DELETE_OBJECT(strPinInfo);
	}

    topSizer->Add(_lboxInPin, 1, wxGROW | wxALL, 5);
    topSizer->Layout();

    panel->SetSizer(topSizer);
    topSizer->Fit(panel);
    return panel;
}
wxPanel* CFxPropertySheet::CreateOutPinPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

    int flags = wxLB_ALWAYS_SB | wxLB_HSCROLL;

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );

    _lboxOutPin = new wxListBox(panel, OUTPIN_SHEET,
                           wxDefaultPosition, wxDefaultSize,
                           0, NULL,
                           flags);

    Int32 hr;

    wxArrayString items;
	Uint16 wPinCount = 0;
	hr = FEF_GetFxPinCount(_hFxEngine, _hFx, &wPinCount);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_GetFxPinCount failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
	}
    else
    {
	    FX_PIN_STATE PinState;
	    FX_PIN_TYPE PinType;

	    wxString strPinInfo;
        std::string strPinName;
	    std::string strMainMedia;
	    std::string strSubMedia;
	    FX_MEDIA_TYPE FxMediaType;
	    Uint16 wMediaCount;
	    for(Int16 Idx = 0; Idx < wPinCount; Idx++)
	    {
		    IFxPin* pIFxpin;
		    hr = FEF_QueryFxPinInterface(_hFxEngine, _hFx, &pIFxpin, Idx);
		    if(FEF_FAILED(hr))
		    {
			    wxMessageBox(wxT("FEF_QueryFxPinInterface failed !!"), wxT("FxEngine Error"), wxICON_ERROR);
		    }
		    else if(pIFxpin)
		    {
			    pIFxpin->GetPinType(&PinType);
			    if(PinType == PIN_OUT)
			    {
				    pIFxpin->GetPinName(strPinName);

				    pIFxpin->GetPinState(&PinState);
				    if(PinState == PIN_CONNECTED)
				    {
					    pIFxpin->GetConnectionMediaType(&FxMediaType);

					    FEF_GetConstToString(FX_MAINMEDIATYPE_CONST, FxMediaType.MainMediaType, strMainMedia);

					    FEF_GetConstToString(FX_SUBMEDIATYPE_CONST, FxMediaType.SubMediaType, strSubMedia);

					    //wsprintf(strPinInfo, "%d: Name: %s, Main Media: %s, Sub Media: %s", Idx, strPinName.c_str(), strMainMedia.c_str(), strSubMedia.c_str());
                        strPinInfo = wxT("");
					    strPinInfo << Idx; strPinInfo += wxT(": Name: ") + s2ws(strPinName) + wxT(", Main Media: ") + s2ws(strMainMedia) + wxT(", Sub Media: ") + s2ws(strSubMedia);
					    items.Add(strPinInfo);
				    }
				    else
				    {
					    //wsprintf(strPinInfo, "%d: Name: %s", Idx, strPinName.c_str());
                        strPinInfo = wxT("");
					    strPinInfo << Idx; strPinInfo += wxT(": Name: ") + s2ws(strPinName);
					    items.Add(strPinInfo);
					    pIFxpin->GetMediaTypeCount(&wMediaCount);
					    for(Int16 MediaIdx = 0; MediaIdx < wMediaCount; MediaIdx++)
					    {
						    pIFxpin->GetMediaType(&FxMediaType, MediaIdx);
						    FEF_GetConstToString(FX_MAINMEDIATYPE_CONST, FxMediaType.MainMediaType, strMainMedia);

						    FEF_GetConstToString(FX_SUBMEDIATYPE_CONST, FxMediaType.SubMediaType, strSubMedia);

						    //wsprintf(strPinInfo, "    Main Media: %s, Sub Media: %s", strMainMedia.c_str(), strSubMedia.c_str());
						    strPinInfo = wxT("    Main Media: ") + s2ws(strMainMedia) + wxT(", Sub Media: ") + s2ws(strSubMedia);
						    items.Add(strPinInfo);
					    }
				    }
			    }
		    }
        }
		_lboxOutPin->InsertItems(items, 0);
        //SAFE_DELETE_OBJECT(strPinInfo);
	}




    topSizer->Add(_lboxOutPin, 1, wxGROW | wxALL, 5);
    topSizer->Layout();

    panel->SetSizer(topSizer);
    topSizer->Fit(panel);
    return panel;
}
wxPanel* CFxPropertySheet::CreateParameterPage(wxWindow* parent)
{
    wxPanel* panel = new wxPanel(parent, wxID_ANY);

    int flags = wxLB_ALWAYS_SB | wxLB_HSCROLL;

    wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );

    _lboxParam = new wxListBox(panel, INFO_SHEET,
                           wxDefaultPosition, wxDefaultSize,
                           0, NULL,
                           flags);

    Int32 hr;

	IFxParam* pIFxParam;
	hr = FEF_QueryFxParamInterface(_hFxEngine, _hFx, &pIFxParam);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_QueryFxParamInterface failed !!"), wxT("FxEngine Error"), wxICON_ERROR);
	}
    else
    {
		/*! get unit parameters */
		wxArrayString items;
	    Uint16 wParamCount = 0;
	    pIFxParam->GetFxParamCount(&wParamCount);

	    wxString str;
        std::string strUnitName;
	    //wsprintf(str, "Fx Params:\n");
	    str = wxT("Fx Params:");
	    items.Add(str);
	    for(Int16 Idx = 0; Idx < wParamCount; Idx++)
	    {
		    const FX_PARAM *pFxParam = NULL;
		    pIFxParam->GetFxParam(&pFxParam, Idx);
		    FEF_GetConstToString(UNITTYPE_CONST, pFxParam->ParamType, strUnitName);
		    //wsprintf(str, "- %s, Unit:%s, Size:%d of %s", pFxParam->strParamName.c_str(), pFxParam->strParamUnitName.c_str(), pFxParam->dwParamNumber, strUnitName.c_str());
		    str = wxT("- ") + s2ws(pFxParam->strParamName) + wxT(", Unit: ") + s2ws(pFxParam->strParamUnitName);
		    str += wxT(", Size: "); str <<  pFxParam->dwParamNumber;
		    str += wxT(" of ") + s2ws(strUnitName);
		    items.Add(str);
	    }
	     //_lboxParam->InsertItems(items, 0);
	    //SAFE_DELETE_OBJECT(str);

		 /*! get string parameters */
		wParamCount = 0;
	    pIFxParam->GetFxParamStringCount(&wParamCount);

	    for(Int16 Idx = 0; Idx < wParamCount; Idx++)
	    {
		    const FX_PARAM_STRING *pFxParam = NULL;
		    pIFxParam->GetFxParam(&pFxParam, Idx);
		    str = wxT("- ") + s2ws(pFxParam->strParamName) + wxT(", String parameter");
		    items.Add(str);
	    }
	     
		_lboxParam->InsertItems(items, 0);
    }


    topSizer->Add(_lboxParam, 1, wxGROW | wxALL, 5);
    topSizer->Layout();

    panel->SetSizer(topSizer);
    topSizer->Fit(panel);
    return panel;
}
