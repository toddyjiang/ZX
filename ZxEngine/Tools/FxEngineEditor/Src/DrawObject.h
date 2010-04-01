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
#ifndef _DRAW_OBJECT_H
#define _DRAW_OBJECT_H

#include "FxEngine.h"

//! Object Type
typedef enum _OBJECT_TYPE{
	FX_OBJECT,
	LINE_OBJECT,
}OBJECT_TYPE;

class CDrawObject  
{
public:
    CDrawObject(OBJECT_TYPE ObjectType) {_ObjectType = ObjectType;}
    virtual ~CDrawObject(){};

public:
	virtual Void SetMarker(Bool ShouldDraw = FALSE) PURE;
    virtual Void OffSetObject(Int32 dwX, Int32 dwY) PURE;
    virtual wxRect GetRect() PURE;
    virtual Bool IsObjectInRect(wxRect& Rect) PURE;
    virtual Void Draw(wxDC* pDC) PURE;
    virtual Void DrawMarker(wxDC* pDC) PURE;
	virtual Void EnableSetPath(Bool EnableSetPath) {return;}
    virtual OBJECT_TYPE GetObjectType() {return _ObjectType;}

private:
	OBJECT_TYPE _ObjectType;			
};
#endif

