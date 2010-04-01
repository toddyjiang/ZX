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
#include "FxEngineCore.h"
#include "Fx.h"
#include "FxXML/XMLRoot.h"
#include <algorithm>
#include <errno.h>
#include "boost/lexical_cast.hpp"

#ifndef WIN32
#include <dlfcn.h>
#endif

namespace FEF {

Uint32 CFxEngineCore::_dwInstanceCount = 0;
//!< dupplicate FX
FxHandlePtrSet CFxEngineCore::_SetFxHandlePtr;


CFxEngineCore::CFxEngineCore()
{
	_qFxEngineRefClock = 0;

	_dwThisInstance = ++_dwInstanceCount; //!< For message trace only

	_FxLogFile = "";
	std::string strFxTrace = "";
#ifdef WIN32
	strFxTrace =  GetTemporaryDir() + "\\SMProcess.log\\FxEngine";
#else
	strFxTrace =  GetTemporaryDir() + "/SMProcess.log/FxEngine";
#endif
#ifdef WIN32
	if (_access(strFxTrace.c_str(), 0) == 0)
#else
    if (access(strFxTrace.c_str(), 0) == 0)
#endif
        
#ifdef WIN32
       _FxLogFile = strFxTrace + "\\FxEngine_Log" + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#else
	   _FxLogFile = strFxTrace + "\\FxEngine_Log" + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#endif
		_FxLogFile.erase( remove(_FxLogFile.begin(),_FxLogFile.end(),' ') , _FxLogFile.end() );
#ifdef WIN32
            if (_access(_FxLogFile.c_str(), 0) == 0) {
#else
            if (access(_FxLogFile.c_str(), 0) == 0) {
#endif
            std::string strOldLogFile;
#ifdef WIN32
            strOldLogFile = strFxTrace + "\\FxEngine_Log" + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#else
			strOldLogFile = strFxTrace + "/FxEngine_Log" + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#endif
			strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
            remove(strOldLogFile.c_str());
            rename(_FxLogFile.c_str(), strOldLogFile.c_str());
		}
		else
			_FxLogFile = "";

	_pLogTrace = new CFxLog(_FxLogFile, LOG_IS_TRACE);

	_pLogTrace->FxTrace(  "********************************************************************************");
	_pLogTrace->FxTrace(  "              FxEngine:         %s",FXENGINE_VERSION);
    _pLogTrace->FxTrace(  "              FxEngine compiler:   %s ",__COMPILER__);
#ifdef _DEBUG
	_pLogTrace->FxTrace(  "              FxEngine Framework Debug Version");
#endif
	_pLogTrace->FxTrace(  "********************************************************************************");
	_pLogTrace->FxTrace(  "");
    _pLogTrace->FxTrace(  "====== FxEngine constructor (instance number is %d) ======", _dwThisInstance);
}

CFxEngineCore::~CFxEngineCore(void)
{
	Int32 hr = FX_OK;

	if(FEF_FAILED( hr = StopFxEngine()) )
		_pLogTrace->FxTrace(  "FxEngine::FEF_ReleaseFxEngine FEF_FAILED !! (StopFx FEF_FAILED !!)");

	if(FEF_FAILED( hr = DisconnectAllFxPin(TRUE)) )
		_pLogTrace->FxTrace(  "FxEngine::FEF_ReleaseFxEngine FEF_FAILED !! (Disconnect Pin FEF_FAILED !!)");

    AutoLock lock(_CS);
    AutoLock lockObserver(_CSObserver);

    ClearFxVector();

	///*!-------  Function Thread -------*/
	//SetEvent(_hTerminateThread);
	//Sleep(20);
 //   TerminateThread(_hThread,0);
 //   CloseHandle(_hThread );
 //   CloseHandle(_hTerminateThread );

	/*! Clear all vectors */


	/*! Clear the CState objects in map */
	_FxStateMap.clear();

	AutoLock lockClock(_CSRefClock);
	_FxRefClock.clear();
	lockClock.unlock();

	lock.unlock();

	_pLogTrace->FxTrace(  "===== FxEngine::FEF_ReleaseFxEngine (instance number is %d) =====", _dwThisInstance);
	
	SAFE_DELETE_OBJECT(_pLogTrace);
	_dwInstanceCount--;
}

Int32 CFxEngineCore::AddFx(const std::string strFx, FX_HANDLE* phFx, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);
	_pLogTrace->FxTrace(  "CFxEngineCore::AddFx entry");

	std::string strError;
	std::string strFxPath = strFx;

	if(phFx == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFx FEF_FAILED !! (Invalid Parameter: phFx)");
		return FX_INVALPARAM;
	}
	*phFx = NULL;

    if(strFx.empty())
	{
		_pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFx FEF_FAILED !! (Invalid Parameter: strFx)");
		return FX_INVALPARAM;
	}

	/*! trace file */
	_pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFx  --> %s", strFx.c_str());

	Int32 hr;
	FxBaseFnct* lpfnDllFunc = NULL;    // Function pointer

	FX_COMPONENT* pFxComponent = NULL;
	pFxComponent = new FX_COMPONENT;
	if(pFxComponent == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFx FEF_FAILED !! (No free memory)");
		return FX_NOMEM;
	}

	pFxComponent->hDLL			= (FX_PTR)NULL;
	pFxComponent->strFxPath		= "";
	pFxComponent->strFxPathOnDisk = "";
	pFxComponent->IsTemporaryFx = FALSE;
	pFxComponent->pFxBase		= NULL;
	pFxComponent->pIFx			= NULL;
	pFxComponent->PinRunState   = PIN_STOP;
	pFxComponent->pFxParam		= NULL;
	pFxComponent->pFxPinManager	= NULL;

	CState* pState = NULL;

    //if(NULL != GetModuleHandle(strFxPath.c_str()))
    char strFxFull[MAX_PATH];
#ifdef WIN32
    if( _fullpath( strFxFull, strFxPath.c_str(), MAX_PATH ) == NULL )
#else
    if( realpath(strFxPath.c_str(), strFxFull) == NULL )
#endif
    {
        strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (Cannot get Fx full path)";
		hr = FX_INVALPARAM;
		goto Error;
    }

	/*! Replace by the full path */
	strFxPath = strFxFull;

    if(_SetFxHandlePtr.find(strFxPath) != _SetFxHandlePtr.end())
	{
		/* Create temporary file for loading. */
		if((FEF_FAILED(hr = DuplicateFx(strFxPath))))
		    goto Error;

        _SetFxHandlePtr.insert(strFxPath); //!< Push the new one

		pFxComponent->IsTemporaryFx = TRUE;
	}
    else
    {
        /*! Save Fx handle */
        _SetFxHandlePtr.insert(strFxFull);
    }

	pFxComponent->hDLL = LoadFxLibrary(strFxPath);
	if (pFxComponent->hDLL != NULL)
	{
		pFxComponent->strFxPathOnDisk = strFxFull; //!< Duplique to know the file on disk
		pFxComponent->strFxPath = strFxPath; //!< Contains the new one
		lpfnDllFunc = (FxBaseFnct*)GetFxProcAddress(pFxComponent->hDLL, "FEF_GetFxBase");
		if (lpfnDllFunc == NULL)
		{
			// handle the error
			FreeFxLibrary(pFxComponent->hDLL);
			pFxComponent->hDLL = (FX_PTR)NULL;
			strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (Cannot get the Fx entry point: FEF_GetFxBase)";
			hr = FX_ERROR;
			goto Error;
		}
		else
		{
			/*! Create new state */
	        pState = new CState((FX_PTR)pFxComponent);
	        if(pState == NULL)
	        {
		        strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (IFxState FEF_FAILED, No free memory !!)";
		        hr = FX_NOMEM;
		        goto Error;
	        }
			pState->PublishFxState(FX_LOADING_STATE);

			// call the function
			pFxComponent->pFxBase = lpfnDllFunc();

			hr = pFxComponent->pFxBase->InitFx(&pFxComponent->pIFx);
			if(FEF_FAILED(hr))
			{
				strError = "\tFxEngine::FEF_AddFx FEF_FAILED !!, hr=%d  (IFxBase::Init FEF_FAILED !!)";
				goto Error;
			}

			const FX_DESCRIPTOR* pFxDescriptor = NULL;
            hr = pFxComponent->pFxBase->GetFxInfo(&pFxDescriptor);
			if((!pFxDescriptor) || FEF_FAILED(hr))
			{
				strError = "\tFxEngine::FEF_AddFx FEF_FAILED !!, hr=%d (IFxBase::GetFxInfo FEF_FAILED !!)";
				goto Error;
			}
            pFxComponent->strFxName = pFxDescriptor->strName;

			/*!------- IFX_PINMANGER  -------*/
			IFxPinManager* pFxPinManager;
			hr = pFxComponent->pIFx->FxGetInterface(IFX_PINMANGER, (Void**)&pFxPinManager);
			if(FEF_FAILED(hr))
			{
				strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (IFx::FxGetInterface IFX_PINMANGER FEF_FAILED !!)";
				goto Error;
			}
			pFxComponent->pFxPinManager = static_cast<CFxPinManager*>(pFxPinManager);
			pFxComponent->pFxPinManager->FxReleaseInterface();

			/*!------- IFX_PARAM  -------*/
			IFxParam* pFxParam;
			hr = pFxComponent->pIFx->FxGetInterface(IFX_PARAM, (Void**)&pFxParam);
			if(FEF_FAILED(hr))
			{
				strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (IFx::FxGetInterface IFX_PARAM FEF_FAILED !!)";
				goto Error;
			}
			pFxComponent->pFxParam = static_cast<CFxParam*>(pFxParam);
			pFxComponent->pFxParam->FxReleaseInterface();

			/*!------- IFX_REFCLOCK  -------*/
			IFxRefClock* pFxRefClock;
			hr = pFxComponent->pIFx->FxGetInterface(IFX_REFCLOCK, (Void**)&pFxRefClock);
			if(FEF_FAILED(hr))
			{
				strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (IFx::FxGetInterface IFX_RefClock FEF_FAILED !!)";
				goto Error;
			}
			CFxRefClock* pClassRefClock = static_cast<CFxRefClock*>(pFxRefClock);
			pClassRefClock->AttachObserver(this, (FX_PTR)pFxComponent);
			pFxRefClock->FxReleaseInterface();

			/*!------- IFX_STATE  -------*/
			IFxState* pFxState;
			hr = pFxComponent->pIFx->FxGetInterface(IFX_STATE, (Void**)&pFxState);
			if(FEF_FAILED(hr))
			{
				strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (IFx::FxGetInterface IFX_State FEF_FAILED !!)";
				goto Error;
			}
			CFxState* pClassFxState = static_cast<CFxState*>(pFxState);
			pClassFxState->AttachFxObserver(pState);
			pFxState->FxReleaseInterface();

			FX_HANDLE hObsvId;
			pState->AttachFxObserver(this, (FX_PTR)pFxComponent, &hObsvId);

			Uint16 wPinCount;
			if( FEF_FAILED(hr = pFxComponent->pFxPinManager->GetPinCount(wPinCount)) )
				wPinCount = 0;

            _pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFx  --> Fx(Name: %s, Version: %s, Author: %s, Fx Type: %s, Fx Scope: %s, Pin Number: %d) OK",
                                                                        pFxDescriptor->strName.c_str(),
																		pFxDescriptor->strVersion.c_str(),
                                                                        pFxDescriptor->strAuthor.c_str(),
																		(GetConstToString(FX_TYPE_CONST, pFxDescriptor->FxType).c_str()),
                                                                        (GetConstToString(FX_SCOPE_CONST, pFxDescriptor->FxScope).c_str()),
																		wPinCount);
			pFxComponent->pFxPinManager->InitFxPins(this, pFxComponent);
/*			for(Int16 Idx = 0; Idx < wPinCount; Idx++)
			{
				IFxPin* pFxPin = NULL;
				CFxPin* pFxPinClass = NULL;
				pFxComponent->pFxPinManager->GetPin(&pFxPin, Idx);
				if(pFxPin)
				{
                    pFxPinClass = static_cast<CFxPin*>(pFxPin);
					pFxPinClass->SetFxMember(pFxComponent);
					pFxPinClass->AttachMediaPool(_pFxMediaPool);
					pFxPinClass->AttachObservers(this,(FX_PTR)pFxComponent);
				}
			}
*/
			{
                AutoLock lock(_CSObserver);
				CFxStateVectorIter Iter;
				/*! Get the Fx state  if address already exists, because pState is not removed in ReleaseFx */
				Bool IsFound = FALSE;
				CState* pExistingState;
				for (Iter = _FxStateVector.begin(); (Iter != _FxStateVector.end()) && !(_FxStateVector.empty()); Iter++)
				{
					if( (*Iter)->GetFxptr() == (FX_PTR)pFxComponent )
					{
						pExistingState = (*Iter);
						IsFound = TRUE;
						break;
					}
				}
				if(IsFound == TRUE)
				{
					_FxStateVector.erase(Iter);
					SAFE_DELETE_OBJECT(pExistingState);
				}
				_FxStateVector.push_back(pState);
                lock.unlock();
				_FxVector.push_back(pFxComponent);
				/*! End of interfaces */

				*phFx = pFxComponent;
				_pLogTrace->FxTrace(  "CFxEngineCore::AddFx Quit");
				return FX_OK;
			}
		}
	}
	else
	{
		hr = FX_INVALPARAM;
		strError = "\tFxEngine::FEF_AddFx FEF_FAILED !!, hr=%d  (Invalid Fx !!)";
	}

Error:
	if(pFxComponent)
	{
		if(pFxComponent->pFxBase)
			if(pFxComponent->pIFx)
				pFxComponent->pFxBase->ReleaseFx();
		if(pFxComponent->hDLL)
			FreeFxLibrary(pFxComponent->hDLL);
		if(pFxComponent->IsTemporaryFx == TRUE)
			remove(strFxPath.c_str());

        /*! Remove Fx handle */
        _SetFxHandlePtr.erase(strFxPath);

	}

