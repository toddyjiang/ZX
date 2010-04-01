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

	Copyrights (c) 2008 SMProcess, all rights reserved.
    __________________________________________________________________________
*//*!
   @file  FxPcmGain.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/
#include <gtk/gtk.h>
#include "FxPcmGain.h"
#include <math.h>

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

//! The Fx names
const Char g_strFxName[] = "FxPcmGain Demo";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2008)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";

//! Defaults values
Float g_fDefaultGain = -3.0f; //!< -3dB
Float g_fMinGain = -96.0f; //!< -96dB
Float g_fMaxGain = +12.0f; //!< -12dB

//! GTK "delete_event" callback  
gint quit(GtkWidget *widget, GdkEvent *event, gpointer *data) {
	/* Do not close window now */
	gtk_widget_hide(widget);
	return TRUE;
}

//! inline functions
static Float inline GetRatioFromdB(Float fdB, Float fdBMin)
{
	if(fdB <= fdBMin)
		return (0.0f);
	return (powf(10.0f, fdB / 20.0));
}

/*-----------------------------------------------------------------------------*//*!
	FEF_GetFxBase()
	The Fx entry point.
*//*-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
FXENGINE_EXP IFxBase* FXENGINE_API FEF_GetFxBase()
{
	return (IFxBase*)(new CFxPcmGain);
}
#ifdef __cplusplus
}
#endif

CFxPcmGain::CFxPcmGain()
{
	/*! Initialize the Fx components */
	_FxState = FX_STOP_STATE;

	_pFxState		= NULL;

	_pFxParam 		= NULL;

    _pFxDescriptor  = NULL;

	_OutPinState	= PIN_NOT_CONNECTED;
	_InPinState		= PIN_NOT_CONNECTED;

	_pFxOutputPin	= NULL;
	_pFxInputPin	= NULL;

	_IsFormatSet    = FALSE; 
}

