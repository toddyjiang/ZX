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

#include "Resource.h"
#include "FxPropertySheet.h"
#include "FxEngineEditor.h"
#include "FxEngineEditorDoc.h"
#include "FxEngineEditorView.h"
#include "NodeManager.h"

#define TIMER_TXRX 500

extern wxPageSetupDialogData* g_pageSetupData;
extern bool        g_isDesignLock;
extern bool        g_showAllTxRx;

IMPLEMENT_DYNAMIC_CLASS(DrawingView, wxView)

Void    FEF_CALLBACK FxStateProc(FX_STATE FxState, FX_HANDLE hObserverId, FX_PTR dwParam);

BEGIN_EVENT_TABLE(DrawingView, wxView)
    EVT_MENU(DOODLE_CUT, DrawingView::OnCut)
    EVT_MENU(MDI_CHANGE_POSITION, DrawingView::OnChangePosition)
    EVT_MENU(MDI_CHANGE_SIZE, DrawingView::OnChangeSize)
    EVT_SIZE(DrawingView::OnSize)
    EVT_MOVE(DrawingView::OnMove)
END_EVENT_TABLE()

// What to do when a view is created. Creates actual
// windows for displaying the view.
bool DrawingView::OnCreate(wxDocument *doc, long WXUNUSED(flags) )
{
    _frame = wxGetApp().CreateChildFrame(doc, this, true);
    canvas = GetMainFrame()->CreateCanvas(this, _frame);
#ifdef __X__
    // X seems to require a forced resize
    int x, y;
    _frame->GetSize(&x, &y);
    _frame->SetSize(wxDefaultCoord, wxDefaultCoord, x, y);
#endif
#ifdef WIN32 //!< Under xin32 we have window client, under gtk we have tab
	canvas->SetMaxSize(wxSize(100, 100));
    _frame->SetStatusText( wxT("   State of selected Fx   ") );
#endif

    _frame->Show(true);
    Activate(true);

	_frame->SetDropTarget(canvas);

	return true;
}

// Sneakily gets used for default print/preview
// as well as drawing on the screen.
Void DrawingView::OnDraw(wxDC *dc)
{
    DrawingDocument *pDoc = (DrawingDocument *)GetDocument();

    Float fZoom = canvas->GetZoom() * canvas->GetPrintZoom();
    wxSize size = canvas->GetDrawSize();

    size.x /= fZoom;
    size.y /= fZoom;

    dc->SetUserScale( fZoom, fZoom );

    wxPoint BeginPoint = canvas->GetBeginPoint();

    wxRect rect((int)(BeginPoint.x/fZoom), (int)(BeginPoint.y/fZoom), size.x, size.y);

    //dc->SetClippingRegion(rect);

    wxMemoryDC memdc;
    memdc.SetDeviceOrigin((int)(-BeginPoint.x/fZoom), (int)(-BeginPoint.y/fZoom));
    wxBitmap bitmap(size.x, size.y);
    memdc.SelectObjectAsSource(bitmap);

    memdc.SetPen( *wxTRANSPARENT_PEN );
#ifdef WIN32
    wxBrush	brush = dc->GetBackground();
    memdc.SetBrush(brush);
#else
    memdc.SetBrush(wxColour(BACKGROUND_R, BACKGROUND_G, BACKGROUND_V));
#endif
    memdc.DrawRectangle(rect);
	memdc.SetPen(wxNullPen);
	memdc.SetBrush(wxNullBrush);

    /*wxPen	pen(wxColour(0,255,0), 1,  wxSOLID);
	memdc.SetPen(pen);*/


    memdc.SetPen(wxNullPen);
    memdc.SetPen( *wxBLACK_PEN);

    /*! Draw all FxLines added */
#ifdef WIN32
	std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun1(&CFxLine::Draw), &memdc));
#else
    std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun(&CFxLine::Draw), &memdc));
#endif
	FxMap::iterator Itmap;
    /*! Draw all FXs added */
	for ( Itmap = pDoc->GetFxMap().begin( ); Itmap != pDoc->GetFxMap().end( ); Itmap++ )
    {
        Itmap->second->Draw(&memdc);
    }

    /*! Draw last selected object */
    CDrawObject* pObject = NULL;
    if(canvas->GetSelectedObject().empty() != TRUE)
        pObject = (canvas->GetSelectedObject()).back();
    if(pObject)
    {
        pObject->Draw(&memdc);

    }

    /*! Draw all FxLines Marker */
#ifdef WIN32
    std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun1(&CFxLine::DrawMarker), &memdc));
#else
    std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun(&CFxLine::DrawMarker), &memdc));
#endif

	if(canvas->IsSelect() == TRUE)
    {
        wxPen	pen(wxColour(0,255,0), 1,  wxSOLID);
        memdc.SetPen(pen);
        memdc.SetBrush(*wxTRANSPARENT_BRUSH);
        memdc.DrawRectangle(canvas->GetSelectRect());
    }

    memdc.SetPen(wxNullPen);
    memdc.SetBrush(wxNullBrush);
    memdc.SelectObject(wxNullBitmap);

    dc->DrawBitmap(bitmap, (int)(BeginPoint.x/fZoom), (int)(BeginPoint.y/fZoom));

    dc->DestroyClippingRegion();

    memdc.SetPen(wxNullPen);
    memdc.SetBrush(wxNullBrush);
    memdc.SelectObject(wxNullBitmap);

}

Void DrawingView::OnUpdate(wxView *WXUNUSED(sender), wxObject *WXUNUSED(hint))
{
    if (canvas) {
        canvas->Update();
        canvas->Refresh();
    }

/* Is the following necessary?
#ifdef __WXMSW__
  if (canvas)
    canvas->Refresh();
#else
  if (canvas)
    {
      wxClientDC dc(canvas);
      dc.Clear();
      OnDraw(& dc);
    }
#endif
*/
}

Void DrawingView::OnChangePosition(wxCommandEvent& WXUNUSED(event))
{
    _frame->Move(10, 10);
}

Void DrawingView::OnChangeSize(wxCommandEvent& WXUNUSED(event))
{
#ifdef WIN32 //!< Under win32 we have window client, under gtk we have tab
    _frame->SetClientSize(100, 100);
#endif
}

Void DrawingView::OnMove(wxMoveEvent& event)
{
    event.Skip();
}

Void DrawingView::OnSize(wxSizeEvent& event)
{
    if(canvas)
        canvas->Refresh();
    event.Skip();
}

// Clean up windows used for displaying the view.
bool DrawingView::OnClose(bool deleteWindow)
{
  if (!GetDocument()->Close())
    return false;

  ((DrawingDocument *)GetDocument())->CloseDocument();

  // Clear the canvas in  case we're in single-window mode,
  // and the canvas stays.
  canvas->ClearBackground();
  canvas->view = (wxView *) NULL;
  canvas = (Canvas *) NULL;

  wxString s(wxTheApp->GetAppName());
  if (_frame)
    _frame->SetTitle(s);

  _frame->SetDropTarget(NULL);

  SetFrame((wxFrame*)NULL);

  Activate(false);

  if (deleteWindow)
  {
    delete _frame;
    return true;
  }
  return true;
}

Void DrawingView::OnCut(wxCommandEvent& WXUNUSED(event) )
{
    DrawingDocument *doc = (DrawingDocument *)GetDocument();
    doc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Cut Last Segment"), DOODLE_CUT, doc, NULL, NULL));
}

/*
 * Window implementations
 */

BEGIN_EVENT_TABLE(Canvas, wxScrolledWindow)
    EVT_MOUSE_EVENTS(Canvas::OnMouseEvent)
    EVT_KEY_DOWN(Canvas::OnKeyDown)
    EVT_KEY_UP(Canvas::OnKeyUp)
    EVT_ERASE_BACKGROUND (Canvas::OnEraseBackGround)
    EVT_MENU(CHANGE_FX_NAME, Canvas::OnRenameFx)
    //EVT_MENU(ADD_CONNECTION, Canvas::OnAddConnectionPoint)
    //EVT_MENU(DEL_CONNECTION, Canvas::OnDelConnectionPoint)
    EVT_MENU(FX_PROPERTIES_PAGE, Canvas::OnFxPropertyPage)
    EVT_MENU(FX_PROPERTIES, Canvas::OnFxPropertiesPage)
	EVT_MENU(FX_START_ONLY, Canvas::OnStartFxOnly)
    EVT_MENU(STOP_FX_ONLY, Canvas::OnStopFxOnly)
    EVT_MENU(PAUSE_FX_ONLY, Canvas::OnPauseFxOnly)
	EVT_TIMER(-1, Canvas::OnTimer)