	SAFE_DELETE_OBJECT(pState);					/*!< Delete the Fx state object */
	SAFE_DELETE_OBJECT(pFxComponent);			/*!< Delete the Fx object */

	*phFx = NULL;

	Char str[MAX_PATH];
#ifdef WIN32
	sprintf_s(str, MAX_PATH, strError.c_str());
#else
	sprintf(str, strError.c_str());
#endif
	_pLogTrace->FxTrace( str);
	return hr;
}

Int32 CFxEngineCore::AddFxEx(IFxBase* pIFxBase, FX_HANDLE* phFx)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::AddFxEx entry");

	const FX_DESCRIPTOR* pFxDescriptor = NULL;

	std::string strError;

	if(pIFxBase == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFxEx FEF_FAILED !! (Invalid Parameter: pIFxBase)");
		return FX_INVALPARAM;
	}

	if(phFx == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFxEx FEF_FAILED !! (Invalid Parameter: phFx)");
		return FX_INVALPARAM;
	}
	*phFx = NULL;

	Int32 hr = FX_OK;

	FX_COMPONENT* pFxComponent = NULL;
	pFxComponent = new FX_COMPONENT;
	if(pFxComponent == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFxEx FEF_FAILED !! (No free memory)");
		return FX_NOMEM;
	}

	CState* pState = NULL;
	pState = new CState((FX_PTR)pFxComponent);
	if(pState == NULL)
	{
		strError = "\tFxEngine::FEF_AddFxEx FEF_FAILED !! (IFxState FEF_FAILED, No free memory !!)";
		goto Error;
	}

	pState->PublishFxState(FX_LOADING_STATE);

	pFxComponent->hDLL			= (FX_PTR)NULL;
	pFxComponent->strFxPath		= "";
	pFxComponent->strFxPathOnDisk = "";
	pFxComponent->IsTemporaryFx = FALSE;
	pFxComponent->pFxBase		= pIFxBase;
	pFxComponent->pIFx			= NULL;
	pFxComponent->PinRunState   = PIN_STOP;
	pFxComponent->pFxParam		= NULL;
	pFxComponent->pFxPinManager	= NULL;

    hr = pFxComponent->pFxBase->InitFx(&pFxComponent->pIFx);
	if(FEF_FAILED(hr))
	{
		strError = "\tFxEngine::FEF_AddFxEx FEF_FAILED !!, hr=%d  (IFxBase::Init FEF_FAILED !!)";
		goto Error;
	}

	{
        hr = pFxComponent->pFxBase->GetFxInfo(&pFxDescriptor);
        if((!pFxDescriptor) || FEF_FAILED(hr))
        {
            strError = "\tFxEngine::FEF_AddFxEx FEF_FAILED !!, hr=%d (IFxBase::GetFxInfo FEF_FAILED !!)";
            goto Error;
        }
        pFxComponent->strFxName = pFxDescriptor->strName;
    }

	/*!------- IFX_PINMANGER  -------*/
	{
	    IFxPinManager* pFxPinManager;
        hr = pFxComponent->pIFx->FxGetInterface(IFX_PINMANGER, (Void**)&pFxPinManager);
        if(FEF_FAILED(hr))
        {
            strError = "\tFxEngine::FEF_AddFxEx FEF_FAILED !! (IFx::FxGetInterface IFX_PINMANGER FEF_FAILED !!)";
            goto Error;
        }
        pFxComponent->pFxPinManager = static_cast<CFxPinManager*>(pFxPinManager);
        pFxComponent->pFxPinManager->FxReleaseInterface();
	}


	/*!------- IFX_PARAM  -------*/
	{
	    IFxParam* pFxParam;
        hr = pFxComponent->pIFx->FxGetInterface(IFX_PARAM, (Void**)&pFxParam);
        if(FEF_FAILED(hr))
        {
            strError = "\tFxEngine::FEF_AddFxEx FEF_FAILED !! (IFx::FxGetInterface IFX_PARAM FEF_FAILED !!)";
            goto Error;
        }
        pFxComponent->pFxParam = static_cast<CFxParam*>(pFxParam);
        pFxComponent->pFxParam->FxReleaseInterface();
	}


	/*!------- IFX_REFCLOCK  -------*/
	{
	    IFxRefClock* pFxRefClock;
        hr = pFxComponent->pIFx->FxGetInterface(IFX_REFCLOCK, (Void**)&pFxRefClock);
        if(FEF_FAILED(hr))
        {
            strError = "\tFxEngine::FEF_AddFxEx FEF_FAILED !! (IFx::FxGetInterface IFX_RefClock FEF_FAILED !!)";
            goto Error;
        }
        CFxRefClock* pClassRefClock = static_cast<CFxRefClock*>(pFxRefClock);
        pClassRefClock->AttachObserver(this, (FX_PTR)pFxComponent);
        //pClassRefClock->GetAttach(&pFxComponent->pIFxRefClockCallBack);
        pFxRefClock->FxReleaseInterface();
	}

	/*!------- IFX_STATE  -------*/
	{
	    IFxState* pFxState;
        hr = pFxComponent->pIFx->FxGetInterface(IFX_STATE, (Void**)&pFxState);
        if(FEF_FAILED(hr))
        {
            strError = "\tFxEngine::FEF_AddFx FEF_FAILED !! (IFx::FxGetInterface IFX_State FEF_FAILED !!)";
            goto Error;
        }
        CFxState* pClassFxState = static_cast<CFxState*>(pFxState);
        pClassFxState->AttachFxObserver(pState);
        pFxState->FxReleaseInterface();

        FX_HANDLE hObsvId;
        pState->AttachFxObserver(this, (FX_PTR)pFxComponent, &hObsvId);

        Uint16 wPinCount;
        if( FEF_FAILED(hr = pFxComponent->pFxPinManager->GetPinCount(wPinCount)) )
            wPinCount = 0;

        _pLogTrace->FxTrace(  "\tFxEngine::FEF_AddFxEx  --> Fx(Name: %s, Version: %s, Author: %s, Fx Type: %s, Fx Scope: %s, Pin Number: %d) OK",
                                                                pFxDescriptor->strName.c_str(),
                                                                pFxDescriptor->strVersion.c_str(),
                                                                pFxDescriptor->strAuthor.c_str(),
																(GetConstToString(FX_TYPE_CONST, pFxDescriptor->FxType).c_str()),
                                                                (GetConstToString(FX_SCOPE_CONST, pFxDescriptor->FxScope).c_str()),
																wPinCount);
        pFxComponent->pFxPinManager->InitFxPins(this, pFxComponent);
    }
	/*for(Int16 Idx = 0; Idx < wPinCount; Idx++)
	{
		IFxPin* pFxPin = NULL;
		CFxPin* pFxPinClass = NULL;
		pFxComponent->pFxPinManager->GetPin(&pFxPin, Idx);
		if(pFxPin)
		{
            pFxPinClass = static_cast<CFxPin*>(pFxPin);
			pFxPinClass->SetFxMember(pFxComponent);
			pFxPinClass->AttachMediaPool(_pFxMediaPool);
			pFxPinClass->AttachObservers(this,(FX_PTR)pFxComponent);
		}
	}*/
	{
        AutoLock lock(_CSObserver);
		CFxStateVectorIter Iter;
		/*! Get the Fx state  if address already exists */
		Bool IsFound = FALSE;
		CState* pExistingState;
		for (Iter = _FxStateVector.begin(); (Iter != _FxStateVector.end()) && !(_FxStateVector.empty()); Iter++)
		{
			if( (*Iter)->GetFxptr() == (FX_PTR)pFxComponent )
			{
				pExistingState = (*Iter);
				IsFound = TRUE;
				break;
			}
		}
		if(IsFound == TRUE)
		{
			_FxStateVector.erase(Iter);
			SAFE_DELETE_OBJECT(pExistingState);
		}

		_FxStateVector.push_back(pState);
        lock.unlock();

		_FxVector.push_back(pFxComponent);

		*phFx = pFxComponent;
		_pLogTrace->FxTrace(  "CFxEngineCore::AddFxEx quit");
		//pFxComponent->pIFx->FxPublishState(FX_INIT_STATE);
		return FX_OK;
	}

Error:
	if(pFxComponent)
	{
		if(pFxComponent->pFxBase)
			if(pFxComponent->pIFx)
				pFxComponent->pFxBase->ReleaseFx();
	}

	SAFE_DELETE_OBJECT(pState);					/*!< Delete the Fx state object */
	SAFE_DELETE_OBJECT(pFxComponent);			/*!< Delete the Fx object */

	*phFx = NULL;

	_pLogTrace->FxTrace(  (Char*)strError.c_str());
	return hr;
}

Int32 CFxEngineCore::RemoveFx(FX_HANDLE hFx)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::RemoveFx entry");

	Int32 hr = FX_OK;
	
	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);

		if ( FEF_FAILED(hr = StopFx(hFx, FALSE)) )
		{
			goto Fx_Error;
		}

		_pLogTrace->FxTrace("\tCFxEngineCore::RemoveFx %s", pFx->strFxName.c_str());

		//Uint16 wPinCount = 0;
		//if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) )
		//    wPinCount = 0;
  //      do
  //      {
  //          IFxPin* pFxPin = NULL;
		//	//CFxPin* pFxPinClass = NULL;
  //          pFx->pFxPinManager->GetPin(&pFxPin, 0);
		//	if(pFxPin)
  //              pFx->pFxPinManager->Remove(pFxPin);

  //          if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) )
		//	    wPinCount = 0;
  //      }while(wPinCount > 0);

		if ( FEF_FAILED(hr = pFx->pFxBase->ReleaseFx()) )
		{
			_pLogTrace->FxTrace(  "\tIFxBase::ReleaseFx FEF_FAILED !!, hr=%d (Invalid Parameter: hFx)", hr);
			goto Fx_Error;
		}

Fx_Error:
		if(pFx->hDLL)
			FreeFxLibrary(pFx->hDLL);

		CFxVectorIter Iter;
		/*! Clear fx in list */
		Iter = std::find(_FxVector.begin(), _FxVector.end(), (FX_COMPONENT*)hFx);
		if( Iter != _FxVector.end() )
		{
            /*! Remove Fx handle */
            _SetFxHandlePtr.erase(pFx->strFxPath);

            if(pFx->IsTemporaryFx == TRUE)
				remove(pFx->strFxPath.c_str());
			_FxVector.erase(Iter);
			SAFE_DELETE_OBJECT(pFx);
		}

		/*
		Bool FxFound = FALSE;
				for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
				{
					if((*Iter) == (FX_COMPONENT*)hFx)
					{
						FxFound = TRUE;
						break;
					}
				}
				if(FxFound == TRUE)
				{
					if(pFx->IsTemporaryFx == TRUE)
						remove(pFx->strFxPath.c_str());
					_FxVector.erase(Iter);
					SAFE_DELETE_OBJECT(pFx);			/ *!< Delete the Pin object * /
				}*/

		_pLogTrace->FxTrace(  "CFxEngineCore::RemoveFx quit");
		return hr;
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::RemoveFx FEF_FAILED !! (Invalid Parameter: hFx)");
		_pLogTrace->FxTrace(  "CFxEngineCore::RemoveFx quit");
		return FX_INVALPARAM;
	}
}

