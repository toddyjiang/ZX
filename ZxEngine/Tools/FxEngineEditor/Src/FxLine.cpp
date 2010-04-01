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

#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace std;

#include "Resource.h"
#include "EditorDefs.h"

#include "FxEngine.h"

#include "Node.h"
#include "AdjustNode.h"

#include "NodeManager.h"

//#include "FxDC.h"
#include "FxLine.h"

#ifdef WIN32
#include "wingdi.h"     //!< LineDDA
#endif


#ifdef WIN32
Void FEF_CALLBACK HitTest( int X, int Y, LPARAM data );
#endif

const int g_sdwPixelConst = 10;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFxLine::CFxLine(CNode** ppNode, Uint8 wNbPoint):
CDrawObject(LINE_OBJECT)
{
	_rgbHandleColor = wxColour(0,255,0);
	_rgbAdjustedColor = wxColour(255,255,0);
	_rgbHidleColor = wxColour(0,0,0);

	_IsConnected = FALSE;
	_EnableSetPath = FALSE;
	
    _MediaType.MainMediaType = MAIN_TYPE_UNDEFINED;
    _MediaType.SubMediaType = SUB_TYPE_UNDEFINED;

	for(Uint8 i = 0; i < wNbPoint; i++) {
		if(ppNode[i] != NULL)
			if( (i == 0) || (i == (wNbPoint-1)) ){
				ppNode[i]->SetEnd();
				AddPoint(ppNode[i], TRUE);
			}
			else
				AddPoint(ppNode[i]);
			//_NodeVector.push_back(ppNode[i]);
	}

	for(Uint8 i = 0; i < wNbPoint; i++) {
		if(ppNode[i] == NULL)
			AddConnectionPoint();
	}

	_pAdjustedNodes[0] = NULL;
	_pAdjustedNodes[1] = NULL;
	_pAdjustedNodes[2] = NULL;
	
	wxPoint point = *_NodeVector[0]->GetPoint();
	_pAdjustedNodes[0] = new CAdjustNode(point, _NodeVector[1], _NodeVector[2], ADJUST_X);
	CNodeManager::Instance()->Add(_pAdjustedNodes[0]);
	_pAdjustedNodes[1] = new CAdjustNode(point, _NodeVector[2], _NodeVector[3], ADJUST_X);
	CNodeManager::Instance()->Add(_pAdjustedNodes[1]);
	_pAdjustedNodes[2] = new CAdjustNode(point, _NodeVector[3], _NodeVector[4], ADJUST_X);
	CNodeManager::Instance()->Add(_pAdjustedNodes[2]);
}

CFxLine::CFxLine(CNode* pBeginPoint, CNode* pEndPoint):
CDrawObject(LINE_OBJECT)
{
	_NodeVector.push_back(pBeginPoint);
	CNodeManager::Instance()->Add(pBeginPoint);
	_NodeVector.push_back(pEndPoint);
	CNodeManager::Instance()->Add(pEndPoint);

	/*! Add 4 points to build path */
	for(int p = 0; p < 4; p++)
		AddConnectionPoint();

	_ShouldDrawMarker = FALSE;
	_IsConnected = FALSE;
	_EnableSetPath = TRUE;
	
    _MediaType.MainMediaType = MAIN_TYPE_UNDEFINED;
    _MediaType.SubMediaType = SUB_TYPE_UNDEFINED;

	_pAdjustedNodes[0] = NULL;
	_pAdjustedNodes[1] = NULL;
	_pAdjustedNodes[2] = NULL;

	wxPoint point = *_NodeVector[0]->GetPoint();
	_pAdjustedNodes[0] = new CAdjustNode(point, _NodeVector[1], _NodeVector[2], ADJUST_X);
	CNodeManager::Instance()->Add(_pAdjustedNodes[0]);
	_pAdjustedNodes[1] = new CAdjustNode(point, _NodeVector[2], _NodeVector[3], ADJUST_X);
	CNodeManager::Instance()->Add(_pAdjustedNodes[1]);
	_pAdjustedNodes[2] = new CAdjustNode(point, _NodeVector[3], _NodeVector[4], ADJUST_X);
	CNodeManager::Instance()->Add(_pAdjustedNodes[2]);

	_rgbHandleColor = wxColour(0,255,0);
	_rgbAdjustedColor = wxColour(255,255,0);
}

