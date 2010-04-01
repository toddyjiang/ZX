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
#ifndef __FXENGINEEDITORVIEWH__
#define __FXENGINEEDITORVIEWH__

#include "Node.h"

#include "wx/docview.h"
#include <wx/dnd.h>

class Canvas: public wxScrolledWindow, public wxTextDropTarget
{
public:
    wxView *view;

    Canvas(wxView *v, wxMDIChildFrame *_frame, const wxPoint& pos, const wxSize& size, long style);
	~Canvas();
    virtual Void OnDraw(wxDC& dc);
    virtual Void OnDraw(wxDC* pdc, wxPoint& DrawBeginPoint, Float fPrintZoom);
    Void OnMouseEvent(wxMouseEvent& event);
	Void OnEraseBackGround(wxEraseEvent& event);
    Void OnKeyDown(wxKeyEvent& event);
    Void OnKeyUp(wxKeyEvent& event);
    Void OnRenameFx(wxCommandEvent& event);
    Void OnAddConnectionPoint(wxCommandEvent& event);
    Void OnDelConnectionPoint(wxCommandEvent& event);
    Void OnFxPropertyPage(wxCommandEvent& event);
    Void OnFxPropertiesPage(wxCommandEvent& event);
	Void OnInsertFx(wxCommandEvent& event);
	Void OnInsertLink(wxCommandEvent& event);
	Void OnFxEngineStart(wxCommandEvent& event);
	Void OnFxEngineStop(wxCommandEvent& event);
	Void OnFxEnginePause(wxCommandEvent& event);
	Void OnExport(wxCommandEvent& event);
	Void OnZoomIn(wxCommandEvent& event);
    Void OnZoomIn(wxPoint& point);
    Void OnZoomOut(wxCommandEvent& event);
	Void OnStopFxOnly(wxCommandEvent& event);
    Void OnPauseFxOnly(wxCommandEvent& event);
    Void OnStartFxOnly(wxCommandEvent& event);
    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);

	Void OnTxRx(Bool showTxRx);
	Void OnTimer(wxTimerEvent& event);
	
	FX_HANDLE AttachFxObserver(FX_HANDLE hFxEngine, FX_HANDLE hFx);


	Void InsertFxLine(){_ShouldInsertLink = TRUE;}
	inline Bool IsSelect() {return _IsSelect;};
	inline wxRect& GetSelectRect() { return _SelectRect; };
    wxPoint GetDocSize();
	Void MoveObjects(int Dx, int Dy);

    wxSize GetDrawSize() { return _DrawSize; }

    Void TryConnection() {_TryConnect = TRUE;}

    vSelectedObject& GetSelectedObject()  { return _SelectedObject; }
private:
	wxMDIChildFrame *_Child;
    /*! FxEngine objects */
    wxPoint		_Hitpoint;
	wxPoint		_Movepoint;
	CNode* _pEndNode;
	CFxLine*	_pFxLine;
	CFxLine*	_pLastFxLine;
    CNode* _pLastOverNode;
    CNode* _pCurrOverNode;
	FX_HANDLE	_hLastSelectedFx;
	FX_HANDLE	_hSelectedFx;

	Bool		_IsCreateLine;
	Bool		_ShouldInsertLink;
    Bool        _ShouldZoomIn;

    /* Context Menu */
	CNode* _pCtxNode;
	CFxLine*	_pCtxFxLine;
	FX_HANDLE	_hCtxSelectedFx;

    MOUSE_EVT	_MouseEvt;

	// Tx/Rx bytes timer
    wxTimer *_pTimer;

private:
    wxPoint     _SelectPoint;
    wxRect      _SelectRect;
    vSelectedObject   _SelectedObject;
    Bool        _IsSelect;
    Bool        _IsKeyShift;
    Bool        _TryConnect;

public:
    wxRect      _Drawrec2;
    wxSize      _DrawSize;
	Float		_fZoom;
    Float       _fPrintZoom;

private:
    wxCursor* _pCursorCross;
    wxCursor* _pCursorMagnifier;
    wxCursor* _pCursorArrow;

private:
	/*! Scroll properties */
    int _xScrollUnit;
    int _yScrollUnit;
	int _xUnit;
	int _yUnit;
    wxPoint _BeginPoint;

public:
	//! Critical Section
	boost::mutex _CSDraw;
    
public:
	inline wxPoint& GetBeginPoint() { return _BeginPoint; };
	/*inline int& GetScrollUnitY() { return _yScrollUnit; };
	inline int& GetUnitX() { return _xUnit; };
	inline int& GetUnitY() { return _yUnit; };*/
	inline Float& GetZoom(){return _fZoom;}
    inline Float& GetPrintZoom(){return _fPrintZoom;}


private:
    Void SetScroll(wxPoint& Point);
	Void GetCenter(wxPoint& Point);
    Void SetCenter(wxPoint& Point);

private:
    DECLARE_EVENT_TABLE()
};

class DrawingView: public wxView
{
public:
	wxMDIChildFrame *_frame;
    Canvas *canvas;

    DrawingView() { canvas = (Canvas *) NULL; _frame = (wxMDIChildFrame *) NULL; }
    ~DrawingView() {}

    bool OnCreate(wxDocument *doc, long flags);
    Void OnDraw(wxDC *dc);
    Void OnSize(wxSizeEvent& event);
    Void OnMove(wxMoveEvent& event);
    Void OnChangePosition(wxCommandEvent& event);
    Void OnChangeSize(wxCommandEvent& event);
    Void OnUpdate(wxView *sender, wxObject *hint = (wxObject *) NULL);
	bool OnClose(bool deleteWindow = true);

	Void OnCut(wxCommandEvent& event);


protected:
    wxString _text;

public:
    wxBitmap	_bitmap;

private:
    DECLARE_DYNAMIC_CLASS(DrawingView)
    DECLARE_EVENT_TABLE()
};

class CPrintout: public wxPrintout
{
 public:
  CPrintout(Canvas* pCanvas, const wxChar *title = wxT("FxEngine Print")):wxPrintout(title), _pCanvas(pCanvas) {}
  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  Void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);

  Void DrawPage(wxPoint& Point, int page);

private:
    Canvas*     _pCanvas;
    Float       _fPrintZoom;
    Float       _fPreviewPrintZoom;
    Uint16      _wVPageNumber;
    Uint16      _wHPageNumber;
    Uint32      _dwDCPageWidth;
    Uint32      _dwDCPageHeight;

    int       _sdwPageWidth;
    int       _sdwPageHeight;
};

#endif