Int32 CFxEngineCore::GetFxCount(Uint16* pwFxCount, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);
	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxCount entry");

	if(pwFxCount == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxCount FEF_FAILED !! (Invalid Parameter: pwFxCount)");
		return FX_INVALPARAM;
	}

	*pwFxCount = (Uint16)_FxVector.size();

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxCount quit");
	return FX_OK;
}

Int32 CFxEngineCore::GetFx(FX_HANDLE* phFx, Uint16 wFxIndex, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);
	_pLogTrace->FxTrace(  "CFxEngineCore::GetFx entry");

	if(phFx == NULL)
	{
		_pLogTrace->FxTrace(  "FxEngine::GetFx FEF_FAILED !! (Invalid Parameter: phFx)");
		return FX_INVALPARAM;
	}

	if((Uint16)_FxVector.empty() == TRUE)
	{
		*phFx = NULL;
		_pLogTrace->FxTrace(  "CFxEngineCore::GetFx quit");
		return FX_OK;
	}

	if( wFxIndex > ((Uint16)_FxVector.size() - 1) )
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFx FEF_FAILED !! (Invalid Parameter: wFxIndex too big)");
		return FX_INVALPARAM;
	}

	CFxVectorIter Iter = _FxVector.begin() + wFxIndex;
	*phFx = static_cast<FX_HANDLE>((*Iter));

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFx quit");
	return FX_OK;
}

Int32 CFxEngineCore::GetFxNumber(FX_HANDLE hFx, Uint16* pwFxNumber, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);
	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxNumber entry");

	if(pwFxNumber == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxNumber FEF_FAILED !! (Invalid Parameter: pwFxNumber)");
		return FX_INVALPARAM;
	}

	if((Uint16)_FxVector.empty() == TRUE)
	{
		_pLogTrace->FxTrace(  "CFxEngineCore::GetFxNumber quit");
		return FX_OK;
	}

	/*! Find address of Fx handle created in vector */
	CFxVectorIter Iter;
	Iter = std::find(_FxVector.begin(), _FxVector.end(), hFx);
	if( Iter != _FxVector.end() )
	{
		*pwFxNumber = (Uint16)distance(_FxVector.begin(), Iter); /*!< Copy the Fx Number */
		_pLogTrace->FxTrace(  "CFxEngineCore::GetFxNumber quit");
		return FX_OK;
	}
	/*
	Uint16 Idx;
	for (Iter = _FxVector.begin(), Idx = 0; (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++, Idx++)
		{
			if((*Iter) == hFx)
			{
				*pwFxNumber = Idx;			/ *!< Copy the Fx Number * /
				return FX_OK;
			}
		}*/

	_pLogTrace->FxTrace( "\tFxPinManager::GetFxNumber FEF_FAILED !! (Invalid Parameters: pIFxPin)");
	return FX_ERROR;
}

Int32 CFxEngineCore::StartFx(FX_HANDLE hFx, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);

	_pLogTrace->FxTrace(  "CFxEngineCore::StartFx entry");

	Int32 hr = FX_OK;

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);

		_pLogTrace->FxTrace("\tCFxEngineCore::StartFx %s", pFx->strFxName.c_str());

		Uint16 wPinCount;
		if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StartFx FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		IFxPin* pFxPin = NULL;
		CFxPin* pFxPinClass = NULL;
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			pFx->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->SetPinRunState(PIN_START);
			}
		}

		if( FEF_FAILED(hr = pFx->pFxBase->StartFx()) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::StartFx FEF_FAILED !!, hr=%d (Cannot Start Fx)", hr);
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			return FX_ERROR;
		}

		if( FEF_FAILED(hr = pFx->pFxBase->RunFx()) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::StartFx FEF_FAILED !!, hr=%d (Cannot Run Fx)", hr);
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			return FX_ERROR;
		}

		/*! Save Fx Pins state */
		pFx->PinRunState   = PIN_START;

		//pFx->pIFx->FxPublishState(FX_START_STATE);
		_pLogTrace->FxTrace(  "CFxEngineCore::StartFx quit");
		return FX_OK;
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::StartFx FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}
}

Int32 CFxEngineCore::StartFxEngine()
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::StartFxEngine entry");

	Int32 hr = FX_OK;
	CFxVectorIter Iter;
	FX_COMPONENT* pFxComponent = NULL;
	/*! All Fxs are != of UNDEFINED_STATE_FX or ERROR_STATE_FX */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		/*pFxComponent = (*Iter);
		if( ((*Iter)->FxState != FX_INIT_STATE) && ((*Iter)->FxState != FX_PAUSE_STATE) && ((*Iter)->FxState != FX_STOP_STATE) && ((*Iter)->FxState != FX_START_STATE) )
		{
			_pLogTrace->FxTrace(  "CFxEngineCore::StartFx FEF_FAILED !! (Invalid Fx State)");
			return FX_ERRORSTATE;
		}*/

		Uint16 wPinCount;
		if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StartFxEngine FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		IFxPin* pFxPin = NULL;
		CFxPin* pFxPinClass = NULL;
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			(*Iter)->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->SetPinRunState(PIN_START);
			}
		}
		//(*Iter)->FxState = FX_START_STATE;
	}

	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		if( FEF_FAILED(hr = (*Iter)->pFxBase->StartFx()) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StartFxEngine FEF_FAILED !!, hr=%d (Cannot Start Fx)", hr);
			SetFxState((FX_PTR)(*Iter), FX_ERROR_STATE);
			return FX_ERROR;
		}
	}

	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		if( FEF_FAILED(hr = (*Iter)->pFxBase->RunFx()) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StartFxEngine FEF_FAILED !!, hr=%d (Cannot Run Fx)", hr);
			SetFxState((FX_PTR)(*Iter), FX_ERROR_STATE);
			return FX_ERROR;
		}
		/*! Save Fx Pins state */
		(*Iter)->PinRunState   = PIN_START;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::StartFxEngine quit");
	return FX_OK;
}

Int32 CFxEngineCore::PauseFx(FX_HANDLE hFx, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);

	_pLogTrace->FxTrace(  "CFxEngineCore::PauseFx entry");

	Int32 hr = FX_OK;

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);

		_pLogTrace->FxTrace("\tCFxEngineCore::PauseFx %s", pFx->strFxName.c_str());

		Uint16 wPinCount;
		if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::PauseFx FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			pFx->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->SetPinRunState(PIN_PAUSE);
			}
		}

		/*! Remove infinite time waiting */
        for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			pFx->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->QuitInfiniteTime();
            }
        }

		if( FEF_FAILED(hr = pFx->pFxBase->PauseFx()) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::PauseFx FEF_FAILED !!, hr=%d (Cannot Pause Fx)", hr);
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			return FX_ERROR;
		}

		/*! Save Fx Pins state */
		pFx->PinRunState   = PIN_PAUSE;

		//pFx->FxState = FX_PAUSE_STATE;
		_pLogTrace->FxTrace(  "CFxEngineCore::PauseFx quit");
		return FX_OK;
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::PauseFx FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}
}

Int32 CFxEngineCore::PauseFxEngine()
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::PauseFxEngine entry");

	Int32 hr = FX_OK;
	CFxVectorIter Iter;
	/*! Set pin state to pause */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		Uint16 wPinCount;
		if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::PauseFxEngine FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			(*Iter)->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->SetPinRunState(PIN_PAUSE);
			}
		}
	}
	/*! Stop infinite waiting time */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		Uint16 wPinCount;
		if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::PauseFxEngine FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			(*Iter)->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->QuitInfiniteTime();
			}
		}
	}
    /*! Call Fx Pause */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		if( FEF_FAILED(hr = (*Iter)->pFxBase->PauseFx()) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::PauseFxEngine FEF_FAILED !!, hr=%d (Cannot Pause Fx)", hr);
			SetFxState((FX_PTR)(*Iter), FX_ERROR_STATE);
			return FX_ERROR;
		}

		/*! Save Fx Pins state */
		(*Iter)->PinRunState   = PIN_PAUSE;

		//(*Iter)->FxState = FX_PAUSE_STATE;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::PauseFxEngine quit");
	return FX_OK;
}

Int32 CFxEngineCore::StopFx(FX_HANDLE hFx, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);

	_pLogTrace->FxTrace(  "CFxEngineCore::StopFx entry");

	Int32 hr = FX_OK;

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);

		_pLogTrace->FxTrace("\tCFxEngineCore::StopFx %s", pFx->strFxName.c_str());

		Uint16 wPinCount;
		if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StopFx FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			pFx->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->SetPinRunState(PIN_STOP);
			}
		}
        
		/*! Remove infinite time waiting */
        for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			pFx->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->QuitInfiniteTime();
            }
        }

        /*! Wait for the end of pin data */
        for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			pFx->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
                pFxPinClass->WaitForEndDataInQueue(); /*< Stop pin callbacks when we remove the FX */
            }
        }

		if( FEF_FAILED(hr = pFx->pFxBase->StopFx()) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StopFx FEF_FAILED !!, hr=%d (Cannot Stop Fx)", hr);
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			return FX_ERROR;
		}

		/*! Save Fx Pins state */
		pFx->PinRunState   = PIN_STOP;

		//pFx->FxState = FX_STOP_STATE;
		_pLogTrace->FxTrace(  "CFxEngineCore::StopFx quit");
		return FX_OK;
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::StopFx FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}
}

Int32 CFxEngineCore::StopFxEngine()
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::StopFxEngine entry");

	Int32 hr = FX_OK;
	CFxVectorIter Iter;
	/*! Stop pin first */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		Uint16 wPinCount;
		if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StopFxEngine FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			(*Iter)->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->SetPinRunState(PIN_STOP);
			}
		}
	}
	/*! Stop infinite waiting time */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		Uint16 wPinCount;
		if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StopFxEngine FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			(*Iter)->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->QuitInfiniteTime();
			}
		}
	}
	/*! Stop Fx */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		Uint16 wPinCount;
		if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StopFxEngine FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
        /*! Wait for the end of pin data */
        for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			(*Iter)->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin)
			{
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
                pFxPinClass->WaitForEndDataInQueue(); /*< Stop pin callbacks when we remove the FX */
            }
        }

		if( FEF_FAILED(hr = (*Iter)->pFxBase->StopFx()) )
		{
			_pLogTrace->FxTrace(  "\tCFxEngineCore::StopFxEngine FEF_FAILED !!, hr=%d (Cannot Stop Fx)", hr);
			SetFxState((FX_PTR)(*Iter), FX_ERROR_STATE);
			return FX_ERROR;
		}

		/*! Save Fx Pins state */
		(*Iter)->PinRunState   = PIN_STOP;

		//(*Iter)->FxState = FX_STOP_STATE;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::StopFxEngine quit");
	return FX_OK;
}

Int32 CFxEngineCore::GetFxInfo(FX_HANDLE hFx, const FX_DESCRIPTOR** ppFxDescriptor)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxInfo entry");

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);

		if(ppFxDescriptor == NULL)
		{
			_pLogTrace->FxTrace(  "\tFxEngine::GetFxInfo FEF_FAILED !! (Invalid Parameter: ppFxDescriptor)");
			return FX_INVALPARAM;
		}
		_pLogTrace->FxTrace(  "CFxEngineCore::GetFxInfo quit");

        return (pFx->pFxBase->GetFxInfo(ppFxDescriptor));
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxInfo FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}
}

