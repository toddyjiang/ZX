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

#include "wx/print.h"
#include "wx/printdlg.h"

#include "wx/cmdline.h"     //for wxCmdLineParser (optional)

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#if !wxUSE_DOC_VIEW_ARCHITECTURE
#error You must set wxUSE_DOC_VIEW_ARCHITECTURE to 1 in setup.h!
#endif

#if !wxUSE_MDI_ARCHITECTURE
#error You must set wxUSE_MDI_ARCHITECTURE to 1 in setup.h!
#endif

#if !wxUSE_PRINTING_ARCHITECTURE
#error "You must set wxUSE_PRINTING_ARCHITECTURE to 1 in setup.h, and recompile the library."
#endif

#ifdef __WIN32__
    // this is not supported by native control
    #define NO_VARIABLE_HEIGHT
#endif

#include "Resource.h"
#include "EditorDefs.h"


#include "FxEngine.h"
#include "Node.h"
#include "FxLine.h"
#include "NodeManager.h"
#include "FxColor.h"
#include "Fx.h"

using namespace FEF; //!< Using FxEngineFramework

#include "CTreeCtrl.h"

#include "../Img/chart_line.xpm"
#include "../Img/magnifier_zoom_in.xpm"
#include "../Img/magifier_zoom_out.xpm"
#include "../Img/pause.xpm"
#include "../Img/stop.xpm"
#include "../Img/disk.xpm"
#include "../Img/layout.xpm"
#include "../Img/layout_add.xpm"
#include "../Img/folder.xpm"
#include "../Img/Play.xpm"

#ifndef WIN32
    #include "../Img/FxEngineEditor.xpm" //!< Main FxEngine Icon
#endif

#include "EditorPropertySheet.h"

#include "XMLRoot.h"

#define wxUSE_TOOLBAR 1

#include "FxEngineEditor.h"
#include "FxEngineEditorDoc.h"
#include "FxEngineEditorView.h"

/*bool SetRunAppWithExtension(const char *szAppName,
                            const char *szLibApp,
                            const char *szCommandLine,
                            const char *szExtName)
{
    HKEY hKey=NULL;
    long ret;
    // Creation de la racine szAppName

    if(RegCreateKey(HKEY_CLASSES_ROOT, szAppName, &hKey)!=ERROR_SUCCESS) return false;
    if(RegSetValue(hKey, "", REG_SZ,szLibApp, NULL)!=ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);
    // création de l'entrée pour l'extension szExtname associée à szAppName
    if((ret=RegCreateKey(HKEY_CLASSES_ROOT, szExtName,&hKey))==ERROR_SUCCESS)
        ret=RegSetValue(hKey, "", REG_SZ, szAppName,NULL);
    else
    {
        RegCloseKey(hKey);
        return false;
    }
    RegCloseKey(hKey);
    // fixe la ligne de commande pour szAppName
    if((ret = RegCreateKey(HKEY_CLASSES_ROOT, szAppName,&hKey))==ERROR_SUCCESS)
        ret = RegSetValue(hKey, "shell\\open\\command", REG_SZ, szCommandLine,
                            MAX_PATH);
    if(ret==ERROR_SUCCESS) RegCloseKey(hKey);

    return (ret==ERROR_SUCCESS);
}

bool SetIconApp(const char *szAppName,const char *szPathIcon)
{
    HKEY hKey=NULL;
    long ret;
   // Création de la racine szAppName
     if(RegCreateKey(HKEY_CLASSES_ROOT, szAppName, &hKey)!=ERROR_SUCCESS) return false;
            ret = RegSetValue(hKey, "DefaultIcon", REG_SZ,szPathIcon,MAX_PATH);

     if(ret==ERROR_SUCCESS) RegCloseKey(hKey);
     return (ret==ERROR_SUCCESS);
}
*/

// Global print data, to remember settings during the session
wxPrintData *g_printData = (wxPrintData*) NULL ;

// Global page setup data
wxPageSetupDialogData* g_pageSetupData = (wxPageSetupDialogData*) NULL;

// Declare a frame
Frame *g_frame = (Frame *) NULL;

// Global fonts
wxFont		g_Font;
wxFont		g_PinFont;

bool        g_isDesignLock = FALSE;
bool        g_showAllTxRx = FALSE;

