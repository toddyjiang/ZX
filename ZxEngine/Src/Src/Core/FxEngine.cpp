/* __________________________________________________________________________
	
	FxEngine Framework. 
	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.

	This file is part of FxEngine Framework.
    The FxEngine Framework library is free software; you can redistribute it
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library.
	if not,  If not, see <http://www.gnu.org/licenses/>.

   ____________________________________________http://www.SMProcess.com______ */

// FxEngine.cpp : Defines the entry point for the DLL application.
//
#include "FxEngine.h"
#include "FxEngineCore.h"

#include <map>

static std::string  g_FxLogFile;
FEF::CFxLog*		g_pLogTrace = NULL;

static FEF::Bool  SO_MAIN = FALSE;

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_FxLogFile = "";
			std::string strFxTrace = "";
			strFxTrace =  FEF::GetTemporaryDir() + "\\SMProcess.log\\FxEngine";
            if (_access(strFxTrace.c_str(), 0) == 0)
		    {
			    g_FxLogFile = strFxTrace + "\\FxEngineAPI.txt";
				g_FxLogFile.erase( remove(g_FxLogFile.begin(),g_FxLogFile.end(),' ') , g_FxLogFile.end() );
				if (_access(g_FxLogFile.c_str(), 0) == 0) {
				    std::string strOldLogFile;
				    strOldLogFile = strFxTrace + "\\FxEngineAPI.old";
				    strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
				    remove(strOldLogFile.c_str());
					rename(g_FxLogFile.c_str(), strOldLogFile.c_str());
			    }
		    }

			FEF::FX_TRACE(g_FxLogFile.c_str(), "********************************************************************************");
			FEF::FX_TRACE(g_FxLogFile.c_str(), "              FxEngineAPI:         %s ",FXENGINE_VERSION);
            FEF::FX_TRACE(g_FxLogFile.c_str(), "              FxEngine compiler:   %s ",__COMPILER__);
#ifdef _DEBUG
			FEF::FX_TRACE(g_FxLogFile.c_str(), "              FxEngine Framework Debug Version");
#endif
			FEF::FX_TRACE(g_FxLogFile.c_str(), "********************************************************************************");
			FEF::FX_TRACE(g_FxLogFile.c_str(), "");
			FEF::FX_TRACE(g_FxLogFile.c_str(), "====== FxEngineAPI Entry  ======"); 

			/*! Useful optimization for multithreaded  */
			DisableThreadLibraryCalls((HMODULE)hModule);
		}
		break;
	case DLL_PROCESS_DETACH:
		{
			SAFE_DELETE_OBJECT(g_pLogTrace);
			FEF::FX_TRACE(g_FxLogFile.c_str(), "===== FxEngineAPI Exit =====");
		}
		break;
	}

    return TRUE;
}
#else
FEF::Bool SoMain()
{
    /*! Do this only one time */
    if(!SO_MAIN) {
	    g_FxLogFile = "";

	    std::string strFxTrace = "";
		strFxTrace =  FEF::GetTemporaryDir() + "/SMProcess.log/FxEngine";
        if (access(strFxTrace.c_str(), 0) == 0)
	    {
		    g_FxLogFile = strFxTrace + "/FxEngineAPI.txt";
			g_FxLogFile.erase( remove(g_FxLogFile.begin(),g_FxLogFile.end(),' ') , g_FxLogFile.end() );
			if (access(g_FxLogFile.c_str(), 0) == 0) {
			    std::string strOldLogFile;
			    strOldLogFile = strFxTrace + "/FxEngineAPI.old";
			    strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
			    remove(strOldLogFile.c_str());
				rename(g_FxLogFile.c_str(), strOldLogFile.c_str());
		    }
	    }

	    FEF::FX_TRACE(g_FxLogFile.c_str(), "********************************************************************************");
	    FEF::FX_TRACE(g_FxLogFile.c_str(), "              FxEngineAPI:         %s ",FXENGINE_VERSION);
        FEF::FX_TRACE(g_FxLogFile.c_str(), "              FxEngine compiler:   %s ",__COMPILER__);
	    FEF::FX_TRACE(g_FxLogFile.c_str(), "********************************************************************************");
	    FEF::FX_TRACE(g_FxLogFile.c_str(), "");
	    FEF::FX_TRACE(g_FxLogFile.c_str(), "====== FxEngineAPI Entry  ======");

        SO_MAIN = TRUE;
    }

    return TRUE;
}
#endif

