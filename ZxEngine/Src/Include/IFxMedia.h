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
   @file	 IFxMedia.h
   @brief This is the main interface file for the IFxMedia interface.

   IFxMedia interface contains methods to manage properties on FxEngine data.
   A FxEngine data is a memory object that contains a block of data.
   Some IFxMedia objects support the GetFormatInterface method to get format
   properties (see IFxPcmFormat, IFxVideoImgFormat, IFxVectorFormat and
   IfxMatrixFormat interfaces).
*//*_______________________________________________________________________*/
#pragma once

//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxMedia;
class IFxPcmFormat;
class IFxVectorFormat;
class IFxMatrixFormat;
class IFxVideoImgFormat;


//!  The IFxMedia class is the FxMedia interface.
class IFxMedia {
public:
	/*----------------------------------------------------------------------*//*!
	Copy().

	The Copy method enables you to copy FxMedia data and properties.

    @param pIFxMedia:		Pointer to a variable that contains the
							FxMedia to copy.
	       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 Copy(IFxMedia* const pIFxMedia) PURE;

	/*----------------------------------------------------------------------*//*!
	CheckMediaType().

	The CheckMediaType method determines if the FxEngine data matchs to a specific
	FxMedia type.

    @param pMediaType:		Pointer to a variable that contains the
							FX_MEDIA_TYPE structure to compare (see FX_MEDIA_TYPE).
	       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 CheckMediaType(/* [in] */PFX_MEDIA_TYPE pMediaType) PURE;

	/*----------------------------------------------------------------------*//*!
	SetMediaType().

	The SetMediaType method sets a FxMedia type of the FxMedia data.

    @param pMediaType:		Pointer to a variable that contains the new 
							FX_MEDIA_TYPE structure to set (see FX_MEDIA_TYPE).
	       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetMediaType(/* [in] */PFX_MEDIA_TYPE pMediaType) PURE;	

	/*----------------------------------------------------------------------*//*!
	GetMediaType().

	The GetMediaType method gets a FxMedia type of the FxMedia data.

    @param pMediaType:		Pointer to a variable that receives the  
							FX_MEDIA_TYPE structure (see FX_MEDIA_TYPE).
	       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetMediaType(/* [out] */PFX_MEDIA_TYPE pMediaType) PURE;

	/*----------------------------------------------------------------------*//*!
	GetFormatInterface().

	The GetFormatInterface method gets a specific Format interface on a FxEngine
	data object. 
	FEF provides several format interfaces: IFxPcmFormat for audio PCM data
	(PCM sub-type), IFxVideoImgFormat for Video and Image data
	(BGR to Y800 sub-types), IFxVectorFormat for vector (VECTOR sub-type),
	and IFxMatrixFormat for Matrix (MATRIX sub-type).
	Please contact SMProcess for additional interfaces or additional media sub-types.


    @param SubMediaType:	Variable that contains the FX_SUB_MEDIA_TYPE 
							interface type to get (see FX_SUB_MEDIA_TYPE).

	@param ppFormatInterface:	Address of a variable that receives a pointer
							to a Format interface (see FX_SUB_MEDIA_TYPE).
	       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFormatInterface(	/* [in] */FX_SUB_MEDIA_TYPE SubMediaType,
										/* [out] */Void** ppFormatInterface) PURE;

	/*----------------------------------------------------------------------*//*!
	GetDataLength().

	The GetDataLength method retrieves the length of the valid data in the
	FxMedia data.

    @param pdwLength:	Pointer to a variable that receives the data size.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetDataLength(/* [out] */Uint32* pdwLength) PURE; 

	/*----------------------------------------------------------------------*//*!
	SetDataLength().

	The SetDataLength method sets the length of the valid data in the
	FxMedia data.

    @param dwLength:	Variable that contains the data size.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetDataLength(/* [in] */Uint32 dwLength) PURE; 
	
	/*----------------------------------------------------------------------*//*!
	GetMediaPointer().

	The GetMediaPointer method retrieves a read/write pointer to the current
	block of data. 

    @param ppbMediaData:	Address of a variable that receives a pointer
							to a buffer's memory.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetMediaPointer(/* [out] */Uint8** ppbMediaData) PURE; 

	/*----------------------------------------------------------------------*//*!
	GetSize().

	The GetSize method retrieves the size of the current block in byte.
	(see IFxPinCallback::FxMedia method). 

    @param pdwSize:	Pointer to a variable that receives the data size.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetSize(/* [out] */Uint32* pdwSize) PURE; 

	/*----------------------------------------------------------------------*//*!
	SetSize().

	The SetSize method allows to re-allocate the memory block. Use this method
	when it's necessary to allocate bigger memory than allocation done in
	IFxPinCallback::FxMedia method. Because this method decreases the
	Fx performance, try to allocate the right memory in
	IFxPinCallback::FxMedia method.

    @param dwSize:	Variable that contains the block size in byte.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetSize(/* [in] */Uint32 dwSize) PURE; 
	
	/*----------------------------------------------------------------------*//*!
	Release().

	The Release method releases an IFxMedia object given by the
	IFxPin::GetDeliveryMedia method and which will be not delivered.

	@param None.
	       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 Release() PURE;

	/*----------------------------------------------------------------------*//*!
	GetTimeStamp().

	The GetTimeStamp method retrieves the current TimeStamp of the
	FxMedia data.
	The TimeStamp value depends of the IFxMedia data. For example, it can be
	a packet number	or a sample number.

    @param pqTimeStamp:	Pointer to a variable that receives the time stamp.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetTimeStamp(/* [out] */Uint64* pqTimeStamp) PURE;

	/*----------------------------------------------------------------------*//*!
	SetTimeStamp().

	The SetTimeStamp method sets the current TimeStamp to the
	FxMedia data.
	The TimeStamp value depends of the IFxMedia data.
	
    @param qTimeStamp:	Variable that contains the new time stamp to set.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetTimeStamp(/* [in] */Uint64 qTimeStamp) PURE;

	/*----------------------------------------------------------------------*//*!
	GetMediaMarker().

	The GetMediaMarker method gets the MediaMarker of the FxMedia data.
	
    @param pMediaMarker:	Pointer to a variable that receives the MediaMarker.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetMediaMarker(/* [out] */FX_MEDIA_MARKER* pMediaMarker) PURE;

	/*----------------------------------------------------------------------*//*!
	SetMediaMarker().

	The SetMediaMarker method sets the MediaMarker to the FxMedia data.
	(see MEDIA_MARKER).

    @param MediaMarker:	Variable that contains the MediaMarker to set.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetMediaMarker(/* [in] */FX_MEDIA_MARKER MediaMarker) PURE;

	/*----------------------------------------------------------------------*//*!
	SetUserParams().

	The SetUserParams method sets the User parameters to the FxMedia data.
	
	@param dwUserParam1:	Variable that contains the first FX_PTR value to set.
    @param dwUserParam2:	Variable that contains the second FX_PTR value to set.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 SetUserParams(/* [in] */FX_PTR dwUserParam1,
								/* [in] */FX_PTR dwUserParam2) PURE;

	/*----------------------------------------------------------------------*//*!
	GetUserParams().

	The GetUserParams method gets the User parameters of the FxMedia data.
	
	@param pdwUserParam1:	Pointer to a variable that receives the first FX_PTR value.
    @param pdwUserParam2:	Pointer to a variable that receives the second FX_PTR value.
							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetUserParams(/* [out] */FX_PTR* pdwUserParam1,
								/* [out] */FX_PTR* pdwUserParam2) PURE;

	/*----------------------------------------------------------------------*//*!
	SetFxMediaName().

	The SetFxMediaName method sets the FxMedia name.
	
	@param strFxMediaName:	Variable that contains the FxMedia name.
    						
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 SetFxMediaName(/* [in] */const std::string strFxMediaName) PURE;
	
	/*----------------------------------------------------------------------*//*!
	GetFxMediaName().

	The GetFxMediaName method retrieves the FxMedia name.
	
	@param strFxMediaName:	Reference on variable that receives the FxMedia name.
   							
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxMediaName(/* [out] */std::string& strFxMediaName) PURE;
};

} //namespace FEF
