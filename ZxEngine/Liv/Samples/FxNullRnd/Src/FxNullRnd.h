/* __________________________________________________________________________

	Fx sample code. 

	This material has been supplied as part of the FxEngine Framework.
	Under copyright laws, this material may not be duplicated in whole
	or in part, except for personal use, without the express written consent
	of SMProcess. Refer to the license agreement contained with this Framework
	before using any part of this material.

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	PURPOSE.

	Email:  info@SMProcess.com

	Copyrights (c) 2007 SMProcess, all rights reserved.

   __________________________________________________________________________
*//*!
   @file  FxNullRnd.h
   @brief This Fx is a null renderer Fx. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/
#include "IFxBase.h"

#pragma warning(disable : 4995)	//!< stdio.h
#pragma warning(disable : 4996) //!< stdlib.h

using namespace FEF; //!< Using FxEngineFramework

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
#define FX_VERSION        ("1.3.0.0")

class CFxNullRnd:public IFxBase, 
			    public IFxPinCallback
{
public:
	//! The IFxBase classe implementation
	virtual Int32 GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor);
	virtual Int32 InitFx(IFx **ppFx);
	virtual Int32 GetFxFrame(const Char** ppbFxFrame);
    virtual Int32 StartFx();
	virtual Int32 StopFx();
	virtual Int32 ReleaseFx();

	//! The IFxPinCallback interface implementation
	virtual Int32 FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState);
	virtual Void FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState);
	virtual Void FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber);
	
private:
	FX_STATE		_FxState;		/*!< Contains the Fx state */
	FX_PIN_STATE	_InPinState;	/*!< Contains the InPin state */
	
    FX_DESCRIPTOR   *_pFxDescriptor;  /*!< Contains the Fx description */
	
	IFx *_pFx;					/*!< The IFx interface */
	IFxParam        *_pFxParam; /*!< The IFxParam interface on Fx */

	IFxState*		_pFxState;	/*!< The IFxState interface on Fx */
    IFxPin*         _pFxInputPin;			/*!< The Fx input pin */	
};