CFxLine::~CFxLine()
{
	/*! Clear points */
	vPointIter Iter;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		CNodeManager::Instance()->UnLink((*Iter));
		CNodeManager::Instance()->Remove((*Iter));
		SAFE_DELETE((*Iter));
	}
	_NodeVector.clear();

	for(int i = 0; i < 3; i++) {
		CNodeManager::Instance()->UnLink(_pAdjustedNodes[i]);
		CNodeManager::Instance()->Remove(_pAdjustedNodes[i]);
	}

	SAFE_DELETE_OBJECT(_pAdjustedNodes[0]);
	SAFE_DELETE_OBJECT(_pAdjustedNodes[1]);
	SAFE_DELETE_OBJECT(_pAdjustedNodes[2]);
}

Void CFxLine::AddPoint(CNode* pPoint, Bool isEnd)
{
	_NodeVector.push_back(pPoint);
	if(isEnd)
		CNodeManager::Instance()->Add(pPoint);
	_ShouldDrawMarker = FALSE;

	return;
}

Void CFxLine::Draw(wxDC* pDC)
{
	/*! Draw line in vector */
	wxBrush	brush;
	//wxBrush* pOldBrush;
	vPointIter Iter;
	wxPoint CurrentPoint;
	wxPoint PrevPoint;

	/*! Test if line is connected */
	IFxPin* pIfxPin1 = NULL;
	IFxPin* pIfxPin2 = NULL;
	GetBeginIFxPin(&pIfxPin1);
	GetEndIFxPin(&pIfxPin2);
	
	if(pIfxPin1 && pIfxPin2){
		pDC->SetPen(*wxWHITE);
	}
    else
        pDC->SetPen(*wxBLACK);

	if(_EnableSetPath)
		SetPath();

	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		(*Iter)->GetPoint(&CurrentPoint);

         //pDC->MoveTo(CurrentPoint.x, CurrentPoint.y);
		if(Iter != _NodeVector.begin())
		{
			pDC->DrawLine(CurrentPoint.x, CurrentPoint.y, PrevPoint.x, PrevPoint.y);
		}

		PrevPoint = CurrentPoint;
	}
	pDC->SetPen(wxNullPen);

	if(_ShouldDrawMarker)
	{
		brush.SetStyle(wxSOLID);
		brush.SetColour(_rgbAdjustedColor);
		pDC->SetBrush(brush);

		/*! Draw Adjusted points */
		for(int i = 0; i < 3; i++) {
			if(_pAdjustedNodes[i]->IsDrawOver()) {
				_pAdjustedNodes[i]->GetPoint(&CurrentPoint);
				pDC->DrawRectangle(CurrentPoint.x - (PIN_SIZE/2),
									CurrentPoint.y - (PIN_SIZE/2),
									(PIN_SIZE),
									(PIN_SIZE));
			}
		}

		/*for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
		{
			(*Iter)->GetPoint(&CurrentPoint);*/

			//brush.CreateSolidBrush(_rgbHandleColor);
			
			brush.SetColour(_rgbHandleColor);
			pDC->SetBrush(brush);
			_NodeVector[0]->GetPoint(&CurrentPoint);
			pDC->DrawRectangle(CurrentPoint.x - (PIN_SIZE/2),
							CurrentPoint.y - (PIN_SIZE/2),
							(PIN_SIZE),
							(PIN_SIZE));

			_NodeVector[5]->GetPoint(&CurrentPoint);
			pDC->DrawRectangle(CurrentPoint.x - (PIN_SIZE/2),
							CurrentPoint.y - (PIN_SIZE/2),
							(PIN_SIZE),
							(PIN_SIZE));

			//pDC->SelectObject(pOldBrush);
			//brush.DeleteObject();
		/*}*/
		pDC->SetBrush(wxNullBrush);
	}
		/*else if((Iter != _NodeVector.begin()) && (Iter != _NodeVector.end()-1))
		{
			brush.CreateSolidBrush(_rgbHidleColor);
			pOldBrush = pDC->SelectObject(&brush);
			wxRect handleRect;

			pDC->Rectangle(CurrentPoint.x - (PIN_SIZE/2),
							CurrentPoint.y - (PIN_SIZE/2),
							CurrentPoint.x + (PIN_SIZE/2),
							CurrentPoint.y + (PIN_SIZE/2));

			pDC->SelectObject(pOldBrush);
			brush.DeleteObject();
		}*/

	return;
}

