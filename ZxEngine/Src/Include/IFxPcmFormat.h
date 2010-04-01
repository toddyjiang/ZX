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
   @file	 IFxPcmFormat.h
   @brief This is the main interface file for the PCM format.

   IFxPcmFormat interface contains methods to get and set the PCM format
   definition.
   IFxPcmFormat interface is given by the IFxMedia::GetFormatInterface
   method with PCM sub format parameter.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxPcmFormat;

//!  The IFxPcmFormat class is the PCM format interface.
class IFxPcmFormat {
public:
	/*----------------------------------------------------------------------*//*!
	GetPcmFormat().

	The GetPcmFormat method retrieves the PCM format of the FxMedia.
    @param pPcmFormat:		Pointer to a variable that receives the
							PFX_PCM_FORMAT structure (see FX_PCM_FORMAT).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetPcmFormat(/* [out] */PFX_PCM_FORMAT pPcmFormat) PURE;

	/*----------------------------------------------------------------------*//*!
	SetPcmFormat().

	The SetPcmFormat method sets the PCM format of the FxMedia.
    @param pPcmFormat:		Pointer to a variable that contains the
							PFX_PCM_FORMAT structure (see FX_PCM_FORMAT).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetPcmFormat(/* [in] */PFX_PCM_FORMAT pPcmFormat) PURE;

	/*----------------------------------------------------------------------*//*!
	GetBitsPerSample().

	The GetBitsPerSample method returns the number of bits per sample for
	the format type specified by FormatTag.
    @param pwBitsPerSample:		Pointer to a variable that receives the
								number of bits per sample.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetBitsPerSample(/* [out] */Uint16* pwBitsPerSample) PURE;
};

} //namespace FEF
