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
   @file	 IFxRefClock.h
   @brief This is the main interface file for the Fx plugin reference clock.

   IFxRefClock interface allows to a Fx plugin to publish and/or receive a
   reference clock.
   Reference clock can come from the FxEngine and/or any Fx plugin inside it.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxRefClock;

//! The IFxRefClock class is the Fx plugin reference clock interface.
class IFxRefClock {
public:
	
	/*----------------------------------------------------------------------*//*!
	FxReleaseInterface().

	The FxReleaseInterface method releases the IFxRefClock interface.
	See FxGetInterface method of IFx interface.
    @param None.

	@return	The new reference count.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxReleaseInterface() PURE;

	/*----------------------------------------------------------------------*//*!
	SetFxRefClock().

	The SetFxRefClock method publishes a reference clock.
    @param qRefClock:		Variable that contains the reference clock to
							publish.
	@param dwId:			Variable that contains the Fx reference clock ID.
	
	
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetFxRefClock(/* [in] */Uint64 qRefClock,
							 /* [in] */Uint32 dwId) PURE;

	/*----------------------------------------------------------------------*//*!
	GetFxRefClock().

	The GetFxRefClock method gets the latest reference clock sent by a Fx.
    @param pqRefClock:		Variable that receives the reference clock.
							Can be null if no reference clock exists.
	@param dwId:			Variable that contains the Fx reference clock ID.
	
	
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxRefClock(/* [out] */Uint64* pqRefClock,
							   /* [in] */Uint32 dwId) PURE;

	/*----------------------------------------------------------------------*//*!
	GetFxEngineRefClock().

	The GetFxEngineRefClock method gets the latest reference of the Fx Engine clock.
    @param qRefClock:		Variable that receives the reference clock.
							Can be null if no reference clock exists.
	
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxEngineRefClock(/* [out] */Uint64* pqRefClock) PURE;
};

} //namespace FEF