Void CFxLine::DrawMarker(wxDC* pDC)
{
	wxBrush	brush;
	//wxBrush* pOldBrush;
	vPointIter Iter;
	wxPoint CurrentPoint;

	if(_ShouldDrawMarker)
	{
		brush.SetStyle(wxSOLID);
		brush.SetColour(_rgbAdjustedColor);
		pDC->SetBrush(brush);
		
		/*! Draw Adjusted points */
		for(int i = 0; i < 3; i++) {
			if(_pAdjustedNodes[i]->IsDrawOver()) {
				_pAdjustedNodes[i]->GetPoint(&CurrentPoint);
				pDC->DrawRectangle(CurrentPoint.x - (PIN_SIZE/2),
									CurrentPoint.y - (PIN_SIZE/2),
									(PIN_SIZE),
									(PIN_SIZE));
			}
		}
	
		/*for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
		{*/
			brush.SetColour(_rgbHandleColor);
			pDC->SetBrush(brush);
			_NodeVector[0]->GetPoint(&CurrentPoint);
			pDC->DrawRectangle(CurrentPoint.x - (PIN_SIZE/2),
							CurrentPoint.y - (PIN_SIZE/2),
							(PIN_SIZE),
							(PIN_SIZE));

			_NodeVector[5]->GetPoint(&CurrentPoint);
			pDC->DrawRectangle(CurrentPoint.x - (PIN_SIZE/2),
							CurrentPoint.y - (PIN_SIZE/2),
							(PIN_SIZE),
							(PIN_SIZE));

			

		//}
		pDC->SetBrush(wxNullBrush);
	}
	return;
}

Void CFxLine::GetBeginIFxPin(IFxPin** ppIfxPin)
{
	CNode* pNode = (*(_NodeVector.begin()));
	CNode* pFirstNode = NULL;
	pFirstNode = CNodeManager::Instance()->GetFirstLink(pNode);
	if(pFirstNode)
	{
		*ppIfxPin = pFirstNode->GetIFxPin();
		return;
	}
	*ppIfxPin = NULL;
	return;
}

Void CFxLine::GetEndIFxPin(IFxPin** ppIfxPin)
{
	CNode* pNode = (_NodeVector[_NodeVector.size() - 1]);
	CNode* pFirstNode = NULL;
	pFirstNode = CNodeManager::Instance()->GetFirstLink(pNode);
	if(pFirstNode)
	{
		*ppIfxPin = pFirstNode->GetIFxPin();
		return;
	}
	*ppIfxPin = NULL;
	return;
}

Void CFxLine::SetMarker(Bool ShouldDraw)
{
	_ShouldDrawMarker = ShouldDraw;
}

int round(float a)
{
	if(a>=0)
   	return (int)(a+0.5);
   else
   	return (int)(a-0.5);
}

Bool CFxLine::IsObjectInRect(wxRect& Rect)
{
    vPointIter Iter;
	wxPoint CurrentPoint;
	wxPoint PrevPoint;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		(*Iter)->GetPoint(&CurrentPoint);

		if(Iter != _NodeVector.begin())
		{
            int dx=CurrentPoint.x-PrevPoint.x, dy=CurrentPoint.y-PrevPoint.y, steps, k;
            int X;
            int Y;
            float xi, yi, x=PrevPoint.x, y=PrevPoint.y;

            steps = abs(dx)>abs(dy)?abs(dx):abs(dy);
            xi=dx/(float)steps;		//Will be 1 or 1/m
            yi=dy/(float)steps;		//Will be m or 1

            for(k=0;k<steps;k++){
                X = round(x); Y = round(y);
                if( Rect.Contains(X, Y) )
                    return(TRUE);
                x += xi;
                y += yi;
            }
        }
        PrevPoint = CurrentPoint;
    }
    return FALSE;
}