IMPLEMENT_APP(FxEngineEditor)

FxEngineEditor::FxEngineEditor()
{
    _docManager = (wxDocManager *) NULL;
	_menu_bar = NULL;
    _TabNumber = 1;
}

bool FxEngineEditor::OnInit(void)
{
    SetVendorName(wxT("SMProcess"));

    /*! get FxEngine options in the XML file */
    wxStandardPaths StandardPaths;
//    std::string str = "";
//#ifdef WIN32
//    str += "\""; str += ws2s(StandardPaths.GetExecutablePath()); str += "\" \"%1\"" ;
//#else
//    str += "\""; str += ws2s(StandardPaths.GetExecutablePath()); str += "\" \"%1\"" ;
//#endif
//    //str += " %1";

	/*! next line is performed in script instal */
	//SetRunAppWithExtension ("FxEngine", "FxEngine Editor File",str.c_str(),".fxe");

  //// Create a document manager
  _docManager = new wxDocManager;

  //// Create a template relating drawing documents to their views
  (Void) new wxDocTemplate((wxDocManager *) _docManager, wxT("FxEngine Editor"), wxT("*.fxe"), wxT(""), wxT("fxe"), wxT("FxEngine Editor Doc"), wxT("FxEngine Editor View"),
          CLASSINFO(DrawingDocument), CLASSINFO(DrawingView));


  //// Make a menubar
  wxMenu *file_menu = new wxMenu;
  wxMenu *edit_menu = (wxMenu *) NULL;

    file_menu->Append(wxID_NEW, wxT("&New FxEngine\tCtrl-N"), wxT("Create a new FxEngine"));
    file_menu->Append(wxID_OPEN,  wxT("&Open"), wxT("Open FxEngine file"));
    file_menu->AppendSeparator();
    file_menu->Append(wxID_EXIT, wxT("&Exit\tAlt-X"), wxT("Quit the program"));


  // A nice touch: a history of files visited. Use this menu.
  _docManager->FileHistoryUseMenu(file_menu);

  wxMenu *option_menu = new wxMenu;
  option_menu->Append(MDI_CHILD_OPTION, wxT("&FxEngine Options"), wxT("Configure FxEngine Editor"));

  wxMenu *help_menu = new wxMenu;
  help_menu->Append(DOCVIEW_ABOUT, wxT("&About\tF1"));
  help_menu->Append(MENU_FXENGINE_VERSION, wxT("&FxEngine Version"));

  _menu_bar = new wxMenuBar;

  _menu_bar->Append(file_menu, wxT("&File"));
  if (edit_menu)
    _menu_bar->Append(edit_menu, wxT("&Edit"));
  _menu_bar->Append(option_menu, wxT("&Options"));
  _menu_bar->Append(help_menu, wxT("&Help"));

  //// Create the main g_frame window
  g_frame = new Frame((wxDocManager *) _docManager, (wxFrame *) NULL,
                      wxT("FxEngine Editor"), wxPoint(0, 0), wxSize(800, 600),
                      wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxHSCROLL | wxVSCROLL);

  //// Give it an icon (this is ignored in MDI mode: uses resources)
#ifdef __WXMSW__
  g_frame->SetIcon(wxIcon(wxT("fxengine")));
#else
  g_frame->SetIcon(wxIcon(FxEngineEditor_xpm));
#endif

#if wxUSE_STATUSBAR
    g_frame->CreateStatusBar(); //! Create Always
#endif // wxUSE_STATUSBAR

#ifdef __WXMAC__
  wxMenuBar::MacSetCommonMenuBar(menu_bar);
#endif //def __WXMAC__
  //// Associate the menu bar with the g_frame
  g_frame->SetMenuBar(_menu_bar);

  g_frame->Centre(wxBOTH);

#ifndef __WXMAC__
  g_frame->Show(true);
#endif //ndef __WXMAC__

  SetTopWindow(g_frame);

  g_frame->InitTree();

#if wxUSE_CMDLINE_PARSER
    //
    //  What this does is get all the command line arguments
    //  and treat each one as a file to put to the initial playlist
    //
    wxCmdLineEntryDesc cmdLineDesc[2];
    cmdLineDesc[0].kind = wxCMD_LINE_PARAM;
    cmdLineDesc[0].shortName = NULL;
    cmdLineDesc[0].longName = NULL;
    cmdLineDesc[0].description = wxT("input files");
    cmdLineDesc[0].type = wxCMD_LINE_VAL_STRING;
    cmdLineDesc[0].flags = wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE;

    cmdLineDesc[1].kind = wxCMD_LINE_NONE;

    //gets the passed media files from cmd line
    wxCmdLineParser parser (cmdLineDesc, argc, argv);

    // get filenames from the commandline
    if (parser.Parse() == 0)
    {
        for (size_t paramNr=0; paramNr < parser.GetParamCount(); ++paramNr)
        {
            g_frame->_strArray.Add(parser.GetParam (paramNr));

        }
        //Sleep(1000);
        wxCommandEvent theEvent(wxEVT_COMMAND_MENU_SELECTED, COMMAND_LINE);
        g_frame->AddPendingEvent(theEvent);
    }
#endif

    /*_pDropTarget = new DropTarget(g_frame);
    g_frame->SetDropTarget(g_frame);*/

  //wxApp::OnInit();

  return true;
}

