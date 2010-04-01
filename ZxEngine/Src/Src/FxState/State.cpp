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
#include "FxDef.h"
#include "FxState.h"
#include <algorithm>

namespace FEF {

CState::CState(FX_PTR dwFx)
{
	_LastFxState = FX_UNDEFINED_STATE;

	_dwFx = dwFx;

	/*! Launch the Fx thread */
	_pAPC = NULL;
	_pAPC = new CFxAPC(this);
}

CState::~CState()
{
	AutoLock lock(_StateCS);

	/*! APC Thread */
	SAFE_DELETE_OBJECT(_pAPC);

    AutoLock lockObserver(_StateObserverCS);

    StateObserverIter Iter;
	for (Iter = _StateObserverV.begin(); (Iter != _StateObserverV.end()) && !(_StateObserverV.empty()); Iter++)
		SAFE_DELETE_OBJECT((*Iter));

    _StateObserverV.clear();

	lockObserver.unlock();
	lock.unlock();
}

Int32 CState::AttachFxObserver( /* [in] */CFxStateCallback* pFxStateCallback,
								/* [in] */FX_PTR dwParam,
								/* [out] */FX_HANDLE* phObserverId)
{
	AutoLock lock(_StateCS);
    AutoLock lockObserver(_StateObserverCS);

	if(pFxStateCallback == NULL)
	{
		return FX_INVALPARAM;
	}

	if(phObserverId == NULL)
	{
		return FX_INVALPARAM;
	}

	FX_STATE_OBSERVER* pFxObserver = NULL;
	pFxObserver = new FX_STATE_OBSERVER;
	if(pFxObserver == NULL)
		return FX_NOMEM;

	pFxObserver->pObserverClass = pFxStateCallback;
	pFxObserver->pObserverFnct = NULL;
	pFxObserver->dwParam = dwParam;

	*phObserverId = (FX_HANDLE)pFxObserver;


	_StateObserverV.push_back(pFxObserver);

	if(_LastFxState > FX_UNDEFINED_STATE)
	{
		CNotifyObjects* pParam = CNotifyObjects::Create( (FX_PTR)this, (FX_PTR)pFxObserver, (FX_PTR)_LastFxState);
		if(_pAPC)
			_pAPC->AddAPCObject(pParam);
	}

	return FX_OK;
}

Int32 CState::AttachFxObserverEx( /* [in] */FXSTATECALLBACK* pFxStateCallback,
									/* [in] */FX_PTR dwParam,
									/* [out] */FX_HANDLE* phObserverId)
{
	AutoLock lock(_StateCS);
    AutoLock lockObserver(_StateObserverCS);

	if(pFxStateCallback == NULL)
	{
		return FX_INVALPARAM;
	}

	if(phObserverId == NULL)
	{
		return FX_INVALPARAM;
	}

	FX_STATE_OBSERVER* pFxObserver = NULL;
	pFxObserver = new FX_STATE_OBSERVER;
	if(pFxObserver == NULL)
		return FX_NOMEM;

	pFxObserver->pObserverClass = NULL;
	pFxObserver->pObserverFnct = pFxStateCallback;
	pFxObserver->dwParam = dwParam;

	*phObserverId = (FX_HANDLE)pFxObserver;


	_StateObserverV.push_back(pFxObserver);

	if(_LastFxState > FX_UNDEFINED_STATE)
	{
		CNotifyObjects* pParam = CNotifyObjects::Create( (FX_PTR)this, (FX_PTR)pFxObserver, (FX_PTR)_LastFxState);
		if(_pAPC)
			_pAPC->AddAPCObject(pParam);
	}

	return FX_OK;
}

Int32 CState::DetachFxObserver(/* [in] */FX_HANDLE hObserverId)
{
	AutoLock lock(_StateCS);
    AutoLock lockObserver(_StateObserverCS);

	if(hObserverId == NULL)
	{
		return FX_INVALPARAM;
	}

	StateObserverIter Iter;
	Iter = std::find(_StateObserverV.begin(), _StateObserverV.end(), hObserverId);
	if( Iter != _StateObserverV.end() )
	{
		SAFE_DELETE_OBJECT((*Iter));
		_StateObserverV.erase(Iter);
	}

	return FX_OK;
}

Int32 CState::PublishFxState( /* [in] */FX_STATE FxState)
{
	AutoLock lock(_StateCS); //!< different than lockObserver to allow FEF_XXX Call in callback state

	_LastFxState = FxState;

	StateObserverIter Iter;
	for (Iter = _StateObserverV.begin(); (Iter != _StateObserverV.end()) && !(_StateObserverV.empty()); Iter++)
	{
		CNotifyObjects* pParam = CNotifyObjects::Create( (FX_PTR)this, (FX_PTR)(*Iter), (FX_PTR)FxState);
		if(_pAPC)
			_pAPC->AddAPCObject(pParam);
	}

	return FX_OK;
}

Bool CState::IsObserverExist(FX_STATE_OBSERVER* pStateObsvr)
{
	StateObserverIter Iter;
	Iter = std::find(_StateObserverV.begin(), _StateObserverV.end(), pStateObsvr);
	if( Iter != _StateObserverV.end() )
	{
		return TRUE;
	}
	/*
	for (Iter = _StateObserverV.begin(); (Iter != _StateObserverV.end()) && !(_StateObserverV.empty()); Iter++)
		{
			if( (*Iter) == pStateObsvr )
			{
				return TRUE;
			}
		}*/

	return FALSE;
}

Int32 CState::APCCallBack(void* pObject)
{
	Int32 hr = FX_OK;

	CNotifyObjects* pParam = (CNotifyObjects*)pObject;
	CState* pFxState = (CState*)(pParam->_qInstance);

	AutoLock lock(pFxState->_StateObserverCS);

	FX_STATE_OBSERVER* pStateObsvr = (FX_STATE_OBSERVER*)(pParam->_qParam1);

	if(pFxState->IsObserverExist(pStateObsvr))
	{
		if(pStateObsvr->pObserverClass)
			pStateObsvr->pObserverClass->FxStateCallback((FX_STATE)(pParam->_qParam2), pStateObsvr, pStateObsvr->dwParam);
		if(pStateObsvr->pObserverFnct)
			pStateObsvr->pObserverFnct((FX_STATE)(pParam->_qParam2), pStateObsvr, pStateObsvr->dwParam);
	}
	pParam->Release();

	return hr;
}

 } //namespace FEF