END_EVENT_TABLE()

// Define a constructor for my canvas
Canvas::Canvas(wxView *v, wxMDIChildFrame *pChild, const wxPoint& pos, const wxSize& size, long style):
 wxScrolledWindow(pChild, wxID_ANY, pos, size,
                           wxSUNKEN_BORDER |
                           wxNO_FULL_REPAINT_ON_RESIZE |
                           wxVSCROLL | wxHSCROLL)
{
	view = v;
	_Child = pChild;

    _hSelectedFx = NULL;
	_pEndNode = NULL;
	_pFxLine = NULL;
    _pLastOverNode = NULL;
    _pCurrOverNode = NULL;
	_hLastSelectedFx = NULL;
	_pLastFxLine = NULL;

    _SelectedObject.clear();
    _IsSelect = FALSE;

    _TryConnect = FALSE;

    _IsKeyShift = FALSE;
	_IsCreateLine = FALSE;
	_ShouldInsertLink = FALSE;
    _ShouldZoomIn = FALSE;

	_MouseEvt = MOUSE_NONE;

	_pCursorCross = new wxCursor(wxCURSOR_CROSS);
    _pCursorArrow = new wxCursor(wxCURSOR_ARROW);
    _pCursorMagnifier = new wxCursor(wxCURSOR_MAGNIFIER);

	/*! Update zoom */
	_fZoom = 1.0f;
    _fPrintZoom = 1.0f;

#ifdef WIN32 //!< Under xin32 we have window client, under gtk we have tab
    SetMaxSize(wxSize(100, 100));
#endif

    SetBackgroundColour(wxColour(BACKGROUND_R, BACKGROUND_G, BACKGROUND_V));

	_pTimer = new wxTimer(this);
	if(g_showAllTxRx)
		_pTimer->Start(TIMER_TXRX);
}

 Canvas::~Canvas()
 {

	delete(_pCursorCross);
    delete(_pCursorArrow);
    delete(_pCursorMagnifier);
	delete(_pTimer);
 }

// Define the repainting behaviour
Void Canvas::OnDraw(wxDC& dc)
{
  SetBackgroundColour(wxColour(BACKGROUND_R, BACKGROUND_G, BACKGROUND_V));
  if (view)
  {
    DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
    AutoLock lockDoc(pDoc->_CSDoc); //! protection when receive release Fx

    _fPrintZoom = 1.0f;

	GetViewStart(&_xUnit, &_yUnit);
    GetScrollPixelsPerUnit(&_xScrollUnit, &_yScrollUnit);
    _BeginPoint.x = _xUnit * _xScrollUnit;
    _BeginPoint.y = _yUnit * _yScrollUnit;

	AutoLock lock(_CSDraw);
    if(_hLastSelectedFx)
    {
        std::string str;
	   	pDoc->GetFxState(_hLastSelectedFx, str);
#ifdef WIN32
        _Child->SetStatusText( s2ws(str) );
#else
        GetMainFrame()->SetStatusText( s2ws(str) );
#endif
    }
    else
#ifdef WIN32
        _Child->SetStatusText( wxT("   State of selected Fx   ") );
#else
        GetMainFrame()->SetStatusText( wxT("   State of selected Fx   ") );
#endif

    _DrawSize = GetClientSize();

    view->OnDraw(& dc);

    /*! Try connect when fx are initializzed */
    if(_TryConnect == TRUE)
    {
        /*! Try to link all points between them */
        CNodeManager::Instance()->Link();
        FX_MEDIA_TYPE MediaType;
        lFxLineIter It;
	    for ( It = pDoc->GetFxLine().begin( ); It != pDoc->GetFxLine().end( ); It++ )
	    {
		    /*! media type and connect */
            (*It)->GetMediaTypeConnection(&MediaType);
            if( (MediaType.MainMediaType == MAIN_TYPE_UNDEFINED) &&
                (MediaType.SubMediaType == SUB_TYPE_UNDEFINED) )
		        pDoc->Connect((*It), NULL);
            else
                pDoc->Connect((*It), &MediaType);
	    }

        view->OnDraw(& dc);
        _TryConnect = FALSE;
    }
  }
}

Void Canvas::OnDraw(wxDC* pdc, wxPoint& DrawBeginPoint, Float fPrintZoom)
{
  pdc->SetBackgroundMode(wxTRANSPARENT);
  pdc->SetBackground(*wxWHITE_BRUSH);
  if (view)
  {
    DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
    AutoLock lockDoc(pDoc->_CSDoc); //! protection when receive release Fx

    _fPrintZoom = fPrintZoom;

    _BeginPoint.x = DrawBeginPoint.x;
    _BeginPoint.y = DrawBeginPoint.y;

    GetScrollPixelsPerUnit(&_xScrollUnit, &_yScrollUnit);

	AutoLock lock(_CSDraw);
    if(_hLastSelectedFx)
    {
        std::string str;
	   	pDoc->GetFxState(_hLastSelectedFx, str);
#ifdef WIN32
        _Child->SetStatusText( s2ws(str) );
#else
        GetMainFrame()->SetStatusText( s2ws(str) );
#endif
    }
    else
#ifdef WIN32
        _Child->SetStatusText( wxT("   State of selected Fx   ") );
#else
        GetMainFrame()->SetStatusText( wxT("   State of selected Fx   ") );
#endif

    _DrawSize = wxSize(HWORKINGSIZE, VWORKINGSIZE);
    //view->OnDraw(pdc);


    Float fZoom = GetZoom() * GetPrintZoom();
    wxSize size = GetDrawSize();

    size.x /= fZoom;
    size.y /= fZoom;

    pdc->SetUserScale( fZoom, fZoom );

    wxPoint BeginPoint = GetBeginPoint();


    pdc->SetPen( *wxTRANSPARENT_PEN );
    wxBrush	brush = pdc->GetBackground();
    pdc->SetBrush(brush);
    pdc->SetPen(wxNullPen);
	pdc->SetBrush(wxNullBrush);

    /*wxPen	pen(wxColour(0,255,0), 1,  wxSOLID);
	pdc->SetPen(pen);*/


    pdc->SetPen(wxNullPen);
    pdc->SetPen( *wxBLACK_PEN);

    /*! Draw all FxLines added */
#ifdef WIN32
	std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun1(&CFxLine::Draw), pdc));
#else
	std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun(&CFxLine::Draw), pdc));
#endif
    FxMap::iterator Itmap;
    /*! Draw all FXs added */
	for ( Itmap = pDoc->GetFxMap().begin( ); Itmap != pDoc->GetFxMap().end( ); Itmap++ )
    {
        Itmap->second->Draw(pdc);
    }

    /*! Draw all FxLines Marker */
#ifdef WIN32
	std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun1(&CFxLine::DrawMarker), pdc));
#else
    std::for_each(pDoc->GetFxLine().begin(), pDoc->GetFxLine().end(),
		std::bind2nd(std::mem_fun(&CFxLine::DrawMarker), pdc));
#endif

	pdc->SetPen(wxNullPen);
    pdc->SetBrush(wxNullBrush);
  }
}

