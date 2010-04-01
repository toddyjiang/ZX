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
#ifndef __FXENGINEEDITORH__
#define __FXENGINEEDITORH__

#include "FxEngine.h"
using namespace FEF; //!< Using FxEngineFramework

#include "wx/mdi.h"
#include "wx/docview.h"
#include "wx/docmdi.h"

#include "wx/laywin.h"
#include "wx/dnd.h"
#include "wx/toolbar.h"
#include "wx/sysopt.h"
#include "wx/stdpaths.h"
#include "wx/spinctrl.h"
#include "wx/image.h"
#include "wx/bookctrl.h"
#include <wx/propdlg.h>

class wxDocManager;

class Frame;
class DropTarget : public wxFileDropTarget
{
public:
    DropTarget(Frame* pFrame)
        : _pFrame(pFrame)
        { }
    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);

private:
    Frame* _pFrame;
};

// Define a new application
class FxEngineEditor: public wxApp
{
  public:
    FxEngineEditor();
    bool OnInit(void);
   // Void OnInitCmdLine(wxCmdLineParser& parser);
    int OnExit(void);

    wxMDIChildFrame *CreateChildFrame(wxDocument *doc, wxView *view, bool isCanvas);

  protected:
    wxDocManager* _docManager;
	wxMenuBar *_menu_bar;
    Uint32 _TabNumber;
};

DECLARE_APP(FxEngineEditor)


// Define a new g_frame
class CTreeCtrl;
class Canvas;
class Frame: public wxDocMDIParentFrame, public wxFileDropTarget
{
  DECLARE_CLASS(Frame)
 public:
  wxMenu *editMenu;

  Frame(wxDocManager *manager, wxFrame *g_frame, const wxString& title, const wxPoint& pos, const wxSize& size,
    long type);
  virtual ~Frame();

public:

	Void OnSize(wxSizeEvent& event);
    Void OnClose(wxCloseEvent& event);
	Void OnAbout(wxCommandEvent& event);
	Void OnSashDrag(wxSashEvent& event);
	Void OnOptions(wxCommandEvent& event);
    Void OnDrawingLock(wxCommandEvent& event);
	Void OnTxRx(wxCommandEvent& event);
	Void OnVersion(wxCommandEvent& event);
	Void OnInsertFx(wxCommandEvent& event);
	Void OnInsertLink(wxCommandEvent& event);
	Void OnFxEngineStart(wxCommandEvent& event);
	Void OnFxEngineStop(wxCommandEvent& event);
	Void OnFxEnginePause(wxCommandEvent& event);
	Void OnExport(wxCommandEvent& event);
	Void OnZoomIn(wxCommandEvent& event);
	Void OnZoomOut(wxCommandEvent& event);

    Void OnCmdLineFiles(wxCommandEvent& event);

    Void OnPrint(wxCommandEvent& event);
    Void OnPrintPreview(wxCommandEvent& event);
    Void OnPageSetup(wxCommandEvent& event);

    bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);

  Canvas *CreateCanvas(wxView *view, wxMDIChildFrame *parent);
  Void InitToolBar(wxToolBar* toolBar);
  Void InitTree();
  Void SetChildDrag();

public:
    wxSashLayoutWindow* _leftWindow;
	CTreeCtrl *_treeWindow;
    wxArrayString _strDirectories;
    Bool          _LockOnStart;
    wxSize _Size;
    DropTarget *_pDropTarget;

protected:
    wxDocManager* _pdocManager;

public:
    wxArrayString _strArray;




DECLARE_EVENT_TABLE()
};

extern Frame *GetMainFrame(void);

#define DOCVIEW_CUT     1
#define DOCVIEW_ABOUT   wxID_ABOUT

extern bool singleWindowMode;

#endif
