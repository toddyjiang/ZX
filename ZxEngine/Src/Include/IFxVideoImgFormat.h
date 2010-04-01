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
   @file	 IFxVideoImgFormat.h
   @brief This is the main interface file for the Video and Image format.

   IFxVideoImgFormat interface contains methods to get and set the Video
   and Image format definition.
   IFxVideoImgFormat interface is given by the IFxMedia::GetFormatInterface
   method with BGR to Y800 subformat parameter.
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxVideoImgFormat;

//!  The IFxVideoImgFormat class is the Video and Image format interface.
class IFxVideoImgFormat {
public:
	/*----------------------------------------------------------------------*//*!
	GetVideoImgProperties().

	The GetVideoImgProperties method retrieves the Video/Image properties
	(Width * Height).

    @param pdwWidth:		Pointer to a variable that receives the
							width of video/Img.
	@param pdwHeight:		Pointer to a variable that receives the
							height of video/Img.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 GetVideoImgProperties(/* [out] */Uint32* pdwWidth,
										/* [out] */Uint32* pdwHeight) PURE;

	/*----------------------------------------------------------------------*//*!
	SetVideoImgProperties().

	The SetVideoImgProperties method sets the Video/Image properties
	(Width * Height).

    @param dwWidth:		Variable that contains the new width of the video/Img.
	@param dwHeight:	Variable that receives the new height of the video/Img.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetVideoImgProperties(/* [in] */Uint32 dwWidth,
										/* [in] */Uint32 dwHeight) PURE;
};

} //namespace FEF