bool Canvas::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    Int32 hr;

	/*! Do not move when design is locked */
     if(g_isDesignLock)
        return false;

	if (view)
	{
        AutoLock lock(_CSDraw);
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
		GetViewStart(&_xUnit, &_yUnit);

		GetScrollPixelsPerUnit(&_xScrollUnit, &_yScrollUnit);

		/*! Find Fx full path*/
		FX_HANDLE Fx;
        std::string strFx = ws2s(data);

		/*! Add FX*/
		hr = FEF_AddFx(pDoc->GetFxEngineHandle(), strFx, &Fx);
		if(FEF_FAILED(hr))
		{
			wxString str = wxT("Invalid Fx: "); str += s2ws(strFx);
			wxMessageBox(str, wxT("FxEngineEditor Error"), wxICON_ERROR);
            return false;
		}

		FX_HANDLE hFxState;
		hr = FEF_AttachFxObserverEx(pDoc->GetFxEngineHandle(), Fx, FxStateProc, (FX_PTR)this, &hFxState);
		if(FEF_FAILED(hr))
		{
			FEF_RemoveFx(pDoc->GetFxEngineHandle(), Fx);
			wxMessageBox(wxT("FEF_AttachFxObserverEx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
            return false;
		}

		int xDest = ((int)(x + (_xUnit*_xScrollUnit))/_fZoom);
		int yDest = ((int)(y + (_yUnit*_yScrollUnit))/_fZoom);

		int width, height;
		GetVirtualSize(&width, &height);

		if(xDest > width)
			xDest = width - 10;

		if(yDest > height)
			yDest = height - 10;

		CFx* pFx = NULL;
		pFx = new CFx(pDoc->GetFxEngineHandle(), Fx, hFxState, strFx, xDest, yDest);
		if(pFx == NULL)
		{
			FEF_RemoveFx(pDoc->GetFxEngineHandle(), Fx);
			wxMessageBox(wxT("Internal Fx Error!!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
            return false;
		}
		pDoc->GetFxMap().insert(std::make_pair(Fx, pFx));

		Refresh();

        pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Add Fx"), DOODLE_ADD, pDoc, NULL, NULL));
        pDoc->Modify(true);
	}

	return true;
}

Void Canvas::OnEraseBackGround(wxEraseEvent& event)
{

}

// This implements a tiny doodling program. Drag the mouse using
// the left button.
Void Canvas::OnMouseEvent(wxMouseEvent& event)
{
	/*event.StopPropagation();*/

    if (!view){
        event.Skip();
        return;
    }

  DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();

  wxClientDC dc(this);
  PrepareDC(dc);
  /*_Child->PrepareDC(dc);*/

    Float fZoom = _fZoom;
    wxPoint point(event.GetLogicalPosition(dc));
    point.x /= fZoom;
    point.y /= fZoom;
    wxPoint Point = event.GetPosition();

    if(event.RightDown())
    {
        _pCtxNode = NULL;
	    _pCtxFxLine = NULL;
	    _hCtxSelectedFx = NULL;

	    /*if( (_pCtxNode =  pDoc->PointInNode(point, &_pCtxFxLine)) != NULL )
	    {
            _IsCreateLine = FALSE;
		    wxMenu menu(wxT("Line menu"));
            menu.Append(ADD_CONNECTION, wxT("Add connection point"));
            menu.Append(DEL_CONNECTION, wxT("Delete connection point"));
            PopupMenu(&menu, Point);
	    }
	    else if( (_pCtxFxLine =  pDoc->PointInFxLine(point)) != NULL )
	    {
            wxMenu menu(wxT("Line menu"));
            menu.Append(ADD_CONNECTION, wxT("Add connection point"));
            menu.Append(DEL_CONNECTION, wxT("Delete connection point"));
            PopupMenu(&menu, Point);
	    }
	    else*/ if((_hCtxSelectedFx = pDoc->PointInFx(point)) != NULL)
	    {
            wxMenu menu(wxT("Fx menu"));
            menu.Append(FX_PROPERTIES, wxT("Fx properties"));
            menu.Append(FX_PROPERTIES_PAGE, wxT("Fx property page"));
            menu.AppendSeparator();
            menu.Append(CHANGE_FX_NAME, wxT("Rename Fx"));
            menu.AppendSeparator();
			CFx* pFx = pDoc->GetFx(_hCtxSelectedFx);
			vSelectedObject::iterator It;
			if(pFx) {
				_hLastSelectedFx = _hCtxSelectedFx;
				pFx->SetMarker(TRUE);
				It = std::find(_SelectedObject.begin(),_SelectedObject.end(), pFx);
				if( It == _SelectedObject.end() ) {
					for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(FALSE);
					_SelectedObject.clear();
					_SelectedObject.push_back(pFx);
				}
            }
            menu.Append(FX_START_ONLY, wxT("Start selected Fx(s)"));
			menu.Append(STOP_FX_ONLY, wxT("Stop selected Fx(s)"));
			menu.Append(PAUSE_FX_ONLY, wxT("Pause selected Fx(s)"));

			Refresh();

            PopupMenu(&menu, Point);
	    }

	    _MouseEvt = MOUSE_RD;

    }
    else if(event.LeftDown())
    {
        if(_ShouldZoomIn)
        {
            OnZoomIn(point);
            SetCursor(*_pCursorArrow);
            _ShouldZoomIn = FALSE;
            event.Skip();
			return;
        }
        /*Marker selection */
		if(_pLastFxLine){
		    _pLastFxLine->SetMarker(FALSE);
			_pLastFxLine->EnableSetPath(FALSE);
		}

	    if(_hLastSelectedFx)
	    {
		    CFx* pFx = pDoc->GetFx(_hLastSelectedFx);
            if(pFx)
		        pFx->SetMarker(TRUE);
	    }

	    _hLastSelectedFx = NULL;
	    _pLastFxLine = NULL;
	    _hSelectedFx = NULL;
	    _pEndNode = NULL;
	    _pFxLine = NULL;

		if( (_pEndNode =  pDoc->PointInNode(point, &_pFxLine)) != NULL )
	    {
			if(_pEndNode->GetType() == NODE_ADJUSTABLE)
				_pFxLine->EnableSetPath(FALSE);
			else
				_pFxLine->EnableSetPath(TRUE);
			if(_IsKeyShift == TRUE)
            {
                vSelectedObject::iterator It;
                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), _pFxLine);
                if( It != _SelectedObject.end() )
                {
                    _SelectedObject.erase(It);
                    _pLastFxLine = NULL;
                    _pFxLine->SetMarker(FALSE);
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(TRUE);
                }
                else
                {
                    _SelectedObject.push_back(_pFxLine);
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(TRUE);
                }

            }
            else
            {
                vSelectedObject::iterator It;
                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), _pFxLine);
                if( It == _SelectedObject.end() )
                {
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(FALSE);
                    _SelectedObject.clear();
                    _SelectedObject.push_back(_pFxLine);
                    _pFxLine->SetMarker(TRUE);
                }
                else
                {
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(TRUE);
                }
            }

            _IsCreateLine = FALSE;
		    _ShouldInsertLink = FALSE;
            SetCursor(*_pCursorArrow);
            _ShouldInsertLink = FALSE;

	    }
	    else if( (_pFxLine =  pDoc->PointInFxLine(point)) != NULL )
	    {
			if(_IsKeyShift == TRUE)
            {
                vSelectedObject::iterator It;
                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), _pFxLine);
                if( It != _SelectedObject.end() )
                {
                    _SelectedObject.erase(It);
                    _pLastFxLine = NULL;
                    _pFxLine->SetMarker(FALSE);
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(TRUE);
                }
                else
                {
                    _SelectedObject.push_back(_pFxLine);
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(TRUE);
                }

            }
            else
            {
                vSelectedObject::iterator It;
                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), _pFxLine);
                if( It == _SelectedObject.end() )
                {
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(FALSE);
                    _SelectedObject.clear();
                    _SelectedObject.push_back(_pFxLine);
                    _pFxLine->SetMarker(TRUE);
                }
                else
                {
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(TRUE);
                }
            }

            SetCursor(*_pCursorArrow);
            _ShouldInsertLink = FALSE;
	    }
	    else if((_hSelectedFx = pDoc->PointInFx(point)) == NULL)
	    {
            if( ((_ShouldInsertLink == TRUE) || (_pLastOverNode) ) && !g_isDesignLock)
		    {
                SetCursor(*_pCursorCross);
			    _pEndNode = pDoc->CreateFxLine(point, &_pFxLine);
			    _pFxLine->SetMarker(TRUE);
			    _IsCreateLine = TRUE;
				vSelectedObject::iterator It;
                for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                    (*It)->SetMarker(FALSE);
                _SelectedObject.clear();
		    }
            else /*Begin select rectangle*/
            {
                SetCursor(*_pCursorArrow);
                _ShouldInsertLink = FALSE;
                _SelectPoint = point;
                _IsSelect = TRUE;
                if(_IsKeyShift == FALSE)
                {
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(FALSE);
                    _SelectedObject.clear();
                }
                else
                {
                    vSelectedObject::iterator It;
                    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                        (*It)->SetMarker(TRUE);
                }
            }
	    }
	    else
	    {
            SetCursor(*_pCursorArrow);
            _ShouldInsertLink = FALSE;
		    CFx* pFx = pDoc->GetFx(_hSelectedFx);
            if(pFx)
            {
		        _hLastSelectedFx = _hSelectedFx;
                pFx->SetMarker(TRUE);
                _IsSelect = FALSE;
                if(_IsKeyShift == TRUE)
                {
                    vSelectedObject::iterator It;
	                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), pFx);
	                if( It != _SelectedObject.end() )
	                {
                        _SelectedObject.erase(It);
                        _hLastSelectedFx = NULL;
                        _hSelectedFx = NULL;
                        pFx->SetMarker(FALSE);
                        for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                            (*It)->SetMarker(TRUE);
                    }
                    else
                    {
                        _SelectedObject.push_back(pFx);
                        vSelectedObject::iterator It;
                        for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                            (*It)->SetMarker(TRUE);
                    }

                }
                else
                {
                    vSelectedObject::iterator It;
	                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), pFx);
	                if( It == _SelectedObject.end() )
	                {
                        vSelectedObject::iterator It;
						for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ ) {
							(*It)->EnableSetPath(FALSE);
                            (*It)->SetMarker(FALSE);
						}
                        _SelectedObject.clear();
                        _SelectedObject.push_back(pFx);
                        pFx->SetMarker(TRUE);
                    }
                    else
                    {
                        vSelectedObject::iterator It;
						for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ ){
                            (*It)->SetMarker(TRUE);
							(*It)->EnableSetPath(FALSE);
						}
                    }
                }
            }
	    }

	    _Hitpoint = point;
	    _Movepoint = point;

	    Refresh();
	    _MouseEvt = MOUSE_LD;

        event.Skip();
    }
    else if(event.LeftUp())
    {
        /*! Remove link point focus */
        /*if(_pLastOverNode)
            _pLastOverNode->DrawOver(FALSE);*/

        if( _MouseEvt == MOUSE_M)
        {
            pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Add Fx"), DOODLE_ADD, pDoc, NULL, NULL));
            pDoc->Modify(true);
        }

		//SetFocus();
        if(_IsSelect == TRUE) {_IsSelect = FALSE;
            if(_IsKeyShift == FALSE)
                _SelectedObject.clear();

            if(_SelectRect.height < 0)
            {
                int width = _SelectRect.width;
                int height = _SelectRect.height;
                wxPoint Pt = _SelectRect.GetBottomRight();
                _SelectRect.SetBottomRight(_SelectRect.GetLeftTop());
                _SelectRect.SetLeftTop(Pt);
                _SelectRect.width = -width;
                _SelectRect.height = -height;
            }
            /*! Is fx in selected rect ? */
            FxMap::iterator Itmap;
	        for ( Itmap = pDoc->GetFxMap().begin( ); Itmap != pDoc->GetFxMap().end( ); Itmap++ )
		        if(Itmap->second->IsObjectInRect(_SelectRect))
                {
                    vSelectedObject::iterator It;
	                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), Itmap->second);
	                if( It == _SelectedObject.end() )
	                {
                        _hLastSelectedFx = Itmap->first;
                        _SelectedObject.push_back(Itmap->second);
                        Itmap->second->SetMarker(TRUE);
                    }
                }
            lFxLineIter ItLine;
            for ( ItLine = pDoc->GetFxLine().begin( ); ItLine != pDoc->GetFxLine().end( ); ItLine++ )
            {
                if((*ItLine)->IsObjectInRect(_SelectRect))
                {
                    vSelectedObject::iterator It;
	                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), (*ItLine));
	                if( It == _SelectedObject.end() )
	                {
                        _SelectedObject.push_back((*ItLine));
                        (*ItLine)->SetMarker(TRUE);
                    }
                }
            }
            Refresh();
            _SelectRect.SetLeftTop(_Movepoint);
            _SelectRect.SetBottomRight(_Movepoint);

        }

        if(_hSelectedFx)
	    {
		    _hLastSelectedFx = _hSelectedFx;
		    _hSelectedFx = NULL;
	    }
	    else if(_pEndNode)
	    {

		    if(_IsCreateLine == TRUE)
		    {
			    SetCursor(*_pCursorArrow);
			    _ShouldInsertLink = FALSE;
		    }

		    if(pDoc->EndFxLine(_pEndNode) == FALSE)
		    {
				vSelectedObject::iterator It;
                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), _pFxLine);
                if( It != _SelectedObject.end() )
					_SelectedObject.erase(It);
			    pDoc->RemoveFxLineObject(_pFxLine); _pFxLine = NULL;
			    _IsCreateLine = FALSE;
				_pFxLine = NULL;
				_pLastFxLine = NULL;
		    }
		    else if( (_MouseEvt != MOUSE_M) && (_IsCreateLine == TRUE) )
		    {
				vSelectedObject::iterator It;
                It = std::find(_SelectedObject.begin(),_SelectedObject.end(), _pFxLine);
                if( It != _SelectedObject.end() )
					_SelectedObject.erase(It);
			    pDoc->RemoveFxLineObject(_pFxLine); _pFxLine = NULL;
			    _IsCreateLine = FALSE;
				_pFxLine = NULL;
				_pLastFxLine = NULL;
		    }
		    else if( _pEndNode->IsEnd() )
				    if(pDoc->Connect(_pFxLine) == FALSE)
                    {
						vSelectedObject::iterator It;
						It = std::find(_SelectedObject.begin(),_SelectedObject.end(), _pFxLine);
						if( It != _SelectedObject.end() )
							_SelectedObject.erase(It);
					    pDoc->RemoveFxLineObject(_pFxLine);
						_pFxLine = NULL;
						_pLastFxLine = NULL;
                    }

		    _pEndNode = NULL;
		    _pLastFxLine = _pFxLine;
		    _pFxLine = NULL;

		    Refresh();
	    }
	    else if(_pFxLine)
	    {
		    if( _MouseEvt == MOUSE_M)
		    {
				vSelectedObject::iterator It = _SelectedObject.begin();
				while(It != _SelectedObject.end())
				{
					if((*It)->GetObjectType() == LINE_OBJECT)
					{
						/*if(_pFxLine->Update() == FALSE)
						{
							_pFxLine = NULL;
							_pLastFxLine = NULL;
							pDoc->RemoveFxLineObject(((CFxLine *)(*It)));
							It = _SelectedObject.erase(It);
						}
						else */if(pDoc->Connect((CFxLine *)(*It)) == FALSE)
						{
							_pFxLine = NULL;
							_pLastFxLine = NULL;
							pDoc->RemoveFxLineObject((CFxLine *)(*It));
							It = _SelectedObject.erase(It);
						}
						else
							It++;
					}
					else
						It++;
				}
		    }

		    _pEndNode = NULL;
		    _pLastFxLine = _pFxLine;
		    _pFxLine = NULL;

		    Refresh();
        }

        _MouseEvt = MOUSE_LU;
        //event.Skip();
    }
    else if(event.LeftDClick())
    {
        _MouseEvt = MOUSE_LLD;

        /*Marker selection */
	    if(_pLastFxLine)
		    _pLastFxLine->SetMarker(FALSE);

	    if(_hLastSelectedFx)
	    {
		    CFx* pFx = pDoc->GetFx(_hLastSelectedFx);
            if(pFx)
		        pFx->SetMarker(FALSE);
	    }

	    _hLastSelectedFx = NULL;
	    _pLastFxLine = NULL;
	    _hSelectedFx = NULL;
	    _pEndNode = NULL;
	    _pFxLine = NULL;

	    _hSelectedFx = pDoc->PointInFx(point);
	    if(_hSelectedFx != NULL)
	    {
		    CFx* pFx = pDoc->GetFx(_hSelectedFx);
            if(pFx)
            {
		        pFx->SetMarker(TRUE);
		        _hLastSelectedFx = _hSelectedFx;
		        pDoc->FxProperty(_hLastSelectedFx, this);
            }
	    }

	    _Hitpoint = point;
	    _Movepoint = point;

	    Refresh();
        _MouseEvt = MOUSE_LD;

    }
    else if(event.Dragging())
    {
        /*! Do not move when design is locked excepted to dragging to select*/
        if(g_isDesignLock && (_IsSelect == FALSE)){ 
            event.Skip();
            return;
        }

        if(_IsSelect == TRUE)
        {
            _SelectRect.SetLeftTop(_SelectPoint);
            _SelectRect.SetBottomRight(_Movepoint);
            _Movepoint = point;

            SetScroll(point);


        }
        else if((_SelectedObject.empty() == FALSE) && (_hSelectedFx !=NULL ) )
        {
            MoveObjects(point.x - _Movepoint.x, point.y - _Movepoint.y);
            _Movepoint = point;
            SetScroll(point);

        }
        /*else if(_hSelectedFx !=NULL )
		{
			CFx* pFx = pDoc->GetFx(_hSelectedFx);
            if(pFx)
            {
			    wxRect Drawrec1 = pFx->GetRect();

			    pFx->OffSetObject(point.x - _Movepoint.x,
						    point.y - _Movepoint.y);

			    _Movepoint = point;
                SetScroll(point);

			    _Drawrec2 = pFx->GetRect();
                _Drawrec2.Inflate(10*NODE_REGION_SIZE, 10*NODE_REGION_SIZE);
			    _Drawrec2 = _Drawrec2.Union(Drawrec1);
			    //_Drawrec2.Inflate(10*NODE_REGION_SIZE, 10*NODE_REGION_SIZE);

                //dc.SetClippingRegion(_Drawrec2);
                //RefreshRect(_Drawrec2, FALSE);
            }
		}*/
		else if(_pEndNode)
		{
			_pCurrOverNode = CNodeManager::Instance()->GetNode(point);
			if(_pCurrOverNode != _pLastOverNode)
			{
				if(_pLastOverNode) {
					_pLastOverNode->DrawOver(FALSE);
					_pEndNode->SetType(NODE_UNDIFINED);
					if(!g_showAllTxRx)
						_pTimer->Stop();
				}

				if(_pCurrOverNode)
					if( _pEndNode->IsEnd() ) {
						_pCurrOverNode->DrawOver(TRUE);
						_pEndNode->SetType(_pCurrOverNode->GetType());
						if(!_pTimer->IsRunning())
							_pTimer->Start(TIMER_TXRX);
					}

				_pLastOverNode = _pCurrOverNode;

				Refresh();
			}

			if(_MouseEvt == MOUSE_LD)
			{
				if( _pEndNode->IsEnd() )
				{
					//pDoc->DetachLineToFx(_pFxLine);
					pDoc->Disconnect(_pFxLine);
					_pFxLine->PrepareMoving(_pEndNode);
				}
			}
			
			wxRect Drawrec1 = _pFxLine->GetRect();
			_pEndNode->Move(&point);
			_pFxLine->UpdateAdjustedPoints(_pEndNode);

			_Movepoint = point;
            SetScroll(point);

			_Drawrec2 = _pFxLine->GetRect();
			_Drawrec2.Inflate(10*NODE_REGION_SIZE, 10*NODE_REGION_SIZE);
			_Drawrec2 = _Drawrec2.Union(Drawrec1);
			//_Drawrec2.Inflate(10*NODE_REGION_SIZE, 10*NODE_REGION_SIZE);
			//Drawrec2.Offset(-Point.x, -Point.y);
			//dc.SetClippingRegion(_Drawrec2);
            //RefreshRect(_Drawrec2, FALSE);
		}
		/*else if(_pFxLine)
		{

			MoveObjects(point.x - _Movepoint.x, point.y - _Movepoint.y);
			_Movepoint = point;
			SetScroll(point);
		}*/

        Refresh();
        _MouseEvt = MOUSE_M;
    }
    else if(event.Moving())
    {
        /*if(_ShouldInsertLink == TRUE)
		{*/
            _pCurrOverNode = CNodeManager::Instance()->GetNode(point);
            if(_pCurrOverNode != _pLastOverNode)
            {
                if(_pLastOverNode){
                    _pLastOverNode->DrawOver(FALSE);
					if(!g_showAllTxRx)
						_pTimer->Stop();
				}

                if(_pCurrOverNode){
                    _pCurrOverNode->DrawOver(TRUE);
					if(!_pTimer->IsRunning())
						_pTimer->Start(TIMER_TXRX);
				}

                _pLastOverNode = _pCurrOverNode;

                Refresh();
            }
        //}
    }
    else
        event.Skip();
}

