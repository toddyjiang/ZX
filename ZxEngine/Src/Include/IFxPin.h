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
   @file	 IFxPin.h
   @brief This is the main interface file for the Fx pin.

   IFxPin interface contains methods to control the Fx pin.
   Fx plugin can contain one or several pin of any media types.
*//*_________________________________________________________________________*/
#pragma once
//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces
class IFxPin;
class IFxMedia;
class IFxPinCallback;


//!  The IFxPin class is the Fx plugin pin interface.
class IFxPin {
public:
	/*----------------------------------------------------------------------*//*!
								In/Out Pin methods
	*//*-----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*//*!
	GetPinName().

	The GetPinName method gets the Fx pin name.
    @param strPinName:		Reference to a variable that receives 
                            the Pin name.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 GetPinName(/* [out] */std::string& strPinName) PURE;

	/*----------------------------------------------------------------------*//*!
	GetPinType().

	The GetPinType method gets the Fx pin type.
    @param pPinType:		Pointer to a variable that receives the
							pin type (see FX_PIN_TYPE).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetPinType(/* [out] */FX_PIN_TYPE* pPinType) PURE;

	/*----------------------------------------------------------------------*//*!
	GetPinState().

	The GetPinState method gets the Fx pin state.
    @param pPinState:		Pointer to a variable that receives the
							pin type (see FX_PIN_STATE).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetPinState(/* [out] */FX_PIN_STATE* pPinState) PURE;

	/*----------------------------------------------------------------------*//*!
	GetTxRxBytes().

	The GetTxRxBytes method gets the amount of bytes received or transmitted since 
	the Fx pin is connected.
    @param pqTxRxByte:		Pointer to a variable that receives the
							amount of bytes.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetTxRxBytes(/* [out] */Uint64* pqTxRxByte) PURE;

	/*----------------------------------------------------------------------*//*!
	GetMediaTypeCount().

	The GetMediaTypeCount method gets the Pin MediaType count.
    @param pwMediaTypeCount:	Pointer to a variable that receives the
								Pin MediaType count.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetMediaTypeCount(Uint16* pwMediaTypeCount) PURE;

	/*----------------------------------------------------------------------*//*!
	GetMediaType().

	The GetMediaType method gets the Pin MediaType by its index.
	@param pMediaType:		Pointer to a FX_MEDIA_TYPE variable that receives media
							type.
	@param wFxIndex:		Index of the Pin MediaType to get. From 0 to N-1.
							N is given by the GetMediaTypeCount function.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetMediaType(PFX_MEDIA_TYPE pMediaType, Uint16 wMediaTypeIndex) PURE;

	/*----------------------------------------------------------------------*//*!
	GetPinConnected().

	The GetPinConnected method gets the Fx pin which is connected to it.
    @param ppFxPin:		Address of a variable that receives a pointer
							to an IFxPin interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetPinConnected(/* [out] */IFxPin** ppFxPin) PURE;

	/*----------------------------------------------------------------------*//*!
	GetConnectionMediaType().

	The GetConnectionMediaType method gets current MediaType of the connected pin.
	If the pin is not connected, defaults values are filled (see MAIN_TYPE_UNDEFINED
	and SUB_TYPE_UNDEFINED).
    @param pMediaType:		Pointer to a FX_MEDIA_TYPE structure that receives the
							connected media type.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetConnectionMediaType(/* [out] */PFX_MEDIA_TYPE pMediaType) PURE;

	/*----------------------------------------------------------------------*//*!
	DeliverMedia().

	The DeliverMedia method delivers a media sample to the connected
	input/output pin. STREAM_PROCESS state will be set.
	see (IFxPinCallback::FxPin).
    
    @param pIFxMedia:		Pointer to a IFxMedia interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 DeliverMedia(/* [in] */IFxMedia* pIFxMedia) PURE;

    /*----------------------------------------------------------------------*//*!
	InitStream().

	The InitStream method delivers a media sample to the connected
	output pin (see IFxPinCallback::FxPin).
    In practice, this method is called at the beginning of stream to
    initialize FXs in chain even if the next FXs in chain are stopped.
    It can also called when the stream format is changing.
    STREAM_INIT state will be set.
	see (IFxPinCallback::FxPin).

    @param pIFxMedia:		Pointer to a IFxMedia interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 InitStream(/* [in] */IFxMedia* pIFxMedia) PURE;

	/*----------------------------------------------------------------------*//*!
	GetFreeMediaNumber().

	The GetFreeMediaNumber method retrieves the free Fx Media number.

	@param pdwFreeMediaNumber:	Address of a variable that receives the
								free media number.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFreeMediaNumber(/* [out] */Uint32* pdwFreeMediaNumber) PURE;

	/*----------------------------------------------------------------------*//*!
	GetDeliveryMedia().

	The GetDeliveryMedia method retrieves a free IFxMedia to fill with data.
	The GetDeliveryMedia method returns if the time-out interval elapses,
	or if a free media buffer is available.

	@param ppIFxMedia:		Address of a variable that receives a pointer
							to an IFxMedia interface.
	@param dwTimeOut:		Variable that contains time-out interval in milliseconds.
							If dwTimeOut is equal to zero, the method tries
							to get a free media buffer and returns immediately.
							If dwTimeOut is equal to INFINITE_TIME, the method's time-out
							interval never elapses.


	@return	FX_OK if success,
			FX_TIMEOUT if the time-Out is reached,
			otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetDeliveryMedia(/* [out] */IFxMedia** ppIFxMedia, /* [in] */Uint32 dwTimeOut = 0) PURE;

	/*----------------------------------------------------------------------*//*!
	InitDumpData().

	The InitDumpData method initializes the dump of pin data in a file of an Fx pin.

	@param strFilePath:		Variable that contains the dump file
							path.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 InitDumpData(/* [in] */const std::string strFilePath) PURE;

	/*----------------------------------------------------------------------*//*!
	StartDumpData().

	The StartDumpData method starts the dump of pin data.

	@param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 StartDumpData() PURE;

	/*----------------------------------------------------------------------*//*!
	StopDumptData().

	The StopDumptData method stops the dump of pin data.

	@param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 StopDumptData() PURE;

	/*----------------------------------------------------------------------*//*!
	Flush().

	The Flush method propagates the flush notification. This method is used
	at the end of stream to flush the next FXs in chain.
	Flush methods is implemented by Fx (see IFxPinCallback) on an input pin.
	STREAM_FLUSH state will be set.

	@param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 Flush() PURE;

	/*----------------------------------------------------------------------*//*!
								In Pin methods only
	*//*-----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*//*!
	GetProcessTime().

	The GetProcessTime method gets the processing time of an input pin in ms.

	@param pdwProcessingTime:	Pointer to a variable that receives the	processing time.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetProcessTime(/* [in] */Uint32* pdwProcessingTime) PURE;

	/*----------------------------------------------------------------------*//*!
	WaitForIFxMedia().

	The WaitForIFxMedia method sends an IFxMedia data request to the previous FXs
	in chain.

	@param dwTimeStamp:	Variable that contains the request TimeStamp.
						If dwTimeStamp is 0, the method chooses the request data size.
	@param dwUser:		Variable that contains the user extra parameter.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 WaitForIFxMedia(	/* [in] */Uint32 dwTimeStamp,
									/* [in] */FX_PTR dwUser) PURE;
};

} //namespace FEF