//void FxEngineEditor::OnInitCmdLine(wxCmdLineParser& parser)
//{
//
//  g_frame->OnDropFiles(0,0,g_frame->_strArray);
//
//    return;
//}

int FxEngineEditor::OnExit(void)
{
	CNodeManager* p = CNodeManager::Instance();
	SAFE_DELETE_OBJECT(p);

    g_frame = NULL;
    delete _docManager;
	return 0;
}

/*
 * Centralised code for creating a document g_frame.
 * Called from view.cpp, when a view is created.
 */

wxMDIChildFrame *FxEngineEditor::CreateChildFrame(wxDocument *doc, wxView *view, bool isCanvas)
{
  static wxPoint Point(-10,-10);
  Point += wxPoint(10,10);

  wxString strChildFrame = wxT("Unnamed");

#ifndef WIN32
  wxString strFile = doc->GetFilename();

  if(strFile == wxT("")){
      strChildFrame = wxT("Unnamed");
      strChildFrame << _TabNumber++;
  }

#endif
  //// Make a child g_frame
  wxDocMDIChildFrame *subframe =
      new wxDocMDIChildFrame(doc, view, GetMainFrame(), wxID_ANY, strChildFrame,
                             Point, wxSize(500, 300),
                             wxDEFAULT_FRAME_STYLE |
                             wxNO_FULL_REPAINT_ON_RESIZE);

#ifdef __WXMSW__
  subframe->SetIcon(wxString(wxT("chart")));
#endif
#ifdef __X__
  subframe->SetIcon(wxIcon(wxT("doc.xbm")));
#endif

  //// Make a menubar
  wxMenu *file_menu = new wxMenu;

    file_menu->Append(wxID_NEW,wxT("&New FxEngine\tCtrl-N"), wxT("Create a new FxEngine"));
    file_menu->Append(wxID_OPEN,  wxT("&Open"), wxT("Open FxEngine file"));
    file_menu->Append(wxID_CLOSE, wxT("&Close FxEngine"), wxT("Close this window"));
    file_menu->Append(wxID_SAVE, wxT("&Save"), wxT("Save FxEngine"));
    file_menu->Append(wxID_SAVEAS, wxT("&Save &As..."), wxT("Save FxEngine As"));
    file_menu->Append(MENU_EXPORT, wxT("&Export FxEngine"),  wxT("Export FxEngine"));


     wxMenu *Insert_menu = new wxMenu;
    Insert_menu->Append(MENU_INSERT_FX, wxT("&Insert Fx"), wxT("Insert Fx"));
    Insert_menu->Append(MENU_INSERT_LINK, wxT("&Insert Fx link"), wxT("Insert Link"));

    wxMenu *FxEngine_menu = new wxMenu;
    FxEngine_menu->Append(MENU_FXENGINE_START, wxT("&Start"), wxT("Start FxEngine"));
    FxEngine_menu->Append(MENU_FXENGINE_STOP, wxT("&Stop"), wxT("Stop FxEngine"));
    FxEngine_menu->Append(MENU_FXENGINE_PAUSE, wxT("&Pause"), wxT("Pause FxEngine"));

    wxMenu *option_menu = new wxMenu;
    option_menu->Append(MDI_CHILD_OPTION, wxT("&FxEngine Options"), wxT("Configure FxEngine Editor"));
    option_menu->Append(MENU_DRAWING_LOCK, wxT("&Design lock"), wxT("Lock FxEngine Editor design"), true);
	option_menu->Append(MENU_SHOW_TXRX, wxT("&Show all TxRx bytes"), wxT("Show all TxRx Bytes"), true);
    option_menu->Check(MENU_DRAWING_LOCK, g_isDesignLock);
	option_menu->Check(MENU_SHOW_TXRX, g_showAllTxRx);

    wxMenu *help_menu = new wxMenu;
    help_menu->Append(MENU_FXENGINE_VERSION, wxT("&FxEngine Version"));
    help_menu->Append(DOCVIEW_ABOUT, wxT("&About FxEngine Editor"));

  if (isCanvas)
  {
    file_menu->AppendSeparator();
    file_menu->Append(MENU_PRINT, wxT("&Print..."),              wxT("Print"));
    file_menu->Append(MENU_PAGE_SETUP, wxT("Page Set&up..."),              wxT("Page setup"));
    file_menu->Append(MENU_PREVIEW, wxT("Print Pre&view"),              wxT("Preview"));
  }

  file_menu->AppendSeparator();
  file_menu->Append(wxID_EXIT, wxT("&Exit\tAlt-X"), wxT("Quit the program"));



  if (isCanvas)
  {
	/*wxMenu *edit_menu = (wxMenu *) NULL;
    edit_menu = new wxMenu;
    edit_menu->Append(wxID_UNDO, wxT("&Undo"));
    edit_menu->Append(wxID_REDO, wxT("&Redo"));
    edit_menu->AppendSeparator();
    edit_menu->Append(DOCVIEW_CUT, wxT("&Cut last segment"));

    doc->GetCommandProcessor()->SetEditMenu(edit_menu);*/
  }

    wxMenuBar *menu_bar = new wxMenuBar;

    menu_bar->Append(file_menu, wxT("&File"));
    menu_bar->Append(Insert_menu, wxT("&Insert"));
    menu_bar->Append(FxEngine_menu, wxT("&FxEngine"));
    menu_bar->Append(option_menu, wxT("&Options"));
    menu_bar->Append(help_menu, wxT("&About"));


  //// Associate the menu bar with the g_frame
  subframe->SetMenuBar(menu_bar);

#if wxUSE_STATUSBAR
#ifdef WIN32
    subframe->CreateStatusBar(); //! Create only on win32
#endif
#endif // wxUSE_STATUSBAR


  return subframe;
}