Void Canvas::MoveObjects(int Dx, int Dy)
{
    /*! Do not move when design is locked */
    if(g_isDesignLock)
        return;

	DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();

    wxRect Drawrec1;
    vSelectedObject::iterator It;
    for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
    {
		(*It)->EnableSetPath(TRUE);
        Drawrec1 = (*It)->GetRect();
		//if((*It)->GetObjectType() == LINE_OBJECT)
		//{
		//	//pDoc->DetachLineToFx(((CFxLine *)(*It)));
		//	//((CFxLine *)(*It))->PrepareMoving();
		//}
        (*It)->OffSetObject(Dx, Dy);

	    _Drawrec2 = (*It)->GetRect();
        _Drawrec2.Inflate(10*NODE_REGION_SIZE, 10*NODE_REGION_SIZE);
	    _Drawrec2 = _Drawrec2.Union(Drawrec1);
	    //_Drawrec2.Inflate(10*NODE_REGION_SIZE, 10*NODE_REGION_SIZE);
    }
    //Drawrec2.Offset(-Point.x, -Point.y);
    //dc.SetClippingRegion(_Drawrec2);
    //RefreshRect(_Drawrec2, FALSE);

    Refresh();
    return;
}

wxPoint Canvas::GetDocSize()
{
    /*! Get Fx size */
    wxRect TotalDrawRec, Drawrec;
    FxMap::iterator Itmap;
    DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
    for ( Itmap = pDoc->GetFxMap().begin( ); Itmap != pDoc->GetFxMap().end( ); Itmap++ )
    {
        Drawrec = Itmap->second->GetRect();
        Drawrec.Inflate(10*NODE_REGION_SIZE, 10*NODE_REGION_SIZE);
	    TotalDrawRec = TotalDrawRec.Union(Drawrec);
    }

    /*! Apply zoom */
    TotalDrawRec.SetBottomRight(wxPoint((int)(TotalDrawRec.GetBottomRight().x * _fZoom), (int)(TotalDrawRec.GetBottomRight().y * _fZoom)));

    return TotalDrawRec.GetBottomRight();

}

