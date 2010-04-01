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
#ifndef _FX_LINE_H
#define _FX_LINE_H

#include "DrawObject.h"
#include "AdjustNode.h"

class CFxLine;
typedef std::vector<CNode*>    vPoint;
typedef vPoint::iterator	  vPointIter;

typedef std::list<CFxLine*>    lFxLine;
typedef lFxLine::iterator	  lFxLineIter;

class CFxLine : public CDrawObject
{
public:
	CFxLine(CNode* pBeginPoint, CNode* pEndPoint);
	CFxLine(const CFxLine& Line);
	CFxLine(CNode** ppNode, Uint8 wNbPoint);
	virtual ~CFxLine();

public:
#if wxUSE_STD_IOSTREAM
  Void SavePoints(wxSTD ostream& stream);
#else
  Void SavePoints(wxOutputStream& stream);
#endif
	virtual Void DrawMarker(wxDC* pDC);
	virtual Void Draw(wxDC* pDC);
	Void GetBeginIFxPin(IFxPin** ppIfxPin);
	Void GetEndIFxPin(IFxPin** ppIfxPin);
	Bool IsPointInFx(wxPoint& Point);
	virtual Void SetMarker(Bool ShouldDraw = FALSE);
	virtual wxRect GetRect();
	virtual Void OffSetObject(Int32 dwX, Int32 dwY);
    virtual Bool IsObjectInRect(wxRect& Rect);
	Void PrepareMoving(CNode* pNode = NULL);
	Bool Update();
	Bool PointInNode(wxPoint& Point, CNode** ppNode);
	Void DelConnectionPoint();
	Void AddConnectionPoint(CNode* pPoint = NULL);
	Void AddPoint(CNode* pPoint, Bool isEnd = FALSE);

    Uint32 GetPointCount() const   { return (_NodeVector.size()); }

	Bool IsConnected() { return _IsConnected; }
	Void SetConnected(Bool IsConnected) {_IsConnected = IsConnected; return;}
    Void SetMediaTypeConnection(FX_MEDIA_TYPE MediaType) {_MediaType.MainMediaType = MediaType.MainMediaType; 
    _MediaType.SubMediaType = MediaType.SubMediaType; return; }
    Void GetMediaTypeConnection(FX_MEDIA_TYPE *pMediaType) {pMediaType->MainMediaType = _MediaType.MainMediaType; 
    pMediaType->SubMediaType = _MediaType.SubMediaType; return; }

	Void UpdateAdjustedPoints(CNode* pPoint = NULL);
	
private:
	vPoint _NodeVector;
	Bool _ShouldDrawMarker;
	wxColour _rgbHandleColor;
	wxColour _rgbAdjustedColor;
	wxColour _rgbHidleColor;
	Bool _IsConnected;

	/*! Adjusted Points */
	CAdjustNode* _pAdjustedNodes[3];
	
    FX_MEDIA_TYPE _MediaType;

	Bool _EnableSetPath;
	
public:
	virtual Void EnableSetPath(Bool EnableSetPath) { _EnableSetPath = EnableSetPath; return;}
	Void SetPath();
};
#endif

