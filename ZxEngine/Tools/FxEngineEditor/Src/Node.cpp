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

#include "Resource.h"
#include "EditorDefs.h"

#include "FxEngine.h"

#include "Node.h"

#include <sstream>

extern bool        g_showAllTxRx;

wxPen	g_FocusPen(wxColour(/*253,176,238*/136,242,251)/**wxRED*/, 2,  wxSHORT_DASH/*wxSOLID*/);
wxFont  g_NameFont;
/*----------------------------------------------------------------------*//*!
				 Construction/Destruction
*//*-----------------------------------------------------------------------*/
CNode::CNode(wxPoint& Point, NODE_TYPE NodeType, IFxPin* pIFxPin, std::string strName)
{
#ifdef WIN32
    g_NameFont = wxFont(wxSize(8,12), wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxT( "Courier New" ));
#else
    g_NameFont = wxFont(8, wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxT( "Courier New" ));
#endif
    _ShouldDrawOver = FALSE;
	_NodePoint = Point;
	_NodeRegion = wxRegion( Point.x - NODE_REGION_SIZE/2,
										 Point.y - NODE_REGION_SIZE/2,
										 NODE_REGION_SIZE,
										 NODE_REGION_SIZE);
	_pIFxPin = pIFxPin;

	/*! Set the TxRx drawing offset */
	if(_pIFxPin){
		_pIFxPin->GetPinType(&_FxPinType);
		if(_FxPinType == PIN_IN) {
			_sdwTxRxDrawoffset = - 0.75*PIN_SIZE;
			_sdwTxRxDrawfactor = - 1;
		}
		else {
			_sdwTxRxDrawoffset = 0.75*PIN_SIZE;
			_sdwTxRxDrawfactor = 0;
		}
	}

	_NodeType = NodeType;

	_IsEnd = FALSE;

    _strName = "Undefined Pin Name";
    if(!strName.empty())
        _strName = strName;

	_strName = " " + _strName + " ";

	_qTxRxByte = 0;
}

CNode::~CNode()
{
	_NodeRegion.Clear();
}

Void CNode::Draw(wxDC* pDC)
{
    pDC->SetBrush(*wxWHITE_BRUSH);
    pDC->SetPen(*wxBLACK_PEN);
    pDC->DrawRectangle(_NodePoint.x - (PIN_SIZE/2),
							_NodePoint.y - (PIN_SIZE/2),
							PIN_SIZE,
							PIN_SIZE);

    pDC->SetPen(wxNullPen);
    pDC->SetBrush(wxNullBrush);
    return;
}

Void CNode::DrawName(wxDC* pDC)
{
	FX_PIN_STATE PinState;
	wxCoord  width;
    wxCoord  height;
    if(_ShouldDrawOver)
    {
        pDC->SetBrush(*wxTRANSPARENT_BRUSH);
        pDC->SetPen(g_FocusPen);
        pDC->DrawRectangle(_NodePoint.x - (3+PIN_SIZE/2),
							_NodePoint.y - (3+PIN_SIZE/2),
							2*3+PIN_SIZE,
							2*3+PIN_SIZE);
        if(!_strName.empty()) {
            pDC->SetFont(g_NameFont);
            pDC->SetTextForeground(*wxWHITE);
            wxString strName = s2ws(_strName);
            pDC->GetTextExtent( strName , &width, &height);
            pDC->SetBrush( wxColour(BACKGROUND_R, BACKGROUND_G, BACKGROUND_V));
            pDC->SetPen(*wxBLACK_PEN);
            pDC->DrawRectangle( _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y - 3*PIN_SIZE, width, height );

            pDC->DrawText( strName, _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y - 3*PIN_SIZE);

			/*pDC->DrawRectangle( _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y, width, height );

            pDC->DrawText( strName, _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y);*/

            pDC->SetTextForeground(*wxBLACK);
        }
		_pIFxPin->GetPinState(&PinState);
		if(!g_showAllTxRx && PinState == PIN_CONNECTED)
		{
			pDC->SetFont(g_NameFont);
            pDC->SetTextForeground(*wxBLACK);
            std::stringstream strValue64bit; strValue64bit << _qTxRxByte;
            wxString strName(s2ws(strValue64bit.str())); //strName.sprintf(wxT(" %ld "), _qTxRxByte);
            pDC->GetTextExtent( strName , &width, &height);
            pDC->SetBrush( wxColour(BACKGROUND_R, BACKGROUND_G, BACKGROUND_V));
            //pDC->SetPen(*wxBLACK_PEN);
            //pDC->DrawRectangle( _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y - height/2, width, height );

            pDC->DrawText( strName, _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y - 0.75*height);

            //pDC->SetTextForeground(*wxBLACK);
		}
		pDC->SetBrush(wxNullBrush);
        
    }
	if(g_showAllTxRx)
	{
		_pIFxPin->GetPinState(&PinState);
		if(PinState == PIN_CONNECTED)
		{
			pDC->SetFont(g_NameFont);
            pDC->SetTextForeground(*wxBLACK);
            std::stringstream strValue64bit; strValue64bit << _qTxRxByte;
            wxString strName(s2ws(strValue64bit.str())); //strName.sprintf(wxT(" %ld "), _qTxRxByte);
            pDC->GetTextExtent( strName , &width, &height);
            pDC->SetBrush( wxColour(BACKGROUND_R, BACKGROUND_G, BACKGROUND_V));
            //pDC->SetPen(*wxBLACK_PEN);
            //pDC->DrawRectangle( _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y - height/2, width, height );

            pDC->DrawText( strName, _NodePoint.x + _sdwTxRxDrawoffset + (_sdwTxRxDrawfactor * width), _NodePoint.y - 0.75*height);

            //pDC->SetTextForeground(*wxBLACK);
		}
	}
	
    return;
}