namespace FEF {

typedef std::map<CFxEngineCore*, Uint32> CFxEngineCoreMap;
typedef CFxEngineCoreMap::iterator CFxEngineCoreMapIter;

static CFxEngineCoreMap _FxEngineCoreMap;

static CFxConst2String  _FxConst2String;

CFxEngineCore* FindFxEngineCore(FX_HANDLE hFxEngine)
{
	CFxEngineCoreMapIter Iter;

	if ((Iter = _FxEngineCoreMap.find((CFxEngineCore*)hFxEngine)) == _FxEngineCoreMap.end())
		return NULL;

	CFxEngineCore* pFxEngineCore = (*Iter).first;

	return pFxEngineCore;
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxEngineVersion(Uint16* pwMajor, Uint16* pwMinor, Uint16* pwBuild, Uint16* pwRev)
{
#ifndef WIN32
    if(!SoMain()) {
        return FX_INVALPARAM;
    }
#endif

	if(g_pLogTrace == NULL)
		g_pLogTrace = new CFxLog(g_FxLogFile, LOG_IS_TRACE);

	if(pwMajor == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxEngineVersion FEF_FAILED !! (Invalid Parameter: pwMajor)");
		return FX_INVALPARAM;
	}

	if(pwMinor == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxEngineVersion !! (Invalid Parameter: pwMinor)");
		return FX_INVALPARAM;
	}

	if(pwBuild == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxEngineVersion FEF_FAILED !! (Invalid Parameter: pwBuild)");
		return FX_INVALPARAM;
	}

	if(pwRev == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxEngineVersion FEF_FAILED !! (Invalid Parameter: pwRev)");
		return FX_INVALPARAM;
	}

	Int32 sdwMajor = 0;
	Int32 sdwMinor = 0;
	Int32 sdwBuild = 0;
	Int32 sdwRev = 0;
#ifdef WIN32
	sscanf_s(FXENGINE_VERSION, "%d.%d.%d.%d", &sdwMajor, &sdwMinor, &sdwBuild, &sdwRev); // C4996
#else
	sscanf(FXENGINE_VERSION, "%ld.%ld.%ld.%ld", &sdwMajor, &sdwMinor, &sdwBuild, &sdwRev); // C4996
#endif

	*pwMajor = (Uint16)sdwMajor;
	*pwMinor = (Uint16)sdwMinor;
	*pwBuild = (Uint16)sdwBuild;
	*pwRev = (Uint16)sdwRev;

    return FX_OK;
}

FXENGINE_EXP Int32 FXENGINE_API FEF_CreateFxEngine(FX_HANDLE* phFxEngine)
{
	if(g_pLogTrace == NULL)
		g_pLogTrace = new CFxLog(g_FxLogFile, LOG_IS_TRACE);

#ifndef WIN32
    if(!SoMain()) {
        return FX_INVALPARAM;
    }
#endif

	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = new CFxEngineCore();
	if(pFxEngineCore == NULL)
	{
		return FX_NOMEM;
	}

	_FxEngineCoreMap[pFxEngineCore] = CFxEngineCore::_dwInstanceCount;

	*phFxEngine = (FX_HANDLE)pFxEngineCore;

	return FX_OK;
}

FXENGINE_EXP Int32 FXENGINE_API FEF_ReleaseFxEngine(FX_HANDLE hFxEngine)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_ReleaseFxEngine FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	_FxEngineCoreMap.erase(pFxEngineCore);
	SAFE_DELETE_OBJECT(pFxEngineCore);

	return FX_OK;
}

FXENGINE_EXP Int32 FXENGINE_API FEF_AddFx(FX_HANDLE hFxEngine, const std::string strFx, FX_HANDLE* phFx)
{
    CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_AddFx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->AddFx(strFx, phFx);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_AddFxEx(FX_HANDLE hFxEngine, IFxBase* pIFxBase, FX_HANDLE* phFx)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_AddFxEx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->AddFxEx(pIFxBase, phFx);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_RemoveFx(FX_HANDLE hFxEngine, FX_HANDLE hFx)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_RemoveFx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->RemoveFx(hFx);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxCount(FX_HANDLE hFxEngine, Uint16* pwFxCount)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxCount FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxCount(pwFxCount);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFx(FX_HANDLE hFxEngine,  FX_HANDLE* phFx, Uint16 wFxIndex)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFx(phFx, wFxIndex);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_StartFxEngine(FX_HANDLE hFxEngine)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_StartFxEngine FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->StartFxEngine();
}

FXENGINE_EXP Int32 FXENGINE_API FEF_StartFx(FX_HANDLE hFxEngine, FX_HANDLE hFx)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_StartFx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->StartFx(hFx);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_PauseFxEngine(FX_HANDLE hFxEngine)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_PauseFxEngine FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->PauseFxEngine();
}

FXENGINE_EXP Int32 FXENGINE_API FEF_PauseFx(FX_HANDLE hFxEngine, FX_HANDLE hFx)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_PauseFx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->PauseFx(hFx);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_StopFxEngine(FX_HANDLE hFxEngine)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_StopFxEngine FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->StopFxEngine();
}

FXENGINE_EXP Int32 FXENGINE_API FEF_StopFx(FX_HANDLE hFxEngine, FX_HANDLE hFx)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_StopFx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->StopFx(hFx);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxInfo(FX_HANDLE hFxEngine, FX_HANDLE hFx, const FX_DESCRIPTOR** ppFxDescriptor)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxInfo FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxInfo(hFx, ppFxDescriptor);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxState(FX_HANDLE hFxEngine, FX_HANDLE hFx, FX_STATE* pFxState)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxState FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxState(hFx, pFxState);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetConstToString(FXENGINE_CONST_TYPE FxEngineConstType, Int32 sdwFxEngineConst, std::string& strStateName)
{
	strStateName = _FxConst2String.GetConstToString(FxEngineConstType, sdwFxEngineConst);

	return FX_OK;
}