Void CFxEngineCore::SetFxState(FX_PTR dwFx, FX_STATE FxState)
{
    AutoLock lock(_CSObserver);
	_pLogTrace->FxTrace(  "CFxEngineCore::SetFxState entry");

	CFxStateVectorIter Iter;
	/*! Get the Fx state */
	for (Iter = _FxStateVector.begin(); (Iter != _FxStateVector.end()) && !(_FxStateVector.empty()); Iter++)
	{
		if( (*Iter)->GetFxptr() == dwFx )
		{
			(*Iter)->PublishFxState(FxState);
			break;
		}
	}
	_pLogTrace->FxTrace(  "CFxEngineCore::SetFxState quit");
	return;
}

Int32 CFxEngineCore::GetFxState(FX_HANDLE hFx, FX_STATE* pFxState, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CSObserver, ShouldProtect);
    
	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxState entry");


	if(pFxState == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxState FEF_FAILED !! (Invalid Parameter: pFxState)");
		return FX_INVALPARAM;
	}

	Bool IsFound = FALSE;

	CFxStateVectorIter Iter;
	/*! Get the Fx state */
	for (Iter = _FxStateVector.begin(); (Iter != _FxStateVector.end()) && !(_FxStateVector.empty()); Iter++)
	{
		if( (*Iter)->GetFxptr() == (FX_PTR)hFx )
		{
			IsFound = TRUE;
			*pFxState = (*Iter)->GetFxState();
			break;
		}
	}

	if(IsFound == FALSE)
	{
		*pFxState = FX_UNDEFINED_STATE;
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxState FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxState quit");

	return FX_OK;
}

Int32 CFxEngineCore::AttachFxObserver(FX_HANDLE hFx, CFxStateCallback* pFxStateCallback, FX_PTR dwParam, FX_HANDLE* phObserverId)
{
	AutoLock lock(_CSObserver);
	_pLogTrace->FxTrace(  "CFxEngineCore::AttachFxObserver entry");

	Int32 hr = FX_OK;

	if(pFxStateCallback == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserver FEF_FAILED !! (Invalid Parameter: pFxStateCallback)");
		return FX_INVALPARAM;
	}

	if(phObserverId == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserver FEF_FAILED !! (Invalid Parameter: phObserverId)");
		return FX_INVALPARAM;
	}

	FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);

	_pLogTrace->FxTrace("\tCFxEngineCore::AttachFxObserver %s", pFx->strFxName.c_str());

	CFxStateVectorIter Iter;
	/*! Get the Fx state */
	for (Iter = _FxStateVector.begin(); (Iter != _FxStateVector.end()) && !(_FxStateVector.empty()); Iter++)
	{
		if( (*Iter)->GetFxptr() == (FX_PTR)pFx )
		{
			if(FEF_FAILED( hr = (*Iter)->AttachFxObserver(pFxStateCallback, dwParam, phObserverId)) )
			{
				_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserver FEF_FAILED !!, hr=%d", hr);
				return hr;
			}
			_FxStateMap.insert(make_pair(*phObserverId, (*Iter)));
			_pLogTrace->FxTrace(  "CFxEngineCore::AttachFxObserver quit");
			return FX_OK;
		}
	}
	_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserver FEF_FAILED !! (Invalid Parameter: hFx)");
	return FX_INVALPARAM;
}

Int32 CFxEngineCore::AttachFxObserverEx(FX_HANDLE hFx, FXSTATECALLBACK* pFxStateCallback, FX_PTR dwParam, FX_HANDLE* phObserverId)
{
	AutoLock lock(_CSObserver);
	_pLogTrace->FxTrace(  "CFxEngineCore::AttachFxObserverEx entry");

	Int32 hr = FX_OK;

	if(pFxStateCallback == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserverEx FEF_FAILED !! (Invalid Parameter: pFxStateCallback)");
		return FX_INVALPARAM;
	}

	if(phObserverId == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserverEx FEF_FAILED !! (Invalid Parameter: phObserverId)");
		return FX_INVALPARAM;
	}

	FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);

	_pLogTrace->FxTrace("\tCFxEngineCore::AttachFxObserverEx %s", pFx->strFxName.c_str());

	CFxStateVectorIter Iter;
	/*! Get the Fx state */
	for (Iter = _FxStateVector.begin(); (Iter != _FxStateVector.end()) && !(_FxStateVector.empty()); Iter++)
	{
		if( (*Iter)->GetFxptr() == (FX_PTR)pFx )
		{
			if(FEF_FAILED( hr = (*Iter)->AttachFxObserverEx(pFxStateCallback, dwParam, phObserverId)) )
			{
				_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserverEx FEF_FAILED !!, hr=%d", hr);
				return hr;
			}
			_FxStateMap.insert(make_pair(*phObserverId, (*Iter)));
			_pLogTrace->FxTrace(  "CFxEngineCore::AttachFxObserverEx quit");
			return FX_OK;
		}
	}
	_pLogTrace->FxTrace(  "\tFxEngine::AttachFxObserverEx FEF_FAILED !! (Invalid Parameter: hFx)");
	return FX_INVALPARAM;
}

Int32 CFxEngineCore::DetachFxObserver(FX_HANDLE hObserverId)
{
	AutoLock lock(_CSObserver);
	_pLogTrace->FxTrace(  "CFxEngineCore::DetachFxObserver entry");

	Int32 hr = FX_OK;

	FxStateMap::iterator it;
	it = _FxStateMap.find( hObserverId );
    if( it == _FxStateMap.end() )
    {
        _pLogTrace->FxTrace(  "\tFxEngine::DetachFxObserver FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
    }
    else
	{
		if(FEF_FAILED( hr = it->second->DetachFxObserver(hObserverId)) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::DetachFxObserver FEF_FAILED !!, hr=%d", hr);
			return hr;
		}

	}
	_FxStateMap.erase(hObserverId);
	_pLogTrace->FxTrace(  "CFxEngineCore::DetachFxObserver quit");
	return FX_OK;
}

Int32 CFxEngineCore::GetFxPinCount(FX_HANDLE hFx, Uint16* pwPinCount, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);
	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxPinCount entry");

	Int32 hr = FX_OK;

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
		if(pwPinCount == NULL)
		{
			_pLogTrace->FxTrace(  "\tFxEngine::GetFxPinCount FEF_FAILED !! (Invalid Parameter: pwPinCount)");
			return FX_INVALPARAM;
		}
		*pwPinCount = 0;

		Uint16 wPinCount;
		if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::GetFxPinCount FEF_FAILED !! (Cannot get Pin count)");
			return hr;
		}
		*pwPinCount = wPinCount;
		_pLogTrace->FxTrace(  "CFxEngineCore::GetFxPinCount quit");
		return FX_OK;
	}
	else
	{
		if(pwPinCount != NULL)
			*pwPinCount = 0;

		_pLogTrace->FxTrace(  "\tFxEngine::GetFxPinCount FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}
}

Int32 CFxEngineCore::QueryFxPinInterface(FX_HANDLE hFx, IFxPin** ppIFxPin, Uint16 wPinIndex)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::QueryFxPinInterface entry");

	if(ppIFxPin == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::QueryFxPinInterface FEF_FAILED !! (Invalid Parameter: ppIFxPin)");
		return FX_INVALPARAM;
	}

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
		_pLogTrace->FxTrace("\tCFxEngineCore::QueryFxPinInterface %s", pFx->strFxName.c_str());
		if(ppIFxPin == NULL)
		{
			_pLogTrace->FxTrace(  "\tFxEngine::QueryFxPinInterface FEF_FAILED !! (Invalid Parameter: ppIFxPin)");
			return FX_INVALPARAM;
		}
		_pLogTrace->FxTrace(  "CFxEngineCore::QueryFxPinInterface quit");
		return (pFx->pFxPinManager->GetPin(ppIFxPin, wPinIndex));
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::QueryFxPinInterface FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}
}

Int32 CFxEngineCore::QueryFxParamInterface(FX_HANDLE hFx, IFxParam** ppIFxParam)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace(  "CFxEngineCore::QueryFxParamInterface entry");

	if(ppIFxParam == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::QueryFxParamInterface FEF_FAILED !! (Invalid Parameter: ppIFxParam)");
		return FX_INVALPARAM;
	}

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
		_pLogTrace->FxTrace("\tCFxEngineCore::QueryFxParamInterface %s", pFx->strFxName.c_str());
		if(ppIFxParam == NULL)
		{
			_pLogTrace->FxTrace(  "\tFxEngine::QueryFxParamInterface FEF_FAILED !! (Invalid Parameter: ppIFxParam)");
			return FX_INVALPARAM;
		}
		*ppIFxParam = static_cast<IFxParam*>(pFx->pFxParam);
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::QueryFxParamInterface FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::QueryFxParamInterface quit");
	return FX_OK;
}

Int32 CFxEngineCore::UpdateFxParam(FX_HANDLE hFx, const std::string strParamName, FX_PARAMETER FxParameter, Bool ShouldProtect/* = TRUE*/)
{
    AutoLock lock(_CS, ShouldProtect);

	Int32 hr = FX_OK;
	_pLogTrace->FxTrace(  "CFxEngineCore::UpdateFxParam entry");

    if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
        _pLogTrace->FxTrace("\tCFxEngineCore::UpdateFxParam %s (%s-%ld)", pFx->strFxName.c_str(), (!strParamName.empty()) ? strParamName.c_str() : "Null", FxParameter);
		if( FEF_FAILED( hr = pFx->pFxBase->UpdateFxParam(strParamName, FxParameter)) )
		{
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			_pLogTrace->FxTrace(  "\tFxEngine::UpdateFxParam FEF_FAILED !!, hr=%d (Cannot Get Fx Frame)", hr);
			return hr;
		}
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::UpdateFxParam FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::UpdateFxParam quit");
	return FX_OK;
}