/*
 * This is the top-level window of the application.
 */
 // ---------------------------------------------------------------------------
// event tables
// ---------------------------------------------------------------------------
IMPLEMENT_CLASS(Frame, wxDocMDIParentFrame)
BEGIN_EVENT_TABLE(Frame, wxDocMDIParentFrame)
    EVT_MENU(DOCVIEW_ABOUT, Frame::OnAbout)
    EVT_MENU(MENU_FXENGINE_VERSION, Frame::OnVersion)
	EVT_SIZE(Frame::OnSize)
    EVT_CLOSE(Frame::OnClose)
	EVT_MENU(MDI_CHILD_OPTION, Frame::OnOptions)
    EVT_MENU(MENU_DRAWING_LOCK, Frame::OnDrawingLock)
	EVT_MENU(MENU_SHOW_TXRX, Frame::OnTxRx)
	EVT_MENU(MENU_EXPORT, Frame::OnExport)
	EVT_MENU(MENU_INSERT_FX, Frame::OnInsertFx)
	EVT_MENU(MENU_INSERT_LINK, Frame::OnInsertLink)
	EVT_MENU(MENU_FXENGINE_START, Frame::OnFxEngineStart)
	EVT_MENU(MENU_FXENGINE_STOP, Frame::OnFxEngineStop)
	EVT_MENU(MENU_FXENGINE_PAUSE, Frame::OnFxEnginePause)

    EVT_MENU(MENU_PRINT, Frame::OnPrint)
    EVT_MENU(MENU_PREVIEW, Frame::OnPrintPreview)
    EVT_MENU(MENU_PAGE_SETUP, Frame::OnPageSetup)

	EVT_MENU(TOOL_BAR_ZOOM_IN, Frame::OnZoomIn)
    EVT_MENU(TOOL_BAR_ZOOM_OUT, Frame::OnZoomOut)
    EVT_MENU(TOOL_BAR_ADD_FX, Frame::OnInsertFx)
    EVT_MENU(TOOL_BAR_ADD_LINK, Frame::OnInsertLink)
    EVT_MENU(TOOL_BAR_START, Frame::OnFxEngineStart)
    EVT_MENU(TOOL_BAR_STOP, Frame::OnFxEngineStop)
    EVT_MENU(TOOL_BAR_PAUSE, Frame::OnFxEnginePause)
    EVT_MENU(COMMAND_LINE, Frame::OnCmdLineFiles)

	EVT_SASH_DRAGGED_RANGE(ID_WINDOW_LEFT, ID_WINDOW_LEFT, Frame::OnSashDrag)
