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
   @file	 IFxPinCallback.h
   @brief This is the main interface file for the Fx plugin pin callback.

   IFxPinCallback is a callback interface for Fx pins. This interface is used
   by each Fx pin to receive IFxMedia data and control data.
   User implements the IFxPinCallback methods if necessary and According to 
   the pin type. 
   Several Pins can use the same IFxPinCallback interface.
*//*_______________________________________________________________________*/
#pragma once
//! FxEngine Framework definitions
#include "FxDef.h"

namespace FEF {

//! FxEngine interfaces used
class IFxPinCallback;
class IFxPin;
class IFxMedia;

//!  The IFxPinCallback class is the Fx plugin pin callback interface.
class FXENGINE_EXP IFxPinCallback
{
public:
	/*----------------------------------------------------------------------*//*!
								IN / OUT pins methods
	*//*-----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*//*!
	FxPinState().

	Implement the FxPinState method to receive the state of an input/output pin.
	WARNING: This method doesn't have to be blocking !!.

    @param pFxPin:		Address of an IFxPin that receives the state.
	@param PinState:	The state of pFxPin (see FX_PIN_STATE).
		       
	@return	None.
    *//*-----------------------------------------------------------------------*/
	virtual Void FxPinState(	/* [in] */IFxPin* pFxPin ,
								/* [in] */FX_PIN_STATE PinState);    

	/*----------------------------------------------------------------------*//*!
	FxMedia().

	Implement the FxMedia method if you want to decide the media buffer
	properties, otherwise the media buffer properties are defined by the default
	values: size = 0x2800 (10Ko) and number = 0x14 (20).

	During the pins connection, each pin gives the media buffer properties
	that they need.
	The finite pool is created with the maximum of both properties.  
	When two pins are connected, they share the finite pool of media buffer.
	Each pin calls the IFxPin::GetDeliverMedia to obtain a free media buffer
	to use.

	If one of both returned values is 0, it is replaced by its default value. 

	@param pFxPin:		Address of the out pin which decides the FxMedia
						properties.
	@param pdwFxMediaSize:	Pointer to a variable that receives the size
						of each media buffer in bytes.
						
	@param pdwFxMediaNumber:Pointer to a variable that receives the Number
						of media buffers. 
						
		       
	@return	None.
    *//*-----------------------------------------------------------------------*/
	virtual Void FxMedia(/* [in] */IFxPin* pFxPin,
						  /* [out] */Uint32* pdwFxMediaSize,
						  /* [out] */Uint32* pdwFxMediaNumber);


	/*----------------------------------------------------------------------*//*!
								IN pins methods
	*//*-----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*//*!
	FxPin().

	Implement the FxPin method if you want to receive new Fx Media on the input
	pin (see IFxPin::DeliverMedia method).
	This callback can rejects the Fx Media, returns immediately and processes the
	Fx Media in a thread or processes the Fx media before returning.
	WARNING: This method doesn't have to be blocking !!.
	On an Output pin, FxPin method shall return NOT_IMPLEMENTED or is not
	implemented.

    @param pFxPin:		Address of the input pin that contains the new IFxMedia.
	@param pIFxMedia:	Address of an IFxMedia that contains the new Fx Media. This
						object doesn't have to be modified. Call GetDeliveryMedia
						method from the IFxPin interface to get free IFxMedia to fill.
	@param StreamState:	Variable that contains the stream state.
						STREAM_PROCESS: pin must process incomming data.
							In this case, the pIFxMedia cannot be null.
					 	STREAM_INIT: new stream Media format.
							In this case, the pIFxMedia cannot be null.
						STREAM_FLUSH: the pin must be flush.
							 In this case, the pIFxMedia is null.
						Each Fx has to propagate the STREAM_INIT and STREAM_FLUSH
						on output Fx pins, respectivly with InitStream() and Flush() methods.
	       
	@return:	FX_OK if success,
				FX_REPEATFXMEDIA if the FxMedia must be repeated,
				otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 FxPin(/* [in] */IFxPin* pFxPin,
						/* [in] */IFxMedia* pIFxMedia,
						/* [in] */FX_STREAM_STATE StreamState = STREAM_PROCESS);

	/*----------------------------------------------------------------------*//*!
								OUT pins methods
	*//*-----------------------------------------------------------------------*/
	/*----------------------------------------------------------------------*//*!
	FxWaitForIFxMedia().

	Implement the FxWaitForIFxMedia method if you want to receive the IFxMedia request
	order on your outputPin (see IFxPin::WaitForIFxMedia method).
	The callback can returns immediately and performs a IFxMedia sending in a thread or
	performs a IFxMedia sending before returning (see IFxPin::DeliverMedia method).
	WARNING: This method doesn't have to be blocking !!.
	On an Input pin, FxWaitForIFxMedia method shall return NOT_IMPLEMENTED
	or is not implemented.

    @param pFxPin:		Address of the out pin that receives the order.
	@param dwTimeStamp:	Variable that contains the request TimeStamp.
						If dwTimeStamp is 0, the Fx chooses the data size to send. 
	@param dwUser:		Variable that contains the user extra parameter. 
						
		       
	@return	None.
    *//*-----------------------------------------------------------------------*/
	virtual void FxWaitForIFxMedia(/* [in] */IFxPin* pFxPin,
									/* [in] */Uint32 dwTimeStamp,
									/* [in] */FX_PTR dwUser);
};

} //namespace FEF


