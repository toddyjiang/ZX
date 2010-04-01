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

   ____________________________________________http://www.SMProcess.com______ 
*//*!
   @file	 State.h
   @brief This is the main interface file for the Fx plugin State object.

   CState object allows to a Fx plugin to mail message with
   FxEngine application. 
*//*_______________________________________________________________________*/
#pragma once
//! FxEngine Framework definitions
#include "FxDef.h"
#include "IFx.h"

#include "FxUtils/FxUtils.h"
#include "FxUtils/NotifyObjects.h"

#include "FxUtils/FxAPC.h"
#include <vector>

namespace FEF {

//! FxEngine interfaces
class CState;

typedef struct _FX_STATE_OBSERVER {
	CFxStateCallback*	pObserverClass; 
	FXSTATECALLBACK*    pObserverFnct;
	FX_PTR				dwParam;
} FX_STATE_OBSERVER, *PFX_STATE_OBSERVER;

typedef std::vector<FX_STATE_OBSERVER*>  StateObserverV;
typedef StateObserverV::iterator   StateObserverIter;

//! The IFxState class is the Fx plugin State interface.
class CState : public CFxAPCCallBack {

public:
	CState(FX_PTR dwFx);
	virtual ~CState();	

public:
	
	/*----------------------------------------------------------------------*//*!
	AttachFxObserver().

	The AttachFxObserver method attaches a Fx state observer to the Fx.

    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 AttachFxObserver( /* [in] */CFxStateCallback* pFxStateCallback,
									/* [in] */FX_PTR dwParam,
									/* [out] */FX_HANDLE* phObserverId);

	/*----------------------------------------------------------------------*//*!
	AttachFxObserverEx().

	The AttachFxObserver method attaches a Fx state observer to the Fx.
    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 AttachFxObserverEx( /* [in] */FXSTATECALLBACK* pFxStateCallback,
									  /* [in] */FX_PTR dwParam,
									  /* [out] */FX_HANDLE* phObserverId);

	/*----------------------------------------------------------------------*//*!
	DetachFxObserver().

	The AttachMailBox method detaches a Fx state observer to the Fx.

    @param qRefClock:		Variable that contains the reference clock to
							publish.
	@param dwId:			Variable that contains the Fx reference clock ID.
	
	
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 DetachFxObserver(/* [in] */FX_HANDLE hObserverId);

	/*----------------------------------------------------------------------*//*!
	PublishFxState().

	The PublishFxState method sends a Fx state to its observer.
    @param FxState:		Variable that contain the Fx State to publish.
								
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 PublishFxState( /* [in] */FX_STATE FxState);

private:
	//! Critical Section
	boost::mutex _StateCS;
    boost::mutex _StateObserverCS;

private:
	StateObserverV _StateObserverV;

private:
	FX_STATE		_LastFxState;
public:
	FX_STATE GetFxState() const   { return (_LastFxState); }
	
private:
	FX_PTR			_dwFx;

public:
	FX_PTR			GetFxptr() const   { return (_dwFx); }

private:
	Bool			IsObserverExist(FX_STATE_OBSERVER* pStateObsvr);

private:
	Uint32			_dwTotalCount;
public:
	Uint32 GetStateCount() const   { return (_pAPC->GetObjectNumberInQueue()); }

//! State callback
public:
	CFxAPC* _pAPC;
	virtual Int32 APCCallBack(void* pObject);
		
};

 } //namespace FEF