END_EVENT_TABLE()

Frame::Frame(wxDocManager *manager, wxFrame *g_frame, const wxString& title,
    const wxPoint& pos, const wxSize& size, long type):
	wxDocMDIParentFrame(manager, g_frame, wxID_ANY, title, pos, size, type, wxT("MDIParentFrame")),
  _leftWindow(NULL)
{
    _pdocManager = manager;

    g_printData = new wxPrintData;
    // You could set an initial paper size here
//    g_printData->SetPaperId(wxPAPER_LETTER); // for Americans
    g_printData->SetOrientation(wxLANDSCAPE);
    g_printData->SetPaperId(wxPAPER_A4);    // for everyone else


    g_pageSetupData = new wxPageSetupDialogData;
    // copy over initial paper size from print record
    //(*g_pageSetupData) = *g_printData;
    g_pageSetupData->SetPrintData(*g_printData);
    // Set some initial page margins in mm.
    g_pageSetupData->SetMarginTopLeft(wxPoint(15, 15));
    g_pageSetupData->SetMarginBottomRight(wxPoint(15, 15));
    g_pageSetupData->SetPaperId(wxPAPER_A4_ROTATED);


    // global fonts
#ifdef WIN32
    g_Font = wxFont(wxSize(8,FONT_HEIGHT), wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, TRUE, wxT( "Courier New" ));
    g_PinFont = wxFont(wxSize(8,PIN_FONT_HEIGHT), wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxT( "Courier New" ));
#else
    g_Font = wxFont(8, wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, TRUE, wxT( "Courier New" ));
    g_PinFont = wxFont(8, wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, FALSE, wxT( "Courier New" ));
#endif
    editMenu = (wxMenu *) NULL;

	/*! get FxEngine options in the XML file */
	wxStandardPaths StandardPaths;
#ifdef WIN32
	std::string str = ws2s(StandardPaths.GetDataDir()) + "\\FxEngineEditor.conf";
#else
    std::string str = ws2s(StandardPaths.GetExecutablePath()) + ".conf";
#endif
    CXMLRoot xmlRoot(str);
	VectorOfPath PathVector;
    _LockOnStart = FALSE;
	xmlRoot.ReadXML(PathVector, _LockOnStart);
    if(_LockOnStart == TRUE)
        g_isDesignLock = TRUE;
	VectorOfPathItr Itr;
	for(Itr = PathVector.begin(); Itr != PathVector.end(); Itr++)
		_strDirectories.Add(s2ws(*Itr));

	if(_strDirectories.IsEmpty())
	{
#ifdef WIN32
		_strDirectories.Add(StandardPaths.GetDataDir());
#else
        _strDirectories.Add(StandardPaths.GetUserConfigDir());
#endif
	}



// A window to the left of the client window
    _leftWindow = new wxSashLayoutWindow((wxMDIParentFrame*)this, ID_WINDOW_LEFT,
                               wxDefaultPosition, wxSize(200, 30),
                               wxNO_BORDER | wxSW_3D | wxCLIP_CHILDREN);
    _leftWindow->SetDefaultSize(wxSize(200, 1000));
    _leftWindow->SetOrientation(wxLAYOUT_VERTICAL);
    _leftWindow->SetAlignment(wxLAYOUT_LEFT);
    _leftWindow->SetBackgroundColour(wxColour(255, 255, 255));
    _leftWindow->SetSashVisible(wxSASH_RIGHT, true);

#if wxUSE_TOOLBAR
    wxSystemOptions::SetOption(wxT("msw.remap"), 0);
    CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL);
    InitToolBar(GetToolBar());
