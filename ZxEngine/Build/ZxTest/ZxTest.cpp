// ZxTest.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "ZxTest.h"

int StartEngine();

#define MAX_LOADSTRING 100

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// �˴���ģ���а����ĺ�����ǰ������:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ZXTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ZXTEST));

	// ����Ϣѭ��:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��:
//
//    ����ϣ��
//    �˴�������ӵ� Windows 95 �еġ�RegisterClassEx��
//    ����֮ǰ�� Win32 ϵͳ����ʱ������Ҫ�˺��������÷������ô˺���ʮ����Ҫ��
//    ����Ӧ�ó���Ϳ��Ի�ù�����
//    ����ʽ��ȷ�ġ�Сͼ�ꡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ZXTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ZXTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // TODO libing.jiang 2010-03-08
   StartEngine();

   return TRUE;
}

//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: �ڴ���������ͼ����...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

#include "Fx.h"
#include "FxEngine.h"
#include <string>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace FEF;

typedef std::map<FX_HANDLE, CFx*> FxMap;

FX_HANDLE _hFxEngine;
FxMap _FxMap;

Void FEF_CALLBACK FxStateProc(FX_STATE FxState, FX_HANDLE hObserverId, FX_PTR dwParam)
{
	return;
}

FX_HANDLE AttachFxObserver(FX_HANDLE hFxEngine, FX_HANDLE hFx)
{
	FX_HANDLE hFxState;
	Int32 hr;
	hr = FEF_AttachFxObserverEx(hFxEngine, hFx, FxStateProc, (FX_PTR)NULL, &hFxState);
	if(FEF_FAILED(hr))
	{
		FEF_RemoveFx(hFxEngine, hFx);
		return NULL;
	}

	return hFxState;
}

void LoadObject(FX_HANDLE _hFxEngine, FxMap _FxMap)
{
	std::stringstream stream;

    TCHAR strcFxPath[MAX_PATH];
	std::string strFxName;
	Uint32 dwFxPosX = 100, dwFxPosY = 100;
	Uint32 dwFxMaxPosX = 500, dwFxMaxPosY = 500;
	Uint32 dwFxCount = 2;
	FX_HANDLE hFx = NULL;
    FX_HANDLE hFxState = NULL;
	Int32 hr = 0;

	IFxParam* pIFxParam;
    std::string strParamName;
    Uint16 wParamCount = 0;
    Uint8* pbParamValue;
	std::string strParamValue;
    Uint32 dwParamSize = 0;

    Bool is10Version = FALSE;
    Bool is11Version = FALSE;
    Bool is12Version = FALSE;
	Bool is13Version = FALSE;
	Bool is14Version = FALSE;

	ZeroMemory(strcFxPath, MAX_PATH * sizeof(Char));

	dwFxMaxPosX = 0;
	dwFxMaxPosY = 0;

	strcpy((char*)strcFxPath, "e:\\work\\ZxEngine\\bin\\Debug");
	is13Version = TRUE;

	if( !is14Version && !is13Version && !is12Version && !is11Version && !is10Version)
	{
		return;
	}

	/* Fx */
	std::vector<std::string> strFxPaths;
	strFxPaths.push_back("E:\\work\\ZxEngine\\bin\\Debug\\ZxSndFileSrc.dll");
	strFxPaths.push_back("E:\\work\\ZxEngine\\bin\\Debug\\FxWavFileRnd.dll");
// 	strFxPaths.push_back("E:\\work\\ZxEngine\\bin\\Debug\\ZxHelloWorld.dll");

	for(Uint32 Idx = 0; Idx < dwFxCount; Idx++)
	{
		std::string strFxPath;

        hFx = NULL;
        hFxState = NULL;
		
		strFxPath = strFxPaths.at(Idx);
        
		/*! Verify the file */
		if (_access(strFxPath.c_str(), 0) != 0)
		{
			continue;
		}
        else {
		    hr = FEF_AddFx(_hFxEngine, strFxPath, &hFx);
		    if(FEF_FAILED(hr))
		    {
				continue;
		    }
            else {
                hFxState = AttachFxObserver(_hFxEngine, hFx);
		        //hr = FEF_AttachFxObserverEx(_hFxEngine, Fx, FxStateProc, (FX_PTR)this, &hFxState);
		        if(hFxState == NULL)
		        {
					continue;
		        }
            }
        }

		CFx* pFx = NULL;
        if(is12Version || is13Version || is14Version)
        {
			pFx = new CFx(_hFxEngine, hFx, hFxState, strFxPath, dwFxPosX, dwFxPosY, strFxName);
            pFx->InitFxPin();
        }
		if(pFx == NULL)
		{
			return;
		}


		_FxMap.insert(std::make_pair(hFx, pFx));

        if(is11Version || is12Version || is13Version || is14Version)
        {
            /*! read fx parameters */
            hr = FEF_QueryFxParamInterface(_hFxEngine, pFx->GethFxHandle(), &pIFxParam);
			if(FEF_FAILED(hr)) {
				return;
			}

            pIFxParam->GetFxParamCount(&wParamCount);
            for(int i = 0; i < wParamCount; i++)
            {
                pIFxParam->SetFxParamValue(strParamName, (Void*)pbParamValue);
            }
			/*! Read string parameters */
			if(is13Version || is14Version) {
				pIFxParam->GetFxParamStringCount(&wParamCount);
				for(int i = 0; i < wParamCount; i++)
				{
					strParamName = "Sound File";
					strParamValue = "C:\\DXSDK\\Samples\\Media\\drumpad-speech.wav";
					pIFxParam->SetFxParamValue(strParamName, strParamValue);
				}
			}			

            pIFxParam->FxReleaseInterface();
            FEF_UpdateFxParam(_hFxEngine, pFx->GethFxHandle(), "", FX_PARAM_ALL);
        }
	}

	IFxPin *pin[2];
	int i = 0;
	FxMap::iterator Itmap;
	for ( Itmap = _FxMap.begin(); Itmap != _FxMap.end(); Itmap++, i++)
	{
		hr = FEF_QueryFxPinInterface(	/* [in] */_hFxEngine,
										/* [in] */Itmap->first,
										/* [out] */&pin[i],
										/* [in] */0);
		if(FEF_FAILED(hr))
		{
			return;
		}
		
	}
	hr = FEF_ConnectFxPin(_hFxEngine, pin[1], pin[0]);
	if(FEF_FAILED(hr))
	{
		return;
	}

	hr = FEF_StartFxEngine(_hFxEngine);
	if(FEF_FAILED(hr))
	{
		return;
	}	

	return;
}

int StartEngine()
{
	int hr;
	_hFxEngine = NULL;
	hr = FEF_CreateFxEngine(&_hFxEngine);
	if(FEF_FAILED(hr))
	{
		return -1;
	}

	FxMap::iterator Itmap;
	LoadObject(_hFxEngine, _FxMap);

// 	if(_hFxEngine != NULL)
// 	{
// 		hr = FEF_ReleaseFxEngine( _hFxEngine );
// 		if(FEF_FAILED(hr))
// 		{
// 		}
// 		_hFxEngine = NULL;
// 	}

	return 0;
}
