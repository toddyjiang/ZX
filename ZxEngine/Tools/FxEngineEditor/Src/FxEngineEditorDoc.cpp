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

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#if wxUSE_STD_IOSTREAM
    #include "wx/ioswrap.h"
#else
    #include "wx/txtstrm.h"
#endif

#include <wx/filename.h>


#include "FxEngine.h"
#include "Node.h"

#include "FxEngineEditorDoc.h"
#include "FxEngineEditorView.h"

#include "NodeManager.h"

#ifndef WIN32
#include <unistd.h>
#endif

map<FX_UNIT_TYPE, int> g_ParamSize;

IMPLEMENT_DYNAMIC_CLASS(DrawingDocument, wxDocument)

DrawingDocument::DrawingDocument()
{
	/*! Create FxEngine instance */
    Int32 hr;
    _hFxEngine = NULL;
	hr = FEF_CreateFxEngine(&_hFxEngine);
	if(FEF_FAILED(hr))
	{
		(Void)wxMessageBox(wxT("CreateFxEngine failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
	    return;
	}

    /*! Initialize parameter size */
    g_ParamSize[NOT_DEFINED_TYPE] = 0;
    g_ParamSize[UINT8_TYPE] = 1;
    g_ParamSize[INT8_TYPE] = 1;
    g_ParamSize[UINT16_TYPE] = 2;
    g_ParamSize[INT16_TYPE] = 2;
    g_ParamSize[UINT32_TYPE] = 4;
    g_ParamSize[INT32_TYPE] = 4;
    g_ParamSize[INT64_TYPE] = 8;
    g_ParamSize[UINT64_TYPE] = 8;
    g_ParamSize[FLOAT32_TYPE] = 4;
    g_ParamSize[FLOAT64_TYPE] = 8;
    g_ParamSize[COMPLEX_TYPE] = 8;
}

DrawingDocument::~DrawingDocument()
{
    Int32 hr;
  	if(_hFxEngine != NULL)
	{
		hr = FEF_ReleaseFxEngine( _hFxEngine );
		if(FEF_FAILED(hr))
		{
			(void)wxMessageBox(wxT("FEF_ReleaseFxEngine failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		}
		_hFxEngine = NULL;

		/*CNodeManager* p = CNodeManager::Instance();
		SAFE_DELETE_OBJECT(p);*/
	}
}

Void DrawingDocument::CloseDocument()
{
    Int32 hr;

	/*! Clear all Observer */
	FxMap::iterator Itmap;
	for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
	{
		CFx* pFx = Itmap->second;
		hr = FEF_DetachFxObserver(_hFxEngine, pFx->GethFxState());
		if(FEF_FAILED(hr))
		{
			    (Void)wxMessageBox(wxT("FEF_DetachFxObserver failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
			continue;
		}
	}

    AutoLock lock(_CSDoc); //! Draw and callback state
	/*! Clear all FXs added */
	FxMap CopyMap;
	for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
        CopyMap.insert(make_pair(Itmap->first, Itmap->second));

    for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
	{
		CFx* pFx = Itmap->second;
		FX_HANDLE hFx= Itmap->first;

		/*! Delete the Fx object */
		if(_hFxEngine)
		{
			SAFE_DELETE_OBJECT(pFx);
		}
	}
	_FxMap.clear();
    lock.unlock(); //! allow to callback to process messages


	for ( Itmap = CopyMap.begin( ); Itmap != CopyMap.end( ); Itmap++ )
	{
		CFx* pFx = Itmap->second;
		FX_HANDLE hFx= Itmap->first;

		/*! Delete the Fx object */
		if(_hFxEngine)
		{
			hr = FEF_RemoveFx(_hFxEngine, hFx);
			if(FEF_FAILED(hr))
			{
				(Void)wxMessageBox(wxT("FEF_RemoveFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
				continue;
			}
		}
	}

	/*! Clear all Fxlines added */
	lFxLineIter It;
	lock.lock(); //! Draw and callback state
	for ( It = _FxLineList.begin( ); It != _FxLineList.end( ); It++ )
	{
		/*! Delete the FxLine object */
		SAFE_DELETE((*It));
	}
	_FxLineList.clear();
}

Void DrawingDocument::FxProperty(FX_HANDLE _hLastSelectedFx, Pvoid hWnd)
{
	Int32 hr;
	hr = FEF_DisplayFxPropertyPage (_hFxEngine, _hLastSelectedFx, hWnd);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_DisplayFxPropertyPage failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return;
	}
	return;
}

FX_HANDLE DrawingDocument::PointInFx(wxPoint& Point)
{
    AutoLock lock(_CSDoc);

	/*! Clear all FXs added */
	FxMap::iterator Itmap;
	for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
		if(Itmap->second->IsPointInFx(Point) == TRUE)
			return Itmap->first;
	return NULL;
}

CFxLine* DrawingDocument::PointInFxLine(wxPoint& Point)
{
    AutoLock lock(_CSDoc);

	/*! Is point is a FxLine */
	lFxLineIter It;
	for ( It = _FxLineList.begin( ); It != _FxLineList.end( ); It++ )
		if((*It)->IsPointInFx(Point) == TRUE)
			return (*It);

	return NULL;
}

Void DrawingDocument::RemoveFxObject(FX_HANDLE hFx)
{
    Int32 hr;
	FxMap::iterator Itmap;
	Itmap = _FxMap.find( hFx );
	if( Itmap != _FxMap.end() )
	{
        lFxLine FxLineListTemp;
        lFxLine::iterator Iter;
		FX_HANDLE hFxState = Itmap->second->GethFxState();

        lFxLine FxLineList = Itmap->second->GetFxLines();
        /*!Delete line attached to it */
		AutoLock lock(_CSDoc); //! Draw and callback state
	    for (Iter = FxLineList.begin(); (Iter != FxLineList.end()) && !(FxLineList.empty()); Iter++)
	    {
			RemoveFxLineObject(*Iter, FALSE);
        }
		lock.unlock(); //! allow to callback to process messages

		if(_hFxEngine)
		{
			FEF_DetachFxObserver(_hFxEngine, hFxState);

			hr = FEF_RemoveFx(_hFxEngine, Itmap->first);
			if(FEF_FAILED(hr))
			{
				wxMessageBox(wxT("FEF_RemoveFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
			}
		}
        /*! Delete the Fx object */
		lock.lock(); //! Draw and callback state
		SAFE_DELETE(Itmap->second);
		_FxMap.erase(Itmap);
	}
	return;
}

Void DrawingDocument::RemoveFxLineObject(CFxLine* pFxLine, Bool ShouldLock)
{
    AutoLock lock(_CSDoc, ShouldLock);

	DetachLineToFx(pFxLine);

	lFxLineIter It;
	It = std::find(_FxLineList.begin(),_FxLineList.end(), pFxLine);
	if( It != _FxLineList.end() )
	{
		Disconnect(pFxLine);
		SAFE_DELETE_OBJECT(pFxLine);
		_FxLineList.erase(It);
	}
    return;
}

CNode* DrawingDocument::CreateFxLine(wxPoint& Point, CFxLine** ppFxLine)
{
	CNode* pBeginPoint = new CNode(Point);
	pBeginPoint->SetEnd();
	CNode* pEndPoint = new CNode(Point);
	pEndPoint->SetEnd();

	CFxLine* pFxLine = new CFxLine(pBeginPoint, pEndPoint);
    _FxLineList.push_back(pFxLine);

	CNodeManager::Instance()->Link(pBeginPoint);

	if(ppFxLine)
		*ppFxLine = pFxLine;

	return pEndPoint;
}

Bool DrawingDocument::EndFxLine(CNode* pNode)
{
	if( pNode->IsEnd() )
		return CNodeManager::Instance()->Link(pNode);
	/*else
		return CNodeManager::Instance()->Link(pNode, FALSE);*/
    return TRUE;
}

Void DrawingDocument::AttachLineToFx(CFxLine* pFxLine, IFxPin* pInIfxPin, IFxPin* pOutIfxPin)
{
	FxMap::iterator Itmap;
	for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
	{
		if(pOutIfxPin)
			//if(OutPinState == PIN_NOT_CONNECTED)
				Itmap->second->AttachFxLine(pOutIfxPin, pFxLine);
		if(pInIfxPin)
			//if(InPinState == PIN_NOT_CONNECTED)
				Itmap->second->AttachFxLine(pInIfxPin, pFxLine);
	}
	return;
}

Void DrawingDocument::DetachLineToFx(CFxLine* pFxLine, IFxPin* pIFxPin)
{
	FxMap::iterator Itmap;
	for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
	{
		Itmap->second->DetachFxLine(pFxLine, pIFxPin);
	}
	return;
}

CNode* DrawingDocument::PointInNode(wxPoint& Point, CFxLine** ppFxLine)
{
	CNode* pNode;
	/*! Is point is a FxLine */
	lFxLineIter It;
	for ( It = _FxLineList.begin( ); It != _FxLineList.end( ); It++ )
		if((*It)->PointInNode(Point, &pNode) == TRUE)
		{
			*ppFxLine = (*It);
			return (pNode);
		}

	return NULL;
}

Bool DrawingDocument::Connect(CFxLine* pFxLine, FX_MEDIA_TYPE* pMediaType)
{
    Int32 hr;

	if(pFxLine->IsConnected() == TRUE)
		return TRUE;

    IFxPin* pOutIfxPin;
	IFxPin* pInIfxPin;
	IFxPin* pTempIfxPin;

	pFxLine->GetBeginIFxPin(&pOutIfxPin);
	pFxLine->GetEndIFxPin(&pInIfxPin);


	if( (pOutIfxPin != NULL) && (pInIfxPin != NULL) )
	{
        if(pMediaType != NULL)
        {
            hr = FEF_ConnectFxPinEx(_hFxEngine, pInIfxPin, pOutIfxPin, pMediaType);
	        if(hr != FX_OK)
	        {
		        wxString str = wxT("Cannot connect FxPins, hr="); str << hr;
		        wxMessageBox((str), wxT("FxEngineEditor Error"), wxICON_ERROR);
		        return FALSE;
	        }
        }
        else
        {
		    FX_PIN_TYPE OutPinType, InPinType;
		    pOutIfxPin->GetPinType(&OutPinType);
		    pInIfxPin->GetPinType(&InPinType);
		    /*if(InPinType == OutPinType)
		    {
		        wxMessageBox(wxT("Cannot connect FxPins"), wxT("FxEngineEditor Error"), wxICON_ERROR);
                return FALSE;
		    }*/

		    if(OutPinType == PIN_IN)
		    {
			    pTempIfxPin = pOutIfxPin;
			    pOutIfxPin = pInIfxPin;
			    pInIfxPin = pTempIfxPin;
			}
			
            /*! Set the union between both Pin medias */
            MediaMap InMediaMap, OutMediaMap, UnionMediaMap;
            Uint16 wMediaCount;
            FX_MEDIA_TYPE FxMediaType;
            pInIfxPin->GetMediaTypeCount(&wMediaCount); //!< In pin
            for(Int16 MediaIdx = 0; MediaIdx < wMediaCount; MediaIdx++)
            {
	            pInIfxPin->GetMediaType(&FxMediaType, MediaIdx);
                InMediaMap.insert(make_pair(FxMediaType.SubMediaType, FxMediaType.MainMediaType));
            }
            pOutIfxPin->GetMediaTypeCount(&wMediaCount); //!< Out pin
            for(Int16 MediaIdx = 0; MediaIdx < wMediaCount; MediaIdx++)
            {
	            pOutIfxPin->GetMediaType(&FxMediaType, MediaIdx);
                OutMediaMap.insert(make_pair(FxMediaType.SubMediaType, FxMediaType.MainMediaType));
            }
            MediaMap::iterator Itr;
            MediaMap::iterator ItrIn;
            MediaMap::iterator ItrOut;
            ItrIn = InMediaMap.begin();
            ItrOut = OutMediaMap.begin();
            std::string strMainMedia;
	        std::string strSubMedia;
            wxString strMedia;
            wxArrayString MediaChoices;
            while( (ItrIn != InMediaMap.end()) && (ItrOut != OutMediaMap.end()) )
            {
                if( ((*ItrIn).first == (*ItrOut).first) &&
                    ((*ItrIn).second == (*ItrOut).second) )
                {
                    UnionMediaMap.insert(make_pair((*ItrIn).first, (*ItrIn).second));
                    FEF_GetConstToString(FX_MAINMEDIATYPE_CONST, (*ItrIn).second, strMainMedia);
		            FEF_GetConstToString(FX_SUBMEDIATYPE_CONST, (*ItrIn).first, strSubMedia);
                    //wsprintf(strMedia, "%s - %s", strMainMedia.c_str(), strSubMedia.c_str());
                    strMedia = s2ws(strMainMedia) + s2ws(strSubMedia);
                    MediaChoices.Add(strMedia);
                }
                ItrIn++;
                ItrOut++;
            }

            if(UnionMediaMap.size() <= 1) {
		        hr = FEF_ConnectFxPin(_hFxEngine, pInIfxPin, pOutIfxPin);
		        if(hr != FX_OK)
		        {
			        wxString str = wxT("Cannot connect FxPins, hr="); str << hr;
			        wxMessageBox((str), wxT("FxEngineEditor Error"), wxICON_ERROR);
			        return FALSE;
		        }
            }
            else //!< Show a dialog including the union media list
            {
                FX_MEDIA_TYPE MediaType;
                wxSingleChoiceDialog dialog(NULL,
                                            wxT("Common Pin Media Types:"),
                                            wxT("Please select a media type"),
                                            MediaChoices);

                if (dialog.ShowModal() == wxID_OK)
                {
                    Itr = UnionMediaMap.begin();
                    Int32 sdwIndex = dialog.GetSelection();
                    while(sdwIndex > 0) //!< Get Media type
                    {
                        Itr++;
                        sdwIndex--;
                    }

                    MediaType.MainMediaType = (*Itr).second;
                    MediaType.SubMediaType = (*Itr).first;
                }
                else
                    return FALSE;

                hr = FEF_ConnectFxPinEx(_hFxEngine, pInIfxPin, pOutIfxPin, &MediaType);
		        if(hr != FX_OK)
		        {
                    wxString str = wxT("Cannot connect FxPins, hr="); str << hr;
			        wxMessageBox((str), wxT("FxEngineEditor Error"), wxICON_ERROR);
			        return FALSE;
		        }
            }
        }
		pFxLine->SetConnected(TRUE);
	}
	AttachLineToFx(pFxLine, pInIfxPin, pOutIfxPin);
    
	return TRUE;
}

Void DrawingDocument::Disconnect(CFxLine* pFxLine)
{
	if(pFxLine->IsConnected() == FALSE)
		return;

	Int32 hr;
	IFxPin* pOutIfxPin;
	IFxPin* pInIfxPin;

	pFxLine->GetBeginIFxPin(&pOutIfxPin);
	pFxLine->GetEndIFxPin(&pInIfxPin);

	FX_PIN_STATE OutPinState, InPinState;

	if( pOutIfxPin != NULL )
	{
		DetachLineToFx(pFxLine, pOutIfxPin);
		pOutIfxPin->GetPinState(&OutPinState);
		if(OutPinState == PIN_CONNECTED) {
			hr = FEF_DisconnectFxPin(_hFxEngine, pOutIfxPin);
			if(FEF_FAILED(hr))
			{
				wxMessageBox(wxT("FEF_DisconnectFxPin failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
				return;
			}
		}
	}

	if( pInIfxPin != NULL )
	{
		DetachLineToFx(pFxLine, pInIfxPin);
		pInIfxPin->GetPinState(&InPinState);
		if(InPinState == PIN_CONNECTED) {
			hr = FEF_DisconnectFxPin(_hFxEngine, pInIfxPin);
			if(FEF_FAILED(hr))
			{
				wxMessageBox(wxT("FEF_DisconnectFxPin failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
				return;
			}
		}
	}
	pFxLine->SetConnected(FALSE);
	return;
}

Void DrawingDocument::GetFxState(FX_HANDLE hFx, std::string& strFxState)
{
	FX_STATE FxState;
	Int32 hr;
	hr = FEF_GetFxState(_hFxEngine, hFx, &FxState);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_GetFxState failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return;
	}
	FEF_GetConstToString(FX_STATE_CONST, FxState, strFxState);

	return;
}

Void DrawingDocument::StopFxOnly(FX_HANDLE _hLastSelectedFx)
{
	Int32 hr;
	hr = FEF_StopFx(_hFxEngine, _hLastSelectedFx);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_StopFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return;
	}
	return;
}

Void DrawingDocument::PauseFxOnly(FX_HANDLE _hLastSelectedFx)
{
	Int32 hr;
	hr = FEF_PauseFx(_hFxEngine, _hLastSelectedFx);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_PauseFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return;
	}
	return;
}

Void DrawingDocument::StartFxOnly(FX_HANDLE _hLastSelectedFx)
{
	Int32 hr;
	hr = FEF_StartFx(_hFxEngine, _hLastSelectedFx);
	if(FEF_FAILED(hr))
	{
		wxMessageBox(wxT("FEF_StartFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return;
	}
	return;
}

bool DrawingDocument::OnCreate(const wxString& path, long flags)
{
#ifndef WIN32
    wxFileName strFileName(path);
    wxString strName = strFileName.GetFullName();
    wxString strPathName = strFileName.GetFullPath();

    SetTitle(strName);
    SetFilename(strPathName);
#endif
    return wxDocument::OnCreate(path, flags);
}

#if wxUSE_STD_IOSTREAM
wxSTD ostream& DrawingDocument::SaveObject(wxSTD ostream& stream)
{
    wxDocument::SaveObject(stream);

    Int32 hr;

    Char str[MAX_PATH];
	Uint32 dwFxPosX, dwFxPosY;
	Uint32 dwFxCount;
	Uint32 dwLineCount;

	memset(str, 0, MAX_PATH * sizeof(Char));
	sprintf(str, "FxEngineEditor1.4");

	stream.write(str,MAX_PATH);

	/* Fx */
	dwFxCount = _FxMap.size();
	stream.write((Char*)&dwFxCount, sizeof(Uint32));
	FxMap::iterator Itmap;
    IFxParam* pIFxParam;
    const FX_PARAM* pFxParam;
	const FX_PARAM_STRING* pFxstrParam;
    Uint32 dwParamSize;

    Uint16 wParamCount;
    Uint8* pbParamValue;
    std::string strParamValue;
	for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
	{
		CFx* pFx = Itmap->second;
        /*! write fx parameters */
        hr = FEF_QueryFxParamInterface(_hFxEngine, pFx->GethFxHandle(), &pIFxParam);
        if(FEF_FAILED(hr))
            continue;

		/*! 1.2, write fx path name lenght */
        Uint16 wFxPathLenght = pFx->GetstrFxPath().size();
		stream.write((Char*)&wFxPathLenght, sizeof(Uint16));
		stream.write((Char*)pFx->GetstrFxPath().c_str(), wFxPathLenght);

        /*! 1.4, Get jump to the next Fx */
        Uint32 dwJumpSize = 0;
        dwJumpSize += sizeof(Uint32) + sizeof(Uint32);
        dwJumpSize += sizeof(Uint16) + pFx->GetFxName().size();
        pIFxParam->GetFxParamCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxParam, i);
            dwJumpSize += sizeof(Uint16);
            dwJumpSize += pFxParam->strParamName.size();
            dwJumpSize += sizeof(Uint32);
            dwJumpSize += pFxParam->dwParamNumber * g_ParamSize[pFxParam->ParamType];
        }
		/*! string parameter */
		pIFxParam->GetFxParamStringCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxstrParam, i);
            dwJumpSize += sizeof(Uint16);
			dwJumpSize += pFxstrParam->strParamName.size();
            pIFxParam->GetFxParamValue(pFxstrParam->strParamName, strParamValue);
			dwJumpSize += sizeof(Uint32);
			dwJumpSize += strParamValue.size();
        }
        stream.write((Char*)&dwJumpSize, sizeof(Uint32));

		dwFxPosX = pFx->GetFxPosX();
		stream.write((Char*)&dwFxPosX, sizeof(Uint32));
		dwFxPosY = pFx->GetFxPosY();
		stream.write((Char*)&dwFxPosY, sizeof(Uint32));

        /*! 1.2, write fx name lenght */
        Uint16 wFxNameLenght = pFx->GetFxName().size();
        stream.write((Char*)&wFxNameLenght, sizeof(Uint16));
        stream.write((Char*)pFx->GetFxName().c_str(), wFxNameLenght);

        pIFxParam->GetFxParamCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxParam, i);
            /*! 1.2, write param name lenght */
            Uint16 wParamNameLenght = pFxParam->strParamName.size();
            stream.write((Char*)&wParamNameLenght, sizeof(Uint16));
            stream.write((Char*)pFxParam->strParamName.c_str(), wParamNameLenght);
            dwParamSize = pFxParam->dwParamNumber * g_ParamSize[pFxParam->ParamType];
            stream.write((Char*)&dwParamSize, sizeof(Uint32));
            pbParamValue = new Uint8[dwParamSize];
            pIFxParam->GetFxParamValue(pFxParam->strParamName, (Void*)pbParamValue);
            stream.write((Char*)pbParamValue, dwParamSize);
            SAFE_DELETE_ARRAY(pbParamValue);
        }
		/*! write string parameter */
		pIFxParam->GetFxParamStringCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxstrParam, i);
			Uint16 wParamNameLenght = pFxstrParam->strParamName.size();
            stream.write((Char*)&wParamNameLenght, sizeof(Uint16));
            stream.write(pFxstrParam->strParamName.c_str(), wParamNameLenght);
            pIFxParam->GetFxParamValue(pFxstrParam->strParamName, strParamValue);
			Uint32 wLenght = strParamValue.size();
			stream.write((Char*)&wLenght, sizeof(Uint32));
			stream.write(strParamValue.c_str(), wLenght);
        }

        pIFxParam->FxReleaseInterface();
	}

	/* Lines */
	dwLineCount = _FxLineList.size();
	stream.write((Char*)&dwLineCount, sizeof(Uint32));
	lFxLineIter Itlist;
    FX_MEDIA_TYPE MediaType;
	Uint32 dwPointCount;
	for (Itlist = _FxLineList.begin(); (Itlist != _FxLineList.end()) && !(_FxLineList.empty()); Itlist++)
	{
        /*! Save connexion type if it exists */
        IFxPin* pOutIfxPin;
        IFxPin* pInIfxPin;

        (*Itlist)->GetBeginIFxPin(&pOutIfxPin);
        (*Itlist)->GetEndIFxPin(&pInIfxPin);

        if( (pOutIfxPin != NULL) && (pInIfxPin != NULL) )
        {
	        pOutIfxPin->GetConnectionMediaType(&MediaType);
        }
        else
        {
            MediaType.MainMediaType = MAIN_TYPE_UNDEFINED;
            MediaType.SubMediaType = SUB_TYPE_UNDEFINED;
        }
        stream.write((Char*)&MediaType.MainMediaType, sizeof(FX_MAIN_MEDIA_TYPE));
        stream.write((Char*)&MediaType.SubMediaType, sizeof(FX_SUB_MEDIA_TYPE));

		dwPointCount = (*Itlist)->GetPointCount();
		stream.write((Char*)&dwPointCount, sizeof(Uint32));
		(*Itlist)->SavePoints(stream);
	}

    wxString strTilte = GetTitle();
    wxView *pview = GetFirstView();
    ((DrawingView*)pview)->_frame->SetTitle(strTilte);

    return stream;
}
#else
wxOutputStream& DrawingDocument::SaveObject(wxOutputStream& stream)
{
	wxDocument::SaveObject(stream);

    Int32 hr;

    TCHAR str[MAX_PATH];
	Uint32 dwFxPosX, dwFxPosY;
	Uint32 dwFxCount;
	Uint32 dwLineCount;

	ZeroMemory(str, MAX_PATH * sizeof(Char));
	wsprintf(str, "FxEngineEditor1.4");

	stream.Write(str,MAX_PATH);

	/* Fx */
	dwFxCount = _FxMap.size();
	stream.Write(&dwFxCount, sizeof(Uint32));
	FxMap::iterator Itmap;
    IFxParam* pIFxParam;
    const FX_PARAM* pFxParam;
	const FX_PARAM_STRING* pFxstrParam;
    Uint32 dwParamSize;

    Uint16 wParamCount;
    Uint8* pbParamValue;
	std::string strParamValue;
	for ( Itmap = _FxMap.begin( ); Itmap != _FxMap.end( ); Itmap++ )
	{
		CFx* pFx = Itmap->second;
        hr = FEF_QueryFxParamInterface(_hFxEngine, pFx->GethFxHandle(), &pIFxParam);
        if(FEF_FAILED(hr))
            continue;

		/*! 1.2, write fx path name lenght */
        Uint16 wFxPathLenght = pFx->GetstrFxPath().size();
		stream.Write(&wFxPathLenght, sizeof(Uint16));
		stream.Write(pFx->GetstrFxPath().c_str(), wFxPathLenght);

        /*! 1.4, Get jump to the next Fx */
        /*! write fx parameters */
        Uint32 dwJumpSize = 0;
        dwJumpSize += sizeof(Uint32) + sizeof(Uint32);
        dwJumpSize += sizeof(Uint16) + pFx->GetFxName().size();
        pIFxParam->GetFxParamCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxParam, i);
            dwJumpSize += sizeof(Uint16);
            dwJumpSize += pFxParam->strParamName.size();
            dwJumpSize += sizeof(Uint32);
            dwJumpSize += pFxParam->dwParamNumber * g_ParamSize[pFxParam->ParamType];
        }
		/*! string parameter */
		pIFxParam->GetFxParamStringCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxstrParam, i);
            dwJumpSize += sizeof(Uint16);
			dwJumpSize += pFxstrParam->strParamName.size();
            pIFxParam->GetFxParamValue(pFxstrParam->strParamName, strParamValue);
			dwJumpSize += sizeof(Uint32);
			dwJumpSize += strParamValue.size();
        }
        stream.Write(&dwJumpSize, sizeof(Uint32));

		dwFxPosX = pFx->GetFxPosX();
		stream.Write(&dwFxPosX, sizeof(Uint32));
		dwFxPosY = pFx->GetFxPosY();
		stream.Write(&dwFxPosY, sizeof(Uint32));

        /*! 1.2, write fx name lenght */
        Uint16 wFxNameLenght = pFx->GetFxName().size();
        stream.Write(&wFxNameLenght, sizeof(Uint16));
        stream.Write(pFx->GetFxName().c_str(), wFxNameLenght);

        pIFxParam->GetFxParamCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxParam, i);
            /*! 1.2, write param name lenght */
            Uint16 wParamNameLenght = pFxParam->strParamName.size();
            stream.Write(&wParamNameLenght, sizeof(Uint16));
            stream.Write(pFxParam->strParamName.c_str(), wParamNameLenght);
            dwParamSize = pFxParam->dwParamNumber * g_ParamSize[pFxParam->ParamType];
            stream.Write(&dwParamSize, sizeof(Uint32));
            pbParamValue = new Uint8[dwParamSize];
            pIFxParam->GetFxParamValue(pFxParam->strParamName, (Void*)pbParamValue);
            stream.Write(pbParamValue, dwParamSize);
            SAFE_DELETE_ARRAY(pbParamValue);
        }
		/*! write string parameter */
		pIFxParam->GetFxParamStringCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pIFxParam->GetFxParam(&pFxstrParam, i);
			Uint16 wParamNameLenght = pFxstrParam->strParamName.size();
            stream.Write(&wParamNameLenght, sizeof(Uint16));
            stream.Write(pFxstrParam->strParamName.c_str(), wParamNameLenght);
            pIFxParam->GetFxParamValue(pFxstrParam->strParamName, strParamValue);
			Uint32 wLenght = strParamValue.size();
			stream.Write(&wLenght, sizeof(Uint32));
			stream.Write(strParamValue.c_str(), wLenght);
        }

        pIFxParam->FxReleaseInterface();
	}

	/* Lines */
	dwLineCount = _FxLineList.size();
	stream.Write(&dwLineCount, sizeof(Uint32));
	lFxLineIter Itlist;
    FX_MEDIA_TYPE MediaType;
	Uint32 dwPointCount;
	for (Itlist = _FxLineList.begin(); (Itlist != _FxLineList.end()) && !(_FxLineList.empty()); Itlist++)
	{
        /*! Save connexion type if it exists */
        IFxPin* pOutIfxPin;
        IFxPin* pInIfxPin;

        (*Itlist)->GetBeginIFxPin(&pOutIfxPin);
        (*Itlist)->GetEndIFxPin(&pInIfxPin);

        if( (pOutIfxPin != NULL) && (pInIfxPin != NULL) )
        {
	        pOutIfxPin->GetConnectionMediaType(&MediaType);
        }
        else
        {
            MediaType.MainMediaType = MAIN_TYPE_UNDEFINED;
            MediaType.SubMediaType = SUB_TYPE_UNDEFINED;
        }
        stream.Write(&MediaType.MainMediaType, sizeof(FX_MAIN_MEDIA_TYPE));
        stream.Write(&MediaType.SubMediaType, sizeof(FX_SUB_MEDIA_TYPE));

		dwPointCount = (*Itlist)->GetPointCount();
		stream.Write(&dwPointCount, sizeof(Uint32));
		(*Itlist)->SavePoints(stream);
	}

  return stream;
}
#endif

