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

	Copyrights (c) 2009 SMProcess, all rights reserved.
    __________________________________________________________________________
*//*!
   @file  FxMToS.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/
#include "FxMToS.h"
#include <string.h>

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

//! The Fx names
const Char g_strFxName[] = "Mono to Stereo";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2009)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";


/*-----------------------------------------------------------------------------*//*!
	FEF_GetFxBase()
	The Fx entry point.
*//*-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
FXENGINE_EXP IFxBase* FXENGINE_API FEF_GetFxBase()
{
	return (IFxBase*)(new CFxMToS);
}
#ifdef __cplusplus
}
#endif

CFxMToS::CFxMToS()
{
	/*! Initialize the Fx components */
	_FxState = FX_STOP_STATE;

	_pFxState		= NULL;

	_pFxDescriptor  = NULL;

	_OutPinState	= PIN_NOT_CONNECTED;
	_InPinState		= PIN_NOT_CONNECTED;

	_pFxOutputPin	= NULL;
	_pFxInputPin	= NULL;
	
	FX_PCM_FORMAT FxPcmFmt = {2, FLOAT32_TYPE, 44100};
	_FxPcmFmt  =  FxPcmFmt;
}

CFxMToS::~CFxMToS()
{
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToS::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
{
    if(_pFxDescriptor == NULL) {
        /*! Allocates the Fx description */
        _pFxDescriptor = new FX_DESCRIPTOR;
        if(_pFxDescriptor == NULL)
            return FX_NOMEM;
        
        /*! Describe the Fx */
        _pFxDescriptor->strName = g_strFxName;
        _pFxDescriptor->strVersion = g_strFxVersion;
        _pFxDescriptor->strAuthor = g_strFxAuthor; 
        _pFxDescriptor->strCopyright = g_strFxCopyright;
        _pFxDescriptor->FxType = FX_PROCESS;
        _pFxDescriptor->FxScope = FX_SCOPE_AUDIO;
    }

    *ppFxDescriptor = _pFxDescriptor;
			
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	InitFx()
	Initializes the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToS::InitFx(IFx **ppFxComponent)
{
	Int32 hr = FX_ERROR;

	/*! Create the Fx */
	if(FEF_FAILED(hr = FEF_CreateFx(&(_pFx), g_strFxName)))
		return hr;

	*ppFxComponent = _pFx;

	/*! Get the IFxState and publish the loading state */
	hr = _pFx->FxGetInterface(IFX_STATE, (Void**)&_pFxState);
	if(FEF_FAILED(hr))
		return hr;	

	_pFxState->FxPublishState(_FxState = FX_LOADING_STATE);

	/*! Get the IFxPinManager to create the Fx pins */
	IFxPinManager* pFxPinManager = NULL;
	hr = _pFx->FxGetInterface(IFX_PINMANGER, (Void**)&pFxPinManager);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	FX_PIN _FxPin[] =
    {
		{ 
		    "Mono/Stereo stream"		//!< Pin Name
			, PIN_IN					//!< Pin Type
			, &g_FxMediaType			//!< Media Type
			, 1							//!< Media Type number
			, (IFxPinCallback*)this		//!< Pin Callback
		},
		{	 
		    "Stereo stream"				//!< Pin Name
			, PIN_OUT					//!< Pin Type
			, &g_FxMediaType			//!< Media Type
			, 1							//!< Media Type number
			, (IFxPinCallback*)this		//!< Pin Callback
		}
	};

	/*! Create the Fx pins */
	hr = pFxPinManager->Create(&_FxPin[0], &_pFxInputPin);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	hr = pFxPinManager->Create(&_FxPin[1], &_pFxOutputPin);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! We can release the PinManager */
	SAFE_RELEASE_INTERFACE(pFxPinManager);

	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(_FxState = FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToS::StartFx()
{
	if(_FxState != FX_START_STATE) {
		_pFxState->FxPublishState(_FxState = FX_START_STATE);
	}

    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	RunFx()
	Run the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToS::RunFx()
{
    Int32 hr = FX_OK;
    
    if(_FxState != FX_RUN_STATE) {
        _pFxState->FxPublishState(_FxState = FX_RUN_STATE);
	}

    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToS::StopFx()
{
	if(_FxState != FX_STOP_STATE) {
        _pFxState->FxPublishState(_FxState = FX_STOP_STATE);
	}

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToS::ReleaseFx()
{
	/*! Stop and release Fx components */
	StopFx();

	_pFxState->FxPublishState(_FxState = FX_RELEASE_STATE);	/*!< Update the Fx state to "Release" */

	_pFxState->FxReleaseInterface();
	
	/*! Delete the Fx instance */
	delete this;
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	FxPinState()
	Receives the Fx Pins states.
*//*-----------------------------------------------------------------------------*/
Void CFxMToS::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
{
	if( (_InPinState == PIN_CONNECTED) && (_OutPinState == PIN_CONNECTED) ) {
		if(PinState == PIN_NOT_CONNECTED) {
			_pFxState->FxPublishState(_FxState = FX_DISCONNECT_STATE);
		}
	}

	FX_PIN_TYPE FxPinType;
	pFxPin->GetPinType(&FxPinType);
	if(FxPinType == PIN_IN)
		_InPinState = PinState;
	else {
		_OutPinState = PinState;
		if(PinState == PIN_CONNECTED)
		{
			/*! Get new FxMedia to fill */
			IFxMedia* pIFxMedia;
			if( FEF_FAILED(_pFxOutputPin->GetDeliveryMedia(&pIFxMedia)) ) /*!< The Media type of connection is set !! */  
			{
				_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
				return;
			}

			/*! Get Media PCM format */
			IFxPcmFormat* pIFxPcmFormat;
			if(FEF_FAILED(pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
				/*! Release Media buffer */
				pIFxMedia->Release();
				_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
				return;
			}	
			pIFxMedia->SetDataLength(0);
			/*! Set media properties */
			pIFxPcmFormat->SetPcmFormat(&_FxPcmFmt);
			pIFxMedia->SetFxMediaName("Stream connexion");

			/*! Deliver the FxMedia to the next Fx */
			_pFxOutputPin->InitStream(pIFxMedia);	
		}
	}

	if( (_InPinState == PIN_CONNECTED) && (_OutPinState == PIN_CONNECTED) ) {
        _pFxState->FxPublishState(_FxState = FX_CONNECT_STATE);		
	}
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxMedia()
	Decides the Media buffer size and number.
*//*-----------------------------------------------------------------------------*/
Void CFxMToS::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	FX_PIN_TYPE FxPinType;
	pFxPin->GetPinType(&FxPinType);
	if(FxPinType == PIN_OUT) {
		/*! Define 10 buffers of 50ms (44100, stereo, 32 bits float pcm data) */ 
		*pdwFxMediaNumber = 10;
		*pdwFxMediaSize = (44100 * 4 * 2)/20;
	}
	//!< else return default values
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxPin()
	Receives the new Fx media on input pin.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToS::FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
{
	Int32 hr = FX_OK;
	FX_PCM_FORMAT FxPcmFmt;
	
	/*! have we propagate the flush ? */
	if(StreamState == STREAM_FLUSH) {
		_pFxState->FxPublishState(_FxState = FX_FLUSH_STATE);
		_pFxOutputPin->Flush();
		return hr;
	}

	/*! Check media type */
	if(FEF_FAILED(hr = pIFxMedia->CheckMediaType(&g_FxMediaType))) {
		_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
		return FX_MEDIANOTSUPPORTED;		
	}
	/*! Get Media PCM format */
	IFxPcmFormat* pFxPcmFormat;
	if(FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (Void**)&pFxPcmFormat))) {
		_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
		return hr;
	}
	pFxPcmFormat->GetPcmFormat(&FxPcmFmt);

	/*! have we new format ? */
	if(StreamState == STREAM_INIT) {
		/*! Float audio samples are accepted only */
		if( FxPcmFmt.FormatTag != FLOAT32_TYPE ) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
			return FX_FMTNOTSUPPORTED;
		}
		if( (FxPcmFmt.wChannels != 1) && (FxPcmFmt.wChannels != 2) ) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
			return FX_FMTNOTSUPPORTED;
		}
		
		/*! publish format to the downstream */
		IFxMedia* pDstFxMedia;
		if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pDstFxMedia, INFINITE_TIME)) ) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
			return hr;
		}
		_FxPcmFmt.dwSamplingRate = FxPcmFmt.dwSamplingRate;
		pDstFxMedia->Copy(pIFxMedia);
		IFxPcmFormat* pFxPcmFormat;
		if(FEF_FAILED(hr = pDstFxMedia->GetFormatInterface(PCM, (Void**)&pFxPcmFormat))) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
			return hr;
		}
		pFxPcmFormat->SetPcmFormat(&_FxPcmFmt);
		_pFxOutputPin->InitStream(pDstFxMedia);
		_pFxState->FxPublishState(FX_STREAM_INIT_STATE);
	}
		
	/*! Retrieve the media name */
	std::string strMediaName;
	pIFxMedia->GetFxMediaName(strMediaName);
	
	/*! Loop on input data */
	Uint32 dwInputDatalength;
	pIFxMedia->GetDataLength(&dwInputDatalength);
	
	/*! We suppose that we receive data modulus of sample size */
	if((dwInputDatalength % (4 * FxPcmFmt.wChannels)) > 0) {
		_pFxState->FxPublishState(_FxState = FX_UNDERRUN_STATE);
		dwInputDatalength -= dwInputDatalength % (4 * FxPcmFmt.wChannels);
	}
		
	/*! Get media pointer */
	Uint8 *pDstData, *pSrcData;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaPointer(&pSrcData)) ) {
		_pFxState->FxPublishState(_FxState = FX_UNDERRUN_STATE);
		return hr;
	}
	
	Float *pfSrcData, *pfDstData;
	pfSrcData = ((Float*)pSrcData);
	while( dwInputDatalength > 0 ) {
		/*! Get output data buffer to fill */
		IFxMedia* pDstFxMedia;
		if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pDstFxMedia, INFINITE_TIME)) ) {
    		_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
    		return hr;
		}

		if( FEF_FAILED(hr = pDstFxMedia->GetMediaPointer(&pDstData)) ) {
			/*! Release Media buffer */
			pDstFxMedia->Release();
			_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
			return hr;
		}
		Uint32 dwOutputSize;
		pDstFxMedia->GetSize(&dwOutputSize);
		Uint32 dwDataToProcess;
		if(FxPcmFmt.wChannels == 1)
			dwDataToProcess = ((dwInputDatalength * 2) > dwOutputSize) ?
								dwOutputSize / 2:
								dwInputDatalength;
		else
			dwDataToProcess = ((dwInputDatalength) > dwOutputSize) ?
								dwOutputSize:
								dwInputDatalength;
								
		dwDataToProcess -= dwDataToProcess % (4 * FxPcmFmt.wChannels); //!< Must be modulus of sample
		
		/* transform stream */
		pfDstData = ((Float*)pDstData);
		if(FxPcmFmt.wChannels == 1) {
			Uint32 dwSampleToProcess = dwDataToProcess / 4;
			for(Uint32 Idx = 0; Idx < dwSampleToProcess; Idx++){
				*pfDstData++ = *pfSrcData;
				*pfDstData++ = *pfSrcData++;			
			}
			/*! Update the valid data length */
			pDstFxMedia->SetDataLength(dwDataToProcess * 2);
		}
		else {
			memcpy((void*)pDstData, (void*)pSrcData, dwDataToProcess); //!< Input is already stereo
			/*! Update the valid data length */
			pDstFxMedia->SetDataLength(dwDataToProcess);
		}
		
		dwInputDatalength -= dwDataToProcess;

		/*! Set data properties */
		pDstFxMedia->SetFxMediaName(strMediaName);

		/*! Set Media PCM format */
		IFxPcmFormat* pFxPcmDestFormat;
		if(FEF_FAILED(hr = pDstFxMedia->GetFormatInterface(PCM, (Void**)&pFxPcmDestFormat))) {
			/*! Release Media buffer */
			pDstFxMedia->Release();
			_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
			return hr;
		}
		pFxPcmDestFormat->SetPcmFormat(&_FxPcmFmt);
	
		/*! Deliver the FxMedia to the next Fx */
		_pFxOutputPin->DeliverMedia(pDstFxMedia);
	}
	
	return hr;
}