Void Canvas::OnStopFxOnly(wxCommandEvent& event)
{
	if(_hCtxSelectedFx)
	{
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
		vSelectedObject::iterator It;
		for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ ) {
			if((*It)->GetObjectType() == FX_OBJECT) {
				Int32 hr;
				hr = FEF_StopFx(pDoc->GetFxEngineHandle(), ((CFx*)(*It))->GethFxHandle());
				if(FEF_FAILED(hr))
				{
					wxMessageBox(wxT("FEF_StopFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
					event.Skip();
					Refresh();	
					return;
				}
			}
		}
	}
    event.Skip();
	 Refresh();	
    return;
}

Void Canvas::OnPauseFxOnly(wxCommandEvent& event)
{
	if(_hCtxSelectedFx)
	{
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
        vSelectedObject::iterator It;
		for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ ) {
			if((*It)->GetObjectType() == FX_OBJECT) {
				Int32 hr;
				hr = FEF_PauseFx(pDoc->GetFxEngineHandle(), ((CFx*)(*It))->GethFxHandle());
				if(FEF_FAILED(hr))
				{
					wxMessageBox(wxT("FEF_PauseFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
					event.Skip();
					Refresh();	
					return;
				}
			}
		}
	}
    event.Skip();
	 Refresh();	
    return;
}

Void Canvas::OnStartFxOnly(wxCommandEvent& event)
{
	if(_hCtxSelectedFx)
	{
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
		vSelectedObject::iterator It;
		for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ ) {
			if((*It)->GetObjectType() == FX_OBJECT) {
				Int32 hr;
				hr = FEF_StartFx(pDoc->GetFxEngineHandle(), ((CFx*)(*It))->GethFxHandle());
				if(FEF_FAILED(hr))
				{
					wxMessageBox(wxT("FEF_StartFx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
					event.Skip();	
					Refresh();				
					return;
				}
			}
		}
	}
    event.Skip();
	 Refresh();
    return;
}
Void Canvas::OnZoomIn(wxCommandEvent& event)
{
	_ShouldZoomIn = TRUE;

    SetCursor(*_pCursorMagnifier);

    event.Skip();
    return;
}

Void Canvas::OnZoomIn(wxPoint& point)
{
	if (!view)
    return;

    if(_fZoom < MAX_ZOOM)
        _fZoom *= 2;

    int width, height;
    GetVirtualSize(&width, &height);
    SetVirtualSize((int)(_fZoom * HWORKINGSIZE), (int)(_fZoom * VWORKINGSIZE));


    Refresh();
    Update();

    ClearBackground();
    Refresh();
    Update();

    SetCenter(point);

    return;
}

Void Canvas::OnZoomOut(wxCommandEvent& event)
{
    if (!view){
        event.Skip();
        return;
    }

	if(_fZoom > MIN_ZOOM)
    {
        wxPoint Point;
        GetCenter(Point);
        _fZoom /= 2.0f;

        int width, height;
        GetVirtualSize(&width, &height);
        SetVirtualSize((int)(_fZoom * HWORKINGSIZE), (int)(_fZoom * VWORKINGSIZE));

        Refresh();
        Update();

        ClearBackground();
        Refresh();
        Update();

        SetCenter(Point);
    }
    event.Skip();
    return;
}

Void Canvas::GetCenter(wxPoint& Point)
{
	if (!view)
    return;

	wxSize size = GetClientSize();
    size.x /= _fZoom;
    size.y /= _fZoom;

    int xUnit, yUnit;
    GetViewStart(&xUnit, &yUnit);
    GetScrollPixelsPerUnit(&_xScrollUnit, &_yScrollUnit);
    Point.x = xUnit*_xScrollUnit/_fZoom + (size.x / 2);
    Point.y = yUnit*_yScrollUnit/_fZoom + (size.y / 2);

    return;
}

Void Canvas::SetCenter(wxPoint& Point)
{
	if (!view)
    return;

	wxSize size = GetClientSize();
    size.x /= _fZoom;
    size.y /= _fZoom;

    int xUnit, yUnit;
    GetScrollPixelsPerUnit(&_xScrollUnit, &_yScrollUnit);

    xUnit = (Point.x - (size.x / 2)) / (_xScrollUnit/_fZoom);
    yUnit = (Point.y - (size.y / 2)) / (_yScrollUnit/_fZoom);

    Scroll(xUnit, yUnit);


    return;
}

FX_HANDLE Canvas::AttachFxObserver(FX_HANDLE hFxEngine, FX_HANDLE hFx)
{
	FX_HANDLE hFxState;
	Int32 hr;
	hr = FEF_AttachFxObserverEx(hFxEngine, hFx, FxStateProc, (FX_PTR)this, &hFxState);
	if(FEF_FAILED(hr))
	{
		FEF_RemoveFx(hFxEngine, hFx);
		wxMessageBox(wxT("FEF_AttachFxObserverEx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
		return NULL;
	}

	return hFxState;
}

Void Canvas::OnInsertFx(wxCommandEvent& event)
{
	/*! Do not move when design is locked */
    if(g_isDesignLock){
        event.Skip();
        return;
    }

    Int32 hr;

	/*! Find Fx full path*/
	FX_HANDLE Fx = NULL;

#ifdef WIN32
    wxFileDialog fd(this, wxT("Choose Fx to add"), wxEmptyString, wxEmptyString,
                             wxT("Fx Files (*.dll)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST );
#else
	wxFileDialog fd(this, wxT("Choose Fx to add"), wxEmptyString, wxEmptyString,
                             wxT("Fx Files (*.so)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST );
#endif

    if(fd.ShowModal() == wxID_OK)
    {
		if (view)
		{
            DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();

			/*! Add FX*/
			hr = FEF_AddFx(pDoc->GetFxEngineHandle(), ws2s(fd.GetPath()), &Fx);
			if(FEF_FAILED(hr))
			{
				wxString str = wxT("Invalid Fx: ") + fd.GetPath();
				wxMessageBox(str, wxT("FxEngineEditor Error"), wxICON_ERROR);
                //event.Skip();
                return;
			}

			FX_HANDLE hFxState = NULL;
			hr = FEF_AttachFxObserverEx(pDoc->GetFxEngineHandle(), Fx, FxStateProc, (FX_PTR)this, &hFxState);
			if(FEF_FAILED(hr))
			{
				FEF_RemoveFx(pDoc->GetFxEngineHandle(), Fx);
				wxMessageBox(wxT("FEF_AttachFxObserverEx failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
                //event.Skip();
                return;
			}

			CFx* pFx = NULL;
			pFx = new CFx(pDoc->GetFxEngineHandle(), Fx, hFxState, ws2s(fd.GetPath()));
			if(pFx == NULL)
			{
				FEF_RemoveFx(pDoc->GetFxEngineHandle(), Fx);
				wxMessageBox(wxT("Internal Fx Error!!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
                //event.Skip();
                return;
			}
			(pDoc->GetFxMap()).insert(std::make_pair(Fx, pFx));

            pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Add Fx"), DOODLE_ADD, pDoc, NULL, NULL));
            pDoc->Modify(true);

		}
    }

    //event.Skip();
    Refresh();
    return;
}

Void Canvas::OnInsertLink(wxCommandEvent& event)
{
	/*! Do not move when design is locked */
    if(g_isDesignLock){
        event.Skip();
        return;
    }

    if (view)
    {
	    //DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
        InsertFxLine();

        SetCursor(*_pCursorCross);
    }
    //event.Skip();
    return;
}

Void Canvas::OnFxEngineStart(wxCommandEvent& event)
{
	Int32 hr;
	if (view)
	{
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();

		hr = FEF_StartFxEngine(pDoc->GetFxEngineHandle());
		if(FEF_FAILED(hr))
		{
			wxMessageBox(wxT("Cannot start FxEngine"), wxT("FxEngineEditor Error"), wxICON_ERROR);
            //event.Skip();
			//return;
		}
      Refresh();
	}
    //event.Skip();
	return;
}

Void Canvas::OnFxEngineStop(wxCommandEvent& event)
{
	Int32 hr;
	if (view)
	{
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
		hr = FEF_StopFxEngine(pDoc->GetFxEngineHandle());
		if(FEF_FAILED(hr))
		{
			wxMessageBox(wxT("Cannot stop FxEngine"), wxT("FxEngineEditor Error"), wxICON_ERROR);
            //event.Skip();
			//return;
		}
      Refresh();
	}
    //event.Skip();
	return;
}

Void Canvas::OnFxEnginePause(wxCommandEvent& event)
{
	Int32 hr;
	if (view)
	{
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
		hr = FEF_PauseFxEngine(pDoc->GetFxEngineHandle());
		if(FEF_FAILED(hr))
		{
			wxMessageBox(wxT("Cannot pause FxEngine"), wxT("FxEngineEditor Error"), wxICON_ERROR);
            //event.Skip();
			//return;
		}
        Refresh();
	}
    //event.Skip();
    return;
}

Void Canvas::OnExport(wxCommandEvent& event)
{
	Int32 hr;
    if (view)
	{
		wxFileDialog dialog(this, wxT("Select a file to export"), wxEmptyString, wxEmptyString,
                             wxT("FxEngine Files (*.xml)|*.*"), wxFD_SAVE);
	    wxString strFileName;
        if ( dialog.ShowModal() == wxID_OK )
        {
            strFileName = dialog.GetPath();

            DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
            hr = FEF_SaveFxEngine(pDoc->GetFxEngineHandle(), ws2s(strFileName));
	        if(FEF_FAILED(hr))
	        {
		        wxMessageBox(wxT("FEF_SaveFxEngine failed !!"), wxT("FxEngineEditor Error"), wxICON_ERROR);
                //event.Skip();
		        return;
	        }
        }
    }
    //event.Skip();
    return;
}

Void Canvas::SetScroll(wxPoint& Point)
{
	if (!view)
    return;

	wxSize size = GetClientSize();
    int xUnit, yUnit;

    GetViewStart(&xUnit, &yUnit);
    if((xUnit*_xScrollUnit) + size.GetWidth() - 10 < Point.x*_fZoom)
        Scroll(xUnit + 1, yUnit);
    if((xUnit*_xScrollUnit) + 10 > Point.x*_fZoom)
        Scroll(xUnit - 1, yUnit);
    if((yUnit*_yScrollUnit) + size.GetHeight() - 10 < Point.y*_fZoom)
        Scroll(xUnit, yUnit + 1);
    if((yUnit*_yScrollUnit) + 10 > Point.y*_fZoom)
        Scroll(xUnit, yUnit - 1);
    return;
}

Void Canvas::OnRenameFx(wxCommandEvent& event)
{
	/*! Do not move when design is locked */
    if(g_isDesignLock){
        event.Skip();
        return;
    }

    if(_hCtxSelectedFx)
	{
        DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
        CFx* pFx = pDoc->GetFx(_hCtxSelectedFx);
        if(pFx)
        {
            wxTextEntryDialog dialog(this,
                                 wxT("Enter the new Fx name"),
                                 wxT("Fx name"),
                                 s2ws(pFx->GetFxName()),
                                 wxOK | wxCANCEL);

            if (dialog.ShowModal() == wxID_OK)
            {
                pFx->SetFxName(ws2s(dialog.GetValue()));
                Refresh();
            }
        }
    }
    event.Skip();
    return;
}

Void Canvas::OnAddConnectionPoint(wxCommandEvent& event)
{
	/*! Do not move when design is locked */
    if(g_isDesignLock){
        event.Skip();
        return;
    }

    if(_pLastFxLine)
		_pLastFxLine->SetMarker(FALSE);

	_pCtxFxLine->AddConnectionPoint();
	_pCtxFxLine->SetMarker(TRUE);

	Refresh();

	_pLastFxLine = _pCtxFxLine;
	_pCtxFxLine = NULL;
    event.Skip();
    return;
}

Void Canvas::OnDelConnectionPoint(wxCommandEvent& event)
{
    /*! Do not move when design is locked */
    if(g_isDesignLock){
        event.Skip();
        return;
    }

    if(_pLastFxLine)
		_pLastFxLine->SetMarker(FALSE);

	_pCtxFxLine->DelConnectionPoint();

	_pCtxFxLine->SetMarker(TRUE);

	Refresh();

	_pLastFxLine = _pCtxFxLine;
	_pCtxFxLine = NULL;

    event.Skip();
	return;
}

Void Canvas::OnFxPropertyPage(wxCommandEvent& event)
{
    if (!view){
        event.Skip();
        return;
    }

    if(_hCtxSelectedFx)
	{
        DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();
        pDoc->FxProperty(_hCtxSelectedFx, this);
        pDoc->Modify(true);
    }
    event.Skip();
    return;
}

Void Canvas::OnFxPropertiesPage(wxCommandEvent& event)
{
    if (!view){
        event.Skip();
        return;
    }

    if(_hCtxSelectedFx)
	{
        DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();

        CFxPropertySheet dialog(this, pDoc->GetFxEngineHandle(), _hCtxSelectedFx);
        dialog.ShowModal();
	}
    event.Skip();
    return;
}

Void Canvas::OnKeyUp(wxKeyEvent& event)
{
    if (!view){
        event.Skip();
        return;
    }

    switch ( event.GetKeyCode() )
    {
        case WXK_SHIFT:
            _IsKeyShift = FALSE;
        break;
    }
    event.Skip();
}

Void Canvas::OnKeyDown(wxKeyEvent& event)
{
    if (!view){
        event.Skip();
        return;
    }

    /*! Is usefull key ? */
    Bool ShouldStopPropagation = FALSE;
    if( /*(_pLastFxLine == NULL) && (_hLastSelectedFx == NULL) &&*/ (_SelectedObject.size() != 0) )
    {
        ShouldStopPropagation = TRUE;

        //return;
    }

    wxClientDC dc(this);
    PrepareDC(dc);

    DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();

    switch ( event.GetKeyCode() )
    {
        case WXK_SHIFT:
            _IsKeyShift = TRUE;
        break;
        case WXK_LEFT:
            /*! Do not move when design is locked */
            if(g_isDesignLock){
                event.Skip();
                return;
            }
            _Child->PrepareDC(dc);
            dc.DestroyClippingRegion();
            MoveObjects(-2, 0);
			pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Move Fx"), DOODLE_ADD, pDoc, NULL, NULL));
            pDoc->Modify(true);
            break;
        case WXK_UP:
            /*! Do not move when design is locked */
            if(g_isDesignLock){
                event.Skip();
                return;
            }
            _Child->PrepareDC(dc);
            dc.DestroyClippingRegion();
            MoveObjects(0, -2);
			pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Move Fx"), DOODLE_ADD, pDoc, NULL, NULL));
            pDoc->Modify(true);
            break;
        case WXK_RIGHT:
            /*! Do not move when design is locked */
            if(g_isDesignLock){
                event.Skip();
                return;
            }
            _Child->PrepareDC(dc);
            dc.DestroyClippingRegion();
            MoveObjects(2, 0);
			pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Move Fx"), DOODLE_ADD, pDoc, NULL, NULL));
            pDoc->Modify(true);
            break;
        case WXK_DOWN:
            /*! Do not move when design is locked */
            if(g_isDesignLock){
                event.Skip();
                return;
            }
            _Child->PrepareDC(dc);
            dc.DestroyClippingRegion();
            MoveObjects(0, 2);
			pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Move Fx"), DOODLE_ADD, pDoc, NULL, NULL));
            pDoc->Modify(true);
            break;

        case WXK_DELETE:
            {
                /*! Do not move when design is locked */
                if(g_isDesignLock){
                    event.Skip();
                    return;
                }

				/*! Try to lock Fx first */
				vSelectedObject::iterator It = _SelectedObject.begin();
				while(It != _SelectedObject.end())
				{
					if((*It)->GetObjectType() == FX_OBJECT)
					{
						CFx* pFx = (CFx*)(*It);
						pDoc->StopFxOnly(pFx->GethFxHandle());
					}
					It++;
				}

                /*! Start with line first (case when fx and line are selected in same time */
                It = _SelectedObject.begin();
				while(It != _SelectedObject.end())
				{
					if((*It)->GetObjectType() == LINE_OBJECT)
                    {
						CFxLine* pFxLine = (CFxLine*)(*It);
                        pDoc->RemoveFxLineObject(pFxLine);
                        _pFxLine = NULL;
						_pLastFxLine = NULL;
						It = _SelectedObject.erase(It);
                    }
					else
						It++;
                }
				It = _SelectedObject.begin();
				while(It != _SelectedObject.end())
				{
					if((*It)->GetObjectType() == FX_OBJECT)
					{
						CFx* pFx = (CFx*)(*It);
                        pDoc->RemoveFxObject(pFx->GethFxHandle());
						_hLastSelectedFx = NULL;
						It = _SelectedObject.erase(It);
                    }
					else
						It++;
				}
                _SelectedObject.clear();
                _IsSelect = FALSE;

				pDoc->GetCommandProcessor()->Submit(new DrawingCommand(wxT("Delete Fx"), DOODLE_ADD, pDoc, NULL, NULL));
                pDoc->Modify(true);
                ClearBackground();
                Refresh();
                Update();
            }
            break;
        case WXK_ESCAPE:
            {
                /*Marker selection */
	            if(_pLastFxLine)
		            _pLastFxLine->SetMarker(FALSE);

	            if(_hLastSelectedFx)
	            {
		            CFx* pFx = pDoc->GetFx(_hLastSelectedFx);
                    if(pFx)
		                pFx->SetMarker(FALSE);
	            }
                vSelectedObject::iterator It;
                for ( It = _SelectedObject.begin( ); It != _SelectedObject.end( ); It++ )
                    (*It)->SetMarker(FALSE);
                _SelectedObject.clear();
                _IsSelect = FALSE;
                _hSelectedFx = NULL;
	            _pEndNode = NULL;
	            _pFxLine = NULL;
	            _hLastSelectedFx = NULL;
	            _pLastFxLine = NULL;
	            _IsCreateLine = FALSE;
	            _ShouldInsertLink = FALSE;
	            _MouseEvt = MOUSE_NONE;
                SetCursor(*_pCursorArrow);
            }
            break;

        case WXK_NUMPAD_LEFT:
        case WXK_NUMPAD_UP:
        case WXK_NUMPAD_RIGHT:
        case WXK_NUMPAD_DOWN:
            event.Skip();
            break;
        default:
            event.Skip();
            break;
    }

    //Refresh();
    //Update();

    if(ShouldStopPropagation) {
        event.StopPropagation();
    }
    else
        event.Skip();

    return;
}

Void Canvas::OnTxRx(Bool showTxRx)
{
	if(showTxRx == FALSE)
		_pTimer->Stop();
    else
		if((showTxRx == TRUE) && !_pTimer->IsRunning())
			_pTimer->Start(TIMER_TXRX);
	Refresh();
	return;
}

Void Canvas::OnTimer(wxTimerEvent& event)
{
	if(!g_showAllTxRx) {
		if(_pLastOverNode) //!< Call selected pin only
			_pLastOverNode->UpdateTxRx();
	}
	else //!< Call all pins
	{
		DrawingDocument *pDoc = (DrawingDocument *)view->GetDocument();

    	AutoLock lockDoc(pDoc->_CSDoc); //! protection when receive release Fx

		FxMap::iterator Itmap;
        for ( Itmap = pDoc->GetFxMap().begin( ); Itmap != pDoc->GetFxMap().end( ); Itmap++ )
		{
			Itmap->second->UpdateTxRx();			
		}	
	}
	Refresh();
	return;
}

Void FEF_CALLBACK FxStateProc(FX_STATE FxState, FX_HANDLE hObserverId, FX_PTR dwParam)
{
	Canvas *pCanvas = (Canvas*)dwParam;

	if (!pCanvas->view)
        return;


	DrawingDocument *pDoc = (DrawingDocument *)pCanvas->view->GetDocument();

    AutoLock lockDoc(pDoc->_CSDoc); //! protection when receive release Fx

	if(FxState == FX_PIN_UPDATE)
	{
		FxMap::iterator Itmap;
        for ( Itmap = pDoc->GetFxMap().begin( ); Itmap != pDoc->GetFxMap().end( ); Itmap++ )
		{
			CFx* pFx = Itmap->second;
			if(pFx->GethFxState() == hObserverId)
			{
                AutoLock lock(pCanvas->_CSDraw);
                if(Itmap->second->UpdateFxPin() == TRUE)
                    pCanvas->TryConnection(); //!< Try to connect pin
                lock.unlock();
                break;
			}
		}
        lockDoc.unlock(); //!< It is called by Update()
#ifdef WIN32
        pCanvas->Refresh();
#else
		wxPaintEvent Event;
		pCanvas->GetEventHandler()->AddPendingEvent( Event );

#endif
        //pCanvas->Update();
	}
    else if(FxState == FX_FRAME_UPDATE)
	{
		FxMap::iterator Itmap;
        for ( Itmap = pDoc->GetFxMap().begin( ); Itmap != pDoc->GetFxMap().end( ); Itmap++ )
		{
			CFx* pFx = Itmap->second;
			if(pFx->GethFxState() == hObserverId)
			{
                AutoLock lock(pCanvas->_CSDraw);
                Itmap->second->UpdateFxFrame();
                lock.unlock();
                break;
			}
		}
        lockDoc.unlock(); //!< It is called by Update()
#ifdef WIN32
        pCanvas->Refresh();
#else
		wxPaintEvent Event;
		pCanvas->GetEventHandler()->AddPendingEvent( Event );

#endif
        //pCanvas->Update();
	}

#ifdef WIN32
	pCanvas->Refresh();
#else
	wxPaintEvent Event;
	pCanvas->GetEventHandler()->AddPendingEvent( Event );

#endif


	return;
}

bool CPrintout::OnPrintPage(int page)
{
    wxDC *dc = GetDC();
    if (dc)
    {
        wxPoint point;
        if(IsPreview()) {
            point.x = ((page-1)%_wHPageNumber) * _dwDCPageWidth;
            point.y = ((page-1)/_wHPageNumber) * _dwDCPageHeight;
        }
        else
        {
            point.x = ((page-1)%_wHPageNumber) * _sdwPageWidth;
            point.y = ((page-1)/_wHPageNumber) * _sdwPageHeight;
        }

        DrawPage(point, page);

        return true;
    }
    else
        return false;
}

bool CPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;

    return true;
}

Void CPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    /*! Get Number of page we have */
    wxPoint DocMaxPoint = _pCanvas->GetDocSize();

    /*! Get the page number */
    GetPageSizePixels(&_sdwPageWidth, &_sdwPageHeight);
    // Get the logical pixels per inch of screen and printer
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);

    // This scales the DC so that the printout roughly represents the the screen
    // scaling. The text point size _should_ be the right size but in fact is
    // too small for some reason. This is a detail that will need to be
    // addressed at some point but can be fudged for the moment.
    _fPreviewPrintZoom = (float)((float)ppiPrinterX/(float)ppiScreenX);
    _fPrintZoom = 1.0f;

    _wHPageNumber = (Uint32)((DocMaxPoint.x/(_sdwPageWidth/_fPreviewPrintZoom))+1);
    _wVPageNumber = (Uint32)((DocMaxPoint.y/(_sdwPageHeight/_fPreviewPrintZoom))+1);

    _wHPageNumber = (_wHPageNumber == 0) ? 1:_wHPageNumber;
    _wVPageNumber = (_wVPageNumber == 0) ? 1:_wVPageNumber;

    _dwDCPageWidth = (_sdwPageWidth/_fPreviewPrintZoom)/*/_wHPageNumber*/;
    _dwDCPageHeight = (_sdwPageHeight/_fPreviewPrintZoom)/*/_wVPageNumber*/;

    *minPage = 1;
    *maxPage = _wHPageNumber*_wVPageNumber;
    *selPageFrom = 1;
    *selPageTo = _wHPageNumber*_wVPageNumber;
}

bool CPrintout::HasPage(int pageNum)
{
    if(!IsPreview())
        _fPrintZoom = _fPreviewPrintZoom;
    return (pageNum <= (_wHPageNumber*_wVPageNumber));
}

Void CPrintout::DrawPage(wxPoint& Point, int page)
{
    wxDC *dc = GetDC();

    Float fZoom = _pCanvas->GetZoom() * _fPrintZoom;
    dc->SetLogicalOrigin(Point.x/fZoom, Point.y/fZoom);

    // Draw page numbers at top left corner of printable area, sized so that
    // screen size of text matches paper size.
    //MapScreenSizeToPage();
    wxChar buf[200];
    wxSprintf(buf, wxT("PAGE %d"), page);
    dc->DrawText(buf, Point.x/fZoom, Point.y/fZoom);

   FitThisSizeToPageMargins(wxSize(HWORKINGSIZE, VWORKINGSIZE), *g_pageSetupData);
    _pCanvas->OnDraw(dc, Point, _fPrintZoom);
}