Bool CFxLine::IsPointInFx(wxPoint& Point)
{
	hitParams hit;
	hit.hit = FALSE;
	hit.x = Point.x;
	hit.y = Point.y;

	vPointIter Iter;
	wxPoint CurrentPoint;
	wxPoint PrevPoint;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		(*Iter)->GetPoint(&CurrentPoint);

		if(Iter != _NodeVector.begin())
		{
#ifndef WIN32
	        int dx=CurrentPoint.x-PrevPoint.x, dy=CurrentPoint.y-PrevPoint.y, steps, k;
            int X;
            int Y;
            float xi, yi, x=PrevPoint.x, y=PrevPoint.y;

            steps = abs(dx)>abs(dy)?abs(dx):abs(dy);
            xi=dx/(float)steps;		//Will be 1 or 1/m
            yi=dy/(float)steps;		//Will be m or 1

            for(k=0;k<steps;k++){
                X = round(x); Y = round(y);
                if( hit.x >= X - 2 && hit.x <= X + 2 && hit.y >= Y - 2 && hit.y <= Y + 2 )
		            return(TRUE);
                x += xi;
                y += yi;
            }
#else
			LineDDA( PrevPoint.x,
					 PrevPoint.y,
					 CurrentPoint.x,
				     CurrentPoint.y,
					 HitTest,
					 (LPARAM)&hit );

			if( hit.hit )
				return TRUE;
#endif
		}

		PrevPoint = CurrentPoint;

	}



	return FALSE;
}
#ifdef WIN32
Void FEF_CALLBACK HitTest( int X, int Y, LPARAM data )
{

	// Checks if the coordinate in the hitParams
	// struct falls within +/- 2 of the x, y
	// coordinates of this point of the line.

	hitParams* obj = ( hitParams* ) data;

	if( obj->x >= X - 2 && obj->x <= X + 2 && obj->y >= Y - 2 && obj->y <= Y + 2 )
		obj->hit = TRUE;

}
#endif

wxRect CFxLine::GetRect()
{
	wxRect Rect;
	wxRect RectTemp;

	vPointIter Iter;
	wxPoint CurrentPoint;
	wxPoint PrevPoint;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		(*Iter)->GetPoint(&CurrentPoint);

		if(Iter != _NodeVector.begin())
		{
			RectTemp = wxRect( CurrentPoint, PrevPoint );

			//RectTemp.NormalizeRect( );

			if(Iter == _NodeVector.begin()+1)
			{
				//Rect.NormalizeRect( );
				Rect = RectTemp;
			}
			else
			{
				RectTemp.Inflate(NODE_REGION_SIZE, NODE_REGION_SIZE);
                Rect = wxRect(min(Rect.GetLeft(), RectTemp.GetLeft()),
                    min(Rect.GetTop(), RectTemp.GetTop()),
                    max(Rect.GetRight(), RectTemp.GetRight()),
                    max(Rect.GetBottom(), RectTemp.GetBottom()));

				//Rect |= RectTemp;
			}
			//Rect.n.NormalizeRect( );
		}

		PrevPoint = CurrentPoint;
	}

	return Rect;
}

Void CFxLine::PrepareMoving(CNode* pNode)
{
	if(pNode == NULL)
	{
		vPointIter Iter;
		for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
		{
			CNodeManager::Instance()->UnLink((*Iter));
			if( (*Iter)->IsEnd() )
				CNodeManager::Instance()->UnLinkList((*Iter));
		}
	}
	else
	{
		CNodeManager::Instance()->UnLink(pNode);
		if( pNode->IsEnd() )
			CNodeManager::Instance()->UnLinkList(pNode);
	}
}

Void CFxLine::OffSetObject(Int32 dwX, Int32 dwY)
{
    if(_IsConnected == TRUE)
        return;

	CNode* pNode = NULL;
	vPointIter Iter;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		//pNode = CNodeManager::Instance()->GetFirstLink((*Iter));
		//if(pNode) //!< Do not move if point is linking to a pin
		//	continue;
		//wxPoint point;
		//(*Iter)->GetPoint(&point);
		//point = point + wxPoint(dwX, dwY);
		//CNodeManager::Instance()->Move((*Iter), point);
		(*Iter)->OffSet(dwX, dwY);
	}
	for(int i = 0; i < 3; i++) {
		_pAdjustedNodes[i]->OffSetEx(dwX, dwY);
	}
}

Bool CFxLine::Update()
{
	vPointIter Iter;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		if( (*Iter)->IsEnd() )
		{
			if(CNodeManager::Instance()->Link((*Iter)) == FALSE)
				return FALSE;
		}
		else
		{
			if(CNodeManager::Instance()->Link((*Iter), false) == FALSE)
				return FALSE;
		}
	}
	return TRUE;
}

Bool CFxLine::PointInNode(wxPoint& Point, CNode** ppNode)
{
	vPointIter Iter;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		if( (*Iter)->IsEnd() || (*Iter)->GetType() != NODE_UNDIFINED )
			if((*Iter)->PointInRegion(Point))
			{
				*ppNode = (*Iter);
				return TRUE;
			}
	}

	for(int i = 0; i < 3; i++) {
		if(_pAdjustedNodes[i]->PointInRegion(Point)) {
			*ppNode = _pAdjustedNodes[i];
				return TRUE;
		}
	}

	return FALSE;
}