Int32 CFxEngineCore::ConnectFxPin(IFxPin* pFxPinIn, IFxPin* pFxPinOut, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);
	_pLogTrace->FxTrace(  "CFxEngineCore::ConnectFxPin entry");

	Int32 hr = FX_OK;

	if(pFxPinOut == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (Invalid Parameter: pFxPinOut)");
		return FX_INVALPARAM;
	}

	if(pFxPinIn == NULL)
	{
		_pLogTrace->FxTrace(  "FxEngine::ConnectFxPin FEF_FAILED !! (Invalid Parameter: pFxPinIn)");
		return FX_INVALPARAM;
	}

	/*! Get MediaType Count for each Pin*/
	CFxPin* pFxPinInClass	= NULL;
	CFxPin* pFxPinOutClass	= NULL;

	pFxPinInClass	= static_cast<CFxPin*>(pFxPinIn);
    pFxPinOutClass	= static_cast<CFxPin*>(pFxPinOut);

	/*! Get Fx handles */
	FX_COMPONENT* pFxIn = static_cast<FX_COMPONENT*>(pFxPinInClass->GetFxMember());
	FX_COMPONENT* pFxOut = static_cast<FX_COMPONENT*>(pFxPinOutClass->GetFxMember());

    std::string strPinInName;
	std::string strPinOutName;
	pFxPinInClass->GetPinName(strPinInName);
	pFxPinOutClass->GetPinName(strPinOutName);

    _pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin\n\t\t\t\t%s(%s)\n\t\t\t\t%s(%s)", pFxOut->strFxName.c_str(), strPinOutName.c_str(),
															pFxIn->strFxName.c_str(), strPinInName.c_str());

	if( pFxPinIn == pFxPinOut )
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (Invalid Parameter, must be !=)");
		return FX_INVALPARAM;
	}

	FX_PIN_STATE pFxPinState;
	pFxPinOutClass->GetPinState(&pFxPinState);
	if(pFxPinState == PIN_CONNECTED)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (Pin Out is already connected)");
		return FX_INVALPARAM;
	}

    pFxPinInClass->GetPinState(&pFxPinState);
	if(pFxPinState == PIN_CONNECTED)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (Pin In is already connected)");
		return FX_INVALPARAM;
	}

	Uint16 wMediaTypeCountPinOut = 0;
	Uint16 wMediaTypeCountPinIn = 0;

	if( FEF_FAILED(hr = pFxPinInClass->GetMediaTypeCount(&wMediaTypeCountPinIn)) )
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (IFxPin Cannot get InPin MediaType Count !!)");
		return hr;
	}

	if( FEF_FAILED(hr = pFxPinOutClass->GetMediaTypeCount(&wMediaTypeCountPinOut)) )
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (IFxPin Cannot get OutPin MediaType Count !!)");
		return hr;
	}
    Bool MediaTypeMatch = FALSE;
	/*! For each out pin media type, compare with in pin media type*/
	for(Int16 IdxMediaOut = 0; IdxMediaOut < wMediaTypeCountPinOut; IdxMediaOut++)
	{
		/*! Get out media type*/
		FX_MEDIA_TYPE MediaTypeOut;
		pFxPinOutClass->GetMediaType(&MediaTypeOut, IdxMediaOut);
		if( pFxPinInClass->CheckMediaType(&MediaTypeOut) == FX_OK)
		{
		    MediaTypeMatch = TRUE;
			if( FEF_FAILED(hr = pFxPinOutClass->SetConnectionMediaType(&MediaTypeOut)) )
			{
				_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (IFxPin Cannot set OutPin MediaType !!)");
				return hr;
			}
			if( FEF_FAILED(hr = pFxPinInClass->SetConnectionMediaType(&MediaTypeOut)) )
			{
				_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (IFxPin Cannot set OutPin MediaType !!)");
				return hr;
			}

			if( FEF_FAILED(hr = pFxPinOutClass->ConnectPin(pFxPinIn)) )
			{
				_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (Can't connect PinIn !!)");
				return hr;
			}
			/*! Set Pin state according to the Fx state */
			pFxPinOutClass->SetPinRunState(pFxOut->PinRunState);

            if( FEF_FAILED(hr = pFxPinInClass->ConnectPin(pFxPinOut)) )
			{
				_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (Can't connect PinOut !!)");
				return hr;
			}
			/*! Set Pin state according to the Fx state */
			pFxPinInClass->SetPinRunState(pFxIn->PinRunState);

			pFxPinInClass->SendFxPinState(PIN_CONNECTED);
			pFxPinOutClass->SendFxPinState(PIN_CONNECTED);
			break;
		}
	}

    if(MediaTypeMatch == FALSE)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (No MediaType Match!!)");
		return FX_MEDIANOTSUPPORTED;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::ConnectFxPin quit");
	return FX_OK;
}

Int32 CFxEngineCore::ConnectFxPinEx(IFxPin* pFxPinIn, IFxPin* pFxPinOut, PFX_MEDIA_TYPE pMediaType, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_CS, ShouldProtect);
	_pLogTrace->FxTrace(  "CFxEngineCore::ConnectFxPinEx entry");

	Int32 hr = FX_OK;

	if(pFxPinOut == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (Invalid Parameter: pFxPinOut)");
		return FX_INVALPARAM;
	}

	if(pFxPinIn == NULL)
	{
		_pLogTrace->FxTrace(  "FxEngine::ConnectFxPinEx FEF_FAILED !! (Invalid Parameter: pFxPinIn)");
		return FX_INVALPARAM;
	}

	if(pMediaType == NULL)
	{
		_pLogTrace->FxTrace(  "FxEngine::ConnectFxPinEx FEF_FAILED !! (Invalid Parameter: pMediaType)");
		return FX_INVALPARAM;
	}

	/*! Get MediaType Count for each Pin*/
	CFxPin* pFxPinInClass	= NULL;
	CFxPin* pFxPinOutClass	= NULL;

	pFxPinInClass	= static_cast<CFxPin*>(pFxPinIn);
    pFxPinOutClass	= static_cast<CFxPin*>(pFxPinOut);

	/*! Get Fx handles */
	FX_COMPONENT* pFxIn = static_cast<FX_COMPONENT*>(pFxPinInClass->GetFxMember());
	FX_COMPONENT* pFxOut = static_cast<FX_COMPONENT*>(pFxPinOutClass->GetFxMember());

	std::string  strPinInName;
	std::string  strPinOutName;
	pFxPinInClass->GetPinName(strPinInName);
	pFxPinOutClass->GetPinName(strPinOutName);

	_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx\n\t\t\t\t%s(%s)\n\t\t\t\t%s(%s)\n\t\t\t\tMain: %d, Sub: %d", pFxOut->strFxName.c_str(), strPinOutName.c_str(),
															pFxIn->strFxName.c_str(), strPinInName.c_str(),
															pMediaType->MainMediaType, pMediaType->SubMediaType);

	if( pFxPinIn == pFxPinOut )
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (Invalid Parameter, must be !=)");
		return FX_INVALPARAM;
	}

	FX_PIN_STATE pFxPinState;
	pFxPinOutClass->GetPinState(&pFxPinState);
	if(pFxPinState == PIN_CONNECTED)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (Pin Out is already connected)");
		return FX_INVALPARAM;
	}

	pFxPinInClass->GetPinState(&pFxPinState);
	if(pFxPinState == PIN_CONNECTED)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (Pin In is already connected)");
		return FX_INVALPARAM;
	}

	if( (pFxPinInClass->CheckMediaType(pMediaType) == FX_OK) &&
		(pFxPinOutClass->CheckMediaType(pMediaType) == FX_OK) )
	{
		if( FEF_FAILED(hr = pFxPinOutClass->SetConnectionMediaType(pMediaType)) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (IFxPin Cannot set OutPin MediaType !!)");
			return hr;
		}
		if( FEF_FAILED(hr = pFxPinInClass->SetConnectionMediaType(pMediaType)) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (IFxPin Cannot set OutPin MediaType !!)");
			return hr;
		}
		if( FEF_FAILED(hr = pFxPinOutClass->ConnectPin(pFxPinIn)) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (Can't connect PinIn !!)");
			return hr;
		}
		/*! Set Pin state according to the Fx state */
		pFxPinOutClass->SetPinRunState(pFxOut->PinRunState);

		if( FEF_FAILED(hr = pFxPinInClass->ConnectPin(pFxPinOut)) )
		{
			_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPinEx FEF_FAILED !! (Can't connect PinOut !!)");
			return hr;
		}
		/*! Set Pin state according to the Fx state */
		pFxPinInClass->SetPinRunState(pFxIn->PinRunState);

		pFxPinInClass->SendFxPinState(PIN_CONNECTED);
		pFxPinOutClass->SendFxPinState(PIN_CONNECTED);
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::ConnectFxPin FEF_FAILED !! (No MediaType Match!!)");
		return FX_MEDIANOTSUPPORTED;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::ConnectFxPin quit");
	return FX_OK;
}

Int32 CFxEngineCore::DisconnectAllFxPin(Bool ShouldStopCallBack /*= TRUE*/, Bool ShouldProtect/* = TRUE*/)
{
	Int32 hr = FX_OK;

	_pLogTrace->FxTrace(  "CFxEngineCore::DisconnectAllFxPin entry");

	AutoLock lock(_CS, ShouldProtect);

	CFxVectorIter Iter;
	/*! Clear Pin created in list */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		/*if( ((*Iter)->FxState != FX_INIT_STATE) && ((*Iter)->FxState != FX_PAUSE_STATE) && ((*Iter)->FxState != FX_STOP_STATE) && ((*Iter)->FxState != FX_START_STATE) )
		{
			_pLogTrace->FxTrace(  "CFxEngineCore::StopFx FEF_FAILED !! (Invalid Fx State)");
			return FX_ERRORSTATE;
		}*/

        Uint16 wPinCount = 0;
	    if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
		    wPinCount = 0;
        do
        {
            IFxPin* pFxPin = NULL;
			//CFxPin* pFxPinClass = NULL;
            (*Iter)->pFxPinManager->GetPin(&pFxPin, 0);
            if(pFxPin) {
                if( FEF_FAILED(hr = DisConnectFxPin(pFxPin, ShouldStopCallBack, FALSE) ))
				{
					_pLogTrace->FxTrace(  "\tCFxEngineCore::DisconnectAllFxPin FEF_FAILED !! (Cannot disconnect pin)");
					return hr;
				}
            }

            if( FEF_FAILED(hr = (*Iter)->pFxPinManager->GetPinCount(wPinCount)) )
			    wPinCount = 0;
        }while(wPinCount > 0);
	}
	_pLogTrace->FxTrace(  "CFxEngineCore::DisconnectAllFxPin quit");
	return FX_OK;
}

Int32 CFxEngineCore::DisConnectFxPin(IFxPin* pFxPin, Bool ShouldStopCallBack /*= TRUE*/, Bool ShouldProtect/* = TRUE*/)
{
	Int32 hr = FX_OK;

	_pLogTrace->FxTrace(  "CFxEngineCore::DisconnectFxPin entry");

	AutoLock lock(_CS, ShouldProtect);

	if(pFxPin == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::DisconnectFxPin FEF_FAILED !! (Invalid Parameter: pFxPin)");
		return FX_INVALPARAM;
	}

    std::string strPinName;

    CFxPin* pFxPinClass	= NULL;
    CFxPin* pFxPinConnectedClass	= NULL;
    IFxPin* pFxPinConnected         = NULL;

    pFxPinClass = static_cast<CFxPin*>(pFxPin);
    pFxPinClass->GetPinName(strPinName);
    _pLogTrace->FxTrace(  "\tFxEngine::DisConnectFxPin %s:%s", (static_cast<FX_COMPONENT*>(pFxPinClass->GetFxMember()))->strFxName.c_str(), strPinName.c_str());

    /*! Stop current pin */
    pFxPinClass->SetPinRunState(PIN_STOP);
    pFxPinClass->DetachObservers(this);
    
    /*! Stop connected pin */
    if( FEF_FAILED(hr = pFxPin->GetPinConnected(&pFxPinConnected)) )
    {
	    _pLogTrace->FxTrace(  "\tFxEngine::DisconnectFxPin FEF_FAILED !! (IFxPin Cannot get connected Pin !!)");
	    return hr;
    }
    if(pFxPinConnected)
    {
	    pFxPinConnectedClass = static_cast<CFxPin*>(pFxPinConnected);
        pFxPinConnectedClass->SetPinRunState(PIN_STOP);
        pFxPinConnectedClass->DetachObservers(this);
    }

    /*! Wait for the end of pin data */
    if(ShouldStopCallBack == TRUE)
    {
        /*! Quit infinite waiting time for all pin of both Fx */
		FX_HANDLE hFx;
		FX_COMPONENT* pFx;
		hFx = pFxPinClass->GetFxMember();
		pFx = static_cast<FX_COMPONENT*>(hFx);
		Uint16 wPinCount;
		if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) ) {
			_pLogTrace->FxTrace(  "\tCFxEngineCore::DisConnectFxPin FEF_FAILED !! (Cannot get pin count)");
			return hr;
		}
		for(Int16 Idx = 0; Idx < wPinCount; Idx++)
		{
			IFxPin* pFxPin = NULL;
			CFxPin* pFxPinClass = NULL;
			pFx->pFxPinManager->GetPin(&pFxPin, Idx);
			if(pFxPin) {
				pFxPinClass = static_cast<CFxPin*>(pFxPin);
				pFxPinClass->QuitInfiniteTime(); /*< Stop pin callbacks when we remove the FX */
			}
		}
		if(pFxPinConnected) {
			hFx = pFxPinConnectedClass->GetFxMember();
			pFx = static_cast<FX_COMPONENT*>(hFx);
			Uint16 wPinCount;
			if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) ) {
				_pLogTrace->FxTrace(  "\tCFxEngineCore::DisConnectFxPin FEF_FAILED !! (Cannot get pin count)");
				return hr;
			}
			for(Int16 Idx = 0; Idx < wPinCount; Idx++)
			{
				IFxPin* pFxPin = NULL;
				CFxPin* pFxPinClass = NULL;
				pFx->pFxPinManager->GetPin(&pFxPin, Idx);
				if(pFxPin) {
					pFxPinClass = static_cast<CFxPin*>(pFxPin);
					pFxPinClass->QuitInfiniteTime(); /*< Stop pin callbacks when we remove the FX */
				}
			}
		}
        
		pFxPinClass->WaitForEndDataInQueue(); /*< waiting for end of media buffer in queue */
        if(pFxPinConnected)
            pFxPinConnectedClass->WaitForEndDataInQueue(); /*< waiting for end of media buffer in queue */
	}

    /*! Disconnect current pin first */
    if( FEF_FAILED(hr = pFxPinClass->DisconnectPin()) )
    {
	    _pLogTrace->FxTrace(  "\tFxEngine::DisconnectFxPin FEF_FAILED !! (IFxPin Cannot disconnect current Pin !!)");
	    return FX_ERROR;
    }

    /*! Stop connected pin */
    if(pFxPinConnected)
    {
        pFxPinConnectedClass->GetPinName(strPinName);
        _pLogTrace->FxTrace(  "\tFxEngine::DisConnectFxPin %s:%s", (static_cast<FX_COMPONENT*>(pFxPinConnectedClass->GetFxMember()))->strFxName.c_str(), strPinName.c_str());

        pFxPinConnectedClass = static_cast<CFxPin*>(pFxPinConnected);
        /*! Disconnect current pin first */
        if( FEF_FAILED(hr = pFxPinConnectedClass->DisconnectPin()) )
        {
	        _pLogTrace->FxTrace(  "\tFxEngine::DisconnectFxPin FEF_FAILED !! (IFxPin Cannot disconnect current Pin !!)");
	        return FX_ERROR;
        }
    }

	_pLogTrace->FxTrace(  "CFxEngineCore::DisconnectFxPin quit");
	return FX_OK;
}

