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
   @file	 FxState.h
   @brief This is the main interface file for the Fx plugin State interface.

   IFxState interface allows to a Fx plugin to mail message with
   FxEngine application.
*//*_______________________________________________________________________*/
#pragma once
//! FxEngine Framework definitions
#include "FxDef.h"
#include "IFxState.h"
#include "State.h"
#include "FxConst2String/FxConst2String.h"

namespace FEF {

//! FxEngine interfaces
class CFxState;
class CFxLog;

//! The IFxState class is the Fx plugin State interface.
class CFxState :	public IFxState,
					public CFxConst2String{

public:
	CFxState(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance);
	virtual ~CFxState();

public:

	/*----------------------------------------------------------------------*//*!
	AttachFxObserver().

	The AttachFxObserver method attaches a Fx state observer to the Fx.

    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 AttachFxObserver(/* [in] */CState* pStateObjt);

	/*----------------------------------------------------------------------*//*!
	DetachFxObserver().

	The DetachFxObserver method detaches a Fx state observer to the Fx.

    @param None.


	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 DetachFxObserver();

	/*----------------------------------------------------------------------*//*!
	FxPublishState().

	The FxPublishState method sends a Fx state to its observer.
    @param FxState:		Variable that contain the Fx State to publish.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxPublishState( /* [in] */FX_STATE FxState);

	virtual Int32 FxGetState( /* [out] */FX_STATE *pFxState);

	virtual Int32 FxRePublishState() {return FxPublishState(_LastFxState);}

	std::string GetFxStringState(/* [in] */FX_STATE FxState);

private:
	//! Critical Section
	boost::mutex _CS;

private:
	CState*				_pStateObjt;

private:
	FX_STATE			_LastFxState;

private:
	std::string			_strFxName;

private:
	std::string _FxLogFile;
	CFxLog* _pLogTrace;

	// Instance value to manage the right log messages.
	Uint32			_dwThisInstance;		// current instance
};

} //namespace FEF

