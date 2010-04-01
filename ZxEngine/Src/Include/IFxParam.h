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
   @file	 IFxParam.h
   @brief This is the main interface file for the Fx plugin parameters.

   IFxParam interface allows to a Fx plugin to publish several parameters
   to be read and written.
   The Fx plugin property page can use this interface.
   IFxParam interface is given by the IFxMedia::GetFormatInterface
   method with IFX_PARAM parameter.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxParam;

//! The IFxParam class is the Fx plugin parameter interface.
class IFxParam {
public:
	
	/*----------------------------------------------------------------------*//*!
	AddFxParam().

	The AddFxParam method adds a parameter to the Fx plugin.
    @param pFxParam:		Pointer to a variable that contains the
							FX_PARAM structure (see FX_PARAM).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 AddFxParam(/* [in] */const PFX_PARAM pFxParam) PURE;

    /*----------------------------------------------------------------------*//*!
	AddFxParam.

	The AddFxParam method adds a string parameter to the Fx plugin.
    @param pFxParam:		Pointer to a variable that contains the
							FX_PARAM_STRING structure (see FX_PARAM_STRING).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 AddFxParam(/* [in] */const PFX_PARAM_STRING pFxParam) PURE;

    /*----------------------------------------------------------------------*//*!
	RemoveFxParam().

	The RemoveFxParam method removes a Fx parameter.
    @param strParamName:	Variable that contains the parameter name.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 RemoveFxParam(/* [in] */const std::string strParamName) PURE;

	/*----------------------------------------------------------------------*//*!
	FxReleaseInterface().

	The FxReleaseInterface method releases the IFxParam interface.
	See FxGetInterface method of IFx interface.
    @param None.

	@return	The new reference count.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 FxReleaseInterface() PURE;

	/*----------------------------------------------------------------------*//*!
	SetFxParamValue().

	The SetFxParamValue method sets the value of a Fx parameter.
    @param strParamName:	Variable that contains the parameter name.
	@param pvParamValue:	Pointer to a variable that contains the
							new parameter value. Variable must be allocated 
                            according to its type and its number.

    @return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetFxParamValue(	/* [in] */const std::string strParamName,
									/* [in] */const Void* pvParamValue) PURE;

    /*----------------------------------------------------------------------*//*!
	SetFxParamValue().

	The SetFxParamValue method sets the value of a Fx string parameter.
    @param strParamName:	Variable that contains the parameter name.
	@param strParamValue:	Variable that contains the new parameter value.
                      
    Use this method to set string parameter only.
	
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetFxParamValue(	/* [in] */const std::string strParamName,
									/* [in] */const std::string strParamValue) PURE;

	/*----------------------------------------------------------------------*//*!
	GetFxParamValue().

	The GetFxParamValue method gets the value of a Fx plugin parameter.
    @param strParamName:	Variable that contains the parameter name.
	@param pvParamValue:	Pointer to a variable that receive the
							parameter value. Variable must be allocated 
                            according to its type and its number.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 GetFxParamValue(	/* [in] */const std::string strParamName,
									/* [out] */Void* pvParamValue) PURE;

    /*----------------------------------------------------------------------*//*!
	GetFxParamValue().

	The GetFxParamValue method gets the value of a Fx string parameter.
    @param strParamName:	Variable that contains the parameter name.
	@param strParamValue:	Reference to a variable that receive the
							parameter value. 

    Use this method to get string parameter only.
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 GetFxParamValue(	/* [in] */const std::string strParamName,
									/* [out] */std::string& strParamValue) PURE;

	/*----------------------------------------------------------------------*//*!
	GetFxParamCount().

	The GetFxParamCount method gets the number of Fx parameters.
    @param pwParamCount:	Pointer to a variable that receive the
							number of parameters.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxParamCount(/* [out] */Uint16* pwParamCount) PURE;

    /*----------------------------------------------------------------------*//*!
	GetFxParamStringCount().

	The GetFxParamStringCount method gets the number of String Fx parameters.
    @param pwParamCount:	Pointer to a variable that receive the
							number of parameters.

    Use this method to get string parameter only.
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxParamStringCount(/* [out] */Uint16* pwParamCount) PURE;

	/*----------------------------------------------------------------------*//*!
	GetFxParam().

	The GetFxParam method gets a Fx plugin parameter by its index.
    @param ppFxParam:		Pointer to a variable that receives the FX_PARAM structure.
	@param wFxParamIndex:	Index of the parameter to get. From parameters number - 1.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxParam(	/* [out] */const FX_PARAM** ppFxParam,
								/* [in] */const Uint16 wFxParamIndex) PURE;

    /*----------------------------------------------------------------------*//*!
	GetFxParam().

	The GetFxParam method gets a Fx plugin parameter by its index.
    @param ppFxParam:		Pointer to a variable that receives the FX_PARAM_STRING structure.
	@param wFxParamIndex:	Index of the parameter to get. From string parameters number - 1.

    Use this method to get string parameter only.
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxParam(	/* [out] */const FX_PARAM_STRING** ppFxParam,
								/* [in] */const Uint16 wFxParamIndex) PURE;
};

} //namespace FEF