#endif // wxUSE_TOOLBAR

#if wxUSE_ACCEL
    // Accelerators
    wxAcceleratorEntry entries[3];
    entries[0].Set(wxACCEL_CTRL, (int) 'N', MDI_NEW_WINDOW);
    entries[1].Set(wxACCEL_CTRL, (int) 'X', MDI_QUIT);
    entries[2].Set(wxACCEL_CTRL, (int) 'A', MDI_ABOUT);
    wxAcceleratorTable accel(3, entries);
    SetAcceleratorTable(accel);
#endif // wxUSE_ACCEL

    long treestyle = wxTR_DEFAULT_STYLE | wxTR_EDIT_LABELS | wxSUNKEN_BORDER | wxVSCROLL | wxHSCROLL;

    _treeWindow = new CTreeCtrl(_leftWindow, TreeTest_Ctrl,
               wxDefaultPosition, wxDefaultSize,
               treestyle);

    _pDropTarget = new DropTarget(this);
    SetDropTarget(_pDropTarget);
}

Frame::~Frame()
{

}

Void Frame::OnClose(wxCloseEvent& event)
{
    wxDropTarget *pDropTarget = GetDropTarget();
    if(pDropTarget){
        pDropTarget->SetDataObject(NULL);
        SetDropTarget(NULL);
    }

   //delete(_pDropTarget);

    /*! get FxEngine options in the XML file */
    wxStandardPaths StandardPaths;
#ifdef WIN32
    std::string str = ws2s(StandardPaths.GetDataDir()) + "\\FxEngineEditor.conf";
#else
    std::string str = ws2s(StandardPaths.GetExecutablePath()) + ".conf";
#endif
    CXMLRoot xmlRoot(str);
    VectorOfPath PathVector;

    wxArrayString::iterator Itr;
    for(Itr = _strDirectories.begin(); Itr != _strDirectories.end(); Itr++)
		PathVector.push_back(ws2s(*Itr));

    xmlRoot.WriteXML(PathVector, _LockOnStart);

	/*! Delete printer */
    SAFE_DELETE_OBJECT(g_pageSetupData);
    SAFE_DELETE_OBJECT(g_printData);


    event.Skip();
}

Void Frame::InitTree()
{
	_treeWindow->InitTreeValues(&_strDirectories);
}

Void Frame::OnCmdLineFiles(wxCommandEvent& event)
{
    wxArrayString::const_iterator Itr;
    for(Itr = _strArray.begin(); Itr != _strArray.end(); Itr++)
    {
        _pdocManager->CreateDocument((*Itr), wxDOC_SILENT);
    }

	event.Skip();
    return;
}

bool Frame::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
    x = x; y = y;
    wxArrayString::const_iterator Itr;
    for(Itr = filenames.begin(); Itr != filenames.end(); Itr++)
    {
        wxString strFilePath = (*Itr);
        _pdocManager->CreateDocument((*Itr), wxDOC_SILENT);
    }

    return true;
}

bool DropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
    _pFrame->OnDropFiles(x, y, filenames);

    return true;
}

Void Frame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
#ifdef WIN32
    (Void)wxMessageBox(wxT("FxEngine Editor 1.16\n")
					   wxT(__COMPILER__) wxT("\n")
                       wxT("Author: Sylvain Machel, SMProcess (c) 2009\n")
                       wxT("http://www.smprocess.com"), wxT("About FxEngineEditor"));
#else
    (Void)wxMessageBox(wxT("FxEngine Editor 1.16 linux\n")
                       wxT("Author: Sylvain Machel, SMProcess (c) 2009\n")
                       wxT("http://www.smprocess.com"), wxT("About FxEngineEditor"));
#endif
}

Void Frame::OnDrawingLock(wxCommandEvent& event)
{
	g_isDesignLock = (g_isDesignLock == FALSE) ? TRUE : FALSE;

    //event.Skip();
    return;
}

Void Frame::OnTxRx(wxCommandEvent& event)
{
	g_showAllTxRx = (g_showAllTxRx == FALSE) ? TRUE : FALSE;

	wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnTxRx(g_showAllTxRx);	

    //event.Skip();
    return;
}

