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

#include "AdjustNode.h"

/*----------------------------------------------------------------------*//*!
				 Construction/Destruction
*//*-----------------------------------------------------------------------*/
CAdjustNode::CAdjustNode(wxPoint& Point, CNode* pNode1, CNode* pNode2, ADJUST_TYPE AdjustNodeType):
CNode(Point, NODE_ADJUSTABLE)
{
	_pNode1 = pNode1;
	_pNode2 = pNode2;
	_AdjustNodeType = AdjustNodeType;
}

CAdjustNode::~CAdjustNode()
{
}

Void CAdjustNode::Draw(wxDC* pDC)
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

Int32 CAdjustNode::Move(wxPoint* pPoint)
{
	Int32 sdwX, sdwY;
	wxPoint* pCurrentPoint = GetPoint();
	sdwX = pPoint->x - pCurrentPoint->x;
	sdwY = pPoint->y - pCurrentPoint->y;

	if(_AdjustNodeType == ADJUST_X)
		OffSet(sdwX, 0);
	else
		OffSet(0, sdwY);

	return 0;
}

Void CAdjustNode::OffSet(Int32 sdwX, Int32 sdwY)
{
	_NodePoint.x += sdwX; _NodePoint.y += sdwY; 
	_NodeRegion = wxRegion(_NodePoint.x - NODE_REGION_SIZE/2,
							_NodePoint.y - NODE_REGION_SIZE/2,
							NODE_REGION_SIZE,
							NODE_REGION_SIZE);

	if(_AdjustNodeType == ADJUST_X) {
		_pNode1->OffSet(sdwX, 0);
		_pNode2->OffSet(sdwX, 0);
	}
	else {
		_pNode1->OffSet(0, sdwY);
		_pNode2->OffSet(0, sdwY);
	}
}

Void CAdjustNode::OffSetEx(Int32 sdwX, Int32 sdwY)
{
	_NodePoint.x += sdwX; _NodePoint.y += sdwY; 
	_NodeRegion = wxRegion(_NodePoint.x - NODE_REGION_SIZE/2,
							_NodePoint.y - NODE_REGION_SIZE/2,
							NODE_REGION_SIZE,
							NODE_REGION_SIZE);
}