CFxPcmGain::~CFxPcmGain()
{
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxPcmGain::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
Int32 CFxPcmGain::InitFx(IFx **ppFxComponent)
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
		    "Input pin"					//!< Pin Name
			, PIN_IN					//!< Pin Type
			, &g_FxMediaType			//!< Media Type
			, 1							//!< Media Type number
			, (IFxPinCallback*)this		//!< Pin Callback
		},
		{	 
		    "Output pin"				//!< Pin Name
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

	/*! Get the IFxParam to create the Fx public parameters */
	hr = _pFx->FxGetInterface(IFX_PARAM, (Void**)&_pFxParam);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! Create the Fx public parameter */
    FX_PARAM FxParamGain = {"Gain", "dB", &g_fDefaultGain, &g_fMinGain, &g_fMaxGain, FLOAT32_TYPE, 1};
	hr = _pFxParam->AddFxParam(&FxParamGain);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	/* Create User interface */
	_pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(_pWindow), "Pcm Gain property");
	gtk_window_set_default_size(GTK_WINDOW(_pWindow), 320, 50);
	gtk_container_set_border_width(GTK_CONTAINER(_pWindow), 4);

	_pMainVBox = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(_pWindow), _pMainVBox);

	_pFrame = gtk_frame_new("Pcm Gain value (dB)");
	_pSlider = gtk_hscale_new_with_range(g_fMinGain, g_fMaxGain, 1); 
	gtk_container_add(GTK_CONTAINER(_pFrame), _pSlider);
	gtk_range_set_value(GTK_RANGE(_pSlider), g_fDefaultGain);
	g_signal_connect(G_OBJECT(_pSlider), "value_changed", G_CALLBACK(CFxPcmGain::SliderWrapper), this);
	gtk_box_pack_start(GTK_BOX(_pMainVBox), _pFrame, FALSE, FALSE, 0);

	g_signal_connect(G_OBJECT(_pWindow), "delete_event", G_CALLBACK(quit), _pWindow);

	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(_FxState = FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	SliderCallback()
	Retrieves the slider value.
*//*-----------------------------------------------------------------------------*/
Void CFxPcmGain::SliderCallback()
{
	float fValue = gtk_range_get_value(GTK_RANGE(_pSlider));
	/* Set Fx parameter */
	_pFxParam->SetFxParamValue("Gain", &fValue);

	return;
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFxParam()
	Update Fx Parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CFxPcmGain::UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter)
{
	/*! Update HMI with the new value */
	float fValue;
	_pFxParam->GetFxParamValue("Gain", &fValue);
	gtk_range_set_value(GTK_RANGE(_pSlider), fValue);
	return FX_OK;  
}

/*-----------------------------------------------------------------------------*//*!
	DisplayFxPropertyPage()
	Displays the Fx property page.
*//*-----------------------------------------------------------------------------*/
Int32 CFxPcmGain::DisplayFxPropertyPage(Pvoid pvWndParent)
{
	gtk_widget_show_all(_pWindow);
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxPcmGain::StartFx()
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
Int32 CFxPcmGain::RunFx()
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
Int32 CFxPcmGain::StopFx()
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
Int32 CFxPcmGain::ReleaseFx()
{
	/*! Stop and release Fx components */
	StopFx();

	_pFxState->FxPublishState(_FxState = FX_RELEASE_STATE);	/*!< Update the Fx state to "Release" */

	_pFxState->FxReleaseInterface();

	/* Quit GTK window */
	gtk_widget_destroy(_pWindow);
	
	/*! Delete the Fx instance */
	delete this;
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	FxPinState()
	Receives the Fx Pins states.
*//*-----------------------------------------------------------------------------*/
Void CFxPcmGain::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
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
		if((PinState == PIN_CONNECTED) && (_IsFormatSet == TRUE))
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
Void CFxPcmGain::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	/*! Define 10 buffers of 50ms (44100, stereo, 32 bits float pcm data) */ 
	*pdwFxMediaNumber = 10;
	*pdwFxMediaSize = (44100 * 4 * 2)/20;
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxPin()
	Receives the new Fx media on input pin.
*//*-----------------------------------------------------------------------------*/
Int32 CFxPcmGain::FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
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
		if( FxPcmFmt.wChannels < 1 ) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
			return FX_FMTNOTSUPPORTED;
		}
		_FxPcmFmt = FxPcmFmt;
		_IsFormatSet = TRUE;
		/*! publish format to the downstream */
		IFxMedia* pDstFxMedia;
		if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pDstFxMedia, INFINITE_TIME)) ) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
			return hr;
		}
		pDstFxMedia->Copy(pIFxMedia);
		_pFxOutputPin->InitStream(pDstFxMedia);
		_pFxState->FxPublishState(FX_STREAM_INIT_STATE);
	}
		
	/*! Retrieve the media name */
	std::string strMediaName;
	pIFxMedia->GetFxMediaName(strMediaName);
	
	/*! Retrieve Gain to apply */
	float fGain;
	_pFxParam->GetFxParamValue("Gain", &fGain);
	/*! Loop on input data */
	Uint32 dwInputDatalength;
	pIFxMedia->GetDataLength(&dwInputDatalength);
	/*! We suppose that we receive data modulus of sample size */
	if(dwInputDatalength%(4 * FxPcmFmt.wChannels) > 0) {
		_pFxState->FxPublishState(_FxState = FX_UNDERRUN_STATE);
		dwInputDatalength -= dwInputDatalength%(4 * FxPcmFmt.wChannels);
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
		Uint32 dwDataToProcess = (dwInputDatalength > dwOutputSize) ?
								  dwOutputSize :
								  dwInputDatalength;
		dwDataToProcess -= dwDataToProcess%(4 * FxPcmFmt.wChannels); //!< Must be modulus of sample and channel number
		
		/* Apply gain on each channel */
		Uint32 dwSampleToProcess = dwDataToProcess / 4;
		pfDstData = ((Float*)pDstData);
		for(Uint32 Idx = 0; Idx < dwSampleToProcess; Idx++){
			pfDstData[Idx] = GetRatioFromdB(fGain, g_fMinGain) * *pfSrcData++;
			pfDstData[Idx] = (pfDstData[Idx] > 1.0f) ? 1.0f : pfDstData[Idx];
			pfDstData[Idx] = (pfDstData[Idx] < -1.0f) ? -1.0f : pfDstData[Idx];
		}
		
		dwInputDatalength -= dwDataToProcess;

		/*! Update the valid data length */
		pDstFxMedia->SetDataLength(dwDataToProcess);

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
		pFxPcmDestFormat->SetPcmFormat(&FxPcmFmt);
	
		/*! Deliver the FxMedia to the next Fx */
		_pFxOutputPin->DeliverMedia(pDstFxMedia);
	}
	
	return hr;
}