Void Frame::OnOptions(wxCommandEvent& event)
{
	CEditorPropertySheet dialog(this, &_strDirectories, &_LockOnStart, &_Size);
    if(dialog.ShowModal() == wxID_OK)
    {
        /*! update Fx List */
        _treeWindow->InitTreeValues(&_strDirectories);

	    Refresh();
    }
    //event.Skip();
}

// Creates a canvas. Called from view.cpp when a new drawing
// view is created.
Canvas *Frame::CreateCanvas(wxView *view, wxMDIChildFrame *parent)
{
  int width, height;
  parent->GetClientSize(&width, &height);

  // Non-retained canvas
  Canvas *canvas = new Canvas(view, parent, wxPoint(0, 0), wxSize(width, height), 0);

  // Give it scrollbars
  canvas->SetScrollbars(20, 20, 50, 50);

  canvas->SetVirtualSize(HWORKINGSIZE, VWORKINGSIZE);

  return canvas;
}

Void Frame::OnSize(wxSizeEvent&
                                  #ifdef __WXUNIVERSAL__
                                  event
                                  #else
                                  WXUNUSED(event)
                                  #endif
                                  )
{
#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE
}

Void Frame::OnSashDrag(wxSashEvent& event)
{
    if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
        return;

    switch (event.GetId())
    {
        case ID_WINDOW_LEFT:
        {
            _leftWindow->SetDefaultSize(wxSize(event.GetDragRect().width, 1000));
            break;
        }
    }

#if wxUSE_MDI_ARCHITECTURE
    wxLayoutAlgorithm layout;
    layout.LayoutMDIFrame(this);
#endif // wxUSE_MDI_ARCHITECTURE

    // Leaves bits of itself behind sometimes
    GetClientWindow()->Refresh();
}

Void Frame::SetChildDrag()
{
    SetDropTarget((Canvas*)GetActiveChild());
    return;
}

#if wxUSE_TOOLBAR

Void Frame::InitToolBar(wxToolBar* toolBar)
{
    wxBitmap bitmaps[10];

    bitmaps[0] = wxBitmap( magnifier_zoom_in_xpm);
    bitmaps[1] = wxBitmap( magifier_zoom_out_xpm);
    bitmaps[2] = wxBitmap( chart_line_xpm);
    bitmaps[3] = wxBitmap( pause_xpm);
    bitmaps[4] = wxBitmap( Play_xpm);
    bitmaps[5] = wxBitmap( stop_xpm);
    bitmaps[6] = wxBitmap( disk_xpm);
    bitmaps[7] = wxBitmap( layout_xpm);
    bitmaps[8] = wxBitmap( layout_add_xpm);
    bitmaps[9] = wxBitmap( folder_xpm);

    toolBar->SetToolBitmapSize(wxSize(16,16));

    toolBar->AddTool(wxID_NEW, wxT("New"), bitmaps[7], wxT("New FxEngine"));
    toolBar->AddTool(wxID_OPEN, wxT("Open"), bitmaps[9], wxT("Open file"));
    toolBar->AddTool(wxID_SAVE, wxT("Save"), bitmaps[6], wxT("Save file"));
    toolBar->AddSeparator();
    toolBar->AddTool(TOOL_BAR_ADD_FX, wxT("Add"), bitmaps[8], wxT("Add Fx"));
    toolBar->AddTool(TOOL_BAR_ADD_LINK, wxT("Link"), bitmaps[2], wxT("Add Link"));
    toolBar->AddSeparator();
    toolBar->AddTool(TOOL_BAR_ZOOM_IN, wxT("Zoom +"), bitmaps[0], wxT("Zoom in"));
    toolBar->AddTool(TOOL_BAR_ZOOM_OUT, wxT("Zoom -"), bitmaps[1], wxT("Zoom out"));
    toolBar->AddSeparator();
    toolBar->AddTool(TOOL_BAR_START, wxT("Start"), bitmaps[4], wxT("Start FxEngine"));
    toolBar->AddTool(TOOL_BAR_PAUSE, wxT("Pause"), bitmaps[3], wxT("Pause FxEngine"));
    toolBar->AddTool(TOOL_BAR_STOP, wxT("Stop"), bitmaps[5], wxT("Stop FxEngine"));


    toolBar->Realize();
}
#endif // wxUSE_TOOLBAR