Void CFxLine::DelConnectionPoint()
{
	CNode* pPoint = NULL;
	vPointIter Iter;
	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		if((Iter != _NodeVector.begin()) && (Iter != _NodeVector.end()-1))
		{
			pPoint = (*Iter);
			break;
		}
	}
	if(pPoint != NULL)
	{
		_NodeVector.erase(Iter);
		SAFE_DELETE(pPoint);
	}
	return;
}

Void CFxLine::AddConnectionPoint(CNode* pPoint)
{
    if(pPoint != NULL)
    {
        _NodeVector.push_back(pPoint);
    }
    else
    {
	    CNode* pBeginPoint = NULL;

	    vPointIter Iter;
	    wxPoint CurrentPoint;
	    wxPoint PrevPoint;
	    for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	    {
		    (*Iter)->GetPoint(&CurrentPoint);

		    if(Iter != _NodeVector.begin())
		    {
			    wxPoint Point(PrevPoint.x - ((PrevPoint.x - CurrentPoint.x)/2),
						     PrevPoint.y - ((PrevPoint.y - CurrentPoint.y)/2));
			    pBeginPoint = new CNode(Point);
			    _NodeVector.insert(Iter, pBeginPoint);
			    break;
		    }

		    PrevPoint = CurrentPoint;

	    }
    }
	return;
}

#if wxUSE_STD_IOSTREAM
Void CFxLine::SavePoints(wxSTD ostream& stream)
{
	vPointIter Iter;
	wxPoint Point;

	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		(*Iter)->GetPoint(&Point);
		stream.write((Char*)&(Point.x), sizeof(int));
		stream.write((Char*)&(Point.y), sizeof(int));
      int sdwPointType = (*Iter)->GetType();
      stream.write((Char*)&(sdwPointType), sizeof(int));
	}
	return;
}
#else
Void CFxLine::SavePoints(wxOutputStream& stream)
{
	vPointIter Iter;
	wxPoint Point;

	for (Iter = _NodeVector.begin(); (Iter != _NodeVector.end()) && !(_NodeVector.empty()); Iter++)
	{
		(*Iter)->GetPoint(&Point);
		stream.Write((Char*)&(Point.x), sizeof(int));
		stream.Write((Char*)&(Point.y), sizeof(int));
        int sdwPointType = (*Iter)->GetType();
        stream.Write((Char*)&(sdwPointType), sizeof(int));
	}
	return;
}
#endif

