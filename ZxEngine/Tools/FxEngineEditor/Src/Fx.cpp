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
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <list>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

#include "Resource.h"
#include "EditorDefs.h"

#include "FxEngine.h"

#include "Node.h"

#include "FxLine.h"

#include "NodeManager.h"

#include "FxColor.h"

#include "wx/dcbuffer.h"
#include <boost/lexical_cast.hpp>
#include "Fx.h"

// Global fonts
extern wxFont		g_Font;
extern wxFont		g_PinFont;

#define FX_STATE_SIZE (4)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFx::CFx(FX_HANDLE hFxEngine, FX_HANDLE hFx, FX_HANDLE hFxState, std::string strPath, Uint32 dwDefaultX, Uint32 dwDefaultY, std::string strFxName):
CDrawObject(FX_OBJECT)
{
	Int32 hr;

    _pFxUpleftPoint = new wxPoint;
    _pFxSize = new wxSize;
    _pFxRect = new wxRect;

    _pFxStateUpleftPoint = new wxPoint;
    _pFxStateSize = new wxSize;
    _pFxStateRect = new wxRect;


	_rgbHandleColor = wxColour(0,255,0);
	_ShouldDrawMarker = FALSE;

	_pFxUpleftPoint->x = dwDefaultX;
	_pFxUpleftPoint->y = dwDefaultY;
    _pFxRect->SetLeft(dwDefaultX);
    _pFxRect->SetTop(dwDefaultY);
    _pFxSize->x = 10;
	_pFxSize->y = 10;

    _pFxStateUpleftPoint->x = dwDefaultX;
    _pFxStateUpleftPoint->y = dwDefaultY - FX_STATE_SIZE;
    _pFxStateRect->SetLeft(dwDefaultX);
    _pFxStateRect->SetTop(dwDefaultY - FX_STATE_SIZE);
    _pFxStateSize->x = 10;
	_pFxStateSize->y = FX_STATE_SIZE;

	_hFxEngine	= hFxEngine;
	_hFx		= hFx;
	_hFxState	= hFxState;

    _IsSizesInit = FALSE;

	/*! Get Fx Infos */
    const FX_DESCRIPTOR* pFxDescriptor = NULL;
	hr = FEF_GetFxInfo(hFxEngine, hFx, &pFxDescriptor);
	if(FEF_FAILED(hr))
	{
        wxMessageBox(wxT("FEF_GetFxInfo failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return;
	}
    if(strFxName.empty())
        _strFxName = pFxDescriptor->strName;
    else
        _strFxName = strFxName;

    _strFxPath = strPath;
    _strVersion = pFxDescriptor->strVersion;
    _FxType = pFxDescriptor->FxType;
    _FxScope = pFxDescriptor->FxScope;

    /*! Initialize Fx pin */
    UpdateFxPin();

    /*! Initialize Fx Frame */
    _FxFrame = NULL;
    _FrameHeight = 0;
    _FrameWidth = 0;

    const char* pbFxFrame = NULL;
    hr = FEF_GetFxFrame(_hFxEngine, _hFx, &pbFxFrame);
    if(FEF_FAILED(hr))
	{
		wxLogMessage(NULL, wxT("FEF_GetFxFrame failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return;
	}
    if(pbFxFrame != NULL)
    {
        _FxFrame = new wxImage((const char* const*)pbFxFrame);
        _FrameHeight = _FxFrame->GetHeight();
        _FrameWidth = _FxFrame->GetWidth();

    }
}

CFx::~CFx()
{
	/*! Draw link point */
	ListOfNodeIter Iter;
	for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	{
		CNodeManager::Instance()->UnLink((*Iter));
		CNodeManager::Instance()->Remove((*Iter));
		SAFE_DELETE((*Iter));
	}
	_NodeList.clear();
    _PinTypeMap.clear();
    _PinNameMap.clear();
    _PinVector.clear();
	_FxLineList.clear();

    SAFE_DELETE(_pFxUpleftPoint);
    SAFE_DELETE(_pFxSize);
    SAFE_DELETE(_pFxRect);
    SAFE_DELETE(_FxFrame);
	SAFE_DELETE(_pFxStateUpleftPoint);
	SAFE_DELETE(_pFxStateSize);
	SAFE_DELETE(_pFxStateRect);
}

Void CFx::SetFxName(std::string strFxName)
{
    _strFxName = strFxName;

    UpdateFxPin();
    return;
}

Void CFx::InitFxPin()
{
    _pFxUpleftPoint->x = _pFxRect->GetPosition().x;
    _pFxUpleftPoint->y = _pFxRect->GetPosition().y;

    _wPinW = 8; _wPinH = 8;
    _pFxSize->x = 0;
    if(_wInPinNb > 0)
        _pFxSize->x += _wPinW+3;
    if(_wOutPinNb > 0)
        _pFxSize->x += _wPinW+3;

    _pFxSize->x += std::max((2*TEXT_SPACE), _FrameWidth + TEXT_SPACE);
    _pFxSize->y = VPIN_INTERVAL + ((VPIN_INTERVAL + PIN_SIZE)* max(_wInPinNb, _wOutPinNb));
    _pFxSize->y = std::max(_pFxSize->y, _FrameHeight + TEXT_SPACE);
    _pFxSize->y = std::max(_pFxSize->y, VPIN_INTERVAL + PIN_SIZE);
    _pFxSize->x = std::max(_pFxSize->x, _wPinW+3);

    /*! Adjust link point pos */
    wxPoint point;
    Int16 Idx = 0;
    Uint16 wInter;
    Uint16 wBegin;
    CNode* pNode = NULL;
    PinVector::iterator ItPin;
    PinNameMap::iterator It;
    PinTypeMap::iterator ItType;
    for(Int32 i = 0; i < 2; i++) { //!< Process inpin first
        if( (i == 0) && (_wInPinNb != 0) ){
            wInter = (_pFxSize->y - (_wInPinNb * PIN_SIZE)) / (_wInPinNb + 1);
            wBegin = wInter + PIN_SIZE/2;
        }
        else if ((i == 1) && (_wOutPinNb != 0) ){
            wInter = (_pFxSize->y - (_wOutPinNb * PIN_SIZE)) / (_wOutPinNb + 1);
            wBegin = wInter + PIN_SIZE/2;

        }
        else continue;
        ItPin = _PinVector.begin();
        while(ItPin != _PinVector.end())
        {
            It = _PinNameMap.find((*ItPin));
            ItType = _PinTypeMap.find((*It).first);
            FX_PIN_TYPE PinType = (*ItType).second;
            if( (i == 0) && (PinType == PIN_IN) )
            {
                point.x = _pFxUpleftPoint->x - (PIN_SIZE/2) + 1;
                point.y = _pFxUpleftPoint->y + wBegin + (Idx*wInter) + (Idx*PIN_SIZE);
            }
            else if( (i == 1) && (PinType == PIN_OUT) ) {
                point.x = _pFxUpleftPoint->x + _pFxSize->x + (PIN_SIZE/2) - 1;
                point.y = _pFxUpleftPoint->y + wBegin + (Idx*wInter) + (Idx*PIN_SIZE);
            }
            else {
                ItPin++;
                continue;
            }

            pNode = IsPinExist(It->first);
            if(pNode == NULL)
            {
                pNode = new CNode(point, (NODE_TYPE)PinType, It->first, It->second);
                _NodeList.push_back(pNode);
                CNodeManager::Instance()->Add(pNode);
            }
            else
                CNodeManager::Instance()->Move(pNode, point);

            ItPin++;
            Idx++;
        }
        Idx = 0;
    }

    wxRect r(*_pFxUpleftPoint, *_pFxSize);
    *_pFxRect = r;

    /*! FxState Rect */
    _pFxStateUpleftPoint->x = _pFxUpleftPoint->x;
    _pFxStateUpleftPoint->y = _pFxUpleftPoint->y - FX_STATE_SIZE + 1;
    _pFxStateSize->x = _pFxSize->x;
    _pFxStateSize->y = FX_STATE_SIZE;
    wxRect r2(*_pFxStateUpleftPoint, *_pFxStateSize);
    *_pFxStateRect = r2;

    return;
}

Void CFx::Draw(wxDC* pDC)
{
    AutoLock lock(_CSDraw);

    _pFxUpleftPoint->x = _pFxRect->GetPosition().x;
    _pFxUpleftPoint->y = _pFxRect->GetPosition().y;

    _pFxStateUpleftPoint->x = _pFxStateRect->GetPosition().x;
    _pFxStateUpleftPoint->y = _pFxStateRect->GetPosition().y;

    if(_IsSizesInit == FALSE)
    {
        /*wxFont	Font = *_pFont;*/
        pDC->SetFont(g_Font);

        pDC->GetTextExtent(wxT("00"), &_wPinW, &_wPinH);
        _pFxSize->x = 0;
        if(_wInPinNb > 0)
            _pFxSize->x += _wPinW+3;
        if(_wOutPinNb > 0)
            _pFxSize->x += _wPinW+3;

        _pFxSize->x += std::max((2*TEXT_SPACE), _FrameWidth + TEXT_SPACE);
        _pFxSize->y = VPIN_INTERVAL + ((VPIN_INTERVAL + PIN_SIZE)* max(_wInPinNb, _wOutPinNb));
        _pFxSize->y = std::max(_pFxSize->y, _FrameHeight + TEXT_SPACE);
        _pFxSize->y = std::max(_pFxSize->y, VPIN_INTERVAL + PIN_SIZE);
        _pFxSize->x = std::max(_pFxSize->x, _wPinW+3);

	    /*! Adjust link point pos */
	    wxPoint point;
	    Int16 Idx = 0;
        Uint16 wInter;
        Uint16 wBegin;
        CNode* pNode = NULL;
        PinNameMap::iterator It;
        PinVector::iterator ItPin;
        PinTypeMap::iterator ItType;
        for(Int32 i = 0; i < 2; i++) {
            if( (i == 0) && (_wInPinNb != 0) ){
                wInter = (_pFxSize->y - (_wInPinNb * PIN_SIZE)) / (_wInPinNb + 1);
                wBegin = wInter + PIN_SIZE/2;
            }
            else if ((i == 1) && (_wOutPinNb != 0) ){
                wInter = (_pFxSize->y - (_wOutPinNb * PIN_SIZE)) / (_wOutPinNb + 1);
                wBegin = wInter + PIN_SIZE/2;

            }
            else continue;
            ItPin = _PinVector.begin();
            while(ItPin != _PinVector.end())
            {
                It = _PinNameMap.find((*ItPin));
                ItType = _PinTypeMap.find((*It).first);
                FX_PIN_TYPE PinType = (*ItType).second;
                if( (i == 0) && (PinType == PIN_IN) )
                {
                    point.x = _pFxUpleftPoint->x - (PIN_SIZE/2) + 1;
		            point.y = _pFxUpleftPoint->y + wBegin + (Idx*wInter) + (Idx*PIN_SIZE);
                }
                else if( (i == 1) && (PinType == PIN_OUT) ) {
                    point.x = _pFxUpleftPoint->x + _pFxSize->x + (PIN_SIZE/2) - 1;
		            point.y = _pFxUpleftPoint->y + wBegin + (Idx*wInter) + (Idx*PIN_SIZE);
                }
                else {
                    ItPin++;
                    continue;
                }

                pNode = IsPinExist(It->first);
	            if(pNode == NULL)
	            {
                    pNode = new CNode(point, (NODE_TYPE)PinType, It->first,  It->second);
	                _NodeList.push_back(pNode);
	                CNodeManager::Instance()->Add(pNode);
	            }
	            else
		            CNodeManager::Instance()->Move(pNode, point);

                ItPin++;
                Idx++;
            }
            Idx = 0;
        }

	    wxRect r(*_pFxUpleftPoint, *_pFxSize);
	    *_pFxRect = r;

        /*! FxState Rect */
        _pFxStateUpleftPoint->x = _pFxUpleftPoint->x;
        _pFxStateUpleftPoint->y = _pFxUpleftPoint->y - FX_STATE_SIZE + 1;
        _pFxStateSize->x = _pFxSize->x;
	    _pFxStateSize->y = FX_STATE_SIZE;
        wxRect r2(*_pFxStateUpleftPoint, *_pFxStateSize);
        *_pFxStateRect = r2;
        
        _IsSizesInit = TRUE;
    }
	//Char str[20];
	wxString str;

    wxBrush	brush;
	brush.SetStyle(wxSOLID);
    /*! Draw Fx State */
    FX_STATE FxState;
    FEF_GetFxState(_hFxEngine, _hFx, &FxState);
    brush.SetColour( GetFxStateColor(FxState));
    pDC->SetBrush(brush);
    pDC->SetPen(*wxBLACK_PEN);
    pDC->DrawRectangle(*_pFxStateRect);
    
	/*! Draw Fx rectangle */
	brush.SetColour( GetFxColor(_FxType));
    pDC->SetBrush(brush);
    //pDC->SetPen(*wxBLACK_PEN);
    pDC->DrawRectangle(*_pFxRect);
	pDC->SetPen(wxNullPen);
    pDC->SetBrush(wxNullBrush);

    /*! Draw Fx frame */
    if(_FxFrame) {
        wxBitmap bitmap( *_FxFrame );
        if( (_wInPinNb > 0) && (_wOutPinNb == 0) )
            pDC->DrawBitmap(bitmap, _pFxUpleftPoint->x + _wPinW + 3 + 1,
                                    _pFxUpleftPoint->y + (_pFxSize->y/2) - _FrameHeight/2);
        else if( (_wInPinNb == 0) && (_wOutPinNb > 0) )
            pDC->DrawBitmap(bitmap, _pFxUpleftPoint->x + 3,
                                    _pFxUpleftPoint->y + (_pFxSize->y/2) - _FrameHeight/2);
        else

            pDC->DrawBitmap(bitmap, _pFxUpleftPoint->x + (_pFxSize->x/2) - _FrameWidth/2,
                                    _pFxUpleftPoint->y + (_pFxSize->y/2) - _FrameHeight/2);
    }

	/*! Draw Fx short Name */
	wxString strFxName = s2ws(_strFxName);
	pDC->SetFont(g_Font);
    pDC->SetTextForeground(*wxLIGHT_GREY);
	int mode = pDC->GetBackgroundMode();
    pDC->SetBackgroundMode(wxTRANSPARENT);
    pDC->DrawText( strFxName, _pFxUpleftPoint->x , _pFxUpleftPoint->y + _pFxSize->y + 1);
	pDC->SetBackgroundMode( mode );

    /*! Draw link point */
	wxPoint point;
	ListOfNodeIter Iter;
	for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	    (*Iter)->Draw(pDC);

    DrawMarker(pDC);

    /*! Draw Pin number */
	Int16	wPinNumber = 0;
    int wPinW;
	pDC->SetFont(g_PinFont);
	pDC->SetTextForeground(*wxWHITE);
	for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	{
		//sprintf(str, "%d", wPinNumber);
		str << wPinNumber;
        str = s2ws(boost::lexical_cast<std::string>(wPinNumber));
        pDC->GetTextExtent(str, &wPinW, &_wPinH);
		(*Iter)->GetPoint(&point);
        PinTypeMap::iterator It = _PinTypeMap.find((*Iter)->GetIFxPin());
        if(It->second == PIN_IN)
		{
			pDC->DrawText( str, _pFxUpleftPoint->x + 3, point.y - FONT_HEIGHT/2 + 1);
		}
		else
		{
			pDC->DrawText( str, _pFxRect->GetRight() - (3 + wPinW), point.y - FONT_HEIGHT/2 + 1);
		}
		wPinNumber++;
	}

    for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	    (*Iter)->DrawName(pDC);

	pDC->SetTextForeground(*wxBLACK);

    pDC->SetPen(wxNullPen);
    pDC->SetBrush(wxNullBrush);
	pDC->SetBackgroundMode( mode );
    return;
}

Void CFx::DrawMarker(wxDC* pDC)
{
	if(_ShouldDrawMarker)
	{
	    wxPen	pen(_rgbHandleColor, 1,  wxSOLID);

		pDC->SetPen(pen);
		pDC->DrawLine(_pFxUpleftPoint->x, _pFxStateUpleftPoint->y, _pFxRect->GetRight(), _pFxStateUpleftPoint->y);

		pDC->DrawLine(_pFxRect->GetRight(), _pFxStateUpleftPoint->y, _pFxRect->GetRight(), _pFxRect->GetBottom());

		pDC->DrawLine(_pFxRect->GetRight(), _pFxRect->GetBottom(), _pFxUpleftPoint->x, _pFxRect->GetBottom());

		pDC->DrawLine(_pFxUpleftPoint->x, _pFxRect->GetBottom(), _pFxUpleftPoint->x, _pFxStateUpleftPoint->y);

		pDC->SetPen(wxNullPen);
	}
}

Bool CFx::IsPointInFx(wxPoint& Point)
{
    return(_pFxRect->Contains(Point) || _pFxStateRect->Contains(Point));
}

Bool CFx::IsObjectInRect(wxRect& Rect)
{
    return (Rect.Intersects(*_pFxRect) || Rect.Intersects(*_pFxStateRect));
}

Void CFx::OffSetObject(Int32 dwX, Int32 dwY)
{
    if( (_pFxRect->GetBottomRight().x + dwX > MAX_HWORKINGSIZE) || (_pFxRect->GetBottomRight().y + dwY > MAX_VWORKINGSIZE) )
        return;

    if( (_pFxRect->GetTopLeft().x + dwX < 0) || (_pFxRect->GetTopLeft().y + dwY < 0) )
        return;

	_pFxRect->Offset(dwX, dwY);
    _pFxStateRect->Offset(dwX, dwY);
	ListOfNodeIter Iter;
	for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	{
		wxPoint point;
		(*Iter)->GetPoint(&point);
        point += wxPoint(dwX, dwY);
		CNodeManager::Instance()->Move((*Iter), point);
	}
}

Void CFx::SetMarker(Bool ShouldDraw)
{
	_ShouldDrawMarker = ShouldDraw;
}

wxRect CFx::GetRect()
{
	wxRect Rect = *_pFxRect;
    Rect.Union(*_pFxStateRect);
	lFxLineIter Iter;
	Bool IsFound = FALSE;

	IsFound = FALSE;
	for (Iter = _FxLineList.begin(); (Iter != _FxLineList.end()) && !(_FxLineList.empty()); Iter++)
	{
        Rect.Union((*Iter)->GetRect());
	}
	return Rect;
}

Bool CFx::AttachFxLine(IFxPin* pIFxPin, CFxLine* pFxLine)
{
    lFxLineIter It;
	It = std::find(_FxLineList.begin(), _FxLineList.end(), pFxLine);
	if( It != _FxLineList.end() )
        return TRUE;

	ListOfNodeIter Iter;
	for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	{
		if((*Iter)->GetIFxPin() == pIFxPin)
		{
			_FxLineList.push_back(pFxLine);
			return TRUE;
		}
	}
	return FALSE;
}

Bool CFx::DetachFxLine(CFxLine* pFxLine, IFxPin* pIFxPin)
{
	if(pIFxPin != NULL) {
		Bool IsFound = FALSE;
		PinVector::iterator ItPin;
		ItPin = _PinVector.begin();
		while(ItPin != _PinVector.end()){
			if( (*ItPin) == pIFxPin ){
				IsFound = TRUE;
				break;
			}
			ItPin++;
		}
		if(IsFound == FALSE)
			return FALSE;
	}
	lFxLineIter It;
	It = std::find(_FxLineList.begin(), _FxLineList.end(), pFxLine);
	if( It != _FxLineList.end() )
	{
		_FxLineList.erase(It);
		return TRUE;
	}

/*	Bool IsFound = FALSE;

	IsFound = FALSE;
	for (Iter = _FxLineList.begin(); (Iter != _FxLineList.end()) && !(_FxLineList.empty()); Iter++)
	{
		if((*Iter) == pFxLine)
		{
			IsFound = TRUE;
			break;
		}
	}
	if(IsFound == TRUE)
	{
		_FxLineList.erase(Iter);
		return TRUE;
	}*/

	return FALSE;
}

Void CFx::EnableSetPath(Bool EnableSetPath)
{
	lFxLineIter Iter;
	for (Iter = _FxLineList.begin(); (Iter != _FxLineList.end()) && !(_FxLineList.empty()); Iter++)
		(*Iter)->EnableSetPath(EnableSetPath);
	return;
}

Bool CFx::UpdateFxPin()
{
    AutoLock lock(_CSDraw);
	Int32 hr;
	FX_PIN_TYPE PinType;
    std::string strPinName;

    Uint16 wCurrentInPin = _wInPinNb;
    Uint16 wCurrentOutPin = _wOutPinNb;

    _wOutPinNb = 0;
    _wInPinNb = 0;

    Uint16 wPinCount;
	hr = FEF_GetFxPinCount( /* [in] */_hFxEngine,
						/* [in] */_hFx,
						/* [out] */&wPinCount);
	if(FEF_FAILED(hr))
	{
        wxMessageBox(wxT("FEF_GetFxPinCount failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
        return FALSE;
	}

	IFxPin* pIFxPin;
	wxPoint point;
	Int16 Idx;

    _PinTypeMap.clear();
    _PinNameMap.clear();
    _PinVector.clear();
    for(Idx = 0; Idx < wPinCount; Idx++)
	{
		hr = FEF_QueryFxPinInterface(/* [in] */_hFxEngine,
							/* [in] */_hFx,
							/* [out] */&pIFxPin,
							/* [in] */Idx);
		if(FEF_FAILED(hr))
		{
            wxMessageBox(wxT("FEF_QueryFxPinInterface failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
            return FALSE;
		}

		pIFxPin->GetPinType(/* [out] */&PinType);
        if(PinType == PIN_IN) _wInPinNb++; else _wOutPinNb++;
        pIFxPin->GetPinName(strPinName);
		_PinTypeMap.insert(make_pair(pIFxPin, PinType));
        _PinNameMap.insert(make_pair(pIFxPin, strPinName));
        _PinVector.push_back(pIFxPin);
	}

    /*! Remove pin if necessary */
    ListOfNodeIter Iter;
    lFxIter It;
    Bool IsFound = FALSE;
    for (Iter = _NodeList.begin(); Iter != _NodeList.end();)
	{
        IsFound = FALSE;
        pIFxPin = (*Iter)->GetIFxPin();
        if(_PinTypeMap.find(pIFxPin) == _PinTypeMap.end()) {
            CNode* pNode = (*Iter);
            CNodeManager::Instance()->UnLink(pNode);
		    CNodeManager::Instance()->Remove(pNode);
		    SAFE_DELETE(pNode);
            Iter = _NodeList.erase(Iter);
        }
        else
            ++Iter;
    }

    _IsSizesInit = FALSE;

    /*! Detect new pin */
    if( (wCurrentInPin < _wInPinNb) || (wCurrentOutPin < _wOutPinNb) )
        return TRUE;

    return FALSE;
}

Void CFx::UpdateFxFrame()
{
    AutoLock lock(_CSDraw);
    Int32 hr;

    SAFE_DELETE(_FxFrame);

    _FxFrame = NULL;
    _FrameHeight = 0;
    _FrameWidth = 0;
    const char* pbFxFrame = NULL;
    hr = FEF_GetFxFrame(_hFxEngine, _hFx, &pbFxFrame);
    if(FEF_FAILED(hr))
	{
        wxMessageBox(wxT("FEF_GetFxFrame failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
        return;
	}
    if(pbFxFrame != NULL)
    {
        _FxFrame = new wxImage((const char* const*)pbFxFrame);
        _FrameHeight = _FxFrame->GetHeight();
        _FrameWidth = _FxFrame->GetWidth();
    }

    return;
}

CNode* CFx::IsPinExist(IFxPin* pIFxPin)
{
	ListOfNodeIter Iter;
	for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	{
		if((*Iter)->GetIFxPin() == pIFxPin)
		{
			return (*Iter);
		}
	}
	return NULL;
}

Void CFx::UpdateTxRx()
{
	ListOfNodeIter Iter;
	for (Iter = _NodeList.begin(); (Iter != _NodeList.end()) && !(_NodeList.empty()); Iter++)
	    (*Iter)->UpdateTxRx();
	return;
}
