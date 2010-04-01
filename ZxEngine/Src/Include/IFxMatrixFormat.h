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
   @file	 IFxMatrixFormat.h
   @brief This is the main interface file for the Matrix format.

   IFxMatrixFormat interface contains methods to get and set the matrix format
   properties.
   IFxMatrixFormat interface is given by the IFxMedia::GetFormatInterface
   method with MATRIX subformat parameter.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interface
class IFxMatrixFormat;


//!  The IFxMatrixFormat class is the Matrix format interface.
class IFxMatrixFormat {
public:
	/*----------------------------------------------------------------------*//*!
	GetUnitType().

	The GetUnitType method retrieves the Unit type of the Matrix object
	(see FX_UNIT_TYPE).

    @param pUnitType:		Pointer to a variable that receives the
							Type of matrix component.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 GetUnitType(/* [out] */FX_UNIT_TYPE* pUnitType) PURE;

	/*----------------------------------------------------------------------*//*!
	SetUnitType().

	The SetUnitType method sets the Matrix Unit type (see FX_UNIT_TYPE).

    @param UnitType:		Variable that contains the type of matrix component.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetUnitType(/* [in] */FX_UNIT_TYPE UnitType) PURE;

	/*----------------------------------------------------------------------*//*!
	GetMatrixProperties().

	The GetMatrixProperties method retrieves the matrix properties (N*M).

    @param pdwN:		Pointer to a variable that receives the N parameter
						of the matrix.
	@param pdwM:		Pointer to a variable that receives the M parameter
						of the matrix.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetMatrixProperties(	/* [out] */Uint32* pdwN,
										/* [out] */Uint32* pdwM) PURE;

	/*----------------------------------------------------------------------*//*!
	SetMatrixProperties().

	The SetMatrixProperties method sets the matrix properties (N * M).

    @param dwN:		Variable that contains the new N value.
	@param dwM:		Variable that contains the new M value.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetMatrixProperties(	/* [in] */Uint32 dwN,
										/* [in] */Uint32 dwM) PURE;
};
} //namespace FEF