Void CFxLine::SetPath()
{
	_pAdjustedNodes[0]->DrawOver();
	_pAdjustedNodes[1]->DrawOver();
	_pAdjustedNodes[2]->DrawOver();

	/*CNode* pBeginPoint = (_NodeVector.begin());
	CNode* pEndPoint = (_NodeVector.end());*/
	int sdwDeltaX, sdwDeltaY;

	if((_NodeVector[0]->GetType() == NODE_UNDIFINED) || (_NodeVector[5]->GetType() == NODE_UNDIFINED) ) {
		_NodeVector[1]->SetPoint(_NodeVector[0]->GetPoint());
				
		sdwDeltaX = _NodeVector[0]->GetPoint()->x - _NodeVector[5]->GetPoint()->x;
		sdwDeltaY = _NodeVector[0]->GetPoint()->y - _NodeVector[5]->GetPoint()->y;

		int sdwNewX = _NodeVector[0]->GetPoint()->x - sdwDeltaX / 2;
		_NodeVector[2]->SetPoint(sdwNewX,
								  _NodeVector[0]->GetPoint()->y/* + sdwDeltaY / 2*/);

		/*! Remove Y aliasing */
		_NodeVector[3]->SetPoint(sdwNewX/*_NodeVector[5]->GetPoint()->x + sdwDeltaX / 2*/,
								  _NodeVector[5]->GetPoint()->y/* - sdwDeltaY / 2*/);

		_NodeVector[4]->SetPoint(_NodeVector[5]->GetPoint());

		sdwDeltaY = _NodeVector[2]->GetPoint()->y - _NodeVector[3]->GetPoint()->y;
		_pAdjustedNodes[1]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - sdwDeltaY / 2); 
		_pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
		_pAdjustedNodes[1]->DrawOver(TRUE);
		_pAdjustedNodes[0]->DrawOver();
		_pAdjustedNodes[2]->DrawOver();

	}
	else {
		if( (_NodeVector[0]->GetType() == NODE_OUT) && (_NodeVector[5]->GetType() == NODE_IN) ) {
			if(_NodeVector[0]->GetPoint()->x < _NodeVector[5]->GetPoint()->x) {
				sdwDeltaX = _NodeVector[0]->GetPoint()->x - _NodeVector[5]->GetPoint()->x;
				sdwDeltaY = _NodeVector[2]->GetPoint()->y - _NodeVector[3]->GetPoint()->y;
				int sdwNewX = _NodeVector[0]->GetPoint()->x - sdwDeltaX / 2;
				_NodeVector[1]->SetPoint(_NodeVector[0]->GetPoint());
				_NodeVector[2]->SetPoint(sdwNewX, _NodeVector[0]->GetPoint()->y);
				_NodeVector[3]->SetPoint(sdwNewX, _NodeVector[5]->GetPoint()->y);
				_NodeVector[4]->SetPoint(_NodeVector[5]->GetPoint());

				_pAdjustedNodes[1]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - sdwDeltaY / 2); 
				_pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				_pAdjustedNodes[1]->DrawOver(TRUE);
				_pAdjustedNodes[0]->DrawOver();
				_pAdjustedNodes[2]->DrawOver();
			}
			else {
				sdwDeltaY = _NodeVector[0]->GetPoint()->y - _NodeVector[5]->GetPoint()->y;
				_NodeVector[1]->SetPoint(_NodeVector[0]->GetPoint()->x + g_sdwPixelConst, _NodeVector[0]->GetPoint()->y);
				_NodeVector[2]->SetPoint(_NodeVector[1]->GetPoint()->x, _NodeVector[1]->GetPoint()->y - sdwDeltaY/2 );
				_NodeVector[4]->SetPoint(_NodeVector[5]->GetPoint()->x - g_sdwPixelConst, _NodeVector[5]->GetPoint()->y);
				_NodeVector[3]->SetPoint(_NodeVector[4]->GetPoint()->x, _NodeVector[2]->GetPoint()->y );

				_pAdjustedNodes[0]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - ((_NodeVector[2]->GetPoint()->y - _NodeVector[1]->GetPoint()->y)/2));
				_pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				_pAdjustedNodes[0]->DrawOver(TRUE);
				_pAdjustedNodes[1]->SetPoint(_NodeVector[3]->GetPoint()->x + ((_NodeVector[2]->GetPoint()->x - _NodeVector[3]->GetPoint()->x)/2), _NodeVector[2]->GetPoint()->y); 
				_pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_Y);
				_pAdjustedNodes[1]->DrawOver(TRUE);
				_pAdjustedNodes[2]->SetPoint(_NodeVector[3]->GetPoint()->x, _NodeVector[4]->GetPoint()->y - ((_NodeVector[4]->GetPoint()->y - _NodeVector[3]->GetPoint()->y)/2));
				_pAdjustedNodes[2]->SetAdjustNodeType(ADJUST_X);
				_pAdjustedNodes[2]->DrawOver(TRUE);
			}
			
		}
		else if( (_NodeVector[0]->GetType() == NODE_IN) && (_NodeVector[5]->GetType() == NODE_OUT) ) {
			if(_NodeVector[0]->GetPoint()->x > _NodeVector[5]->GetPoint()->x) {
				sdwDeltaX = _NodeVector[0]->GetPoint()->x - _NodeVector[5]->GetPoint()->x;
				sdwDeltaY = _NodeVector[2]->GetPoint()->y - _NodeVector[3]->GetPoint()->y;
				int sdwNewX = _NodeVector[0]->GetPoint()->x - sdwDeltaX / 2;
				_NodeVector[1]->SetPoint(_NodeVector[0]->GetPoint());
				_NodeVector[2]->SetPoint(sdwNewX, _NodeVector[0]->GetPoint()->y);
				_NodeVector[3]->SetPoint(sdwNewX, _NodeVector[5]->GetPoint()->y);
				_NodeVector[4]->SetPoint(_NodeVector[5]->GetPoint());

				_pAdjustedNodes[1]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - sdwDeltaY / 2); 
				_pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				_pAdjustedNodes[1]->DrawOver(TRUE);
				_pAdjustedNodes[0]->DrawOver();
				_pAdjustedNodes[2]->DrawOver();
			}
			else {
				sdwDeltaY = _NodeVector[0]->GetPoint()->y - _NodeVector[5]->GetPoint()->y;
				_NodeVector[1]->SetPoint(_NodeVector[0]->GetPoint()->x - g_sdwPixelConst, _NodeVector[0]->GetPoint()->y);
				_NodeVector[2]->SetPoint(_NodeVector[1]->GetPoint()->x, _NodeVector[1]->GetPoint()->y - sdwDeltaY/2 );
				_NodeVector[4]->SetPoint(_NodeVector[5]->GetPoint()->x + g_sdwPixelConst, _NodeVector[5]->GetPoint()->y);
				_NodeVector[3]->SetPoint(_NodeVector[4]->GetPoint()->x, _NodeVector[2]->GetPoint()->y );

				_pAdjustedNodes[0]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - ((_NodeVector[2]->GetPoint()->y - _NodeVector[1]->GetPoint()->y)/2));
				_pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				_pAdjustedNodes[0]->DrawOver(TRUE);
				_pAdjustedNodes[1]->SetPoint(_NodeVector[3]->GetPoint()->x + ((_NodeVector[2]->GetPoint()->x - _NodeVector[3]->GetPoint()->x)/2), _NodeVector[2]->GetPoint()->y); 
				_pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_Y);
				_pAdjustedNodes[1]->DrawOver(TRUE);
				_pAdjustedNodes[2]->SetPoint(_NodeVector[3]->GetPoint()->x, _NodeVector[4]->GetPoint()->y - ((_NodeVector[4]->GetPoint()->y - _NodeVector[3]->GetPoint()->y)/2));
				_pAdjustedNodes[2]->SetAdjustNodeType(ADJUST_X);
				_pAdjustedNodes[2]->DrawOver(TRUE);
			}
		}
		else if( (_NodeVector[0]->GetType() == NODE_OUT) && (_NodeVector[0]->GetType() == NODE_OUT) ) {
			sdwDeltaX = _NodeVector[0]->GetPoint()->x - _NodeVector[5]->GetPoint()->x;
			int sdwNewX = _NodeVector[0]->GetPoint()->x - sdwDeltaX / 2;
			_NodeVector[1]->SetPoint(_NodeVector[0]->GetPoint());
			_NodeVector[2]->SetPoint(sdwNewX, _NodeVector[0]->GetPoint()->y);
			_NodeVector[3]->SetPoint(sdwNewX, _NodeVector[5]->GetPoint()->y);
			_NodeVector[4]->SetPoint(_NodeVector[5]->GetPoint());

		}
		else if( (_NodeVector[0]->GetType() == NODE_IN) && (_NodeVector[0]->GetType() == NODE_OUT) ) {
			sdwDeltaX = _NodeVector[0]->GetPoint()->x - _NodeVector[5]->GetPoint()->x;
			int sdwNewX = _NodeVector[0]->GetPoint()->x - sdwDeltaX / 2;
			_NodeVector[1]->SetPoint(_NodeVector[0]->GetPoint());
			_NodeVector[2]->SetPoint(sdwNewX, _NodeVector[0]->GetPoint()->y);
			_NodeVector[3]->SetPoint(sdwNewX, _NodeVector[5]->GetPoint()->y);
			_NodeVector[4]->SetPoint(_NodeVector[3]->GetPoint());
		}
	}

	/*IFxPin **ppIFxPinBegin, **ppIFxPinEnd;
	GetBeginIFxPin(IFxPin** ppIfxPin)*/

	/*! switch */
	//if(pBeginPoint->IsFirst()) { //! process lift to right
	//	
	//}
	//else { //! process right to left
	//	
	//}




	return;
}