#if wxUSE_STD_IOSTREAM
wxSTD istream& DrawingDocument::LoadObject(wxSTD istream& stream)
{
    wxDocument::LoadObject(stream);

    wxView *pview = GetFirstView();

    Char strcFxPath[MAX_PATH];
	std::string strFxName;
	Uint32 dwFxPosX, dwFxPosY;
	Uint32 dwFxMaxPosX, dwFxMaxPosY;
	Uint32 dwFxCount;
	FX_HANDLE hFx = NULL;
    FX_HANDLE hFxState = NULL;
	Int32 hr;

    IFxParam* pIFxParam;
    std::string strParamName;
    Uint16 wParamCount;
    Uint8* pbParamValue;
	std::string strParamValue;
    Uint32 dwParamSize;

    Bool is10Version = FALSE;
    Bool is11Version = FALSE;
    Bool is12Version = FALSE;
	Bool is13Version = FALSE;
	Bool is14Version = FALSE;

	//ZeroMemory(strcFxPath, MAX_PATH * sizeof(Char));
	memset(strcFxPath, 0, MAX_PATH * sizeof(Char));

	dwFxMaxPosX = 0;
	dwFxMaxPosY = 0;

	stream.read((Char*)strcFxPath,MAX_PATH);
    if(strcmp("FxEngineEditor", strcFxPath) == 0)
        is10Version = TRUE;
    else if(strcmp("FxEngineEditor1.1", strcFxPath) == 0)
        is11Version = TRUE;
    else if(strcmp("FxEngineEditor1.2", strcFxPath) == 0)
        is12Version = TRUE;
	else if(strcmp("FxEngineEditor1.3", strcFxPath) == 0)
        is13Version = TRUE;
	else if(strcmp("FxEngineEditor1.4", strcFxPath) == 0)
        is14Version = TRUE;

	if( !is14Version && !is13Version && !is12Version && !is11Version && !is10Version)
	{
		wxMessageBox(wxT("Invalid FxEngineEditor file !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return stream;
	}

	/* Fx */
	stream.read((Char*)&dwFxCount,sizeof(Uint32));
    Char* strcFx;
    std::string strFxPath;
	for(Uint32 Idx = 0; Idx < dwFxCount; Idx++)
	{
        hFx = NULL;
        hFxState = NULL;

		if(is12Version || is13Version || is14Version)
        {
            Uint16 wFxPathLenght;
            stream.read((Char*)&wFxPathLenght, sizeof(Uint16));
            strcFx = new Char[wFxPathLenght + 1];
			strcFx[0] = '\0';
            stream.read((Char*)strcFx, wFxPathLenght);
            strcFx[wFxPathLenght] = '\0';
            strFxPath = strcFx;
            SAFE_DELETE_ARRAY(strcFx);
        }
        else
        {
            //ZeroMemory(strcFxPath, MAX_PATH * sizeof(Char));
            memset(strcFxPath, 0, MAX_PATH * sizeof(Char));
		    stream.read((Char*)strcFxPath,MAX_PATH);
            strFxPath = strcFxPath;
        }

        /*! get jump to next Fx */
        Uint32 dwJumpFx = 0;
        if(is14Version)
            stream.read((Char*)&dwJumpFx, sizeof(Uint32));

		/*! Verify the file */
		if (access(strFxPath.c_str(), 0) != 0)
		{
            wxString strErr = wxT("Cannot open: ") + s2ws(strFxPath);
			wxMessageBox(strErr, wxT("FxEngineEditor Error"), wxICON_ERROR);
			if(is14Version) {
				stream.seekg(dwJumpFx, ios::cur);
				continue;
			}
			else
				return stream;
		}
        else {
		    hr = FEF_AddFx(_hFxEngine, strFxPath, &hFx);
		    if(FEF_FAILED(hr))
		    {
			    wxString strErr = wxT("Invalid Fx: ") + s2ws(strFxPath);
			    wxMessageBox(strErr, wxT("FxEngineEditor Error"), wxICON_ERROR);
				if(is14Version){
					stream.seekg(dwJumpFx, ios::cur);
					continue;
				}
				else
					return stream;
		    }
            else {
		        hFxState = ((DrawingView*)pview)->canvas->AttachFxObserver(_hFxEngine, hFx);
		        //hr = FEF_AttachFxObserverEx(_hFxEngine, hFx, FxStateProc, (FX_PTR)this, &hFxState);
		        if(hFxState == NULL)
		        {
			        wxMessageBox(wxT("FEF_AttachFxObserverEx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
			        if(is14Version){
						stream.seekg(dwJumpFx, ios::cur);
						continue;
					}
					else
						return stream;
		        }
            }
        }

        stream.read((Char*)&dwFxPosX, sizeof(Uint32));
		stream.read((Char*)&dwFxPosY, sizeof(Uint32));
		dwFxMaxPosX = (dwFxPosX > dwFxMaxPosX) ? dwFxPosX : dwFxMaxPosX;
		dwFxMaxPosY = (dwFxPosY > dwFxMaxPosY) ? dwFxPosY : dwFxMaxPosY;

		CFx* pFx = NULL;
        if(is12Version || is13Version || is14Version)
        {
            Uint16 wFxNameLenght;
            stream.read((Char*)&wFxNameLenght, sizeof(Uint16));
            Char* strTemp = new Char[wFxNameLenght + 1];
			strTemp[0] = '\0';
            stream.read((Char*)strTemp, wFxNameLenght);
            strTemp[wFxNameLenght] = '\0';
            strFxName = strTemp;
            SAFE_DELETE_ARRAY(strTemp);
			pFx = new CFx(_hFxEngine, hFx, hFxState, strFxPath, dwFxPosX, dwFxPosY, strFxName);
			pFx->InitFxPin();
        }
        else if(is11Version)
        {
            Char strTemp[60];
            stream.read((Char*)strTemp, 60);
            strFxName = strTemp;
			pFx = new CFx(_hFxEngine, hFx, hFxState, strFxPath, dwFxPosX, dwFxPosY, strFxName);
        }
		else
			pFx = new CFx(_hFxEngine, hFx, hFxState, strFxPath, dwFxPosX, dwFxPosY, NULL);

		if(pFx == NULL)
		{
			wxMessageBox(wxT("Internal Fx Error!!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
			return stream;
		}


		_FxMap.insert(std::make_pair(hFx, pFx));

        if(is11Version || is12Version || is13Version || is14Version)
        {
            /*! read fx parameters */
            hr = FEF_QueryFxParamInterface(_hFxEngine, pFx->GethFxHandle(), &pIFxParam);
			if(FEF_FAILED(hr)) {
                wxMessageBox(wxT("Cannot get Fx Parameter Interface"), wxT("FxEngineEditor Error"), wxICON_ERROR);
				return stream;
			}

            pIFxParam->GetFxParamCount(&wParamCount);
            for(int i = 0; i < wParamCount; i++)
            {
                if(is12Version || is13Version || is14Version) {
                    Uint16 wParamNameLenght;
                    stream.read((Char*)&wParamNameLenght, sizeof(Uint16));
                    Char* strTemp = new Char[wParamNameLenght + 1];
					strTemp[0] = '\0';
                    stream.read((Char*)strTemp, wParamNameLenght);
                    strTemp[wParamNameLenght] = '\0';
                    strParamName = strTemp;
                    SAFE_DELETE_ARRAY(strTemp);
                }
                else if(is11Version) {
                    Char strTemp[60];
                    stream.read((Char*)strTemp, 60);
                    strParamName = strTemp;
                }
                stream.read((Char*)&dwParamSize, sizeof(Uint32));
                pbParamValue = new Uint8[dwParamSize];
				memset(pbParamValue, 0, dwParamSize*sizeof(Uint8));
                stream.read((Char*)pbParamValue, dwParamSize);
                pIFxParam->SetFxParamValue(strParamName, (Void*)pbParamValue);
                SAFE_DELETE_ARRAY(pbParamValue);
            }

			/*! Read string parameters */
			if(is13Version || is14Version) {
				pIFxParam->GetFxParamStringCount(&wParamCount);
				for(int i = 0; i < wParamCount; i++)
				{
					Uint16 wParamNameLenght;
                    stream.read((Char*)&wParamNameLenght, sizeof(Uint16));
                    Char* strTemp = new Char[wParamNameLenght + 1];
					strTemp[0] = '\0';
                    stream.read(strTemp, wParamNameLenght);
                    strTemp[wParamNameLenght] = '\0';
                    strParamName = strTemp;
                    SAFE_DELETE_ARRAY(strTemp);

					stream.read((Char*)&dwParamSize, sizeof(Uint32));
					strTemp = new Char[dwParamSize + 1];
					strTemp[0] = '\0';
                    stream.read(strTemp, dwParamSize);
                    strTemp[dwParamSize] = '\0';
					strParamValue = strTemp;
					pIFxParam->SetFxParamValue(strParamName, strParamValue);
					SAFE_DELETE_ARRAY(strTemp);
				}
			}

            pIFxParam->FxReleaseInterface();
            FEF_UpdateFxParam(_hFxEngine, pFx->GethFxHandle(), "", FX_PARAM_ALL);
        }
    }

    /*((DrawingView*)pview)->canvas->Refresh();
    ((DrawingView*)pview)->canvas->Update();*/

    /* Lines */
	Uint32 dwLineCount;
	Uint32 dwPointCount;
	wxPoint Point;
    int sdwPointType;
    FX_MEDIA_TYPE MediaType;
    MediaType.MainMediaType = MAIN_TYPE_UNDEFINED;
    MediaType.SubMediaType = SUB_TYPE_UNDEFINED;
	stream.read((Char*)&dwLineCount, sizeof(Uint32));
	for(Uint32 IdxLine = 0; IdxLine < dwLineCount; IdxLine++)
	{
        if(is11Version || is12Version || is13Version || is14Version)
        {
            stream.read((Char*)&MediaType.MainMediaType, sizeof(FX_MAIN_MEDIA_TYPE));
            stream.read((Char*)&MediaType.SubMediaType, sizeof(FX_SUB_MEDIA_TYPE));
        }

        CNode* ppNode[6];
		memset(ppNode, NULL, 6*sizeof(CNode*));
		stream.read((Char*)&dwPointCount, sizeof(Uint32));
		for(Uint32 IdxPoint = 0; IdxPoint < dwPointCount; IdxPoint++)
		{
			stream.read((Char*)&Point.x, sizeof(Uint32));
			stream.read((Char*)&Point.y, sizeof(Uint32));
            CNode* pNode = NULL;
            if(is14Version) {
                stream.read((Char*)&sdwPointType, sizeof(int));
			    pNode = new CNode(Point, (NODE_TYPE)sdwPointType);
            }
            else 
                pNode = new CNode(Point);
			if(IdxPoint == 0)
			{
                ppNode[IdxPoint] = pNode;
				//pNode->SetFirst();
				//pFxLine->AddPoint(pNode, TRUE);
				//CNodeManager::Instance()->Link(pNode);
			}
			else if(IdxPoint == (dwPointCount-1))
			{
				if(!is14Version)
					ppNode[5] = pNode;
				else
					ppNode[IdxPoint] = pNode;
            //pNode->SetEnd();
				//pFxLine->AddPoint(pNode, TRUE);
				//CNodeManager::Instance()->Link(pNode);
			}
			else if(is14Version)
			{
				ppNode[IdxPoint] = pNode;
				//pFxLine->AddConnectionPoint(pNode);
				//CNodeManager::Instance()->Link(pNode, FALSE);
			}
         else
				SAFE_DELETE_OBJECT(pNode);
		}
		CFxLine* pFxLine = new CFxLine(ppNode, 6);

		if(is14Version)
			pFxLine->UpdateAdjustedPoints(NULL); //!< set adjusted points

		_FxLineList.push_back(pFxLine);
		//pFxLine->Update();

        if(is11Version || is12Version || is13Version || is14Version)
        {
            pFxLine->SetMediaTypeConnection(MediaType);
            /*if( (MediaType.MainMediaType == MAIN_TYPE_UNDEFINED) &&
                (MediaType.SubMediaType == SUB_TYPE_UNDEFINED) )
		        Connect(pFxLine, NULL);
            else
                Connect(pFxLine, &MediaType);*/
        }
	}

    wxString strTilte = GetTitle();
    ((DrawingView*)pview)->_frame->SetTitle(strTilte);

    ((DrawingView*)pview)->canvas->TryConnection();
  /*((DrawingView*)pview)->canvas->ClearBackground();*/
  ((DrawingView*)pview)->canvas->Refresh();
  ((DrawingView*)pview)->canvas->Update();

  return stream;
}
#else
wxInputStream& DrawingDocument::LoadObject(wxInputStream& stream)
{
  wxDocument::LoadObject(stream);

  wxView *pview = GetFirstView();

    TCHAR strcFxPath[MAX_PATH];
	std::string strFxName;
	Uint32 dwFxPosX, dwFxPosY;
	Uint32 dwFxMaxPosX, dwFxMaxPosY;
	Uint32 dwFxCount;
	FX_HANDLE hFx = NULL;
    FX_HANDLE hFxState = NULL;
	Int32 hr;

	IFxParam* pIFxParam;
    std::string strParamName;
    Uint16 wParamCount;
    Uint8* pbParamValue;
	std::string strParamValue;
    Uint32 dwParamSize;

    Bool is10Version = FALSE;
    Bool is11Version = FALSE;
    Bool is12Version = FALSE;
	Bool is13Version = FALSE;
	Bool is14Version = FALSE;

	ZeroMemory(strcFxPath, MAX_PATH * sizeof(Char));

	dwFxMaxPosX = 0;
	dwFxMaxPosY = 0;

	stream.Read(strcFxPath,MAX_PATH);
    if(strcmp("FxEngineEditor", strcFxPath) == 0)
        is10Version = TRUE;
    else if(strcmp("FxEngineEditor1.1", strcFxPath) == 0)
        is11Version = TRUE;
    else if(strcmp("FxEngineEditor1.2", strcFxPath) == 0)
        is12Version = TRUE;
	else if(strcmp("FxEngineEditor1.3", strcFxPath) == 0)
        is13Version = TRUE;
	else if(strcmp("FxEngineEditor1.4", strcFxPath) == 0)
        is14Version = TRUE;

	if( !is14Version && !is13Version && !is12Version && !is11Version && !is10Version)
	{
		wxMessageBox(wxT("Invalid FxEngineEditor file !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return stream;
	}

	/* Fx */
	stream.Read(&dwFxCount,sizeof(Uint32));
    Char* strcFx;
    std::string strFxPath;
	for(Uint32 Idx = 0; Idx < dwFxCount; Idx++)
	{
        hFx = NULL;
        hFxState = NULL;
		if(is12Version || is13Version || is14Version)
        {
            Uint16 wFxPathLenght;
            stream.Read(&wFxPathLenght, sizeof(Uint16));
            strcFx = new Char[wFxPathLenght + 1];
			strcFx[0] = '\0';
            stream.Read(strcFx, wFxPathLenght);
            strcFx[wFxPathLenght] = '\0';
            strFxPath = strcFx;
            SAFE_DELETE_ARRAY(strcFx);
        }
        else
        {
            ZeroMemory(strcFxPath, MAX_PATH * sizeof(Char));
		    stream.Read(strcFxPath,MAX_PATH);
            strFxPath = strcFxPath;
        }

        /*! get jump to next Fx */
        Uint32 dwJumpFx = 0;
        if(is14Version)
            stream.Read(&dwJumpFx, sizeof(Uint32));
        
		/*! Verify the file */
		if (_access(strFxPath.c_str(), 0) != 0)
		{
            wxString strErr = wxT("Cannot open: ") + strFxPath;
			wxMessageBox(wxT(strErr), wxT("FxEngineEditor Error"), wxICON_ERROR);
			if(is14Version){
				stream.SeekI(dwJumpFx, wxFromCurrent);
				continue;
			}
			else
				return stream;
		}
        else {
		    hr = FEF_AddFx(_hFxEngine, strFxPath, &hFx);
		    if(FEF_FAILED(hr))
		    {
			    wxString strErr = wxT("Invalid Fx: ") + strFxPath;
			    wxMessageBox(wxT(strErr), wxT("FxEngineEditor Error"), wxICON_ERROR);
				if(is14Version){
					stream.SeekI(dwJumpFx, wxFromCurrent);
					continue;
				}
				else
					return stream;
		    }
            else {
                hFxState = ((DrawingView*)pview)->canvas->AttachFxObserver(_hFxEngine, hFx);
		        //hr = FEF_AttachFxObserverEx(_hFxEngine, Fx, FxStateProc, (FX_PTR)this, &hFxState);
		        if(hFxState == NULL)
		        {
			        wxMessageBox(wxT("FEF_AttachFxObserverEx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
					if(is14Version){
						stream.SeekI(dwJumpFx, wxFromCurrent);
						continue;
					}
					else
						return stream;
		        }
            }
        }

        stream.Read(&dwFxPosX, sizeof(Uint32));
		stream.Read(&dwFxPosY, sizeof(Uint32));
        dwFxMaxPosX = (dwFxPosX > dwFxMaxPosX) ? dwFxPosX : dwFxMaxPosX;
		dwFxMaxPosY = (dwFxPosY > dwFxMaxPosY) ? dwFxPosY : dwFxMaxPosY;

		CFx* pFx = NULL;
        if(is12Version || is13Version || is14Version)
        {
            Uint16 wFxNameLenght;
            stream.Read(&wFxNameLenght, sizeof(Uint16));
            Char* strTemp = new Char[wFxNameLenght + 1];
			strTemp[0] = '\0';
            stream.Read(strTemp, wFxNameLenght);
            strTemp[wFxNameLenght] = '\0';
            strFxName = strTemp;
            SAFE_DELETE_ARRAY(strTemp);
			pFx = new CFx(_hFxEngine, hFx, hFxState, strFxPath, dwFxPosX, dwFxPosY, strFxName);
            pFx->InitFxPin();
        }
        else if(is11Version)
        {
            Char strTemp[60];
            stream.Read(strTemp, 60);
            strFxName = strTemp;
			pFx = new CFx(_hFxEngine, hFx, hFxState, strFxPath, dwFxPosX, dwFxPosY, strFxName);
        }
		else
			pFx = new CFx(_hFxEngine, hFx, hFxState, strFxPath, dwFxPosX, dwFxPosY, NULL);

		if(pFx == NULL)
		{
			wxMessageBox(wxT("Internal Fx Error!!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
			return stream;
		}


		_FxMap.insert(std::make_pair(hFx, pFx));

        if(is11Version || is12Version || is13Version || is14Version)
        {
            /*! read fx parameters */
            hr = FEF_QueryFxParamInterface(_hFxEngine, pFx->GethFxHandle(), &pIFxParam);
			if(FEF_FAILED(hr)) {
                wxMessageBox(wxT("Cannot get Fx Parameter Interface"), wxT("FxEngineEditor Error"), wxICON_ERROR);
				return stream;
			}

            pIFxParam->GetFxParamCount(&wParamCount);
            for(int i = 0; i < wParamCount; i++)
            {
                if(is12Version || is13Version || is14Version) {
                    Uint16 wParamNameLenght;
                    stream.Read(&wParamNameLenght, sizeof(Uint16));
                    Char* strTemp = new Char[wParamNameLenght + 1];
					strTemp[0] = '\0';
                    stream.Read(strTemp, wParamNameLenght);
                    strTemp[wParamNameLenght] = '\0';
                    strParamName = strTemp;
                    SAFE_DELETE_ARRAY(strTemp);
                }
                else if(is11Version) {
                    Char strTemp[60];
                    stream.Read(strTemp, 60);
                    strParamName = strTemp;
                }
                stream.Read(&dwParamSize, sizeof(Uint32));
                pbParamValue = new Uint8[dwParamSize];
				memset(pbParamValue, 0, dwParamSize*sizeof(Uint8));
                stream.Read(pbParamValue, dwParamSize);
                pIFxParam->SetFxParamValue(strParamName, (Void*)pbParamValue);
                SAFE_DELETE_ARRAY(pbParamValue);
            }
			/*! Read string parameters */
			if(is13Version || is14Version) {
				pIFxParam->GetFxParamStringCount(&wParamCount);
				for(int i = 0; i < wParamCount; i++)
				{
					Uint16 wParamNameLenght;
                    stream.Read(&wParamNameLenght, sizeof(Uint16));
                    Char* strTemp = new Char[wParamNameLenght + 1];
					strTemp[0] = '\0';
                    stream.Read(strTemp, wParamNameLenght);
                    strTemp[wParamNameLenght] = '\0';
                    strParamName = strTemp;
                    SAFE_DELETE_ARRAY(strTemp);

					stream.Read(&dwParamSize, sizeof(Uint32));
					strTemp = new Char[dwParamSize + 1];
					strTemp[0] = '\0';
                    stream.Read(strTemp, dwParamSize);
                    strTemp[dwParamSize] = '\0';
					strParamValue = strTemp;
					pIFxParam->SetFxParamValue(strParamName, strParamValue);
					SAFE_DELETE_ARRAY(strTemp);
				}
			}

            pIFxParam->FxReleaseInterface();
            FEF_UpdateFxParam(_hFxEngine, pFx->GethFxHandle(), "", FX_PARAM_ALL);
        }
	}
/*	((DrawingView*)pview)->canvas->Refresh();
    ((DrawingView*)pview)->canvas->Update();*/

	/* Lines */
	Uint32 dwLineCount;
	Uint32 dwPointCount;
	wxPoint Point;
    int sdwPointType;
    FX_MEDIA_TYPE MediaType;
    MediaType.MainMediaType = MAIN_TYPE_UNDEFINED;
    MediaType.SubMediaType = SUB_TYPE_UNDEFINED;
	stream.Read(&dwLineCount, sizeof(Uint32));
	for(Uint32 IdxLine = 0; IdxLine < dwLineCount; IdxLine++)
	{
        if(is11Version || is12Version || is13Version || is14Version)
        {
            stream.Read(&MediaType.MainMediaType, sizeof(FX_MAIN_MEDIA_TYPE));
            stream.Read(&MediaType.SubMediaType, sizeof(FX_SUB_MEDIA_TYPE));
        }

		CNode* ppNode[6];
		memset(ppNode, NULL, 6*sizeof(CNode*));
		stream.Read(&dwPointCount, sizeof(Uint32));
		for(Uint32 IdxPoint = 0; IdxPoint < dwPointCount; IdxPoint++)
		{
			stream.Read(&Point.x, sizeof(Uint32));
			stream.Read(&Point.y, sizeof(Uint32));
            CNode* pNode = NULL;
            if(is14Version) {
                stream.Read(&sdwPointType, sizeof(int));
			    pNode = new CNode(Point, (NODE_TYPE)sdwPointType);
            }
            else 
                pNode = new CNode(Point);
			if(IdxPoint == 0)
			{
				ppNode[IdxPoint] = pNode;
				//pNode->SetEnd();
				//pFxLine->AddPoint(pNode, TRUE);
				//CNodeManager::Instance()->Link(pNode);
			}
			else if(IdxPoint == (dwPointCount-1))
			{
				if(!is14Version)
					ppNode[5] = pNode;
				else
					ppNode[IdxPoint] = pNode;
				//pNode->SetEnd();
				//pFxLine->AddPoint(pNode, TRUE);
				//CNodeManager::Instance()->Link(pNode);
			}
			else if(is14Version)
			{
				ppNode[IdxPoint] = pNode;
				//pFxLine->AddPoint(pNode);
				//pFxLine->AddConnectionPoint(pNode);
				//CNodeManager::Instance()->Link(pNode, FALSE);
			}
			else
				SAFE_DELETE_OBJECT(pNode);
		}
		CFxLine* pFxLine = new CFxLine(ppNode, 6);

		if(is14Version)
			pFxLine->UpdateAdjustedPoints(NULL); //!< set adjusted points
				
		_FxLineList.push_back(pFxLine);
		//pFxLine->Update();

        if(is11Version || is12Version || is13Version || is14Version)
        {
            pFxLine->SetMediaTypeConnection(MediaType);

            /*if( (MediaType.MainMediaType == MAIN_TYPE_UNDEFINED) &&
                (MediaType.SubMediaType == SUB_TYPE_UNDEFINED) )
		        Connect(pFxLine, NULL);
            else
                Connect(pFxLine, &MediaType);*/
        }
	}

    ((DrawingView*)pview)->canvas->TryConnection();
  /*((DrawingView*)pview)->canvas->ClearBackground();*/
  ((DrawingView*)pview)->canvas->Refresh();
  ((DrawingView*)pview)->canvas->Update();

  return stream;
}
#endif

CFx* DrawingDocument::GetFx(FX_HANDLE hFx)
{
	FxMap::iterator Itmap;
	Itmap = _FxMap.find( hFx );
	if( Itmap != _FxMap.end() )
	{
		return (Itmap->second);
	}

	return NULL;
}

//DoodleSegment::DoodleSegment(const DoodleSegment& seg)
//              :wxObject()
//{
//  /*wxList::compatibility_iterator node = seg.lines.GetFirst();
//  while (node)
//  {
//    DoodleLine *line = (DoodleLine *)node->GetData();
//    DoodleLine *newLine = new DoodleLine;
//    newLine->x1 = line->x1;
//    newLine->y1 = line->y1;
//    newLine->x2 = line->x2;
//    newLine->y2 = line->y2;
//
//    lines.Append(newLine);
//
//    node = node->GetNext();
//  }*/
//}

//DoodleSegment::~DoodleSegment(Void)
//{
////  WX_CLEAR_LIST(wxList, lines);
//}

//#if wxUSE_STD_IOSTREAM
//wxSTD ostream& DoodleSegment::SaveObject(wxSTD ostream& stream)
//{
//  wxInt32 n = lines.GetCount();
//  /*stream << n << wxT('\n');
//
//  wxList::compatibility_iterator node = lines.GetFirst();
//  while (node)
//  {
//    DoodleLine *line = (DoodleLine *)node->GetData();
//    stream << line->x1 << wxT(" ") <<
//                   line->y1 << wxT(" ") <<
//           line->x2 << wxT(" ") <<
//           line->y2 << wxT("\n");
//    node = node->GetNext();
//  }*/
//
//  return stream;
//}
//#else
//wxOutputStream &DoodleSegment::SaveObject(wxOutputStream& stream)
//{
//  /*wxTextOutputStream text_stream( stream );
//
//  wxInt32 n = lines.GetCount();
//  text_stream << n << wxT('\n');
//
//  wxList::compatibility_iterator node = lines.GetFirst();
//  while (node)
//  {
//    DoodleLine *line = (DoodleLine *)node->GetData();
//    text_stream << line->x1 << wxT(" ") <<
//                   line->y1 << wxT(" ") <<
//           line->x2 << wxT(" ") <<
//           line->y2 << wxT("\n");
//    node = node->GetNext();
//  }*/
//
//  return stream;
//}
//#endif
//
//#if wxUSE_STD_IOSTREAM
//wxSTD istream& DoodleSegment::LoadObject(wxSTD istream& stream)
//{
//  wxInt32 n = 0;
//  /*stream >> n;
//
//  for (int i = 0; i < n; i++)
//  {
//    DoodleLine *line = new DoodleLine;
//    stream >> line->x1 >>
//                   line->y1 >>
//           line->x2 >>
//           line->y2;
//    lines.Append(line);
//  }*/
//
//  return stream;
//}
//#else
//wxInputStream &DoodleSegment::LoadObject(wxInputStream& stream)
//{
//  wxTextInputStream text_stream( stream );
//
//  wxInt32 n = 0;
//  /*text_stream >> n;
//
//  for (int i = 0; i < n; i++)
//  {
//    DoodleLine *line = new DoodleLine;
//    text_stream >> line->x1 >>
//                   line->y1 >>
//           line->x2 >>
//           line->y2;
//    lines.Append(line);
//  }*/
//
//  return stream;
//}
//#endif
//Void DoodleSegment::Draw(wxDC *dc)
//{
//  /*wxList::compatibility_iterator node = lines.GetFirst();
//  while (node)
//  {
//    DoodleLine *line = (DoodleLine *)node->GetData();
//    dc->DrawLine(line->x1, line->y1, line->x2, line->y2);
//    node = node->GetNext();
//  }*/
//}

/*
 * Implementation of drawing command
 */

DrawingCommand::DrawingCommand(const wxString& name, int command, DrawingDocument *ddoc, CFx *pFx, CFxLine *pFxMine)
:wxCommand(true, name)
{
  doc = ddoc;
  _pFx = pFx;
  _pFxMine = pFxMine;

  cmd = command;
}

DrawingCommand::~DrawingCommand()
{
 /* if (_pFx)
    delete _pFx;*/
}

bool DrawingCommand::Do(void)
{
  //switch (cmd)
  //{
  //  case DOODLE_CUT:
  //  {
  //    // Cut the last segment
  //    if (doc->GetDoodleSegments().GetCount() > 0)
  //    {
  //      wxList::compatibility_iterator node = doc->GetDoodleSegments().GetLast();
  //      if (segment)
  //        delete segment;

  //      segment = (DoodleSegment *)node->GetData();
  //      doc->GetDoodleSegments().Erase(node);

  //      doc->Modify(true);
  //      doc->UpdateAllViews();
  //    }
  //    break;
  //  }
  //  case DOODLE_ADD:
  //  {
  //    doc->GetDoodleSegments().Append(new DoodleSegment(*segment));
  //    doc->Modify(true);
  //    doc->UpdateAllViews();
  //    break;
  //  }
  //}
  return true;
}

bool DrawingCommand::Undo(void)
{
  //switch (cmd)
  //{
  //  case DOODLE_CUT:
  //  {
  //    // Paste the segment
  //    if (segment)
  //    {
  //      doc->GetDoodleSegments().Append(segment);
  //      doc->Modify(true);
  //      doc->UpdateAllViews();
  //      segment = (DoodleSegment *) NULL;
  //    }
  //    doc->Modify(true);
  //    doc->UpdateAllViews();
  //    break;
  //  }
  //  case DOODLE_ADD:
  //  {
  //    // Cut the last segment
  //    if (doc->GetDoodleSegments().GetCount() > 0)
  //    {
  //      wxList::compatibility_iterator node = doc->GetDoodleSegments().GetLast();
  //      DoodleSegment *seg = (DoodleSegment *)node->GetData();
  //      delete seg;
  //      doc->GetDoodleSegments().Erase(node);

  //      doc->Modify(true);
  //      doc->UpdateAllViews();
  //    }
  //  }
  //}
  return true;
}
