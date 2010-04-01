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
   @file	 IFxState.h
   @brief This is the main interface file for the Fx plugin State interface.

   IFxState interface allows to a Fx plugin to publish Fx states to the
   FxEngine application.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! State Callback function
typedef Void (FEF_CALLBACK FXSTATECALLBACK)(FX_STATE FxState, FX_HANDLE hObserverId, FX_PTR dwParam);

//! State Callback object
class FXENGINE_EXP CFxStateCallback {
public:
	virtual Void FxStateCallback(FX_STATE FxState, FX_HANDLE hObserverId, FX_PTR dwParam);
};

//! FxEngine interfaces
class IFxState;

//! The IFxState class is the Fx plugin State interface.
class IFxState {
public:
	/*----------------------------------------------------------------------*//*!
	FxPublishState().

	The FxPublishState method sends a Fx state to observers.
    @param FxState:		Variable that contains the Fx State to publish.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxPublishState( /* [in] */FX_STATE FxState) PURE;

	/*----------------------------------------------------------------------*//*!
	FxRePublishState().

	The FxRePublishState method re-sends the latest Fx state to observers.
    
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxRePublishState() PURE;

	/*----------------------------------------------------------------------*//*!
	FxGetState().

	The FxGetState method retrieves the latest Fx state.
    @param FxState:		Pointer on a variable that receives the Fx State.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxGetState( /* [out] */FX_STATE *pFxState) PURE;

	/*----------------------------------------------------------------------*//*!
	FxReleaseInterface().

	The FxReleaseInterface method releases the IFxState interface.
	See FxGetInterface method of IFx interface.
    @param None.

	@return	The new reference count.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxReleaseInterface() PURE;
};
} //namespace FEF