Int32 CFxEngineCore::DisplayFxPropertyPage(FX_HANDLE hFx, Pvoid pvWndParent)
{
	AutoLock lock(_CS);

	Int32 hr = FX_OK;

	_pLogTrace->FxTrace(  "CFxEngineCore::DisplayFxPropertyPage entry");

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
		_pLogTrace->FxTrace("\tCFxEngineCore::DisplayFxPropertyPage %s", pFx->strFxName.c_str());
		if( FEF_FAILED( hr = pFx->pFxBase->DisplayFxPropertyPage(pvWndParent)) )
		{
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			_pLogTrace->FxTrace(  "\tFxEngine::DisplayFxPropertyPage FEF_FAILED !!, hr=%d (Cannot Display Fx PropertyPage)", hr);
			return hr;
		}
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::DisplayFxPropertyPage FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::DisplayFxPropertyPage quit");
	return FX_OK;
}

Int32 CFxEngineCore::GetFxFrame(FX_HANDLE hFx, const Char** ppbFxFrame)
{
    AutoLock lock(_CS);

	Int32 hr = FX_OK;
	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxFrame entry");

    if(ppbFxFrame == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxFrame FEF_FAILED !! (Invalid Parameter: ppbFxFrame)");
		return FX_INVALPARAM;
	}

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
		_pLogTrace->FxTrace("\tCFxEngineCore::GetFxFrame %s", pFx->strFxName.c_str());
		if( FEF_FAILED( hr = pFx->pFxBase->GetFxFrame(ppbFxFrame)) )
		{
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			_pLogTrace->FxTrace(  "\tFxEngine::GetFxFrame FEF_FAILED !!, hr=%d (Cannot Get Fx Frame)", hr);
			return hr;
		}
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxFrame FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxFrame quit");
	return FX_OK;
}

Int32 CFxEngineCore::GetFxUserInterface(FX_HANDLE hFx, Pvoid* ppvUserInterface)
{
	AutoLock lock(_CS);

	Int32 hr = FX_OK;

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxUserInterface entry");

	if(ppvUserInterface == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxUserInterface FEF_FAILED !! (Invalid Parameter: ppvUserInterface)");
		return FX_INVALPARAM;
	}

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
		_pLogTrace->FxTrace("\tCFxEngineCore::GetFxUserInterface %s", pFx->strFxName.c_str());

		if( FEF_FAILED( hr = pFx->pFxBase->GetFxUserInterface(ppvUserInterface)) )
		{
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			_pLogTrace->FxTrace(  "\tFxEngine::GetFxUserInterface FEF_FAILED !!, hr=%d (Cannot get the user interface)", hr);
			return hr;
		}
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxUserInterface FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxUserInterface quit");
	return FX_OK;
}

Int32 CFxEngineCore::GetFxSubFxEngine(FX_HANDLE hFx, FX_HANDLE* phFxEngine)
{
	AutoLock lock(_CS);

	Int32 hr = FX_OK;

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxSubFxEngine entry");

	if(phFxEngine == NULL)
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxSubFxEngine FEF_FAILED !! (Invalid Parameter: phFxEngine)");
		return FX_INVALPARAM;
	}

	if(IsFxLoaded(hFx))
	{
		FX_COMPONENT* pFx = static_cast<FX_COMPONENT*>(hFx);
		if( FEF_FAILED( hr = pFx->pFxBase->GetFxSubFxEngine(phFxEngine)) )
		{
			SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
			_pLogTrace->FxTrace(  "\tFxEngine::GetFxSubFxEngine FEF_FAILED !!, hr=%d (Cannot get the Sub FxEngine)", hr);
			return hr;
		}
	}
	else
	{
		_pLogTrace->FxTrace(  "\tFxEngine::GetFxSubFxEngine FEF_FAILED !! (Invalid Parameter: hFx)");
		return FX_INVALPARAM;
	}

	_pLogTrace->FxTrace(  "CFxEngineCore::GetFxSubFxEngine quit");
	return FX_OK;
}

Bool CFxEngineCore::IsFxLoaded(FX_HANDLE hFx)
{
	CFxVectorIter Iter;
	/*! Find Pin created in list */
	Iter = std::find(_FxVector.begin(), _FxVector.end(), (FX_COMPONENT*)hFx);
	if( Iter != _FxVector.end() )
	{
		return TRUE;
	}
	/*
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
		{
			if((*Iter) == (FX_COMPONENT*)hFx)
				return TRUE;
		}*/


	return FALSE;
}

Bool CFxEngineCore::IsFxLoaded(std::string& str)
{
	CFxVectorIter Iter;
	/*! Find Fx created in list */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
		if(str == (*Iter)->strFxPath)
			return TRUE;
	}

	return FALSE;
}

Void CFxEngineCore::ClearFxVector()
{
	_pLogTrace->FxTrace(  "CFxEngineCore::ClearFxVector entry");

    Bool IsFound = FALSE;
	CFxVectorIter Iter;
    CState* pExistingState;
	CFxStateVectorIter StateIter;
    /*! Remove all fx */
	for (Iter = _FxVector.begin(); (Iter != _FxVector.end()) && !(_FxVector.empty()); Iter++)
	{
        StopFx((*Iter), FALSE);

		(*Iter)->pFxBase->ReleaseFx();
		if((*Iter)->hDLL)
			FreeFxLibrary((*Iter)->hDLL);

        /*! Remove Fx handle */
        _SetFxHandlePtr.erase((*Iter)->strFxPath);

		if((*Iter)->IsTemporaryFx == TRUE)
			remove((*Iter)->strFxPath.c_str());

        /*! Clear state object which is associared with the fx */
        IsFound = FALSE;
        for (StateIter = _FxStateVector.begin(); (StateIter != _FxStateVector.end()) && !(_FxStateVector.empty()); StateIter++)
		{
			if( (*StateIter)->GetFxptr() == (FX_PTR)(*Iter) )
			{
                IsFound = TRUE;
                pExistingState = (*StateIter);
                break;
			}
		}
        if(IsFound == TRUE)
        {
            _FxStateVector.erase(StateIter);
		    SAFE_DELETE_OBJECT(pExistingState);			/*!< Delete the State object */
        }
        SAFE_DELETE_OBJECT((*Iter));			/*!< Delete the Fx object */
	}

	_FxVector.clear();

	/*! */
	/*! Clear all other state objects which are no more associared with a fx */
    for (StateIter = _FxStateVector.begin(); (StateIter != _FxStateVector.end()) && !(_FxStateVector.empty()); StateIter++)
		SAFE_DELETE_OBJECT((*StateIter));			/*!< Delete the State object */

	_FxStateVector.clear();

	_pLogTrace->FxTrace(  "CFxEngineCore::ClearFxVector quit");
	return;
}

Void CFxEngineCore::FxPinUpdate(CFxPin* pPin, Int32 sdwHr, FX_STATE FxState, FX_PTR dwUserParam)
{
	//AutoLock lock(_CS);

	Int32 hr;

	FX_COMPONENT* pFx = (FX_COMPONENT*)(dwUserParam);

    std::string strPinName;
	hr = pPin->GetPinName(strPinName);
	if(FEF_FAILED(hr))
	{
		_pLogTrace->FxTrace(  "\tCFxEngineCore::FxPinUpdate, FxEngine::Error on Fx !! (IFxPin::GetPinName FEF_FAILED !!)");
		return;
	}
	_pLogTrace->FxTrace(  "CFxEngineCore::FxPinUpdate  --> Fx(Short Name: %s, Pin Name: %s, hr: %s, FxState: %s)",
														pFx->strFxName.c_str(),
														strPinName.c_str(),
														(GetConstToString(FXENGINE_ERROR_CONST, sdwHr)).c_str(),
														(GetConstToString(FX_STATE_CONST, FxState)).c_str());
	SetFxState((FX_PTR)pFx, FxState);
	return;
}

Int32 CFxEngineCore::GetFxRefClock(Uint64* pqRefClock, Uint32 dwId )
{
	AutoLock lock(_CSRefClock);

	//_pLogTrace->FxTrace(  "CFxEngineCore::GetFxRefClock entry");

	if(pqRefClock == NULL)
	{
		_pLogTrace->FxTrace(  "FxEngine::GetFxRefClock FEF_FAILED !! (Invalid Parameter: pqRefClock)");
		return FX_INVALPARAM;
	}

	FxRefClockMap::iterator it;

    it = _FxRefClock.find( dwId );
    if( it == _FxRefClock.end() )
    {
        *pqRefClock = 0;
		_pLogTrace->FxTrace(  "FxEngine::GetFxRefClock FEF_FAILED !! (Invalid Parameter: dwId)");
		return FX_INVALPARAM;
    }

	*pqRefClock = it->second;

	return FX_OK;
}

Int32 CFxEngineCore::SetFxRefClock(Uint64 qRefClock, Uint32 dwId )
{
	AutoLock lock(_CSRefClock);

	//_pLogTrace->FxTrace(  "CFxEngineCore::SetFxRefClock entry");

	_FxRefClock[dwId] = qRefClock;

	return FX_OK;
}

Int32 CFxEngineCore::SetFxRefClockError(FX_PTR qUserParam)
{
	AutoLock lock(_CSRefClock);

	//_pLogTrace->FxTrace(  "CFxEngineCore::SetFxRefClockError entry");

	FX_COMPONENT* pFx = (FX_COMPONENT*)(qUserParam);

	if(pFx)
	{
		_pLogTrace->FxTrace(  "FxEngine::Error on Fx Ref Clock --> Fx(Name: %s)", pFx->strFxName.c_str());
		SetFxState((FX_PTR)pFx, FX_ERROR_STATE);
	}
	else
		return FX_ERROR;

	return FX_OK;
}

Int32 CFxEngineCore::GetFxEngineRefClock(Uint64* pqRefClock)
{
	AutoLock lock(_CSRefClock);

	//_pLogTrace->FxTrace(  "CFxEngineCore::GetFxEngineRefClock entry");

	if(pqRefClock)
	{
		_pLogTrace->FxTrace(  "FxEngine::GetFxEngineRefClock FEF_FAILED !! (Invalid Parameter: pqRefClock)");
		return FX_INVALPARAM;
	}

	*pqRefClock = _qFxEngineRefClock;

	return FX_OK;
}

Int32 CFxEngineCore::SetFxEngineRefClock(Uint64 qRefClock)
{
	AutoLock lock(_CSRefClock);

	//_pLogTrace->FxTrace(  "CFxEngineCore::SetFxEngineRefClock entry");

	_qFxEngineRefClock = qRefClock;

	return FX_OK;
}

Void CFxEngineCore::FxStateCallback(FX_STATE FxState, FX_HANDLE hObserverId, FX_PTR dwParam)
{
	FX_COMPONENT* pFx = (FX_COMPONENT*)(dwParam);

	if(pFx == NULL)
		return;

	/*_pLogTrace->FxTrace(  "%s%d: %s",
		pFx->strFxShortName.c_str(),
		(static_cast<CFx*>(pFx->pIFx))->GetFxInstance(),
		(GetConstToString(FX_STATE_CONST, FxState)).c_str());*/

	return;
}

