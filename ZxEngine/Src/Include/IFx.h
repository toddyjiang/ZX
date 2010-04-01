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
   @file	 IFx.h
   @brief This is the main interface for the FXs.

   IFx interface contains methods to manage each Fx.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"
#include "IFxState.h"
#include "IFxParam.h"
#include "IFxRefClock.h"
#include "IFxMedia.h"
#include "IFxPinManager.h"
#include "IFxPinCallback.h"
#include "IFxPin.h"
#include "IFxPcmFormat.h"
#include "IFxMatrixFormat.h"
#include "IFxVectorFormat.h"
#include "IFxVideoImgFormat.h"

namespace FEF {

//! FxEngine interface
class IFx;

//!  The IFx class is the main Fx plugin interface.
class IFx {
public:
	/*----------------------------------------------------------------------*//*!
	GetIFxVersion().

	The GetIFxVersion method gets the IFx interface version.
    @param pwMajor:		Pointer to a variable that receives the Major of IFx version.
	@param pwMinor:		Pointer to a variable that receives the Minor of IFx version.
	@param pwBuild:		Pointer to a variable that receives the Build of IFx version.
	@param pwRev:		Pointer to a variable that receives the Revision of IFx version.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetIFxVersion(/* [out] */Uint16* pwMajor,
								/* [out] */Uint16* pwMinor,
								/* [out] */Uint16* pwBuild,
								/* [out] */Uint16* pwRev) PURE;

	/*----------------------------------------------------------------------*//*!
	FxGetInterface().

	The FxGetInterface method retrieves a specific interface of Fx.

    @param FxInterfaceType:		Variable that contains the Fx Interface to get
								(see FX_INTERFACE).
	@param ppFxInterface:		Address of a variable that receives a pointer
									to the interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxGetInterface(	/* [in] */FX_INTERFACE FxInterfaceType,
									/* [out] */Void** ppFxInterface) PURE;

	/*----------------------------------------------------------------------*//*!
	Release().

	The Release method releases the IFx interface returned by the FEF_CreateFx function.
	WARNING: Release all other interfaces (returned by the FxGetInterface method)
	before calling it.

    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 Release() PURE;

	/*----------------------------------------------------------------------*//*!
	GetConstToString().

	The GetFxConst2String function converts a FxEngine constant to a string.
	@param FxEngineConstType:	Variable that contains the Type of the constant.
	@param sdwFxEngineConst:	Variable that contains the constant to convert.
	@param strStateName:		Reference to a variable that receives the FxEngine
                                constant name.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetConstToString(/* [in] */FXENGINE_CONST_TYPE FxEngineConstType,
								 /* [in] */Int32 sdwFxEngineConst,
								 /* [out] */std::string& strStateName) PURE;
};
 } //namespace FEF

#ifdef __cplusplus
extern "C" {
#endif
namespace FEF {
/*----------------------------------------------------------------------*//*!
FEF_CreateFx().

The FEF_CreateFx method obtains an IFx interface.
Call IFx::Release method to release it.

@param ppFx:		Address of a variable that receives a pointer
								to the IFx interface created.
@param strFxName:	Variable that contains the Fx short name.

@return	FX_OK if success, otherwise an FX error code.
*//*-----------------------------------------------------------------------*/
FXENGINE_EXP Int32 FXENGINE_API FEF_CreateFx(/* [out] */IFx ** ppFx,
										 /* [in] */const std::string strFxName);
 } //namespace FEF
#ifdef __cplusplus
}
#endif