Void CNode::GetPoint(wxPoint* pPoint)
{
	pPoint->x = _NodePoint.x;
	pPoint->y = _NodePoint.y;

	return;
}

wxPoint* CNode::GetPoint()
{
	return &_NodePoint;
}

Void CNode::GetRect(wxRect* pRec)
{
	*pRec = _NodeRegion.GetBox();
	return;
}

Void CNode::OffSet(Int32 sdwX, Int32 sdwY)
{
	_NodePoint.x += sdwX; _NodePoint.y += sdwY; 
	_NodeRegion = wxRegion(_NodePoint.x - NODE_REGION_SIZE/2,
							_NodePoint.y - NODE_REGION_SIZE/2,
							NODE_REGION_SIZE,
							NODE_REGION_SIZE);
}

Int32 CNode::Move(wxPoint* pPoint)
{
	_NodePoint = *pPoint;
	_NodeRegion = wxRegion(pPoint->x - NODE_REGION_SIZE/2,
							pPoint->y - NODE_REGION_SIZE/2,
							NODE_REGION_SIZE,
							NODE_REGION_SIZE);
	return 0;
}

Void CNode::SetPoint(wxPoint* pPoint)
{
	_NodePoint.x = pPoint->x; _NodePoint.y = pPoint->y;
	_NodeRegion = wxRegion(pPoint->x - NODE_REGION_SIZE/2,
							pPoint->y - NODE_REGION_SIZE/2,
							NODE_REGION_SIZE,
							NODE_REGION_SIZE);
}
Void CNode::SetPoint(int sdwX, int sdwY)
{
	_NodePoint.x = sdwX; _NodePoint.y = sdwY; 
	_NodeRegion = wxRegion(_NodePoint.x - NODE_REGION_SIZE/2,
							_NodePoint.y - NODE_REGION_SIZE/2,
							NODE_REGION_SIZE,
							NODE_REGION_SIZE);
}

Bool CNode::NodeInRegion(CNode* pNode)
{
	if( (GetIFxPin() == NULL) && (pNode->GetIFxPin() == NULL) )
		return FALSE;

	if( (GetIFxPin() != NULL) && (pNode->GetIFxPin() != NULL) )
		return FALSE;

	wxRect Rect;
	pNode->GetRect(&Rect);
	if(_NodeRegion.Contains(Rect) != wxOutRegion)
        return TRUE;

    return FALSE;
}

Bool CNode::PointInRegion(wxPoint& point)
{
    wxRect Rect(point.x - (NODE_REGION_SIZE/2),
							point.y - (NODE_REGION_SIZE/2),
							NODE_REGION_SIZE,
							NODE_REGION_SIZE);
    if(_NodeRegion.Contains(Rect) != wxOutRegion)
        return TRUE;

	return FALSE;
}

Bool CNode::IsFx()
{
	return (_pIFxPin != NULL) ? TRUE : FALSE;
}

Void CNode::DrawOver(Bool ShouldDrawOver)
{
	_ShouldDrawOver = ShouldDrawOver;
	return;		
}

Void CNode::UpdateTxRx()
{
	if(_pIFxPin){
		_pIFxPin->GetTxRxBytes(&_qTxRxByte);
	}


	return;
}