Int32 CFxEngineCore::SaveFxEngine(const std::string strFilePath)
{
	Int32 hr;

	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "CFxEngineCore::SaveFxEngine entry");

    if (strFilePath.empty())
	{
		_pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Invalid Parameter: strFilePath)");
		return FX_INVALPARAM;
	}

    /*! Initialize parameter size */
    std::map<FX_UNIT_TYPE, int> ParamSizeMap;
    ParamSizeMap[NOT_DEFINED_TYPE] = 0;
    ParamSizeMap[UINT8_TYPE] = 1;
    ParamSizeMap[INT8_TYPE] = 1;
    ParamSizeMap[UINT16_TYPE] = 2;
    ParamSizeMap[INT16_TYPE] = 2;
    ParamSizeMap[UINT32_TYPE] = 4;
    ParamSizeMap[INT32_TYPE] = 4;
    ParamSizeMap[INT64_TYPE] = 8;
    ParamSizeMap[UINT64_TYPE] = 8;
    ParamSizeMap[FLOAT32_TYPE] = 4;
    ParamSizeMap[FLOAT64_TYPE] = 8;
    ParamSizeMap[COMPLEX_TYPE] = 8;

    /*! Create the XML object */
    CXMLRoot XMLRoot(strFilePath);
    MapFxIdToFxXML FxIdToFxXMLMap;
    MapFxIdToFxXMLItr ItrFx;
    VectorOfFxParamItr ItrFxParam;
    CXMLFx* pFxXMLObj;
    MapIdToConnectionXML IdToConnectionXMLMap;
    MapIdToConnectionXMLItr ItrCon;
    CXMLConnection* pXMLConnectionObj;

    FX_HANDLE hFx = NULL;
	FX_COMPONENT* pFx = NULL;
	Uint16 wPinCount = 0;
	IFxPin* pIFxPinConnected;
	Uint16 wPinNumber;
	Uint16 wFxNumber;
    FX_PIN_TYPE PinType;
	CFxPin* pFxPinConnectedClass = NULL;
	FX_HANDLE hFxConnected;
	FX_COMPONENT* pFxConnected;
	FX_MEDIA_TYPE MediaType;

    Char str[MAX_PATH];

    Uint16 wParamCount;
    FX_XML_PARAM* pFxXMLParam = NULL;
    const FX_PARAM* pFxParam = NULL;
	const FX_PARAM_STRING* pFxStrParam = NULL;

	/*! Open log file */

	_pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Invalid Parameter: strFilePath)");

	Uint16 wFxCount;
	if(FEF_FAILED(hr = GetFxCount(&wFxCount, FALSE)))
	{
		_pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Cannot get Fx number)");
		goto Error;
	}

	/*! Save the Fx Paths */
	for(Uint16 IdxFx = 0; IdxFx < wFxCount; IdxFx++)
	{
		hFx = NULL;
		pFx = NULL;
		if(FEF_FAILED(hr = GetFx(&hFx, IdxFx, FALSE)))
		{
			_pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Cannot get Fx handle)");
			goto Error;
		}
        pFxXMLObj = new CXMLFx;
		pFx = static_cast<FX_COMPONENT*>(hFx);

		if(pFx->strFxPath != "")
		{
			pFxXMLObj->SetFxName(pFx->strFxPathOnDisk);
		}
		else
		{
			memset(str, 0, MAX_PATH * sizeof(Char));
#ifdef WIN32
			sprintf_s(str, MAX_PATH, "Private Fx: %s", pFx->strFxName.c_str());
#else
			sprintf(str, "Private Fx: %s", pFx->strFxName.c_str());
#endif
            pFxXMLObj->SetFxName(str);
		}

        /*! Add Fx parameters */
        pFx->pFxParam->GetFxParamCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pFxXMLParam = new FX_XML_PARAM;
            pFxXMLParam->UnitType = NOT_DEFINED_TYPE;
            pFxXMLParam->pbValue = NULL;
            pFxXMLParam->dwNumber = 0;
            pFx->pFxParam->GetFxParam(&pFxParam, i);

            pFxXMLParam->strName = pFxParam->strParamName;

            pFxXMLParam->dwNumber = pFxParam->dwParamNumber;
            pFxXMLParam->UnitType = pFxParam->ParamType;
            pFxXMLParam->pbValue = new Uint8[ParamSizeMap[pFxXMLParam->UnitType] * pFxXMLParam->dwNumber];
            pFx->pFxParam->GetFxParamValue(pFxParam->strParamName, (Void*)pFxXMLParam->pbValue);
			pFxXMLParam->strValue = "";
            pFxXMLObj->GetVectorOfFxParam().push_back(pFxXMLParam);
        }
		/*! Add Fx string parameters */
        pFx->pFxParam->GetFxParamStringCount(&wParamCount);
        for(int i = 0; i < wParamCount; i++)
        {
            pFxXMLParam = new FX_XML_PARAM;
            pFx->pFxParam->GetFxParam(&pFxStrParam, i);

            pFxXMLParam->strName = pFxStrParam->strParamName;
            pFxXMLParam->UnitType = NOT_DEFINED_TYPE;
            pFxXMLParam->pbValue = NULL;
            pFxXMLParam->dwNumber = 0;
			pFx->pFxParam->GetFxParamValue(pFxStrParam->strParamName, pFxXMLParam->strValue);
            pFxXMLObj->GetVectorOfFxParam().push_back(pFxXMLParam);
        }

        FxIdToFxXMLMap.insert(make_pair(IdxFx, pFxXMLObj));

	}

	/*! Save the Fx connections */
    {
        Uint16 wConnectionCount = 0;
        for(Uint16 IdxFx = 0; IdxFx < wFxCount; IdxFx++)
        {
            hFx = NULL;
            pFx = NULL;
            wPinCount = 0;
            if(FEF_FAILED(hr = GetFx(&hFx, IdxFx, FALSE)))
            {
                _pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Cannot get Fx handle)");
                goto Error;
            }
            pFx = static_cast<FX_COMPONENT*>(hFx);
            if(pFx->strFxPath == "")
                continue;

            pFx->pFxPinManager->GetPinCount(wPinCount);
            for(Uint16 IdxFxPin = 0; IdxFxPin < wPinCount; IdxFxPin++)
            {
                IFxPin* pIFxPin;
                if(FEF_FAILED(hr = pFx->pFxPinManager->GetPin(&pIFxPin, IdxFxPin)))
                {
                    _pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Cannot get Fx Pin)");
                    goto Error;
                }

                /*! Loop only on output pin */
                pIFxPin->GetPinType(&PinType);
                if(PinType == PIN_IN)
                    continue;

                if(FEF_FAILED(hr = pIFxPin->GetPinConnected(&pIFxPinConnected)))
                {
                    _pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Cannot get Fx Pin connected)");
                    goto Error;
                }
                if(pIFxPinConnected == NULL)
                    continue;

                pFxPinConnectedClass = static_cast<CFxPin*>(pIFxPinConnected);
                hFxConnected = pFxPinConnectedClass->GetFxMember();
                pFxConnected = static_cast<FX_COMPONENT*>(hFxConnected);
                if(pFxConnected->strFxPath == "")
                    continue;
                if(FEF_FAILED(hr = pFxConnected->pFxPinManager->GetPinNumber(pIFxPinConnected, &wPinNumber)))
                {
                    _pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Cannot get Pin Number)");
                    goto Error;
                }
                if(FEF_FAILED(hr = GetFxNumber(hFxConnected, &wFxNumber, FALSE)))
                {
                    _pLogTrace->FxTrace( "\tCFxEngineCore::SaveFxEngine FEF_FAILED !! (Cannot get Fx Number)");
                    goto Error;
                }
                pXMLConnectionObj = new CXMLConnection;

                pXMLConnectionObj->SetFxId(IdxFx, 0);
                pXMLConnectionObj->SetFxPinId(IdxFxPin, 0);
                pXMLConnectionObj->SetFxId(wFxNumber, 1);
                pXMLConnectionObj->SetFxPinId(wPinNumber, 1);

                pFxPinConnectedClass->GetConnectionMediaType(&MediaType);
                pXMLConnectionObj->SetMainMedia(MediaType.MainMediaType);
                pXMLConnectionObj->SetSubMedia(MediaType.SubMediaType);
                pXMLConnectionObj->SetId(wConnectionCount);
                IdToConnectionXMLMap.insert(make_pair(wConnectionCount, pXMLConnectionObj));
                wConnectionCount++;
            }
        }
	}
    if(FEF_FAILED(hr = XMLRoot.WriteXML(FxIdToFxXMLMap, IdToConnectionXMLMap)) )
    {
        _pLogTrace->FxTrace(  "CFxEngineCore::SaveFxEngine FEF_FAILED !! (Invalid Parameter: %s)", strFilePath.c_str());
        goto Error;
    }

    /*! Remove the maps */
    {
        ItrFx = FxIdToFxXMLMap.begin();
        ItrFxParam = pFxXMLObj->GetVectorOfFxParam().begin();
        ItrCon = IdToConnectionXMLMap.begin();
        while (ItrFx != FxIdToFxXMLMap.end ())
        {
           /* while(ItrFxParam != pFxXMLObj->GetVectorOfFxParam().end())
            {
                SAFE_DELETE_ARRAY((*ItrFxParam)->pbValue);
                SAFE_DELETE_ARRAY((*ItrFxParam));
                ItrFxParam++;
            }*/
            SAFE_DELETE_OBJECT((*ItrFx).second);
            ItrFx++;
        }
        FxIdToFxXMLMap.clear();
        while (ItrCon != IdToConnectionXMLMap.end ())
        {
            SAFE_DELETE_OBJECT((*ItrCon).second);
            ItrCon++;
        }
        IdToConnectionXMLMap.clear();
    }


	_pLogTrace->FxTrace( "CFxEngineCore::SaveFxEngine quit");
	hr = FX_OK;

Error:
	return hr;
}

