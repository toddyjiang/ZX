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
   @file	 IFxPinManager.h
   @brief This is the main interface file for the Fx plugin pin manager.

   IFxPinManager interface contains methods to create and to release
   Fx plugin pins.
   IFxPinManager interface is given by the IFx::FxGetInterface method
   with IFX_PINMANGER parameter.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxPinManager;
class IFxPin;
class IFxMedia;
class IFxPinCallback;

//! Pin definitions structure
typedef struct _FX_PIN {
    std::string	        strPinName;	                    //!< Specifies the pin name.
	FX_PIN_TYPE			PinType;						//!< Specifies the type of pin.
    PFX_MEDIA_TYPE		pMediaTypes;					//!< Specifies the Fx medias of pin.
	Uint16				wMediaTypeCount;				//!< Specifies the number Fx medias that contains pMediaTypes.
	IFxPinCallback*		pIFxPinCallBack;				//!< Specifies the IFxPinCallback interface of pin.
} FX_PIN, *PFX_PIN;

//! The IFxPinManager class is the Fx plugin pin manager interface.
class IFxPinManager {
public:
	/*----------------------------------------------------------------------*//*!
	Create().

	The Create method creates a new Fx pin.
    Fx can create at any moment a new pin. If the pin is created outside the
    IFxBase::Init method, the FX_PIN_UPDATE state must be sent.

    @param pPinInfo:		Pointer to a variable that contains the
							FX_PIN structure (see FX_PIN).

	@param ppFxPin:			Address of a variable that receives a pointer
							to an IFxPin interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 Create(	/* [in] */PFX_PIN pPinInfo,
							/* [out] */IFxPin** ppFxPin) PURE;

    /*----------------------------------------------------------------------*//*!
	Remove().

	The Remove method removes an existing Fx pin.

    Fx can remove at any moment an existing  pin. If the pin is removed outside the
    IFxBase::Init method, the FX_PIN_UPDATE state must be sent.

    @param ppFxPin:			Address of a variable that receives a pointer
							to an IFxPin interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 Remove(	/* [in] */IFxPin* pFxPin) PURE;

	/*----------------------------------------------------------------------*//*!
	FxReleaseInterface().

	The FxReleaseInterface method releases the IFxPinManager interface.
	See IFx::FxGetInterface method.
    @param None.

	@return	The new reference count.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxReleaseInterface() PURE;
};

} //namespace FEF