Void Frame::OnVersion(wxCommandEvent& WXUNUSED(event) )
{
    Uint16 wMajor	= 0;
    Uint16 wMinor	= 0;
    Uint16 wBuild	= 0;
    Uint16 wRev		= 0;

    wxString strVersion;

    FEF_GetFxEngineVersion(&wMajor, &wMinor, &wBuild, &wRev);
    //sprintf(strVersion, "FxEngine version: \n %d.%d.%d.%d\n", wMajor, wMinor, wBuild, wRev);
    strVersion = wxT("FxEngine version: \n ");
    strVersion << wMajor; strVersion += wxT(".");
    strVersion << wMinor; strVersion += wxT(".");
    strVersion << wBuild; strVersion += wxT(".");
    strVersion << wRev; strVersion += wxT("\n");

    (Void)wxMessageBox(strVersion, wxT("FxEngine Version"));
}

Void Frame::OnInsertFx(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnInsertFx(event);

    //event.Skip();
    return;
}
Void Frame::OnInsertLink(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnInsertLink(event);

    //event.Skip();
    return;
}
Void Frame::OnFxEngineStart(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnFxEngineStart(event);

    //event.Skip();
    return;
}
Void Frame::OnFxEngineStop(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnFxEngineStop(event);

    //event.Skip();
    return;
}
Void Frame::OnFxEnginePause(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnFxEnginePause(event);

    //event.Skip();
    return;
}

Void Frame::OnExport(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnExport(event);

    //event.Skip();
    return;
}

Void Frame::OnZoomIn(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnZoomIn(event);

    //event.Skip();
    return;
}
Void Frame::OnZoomOut(wxCommandEvent& event)
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
        ((DrawingView*)(pChild->GetView()))->canvas->OnZoomOut(event);

    //event.Skip();
    return;
}

Void Frame::OnPrint(wxCommandEvent& WXUNUSED(event))
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
    {
        wxPrintDialogData printDialogData(* g_printData);

        wxPrinter printer(& printDialogData);
        CPrintout printout(((DrawingView*)(pChild->GetView()))->canvas, wxT("FxEngine Editor"));
        if (!printer.Print(this, &printout, true /*prompt*/))
        {
            if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
                wxMessageBox(wxT("There was a problem printing.\nPerhaps your current printer is not set correctly?"), wxT("Printing"), wxOK);
            else
                wxMessageBox(wxT("You canceled printing"), wxT("Printing"), wxOK);
        }
        else
        {
            (*g_printData) = printer.GetPrintDialogData().GetPrintData();
        }
    }
}

Void Frame::OnPrintPreview(wxCommandEvent& WXUNUSED(event))
{
    wxDocMDIChildFrame* pChild = (wxDocMDIChildFrame*)GetActiveChild();
    if(pChild)
    {
        // Pass two printout objects: for preview, and possible printing.
        wxPrintDialogData printDialogData(* g_printData);
        wxPrintPreview *preview = new wxPrintPreview(new CPrintout(((DrawingView*)(pChild->GetView()))->canvas), new CPrintout(((DrawingView*)(pChild->GetView()))->canvas), & printDialogData);
        if (!preview->Ok())
        {
            delete preview;
            wxMessageBox(wxT("There was a problem previewing.\nPerhaps your current printer is not set correctly?"), wxT("Previewing"), wxOK);
            return;
        }

        wxPreviewFrame *frame = new wxPreviewFrame(preview, this, wxT("Print Preview"), wxPoint(100, 100), wxSize(600, 650));
        frame->Centre(wxBOTH);
        frame->Initialize();
        frame->Show();
    }
}

Void Frame::OnPageSetup(wxCommandEvent& WXUNUSED(event))
{
    (*g_pageSetupData) = *g_printData;

    wxPageSetupDialog pageSetupDialog(this, g_pageSetupData);
    pageSetupDialog.ShowModal();

    (*g_printData) = pageSetupDialog.GetPageSetupDialogData().GetPrintData();
    (*g_pageSetupData) = pageSetupDialog.GetPageSetupDialogData();
}

Frame *GetMainFrame(void)
{
  return g_frame;
}