Int32 CFxEngineCore::LoadFxEngine(const std::string strFilePath)
{
	Int32 hr;

	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "CFxEngineCore::LoadFxEngine entry");

    if (strFilePath.empty())
	{
		_pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Invalid Parameter: strFilePath)");
		return FX_INVALPARAM;
	}

    std::ifstream fd(strFilePath.c_str());
    if(fd.is_open() == FALSE)
    {
		_pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Invalid Parameter: strFilePath)");
		return FX_INVALPARAM;
	}

    /*! Check header */
    std::string strHeader="";
	Bool IsSecondVersion = FALSE;
	/*! Get line of xml and create the FEF objects */
    std::string strLine = "";
    std::getline(fd, strLine);
    if(strLine.find("<FEF>") < strLine.size()) //!< XML version
    {
        if(fd.is_open() == TRUE) //!< The root object open it
            fd.close();

        /*! Create the XML object */
        CXMLRoot XMLRoot(strFilePath);
        MapFxIdToFxXML FxIdToFxXMLMap;
        MapFxIdToFxXMLItr ItrFx;
        VectorOfFxParamItr ItrFxParam;
        MapIdToConnectionXML IdToConnectionXMLMap;
        MapIdToConnectionXMLItr ItrCon;

        if(FEF_FAILED(hr = XMLRoot.ReadXML(FxIdToFxXMLMap, IdToConnectionXMLMap)))
        {
            _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (hr = %d)", hr);
			goto Error;
        }

        /*! Load the Fx with Paths */
        ItrFx = FxIdToFxXMLMap.begin();
        FX_HANDLE hFx;
        FX_COMPONENT *pFx;
        while (ItrFx != FxIdToFxXMLMap.end ())
        {
            if(FEF_FAILED(hr = AddFx((*ItrFx).second->GetFxName(), &hFx, FALSE)))
		    {
			    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get add Fx: %s)", (*ItrFx).second->GetFxName().c_str());
			    goto Error;
		    }
            /*! Set Fx parameters */
            if( (XMLRoot.GetMajor() == 1) && (XMLRoot.GetMinor() == 3) )
            {
                /*! read fx parameters */
                pFx = static_cast<FX_COMPONENT*>(hFx);
                ItrFxParam = (*ItrFx).second->GetVectorOfFxParam().begin();
                while(ItrFxParam != (*ItrFx).second->GetVectorOfFxParam().end())
                {
					if((*ItrFxParam)->UnitType != NOT_DEFINED_TYPE)
						pFx->pFxParam->SetFxParamValue((*ItrFxParam)->strName, (Void*)(*ItrFxParam)->pbValue);
					else
						pFx->pFxParam->SetFxParamValue((*ItrFxParam)->strName, (*ItrFxParam)->strValue);

                    ItrFxParam++;
                }
                UpdateFxParam(hFx, "", FX_PARAM_ALL, FALSE);
            }
			ItrFx++;
        }

        /*! Connext Fxs */
        ItrCon = IdToConnectionXMLMap.begin();
        FX_HANDLE hFx1, hFx2;
	    FX_COMPONENT *pFx1, *pFx2;
	    IFxPin *pIFxPinIn, *pIFxPinOut;
	    FX_PIN_TYPE PinType;
	    FX_MEDIA_TYPE MediaType;
        while ( ItrCon != IdToConnectionXMLMap.end ())
        {
            if(FEF_FAILED(hr = GetFx(&hFx1, (*ItrCon).second->GetFxId(0), FALSE)))
		    {
                _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx handle: %d)", 0);
			    goto Error;
		    }
            if(FEF_FAILED(hr = GetFx(&hFx2, (*ItrCon).second->GetFxId(1), FALSE)))
		    {
			    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx handle: %d)", 1);
			    goto Error;
		    }

		    pFx1 = static_cast<FX_COMPONENT*>(hFx1);
            if(FEF_FAILED(hr = pFx1->pFxPinManager->GetPin(&pIFxPinIn, (*ItrCon).second->GetFxPinId(0))))
		    {
			    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin: %d)", 0);
			    goto Error;
		    }

            pFx2 = static_cast<FX_COMPONENT*>(hFx2);
            if(FEF_FAILED(hr = pFx2->pFxPinManager->GetPin(&pIFxPinOut, (*ItrCon).second->GetFxPinId(1))))
		    {
			    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin: %d)", 1);
			    goto Error;
		    }

            /* Check pin type */
            if(FEF_FAILED(hr = pIFxPinIn->GetPinType(&PinType)))
		    {
			    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin type)");
			    goto Error;
		    }
		    if(PinType != PIN_IN)
		    {
			    IFxPin *pIFxPinTemp;
			    pIFxPinTemp = pIFxPinOut;
			    pIFxPinOut = pIFxPinIn;
			    pIFxPinIn = pIFxPinTemp;
		    }

		    IFxPin *pIFxPinConnected;
		    if(FEF_FAILED(hr = pIFxPinIn->GetPinConnected(&pIFxPinConnected)))
		    {
			    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin connected)");
			    goto Error;
		    }

		    if(pIFxPinConnected == NULL)
		    {
			    /* Connect Pins */
                MediaType.MainMediaType = (FX_MAIN_MEDIA_TYPE)(*ItrCon).second->GetMainMedia();
                MediaType.SubMediaType = (FX_SUB_MEDIA_TYPE)(*ItrCon).second->GetSubMedia();
			    if(FEF_FAILED(hr = ConnectFxPinEx(pIFxPinIn, pIFxPinOut, &MediaType, FALSE)))
			    {
				    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot connect Pins)");
				    goto Error;
			    }
			}
			ItrCon++;
        }
        /*! Remove the maps */
        ItrFx = FxIdToFxXMLMap.begin();
        ItrCon = IdToConnectionXMLMap.begin();
        while (ItrFx != FxIdToFxXMLMap.end ())
		{
            SAFE_DELETE_OBJECT((*ItrFx).second);
			ItrFx++;
		}
        FxIdToFxXMLMap.clear();
        while (ItrCon != IdToConnectionXMLMap.end ())
		{
            SAFE_DELETE_OBJECT((*ItrCon).second);
			ItrCon++;
		}
        IdToConnectionXMLMap.clear();
    }
	else
    {
        if(strLine.find("FEF Design 1.2") < strLine.size())
	    {
		    IsSecondVersion = TRUE;
	    }

        if(fd.is_open() == TRUE)
            fd.close();

        /*! Open log file */
	    FILE* fd = NULL;

#ifdef WIN32
        errno_t hr;
        hr = fopen_s(&fd, strFilePath.c_str(), "rb");
        if (hr)
#else
		fd = fopen(strFilePath.c_str(), "rb");
		if (fd != NULL)
#endif
	    {
		    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Invalid Parameter: strFilePath)");
		    return FX_INVALPARAM;
	    }

	    /* Get the Fx to load */
	    Uint16 wFxCount;
	    fread(&wFxCount,2,1,fd);

	    FX_HANDLE hFx;
	    Char str[MAX_PATH];
	    /*! Load the Fx with Paths */
	    for(Uint16 IdxFx = 0; IdxFx < wFxCount; IdxFx++)
	    {
		    fread(str,MAX_PATH,1,fd);
		    if(FEF_FAILED(hr = AddFx(str, &hFx, FALSE)))
		    {
			    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get add Fx: %s)", str);
			    goto Error;
		    }
	    }

	    /* Get the pin count of the first Fx loaded */
	    Uint16 wPinCount = 0;
	    FX_HANDLE hFx1, hFx2;
	    FX_COMPONENT *pFx1, *pFx2;
	    IFxPin *pIFxPinIn, *pIFxPinOut;
	    FX_PIN_TYPE PinType;
	    Uint16 wFxNumber, wPinNumber;
	    FX_MEDIA_TYPE MediaType;

	    for(Uint16 IdxFx = 0; IdxFx < wFxCount; IdxFx++)
	    {
		    fread(&wPinCount,2,1,fd);
		    for(Uint16 IdxFxPin = 0; IdxFxPin < wPinCount; IdxFxPin++)
		    {
			    fread(&wFxNumber,2,1,fd);
			    fread(&wPinNumber,2,1,fd);
			    if(FEF_FAILED(hr = GetFx(&hFx1, wFxNumber, FALSE)))
			    {
				    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx handle)");
				    goto Error;
			    }
			    pFx1 = static_cast<FX_COMPONENT*>(hFx1);
			    if(FEF_FAILED(hr = pFx1->pFxPinManager->GetPin(&pIFxPinIn, wPinNumber)))
			    {
				    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin)");
				    goto Error;
			    }

			    fread(&wFxNumber,2,1,fd);
			    fread(&wPinNumber,2,1,fd);

			    if(IsSecondVersion == TRUE)
			    {
				    fread(&MediaType.MainMediaType,sizeof(FX_MAIN_MEDIA_TYPE),1,fd);
				    fread(&MediaType.SubMediaType,sizeof(FX_SUB_MEDIA_TYPE),1,fd);
			    }

			    if(FEF_FAILED(hr = GetFx(&hFx2, wFxNumber, FALSE)))
			    {
				    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx handle)");
				    goto Error;
			    }
			    pFx2 = static_cast<FX_COMPONENT*>(hFx2);
			    if(FEF_FAILED(hr = pFx2->pFxPinManager->GetPin(&pIFxPinOut, wPinNumber)))
			    {
				    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin)");
				    goto Error;
			    }

			    /* Check pin type */
                if(FEF_FAILED(hr = pIFxPinIn->GetPinType(&PinType)))
			    {
				    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin type)");
				    goto Error;
			    }
			    if(PinType != PIN_IN)
			    {
				    IFxPin *pIFxPinTemp;
				    pIFxPinTemp = pIFxPinOut;
				    pIFxPinOut = pIFxPinIn;
				    pIFxPinIn = pIFxPinTemp;
			    }

			    IFxPin *pIFxPinConnected;
			    if(FEF_FAILED(hr = pIFxPinIn->GetPinConnected(&pIFxPinConnected)))
			    {
				    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot get Fx Pin connected)");
				    goto Error;
			    }

			    if(pIFxPinConnected == NULL)
			    {
				    /* Connect Pins */
				    if(IsSecondVersion == TRUE)
				    {
					    if(FEF_FAILED(hr = ConnectFxPinEx(pIFxPinIn, pIFxPinOut, &MediaType, FALSE)))
					    {
						    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot connect Pins)");
						    goto Error;
					    }
				    }
				    else
				    {
					    if(FEF_FAILED(hr = ConnectFxPin(pIFxPinIn, pIFxPinOut, FALSE)))
					    {
						    _pLogTrace->FxTrace( "\tCFxEngineCore::LoadFxEngine FEF_FAILED !! (Cannot connect Pins)");
						    goto Error;
					    }
				    }
			    }
		    }
        }
        /*! Close log file */
        if(fd)
            fclose(fd);
    }

	_pLogTrace->FxTrace( "CFxEngineCore::LoadFxEngine quit");
	hr = FX_OK;

Error:
	return hr;
}

inline Uint64 FEFGetFileSize( std::ifstream & strFile )
{
    /*! get current pos */
    Uint64 qPos = strFile.tellg();
    /*! Moves the read position at the end */
    strFile.seekg( 0 , std::ios_base::end );
    /*! get new pos = file size */
    Uint64 qSize = strFile.tellg() ;
    /*! restaure file pos to begin */
    strFile.seekg( qPos,  std::ios_base::beg ) ;
    return qSize ;
}

#define BUFFER_SIZE (512)
inline Int32 FEFCopyFile(std::string& strSrc, std::string& strDst)
{
	Char pbData[BUFFER_SIZE];
	Uint32 dwSizetoRead = BUFFER_SIZE;
	Uint64 qFileSize;

	/*! Open Files */
	ifstream ifd(strSrc.c_str(), ios::binary);
	if(!ifd.is_open())
	{
		return FX_INVALPARAM;
	}
	/*! get file size */
	qFileSize = FEFGetFileSize(ifd);

	ofstream ofd(strDst.c_str(), ios::binary);
	if(!ofd.is_open())
	{
		ifd.close();
		return FX_ERROR;
	}

	/*! Copy file */
	while(qFileSize != 0)
	{
		dwSizetoRead = (qFileSize < dwSizetoRead) ? qFileSize : dwSizetoRead;
        try
        {
		    ifd.read(pbData, dwSizetoRead);
		    ofd.write(pbData, dwSizetoRead);
        }
        catch(exception& e)
        {
            return FX_ERROR; 
        }


		qFileSize -= dwSizetoRead;
	}

	/*! Close Files */
	ifd.close();
	ofd.close();

	return 0;
}

Int32 CFxEngineCore::DuplicateFx(std::string& strFxPath)
{
    Int32 hr = FX_OK;

    std::string strFxSrc = strFxPath;
    Uint64 qFileSize = 0;

    char* strTmpFile = NULL;
#ifdef WIN32
    if( ( strTmpFile = _tempnam( ".\\", "Fx" ) ) != NULL )
    {
        strFxPath = strTmpFile;
        strFxPath += ".tmp"; //!< Add extension
    }
#else
    strFxPath = GetTemporaryDir();
    strFxPath += "FxXXXXXX";
    strTmpFile = new Char[strFxPath.size() + 2];
    sprintf(strTmpFile, strFxPath.c_str());
    if( mkstemp(strTmpFile) >= 0 )
    {
        strFxPath = strTmpFile;
    }
#endif
    else {
        _pLogTrace->FxTrace( "\tFxEngine::FEF_AddFx FEF_FAILED !! (Cannot get temporary file)");
        return FX_ERROR;
    }
    SAFE_DELETE_OBJECT(strTmpFile);

    /*! Duplicate Fx File */
    /*! ***************** */
    if(FEF_FAILED(hr = FEFCopyFile(strFxSrc, strFxPath))) {
        _pLogTrace->FxTrace( "\tFxEngine::FEF_AddFx FEF_FAILED (DuplicateFx)!! (Cannot create temporary file)");
		return hr;
    }

    return hr;
}

FX_PTR CFxEngineCore::LoadFxLibrary(std::string& strFx)
{
#ifdef WIN32
    return (FX_PTR)LoadLibrary(strFx.c_str());
#else
    return (FX_PTR)dlopen( strFx.c_str(), RTLD_NOW );
#endif

}

Void CFxEngineCore::FreeFxLibrary(FX_PTR hFx)
{
#ifdef WIN32
    FreeLibrary((HMODULE)hFx);
    return;
#else
    dlclose((Void*)hFx);
    return;
#endif
}

FxBaseFnct* CFxEngineCore::GetFxProcAddress(FX_PTR hFx, std::string strFctName)
{
#ifdef WIN32
    return (FxBaseFnct*)GetProcAddress((HMODULE)hFx, strFctName.c_str());
#else
    return (FxBaseFnct*)dlsym((Void*)hFx, strFctName.c_str());
#endif
}

 } //namespace FEF
