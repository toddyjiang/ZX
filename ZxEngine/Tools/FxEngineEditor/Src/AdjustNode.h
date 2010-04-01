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
#ifndef _ADJUST_NODE_H
#define _ADJUST_NODE_H

#include "Node.h"

//! Adjust types
typedef enum _ADJUST_TYPE {
	ADJUST_X = 0,		//!< Adjust X only
	ADJUST_Y			//!< Adjust Y only
} ADJUST_TYPE;

//! The CAdjustNode definition
class CAdjustNode : public CNode
{
public:
	CAdjustNode(wxPoint& Point, CNode* pNode1, CNode* pNode2, ADJUST_TYPE AdjustNodeType);
	virtual ~CAdjustNode();

public:
    virtual Void Draw(wxDC* pDC);
	virtual Int32 Move(wxPoint* pPoint);
	virtual Void OffSet(Int32 sdwX, Int32 sdwY);
	virtual Void OffSetEx(Int32 sdwX, Int32 sdwY);
	Void SetAdjustNodeType(ADJUST_TYPE AdjustNodeType) {_AdjustNodeType = AdjustNodeType; return; }

private:
	CNode* _pNode1;
	CNode* _pNode2;
	ADJUST_TYPE _AdjustNodeType;
};
#endif

