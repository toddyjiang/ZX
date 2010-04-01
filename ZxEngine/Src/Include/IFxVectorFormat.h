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
   @file	 IFxVectorFormat.h
   @brief This is the main interface file for the Vector format.

   IFxVectorFormat interface contains methods to get and set the vector format
   definition.
   IFxVectorFormat interface is given by the IFxMedia::GetFormatInterface
   method with VECTOR subformat parameter.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxVectorFormat;

//!  The IFxVectorFormat class is the Vector format interface.
class IFxVectorFormat {
public:
	/*----------------------------------------------------------------------*//*!
	GetUnitType().

	The GetUnitType method retrieves the Unit type of the Vector object
	(see FX_UNIT_TYPE).

    @param pUnitType:		Pointer to a variable that receives the
							Type of the vector component.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetUnitType(/* [out] */FX_UNIT_TYPE* pUnitType) PURE;

	/*----------------------------------------------------------------------*//*!
	SetUnitType().

	The SetUnitType method sets the Vector Unit type (see FX_UNIT_TYPE).

    @param UnitType:		Variable that contains the type of the Vector component.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetUnitType(/* [in] */FX_UNIT_TYPE UnitType) PURE;

	/*----------------------------------------------------------------------*//*!
	GetVectorProperty().

	The GetVectorProperty method retrieves the vector component number (M).

    @param pdwM:		Pointer to a variable that receives the vector
						component number.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetVectorProperties(/* [out] */Uint32* pdwM) PURE;

	/*----------------------------------------------------------------------*//*!
	SetVectorProperty().

	The SetVectorProperty method sets the vector component number (M).

    @param dwM:		Variable that contains the new vector component number.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetVectorProperties(/* [in] */Uint32 dwM) PURE;
};

} //namespace FEF