Void CFxLine::UpdateAdjustedPoints(CNode* pPoint)
{
	if(pPoint == NULL) {
		_pAdjustedNodes[0]->DrawOver();
	    _pAdjustedNodes[1]->DrawOver();
	    _pAdjustedNodes[2]->DrawOver();

	    int sdwDeltaY;

	    if((_NodeVector[0]->GetType() == NODE_UNDIFINED) || (_NodeVector[5]->GetType() == NODE_UNDIFINED) ) {
		    sdwDeltaY = _NodeVector[2]->GetPoint()->y - _NodeVector[3]->GetPoint()->y;
		    _pAdjustedNodes[1]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - sdwDeltaY / 2); 
		    _pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
		    _pAdjustedNodes[1]->DrawOver(TRUE);
		    _pAdjustedNodes[0]->DrawOver();
		    _pAdjustedNodes[2]->DrawOver();
	    }
	    else {
		    if( (_NodeVector[0]->GetType() == NODE_OUT) && (_NodeVector[5]->GetType() == NODE_IN) ) {
			    if(_NodeVector[0]->GetPoint()->x < _NodeVector[5]->GetPoint()->x) {
				    sdwDeltaY = _NodeVector[2]->GetPoint()->y - _NodeVector[3]->GetPoint()->y;
				    
                    _pAdjustedNodes[1]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - sdwDeltaY / 2); 
				    _pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				    _pAdjustedNodes[1]->DrawOver(TRUE);
				    _pAdjustedNodes[0]->DrawOver();
				    _pAdjustedNodes[2]->DrawOver();
			    }
			    else {
				    _pAdjustedNodes[0]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - ((_NodeVector[2]->GetPoint()->y - _NodeVector[1]->GetPoint()->y)/2));
				    _pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				    _pAdjustedNodes[0]->DrawOver(TRUE);
				    _pAdjustedNodes[1]->SetPoint(_NodeVector[3]->GetPoint()->x + ((_NodeVector[2]->GetPoint()->x - _NodeVector[3]->GetPoint()->x)/2), _NodeVector[2]->GetPoint()->y); 
				    _pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_Y);
				    _pAdjustedNodes[1]->DrawOver(TRUE);
				    _pAdjustedNodes[2]->SetPoint(_NodeVector[3]->GetPoint()->x, _NodeVector[4]->GetPoint()->y - ((_NodeVector[4]->GetPoint()->y - _NodeVector[3]->GetPoint()->y)/2));
				    _pAdjustedNodes[2]->SetAdjustNodeType(ADJUST_X);
				    _pAdjustedNodes[2]->DrawOver(TRUE);
			    }    			
		    }
		    else if( (_NodeVector[0]->GetType() == NODE_IN) && (_NodeVector[5]->GetType() == NODE_OUT) ) {
			    if(_NodeVector[0]->GetPoint()->x > _NodeVector[5]->GetPoint()->x) {
				    sdwDeltaY = _NodeVector[2]->GetPoint()->y - _NodeVector[3]->GetPoint()->y;
				    
                    _pAdjustedNodes[1]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - sdwDeltaY / 2); 
				    _pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				    _pAdjustedNodes[1]->DrawOver(TRUE);
				    _pAdjustedNodes[0]->DrawOver();
				    _pAdjustedNodes[2]->DrawOver();
			    }
			    else {
				    _pAdjustedNodes[0]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - ((_NodeVector[2]->GetPoint()->y - _NodeVector[1]->GetPoint()->y)/2));
				    _pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_X);
				    _pAdjustedNodes[0]->DrawOver(TRUE);
				    _pAdjustedNodes[1]->SetPoint(_NodeVector[3]->GetPoint()->x + ((_NodeVector[2]->GetPoint()->x - _NodeVector[3]->GetPoint()->x)/2), _NodeVector[2]->GetPoint()->y); 
				    _pAdjustedNodes[1]->SetAdjustNodeType(ADJUST_Y);
				    _pAdjustedNodes[1]->DrawOver(TRUE);
				    _pAdjustedNodes[2]->SetPoint(_NodeVector[3]->GetPoint()->x, _NodeVector[4]->GetPoint()->y - ((_NodeVector[4]->GetPoint()->y - _NodeVector[3]->GetPoint()->y)/2));
				    _pAdjustedNodes[2]->SetAdjustNodeType(ADJUST_X);
				    _pAdjustedNodes[2]->DrawOver(TRUE);
			    }
		    }
	    }       
        return;
	}

	if( (pPoint == _pAdjustedNodes[0]) || (pPoint == _pAdjustedNodes[2]) )
	{
		_pAdjustedNodes[1]->SetPoint(_NodeVector[3]->GetPoint()->x + ((_NodeVector[2]->GetPoint()->x - _NodeVector[3]->GetPoint()->x)/2), _NodeVector[2]->GetPoint()->y); 
	}
	else
	{
		_pAdjustedNodes[0]->SetPoint(_NodeVector[2]->GetPoint()->x, _NodeVector[2]->GetPoint()->y - ((_NodeVector[2]->GetPoint()->y - _NodeVector[1]->GetPoint()->y)/2));
		_pAdjustedNodes[2]->SetPoint(_NodeVector[3]->GetPoint()->x, _NodeVector[4]->GetPoint()->y - ((_NodeVector[4]->GetPoint()->y - _NodeVector[3]->GetPoint()->y)/2));
	}

	return;
}
