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
#ifndef _NODE_H
#define _NODE_H

#include "EditorDefs.h"

//! Node types
typedef enum _POINT_TYPE {
	NODE_IN = PIN_IN,		//!< Input pin type
	NODE_OUT = PIN_OUT,	//!< Output pin type
	NODE_ADJUSTABLE,
	NODE_UNDIFINED
} NODE_TYPE;

//
//! The CNode definition
class CNode
{
public:
	CNode(wxPoint& Point, NODE_TYPE NodeType = NODE_UNDIFINED, IFxPin* pIFxPin = NULL, std::string strName = "");
	virtual ~CNode();

public:
    virtual Void Draw(wxDC* pDC);
    virtual Void DrawName(wxDC* pDC);
    virtual Void DrawOver(Bool ShouldDrawOver = FALSE);
	virtual Bool IsDrawOver() { return _ShouldDrawOver; }
	virtual Void GetPoint(wxPoint* pPoint);
	virtual Void SetPoint(wxPoint* pPoint);
	virtual Void SetPoint(int sdwX, int sdwY);
	virtual wxPoint* GetPoint();
	virtual Void GetRect(wxRect* pRec);
	virtual IFxPin* GetIFxPin() const   { return (_pIFxPin); }
	virtual Int32 Move(wxPoint* pPoint);
	virtual Void OffSet(Int32 sdwX, Int32 sdwY);
	virtual Bool NodeInRegion(CNode* pNode);
	virtual Bool PointInRegion(wxPoint& point);
	virtual Bool IsFx();
	virtual Void SetType(NODE_TYPE NodeType) { _NodeType = NodeType; return;}
	virtual NODE_TYPE GetType() { return _NodeType;}
	virtual Void SetEnd() {_IsEnd = TRUE;}
	virtual Bool IsEnd() {return _IsEnd;}
	virtual Void UpdateTxRx();
	    
protected:
    Bool        _ShouldDrawOver;
	wxRegion	_NodeRegion;
	wxPoint		_NodePoint;
	IFxPin*		_pIFxPin;
	FX_PIN_TYPE _FxPinType;
	NODE_TYPE  _NodeType;

	Bool		_IsEnd;

    std::string _strName;

	Uint64 _qTxRxByte;
	Int32 _sdwTxRxDrawoffset;
	Int32 _sdwTxRxDrawfactor;

};
#endif