FXENGINE_EXP Int32 FXENGINE_API FEF_AttachFxObserver(FX_HANDLE hFxEngine,
					   FX_HANDLE hFx,
					   CFxStateCallback* pFxStateCallback,
					   FX_PTR dwParam,
                       FX_HANDLE* phObserverId)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_AttachFxObserver FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->AttachFxObserver(hFx, pFxStateCallback, dwParam, phObserverId);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_AttachFxObserverEx(FX_HANDLE hFxEngine,
						 FX_HANDLE hFx,
						 FXSTATECALLBACK* pFxStateCallback,
						 FX_PTR dwParam,
                         FX_HANDLE* phObserverId)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_AttachFxObserverEx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->AttachFxObserverEx(hFx, pFxStateCallback, dwParam, phObserverId);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_DetachFxObserver(FX_HANDLE hFxEngine, FX_HANDLE hObserverId)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_DetachFxObserver FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->DetachFxObserver(hObserverId);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxPinCount(FX_HANDLE hFxEngine, FX_HANDLE hFx, Uint16* pwPinCount)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxPinCount FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxPinCount(hFx, pwPinCount);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_QueryFxPinInterface(FX_HANDLE hFxEngine, FX_HANDLE hFx, IFxPin** ppIFxPin, Uint16 wPinIndex)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_QueryFxPinInterface FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->QueryFxPinInterface(hFx, ppIFxPin, wPinIndex);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_QueryFxParamInterface(FX_HANDLE hFxEngine, FX_HANDLE hFx, IFxParam** ppIFxParam)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_QueryFxParamInterface FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->QueryFxParamInterface(hFx, ppIFxParam);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_UpdateFxParam(	/* [in] */FX_HANDLE hFxEngine,
												/* [in] */FX_HANDLE hFx,
                                                /* [in] */const std::string strParamName,
                                                /* [in] */FX_PARAMETER FxParameter)
{
    CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_UpdateFxParam FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->UpdateFxParam(hFx, strParamName, FxParameter);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_ConnectFxPin(FX_HANDLE hFxEngine, IFxPin* pFxPinIn, IFxPin* pFxPinOut)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_ConnectFxPin FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->ConnectFxPin(pFxPinIn, pFxPinOut);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_ConnectFxPinEx(FX_HANDLE hFxEngine, IFxPin* pFxPinIn, IFxPin* pFxPinOut, PFX_MEDIA_TYPE pMediaType)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_ConnectFxPinEx FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->ConnectFxPinEx(pFxPinIn, pFxPinOut, pMediaType);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_DisconnectFxPin(FX_HANDLE hFxEngine, IFxPin* pFxPin)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_DisconnectFxPin FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->DisConnectFxPin(pFxPin);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_SetFxEngineRefClock(FX_HANDLE hFxEngine, Uint64 qRefClock)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_SetFxEngineRefClock FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->SetFxEngineRefClock(qRefClock);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxEngineRefClock(FX_HANDLE hFxEngine, Uint64* pqRefClock)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxEngineRefClock FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxEngineRefClock(pqRefClock);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxRefClock(FX_HANDLE hFxEngine, Uint64* pqRefClock, Uint32 dwId)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxRefClock FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxRefClock(pqRefClock, dwId);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_DisplayFxPropertyPage(FX_HANDLE hFxEngine, FX_HANDLE hFx, Pvoid pvWndParent)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_DisplayFxPropertyPage FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->DisplayFxPropertyPage(hFx, pvWndParent);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxFrame(FX_HANDLE hFxEngine, FX_HANDLE hFx, const Char** ppbFxFrame)
{
    CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxFrame FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxFrame(hFx, ppbFxFrame);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxUserInterface(FX_HANDLE hFxEngine, FX_HANDLE hFx, Pvoid* ppvUserInterface)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxUserInterface FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxUserInterface(hFx, ppvUserInterface);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxSubFxEngine(FX_HANDLE hFxEngine, FX_HANDLE hFx, FX_HANDLE* phFxEngine)
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_GetFxSubFxEngine FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->GetFxSubFxEngine(hFx, phFxEngine);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_SaveFxEngine(FX_HANDLE hFxEngine, const std::string strFilePath )
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_SaveFxEngine FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->SaveFxEngine(strFilePath);
}

FXENGINE_EXP Int32 FXENGINE_API FEF_LoadFxEngine(/* [in] */FX_HANDLE hFxEngine,
										     /* [in] */const std::string strFilePath )
{
	CFxEngineCore* pFxEngineCore = NULL;
	pFxEngineCore = FindFxEngineCore(hFxEngine);
	if(pFxEngineCore == NULL)
	{
		g_pLogTrace->FxTrace( "FxEngineAPI::FEF_LoadFxEngine FEF_FAILED !! (Invalid Parameter: hFxEngine)");
		return FX_INVALPARAM;
	}

	return pFxEngineCore->LoadFxEngine(strFilePath);
}

 } //namespace FEF
