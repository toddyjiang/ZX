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
   @file  FxMToSMerger.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/
#include <string.h>
#include <gtk/gtk.h>
#include "FxMToSMerger.h"

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

//! The Fx names
const Char g_strFxName[] = "TwotoOneStereo";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2009)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";

//! GTK "delete_event" callback  
gint quit(GtkWidget *widget, GdkEvent *event, gpointer *data) {
	/* Do not close window now */
	gtk_widget_hide(widget);
	return TRUE;
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
	return (IFxBase*)(new CFxMToSMerger);
}
#ifdef __cplusplus
}
#endif

CFxMToSMerger::CFxMToSMerger()
{
	/*! Initialize the Fx components */
	_FxState = FX_STOP_STATE;

	_pFxState		= NULL;

	_pFxDescriptor  = NULL;

	_OutPinState	= PIN_NOT_CONNECTED;
	
	_pFxOutputPin	= NULL;
	
	/*! Thread and mutex */
	_ShouldCloseThread = FALSE;
	_Thread = NULL;
	pthread_mutex_init( &_ThreadMutex, NULL );
    pthread_cond_init(&_ThreadCond, NULL);
		
	FX_PCM_FORMAT FxPcmFmt = {2, FLOAT32_TYPE, 0};
	_FxOutPcmFmt  =  FxPcmFmt;
}

CFxMToSMerger::~CFxMToSMerger()
{
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToSMerger::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
Int32 CFxMToSMerger::InitFx(IFx **ppFxComponent)
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
		    "Mono/Stereo stream" 		//!< Pin Name
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
	hr = pFxPinManager->Create(&_FxPin[0], &_FxInputPinL._pInputPin);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	_FxInputPin.insert(make_pair(_FxInputPinL._pInputPin, &_FxInputPinL));
	hr = pFxPinManager->Create(&_FxPin[0], &_FxInputPinR._pInputPin);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	_FxInputPin.insert(make_pair(_FxInputPinR._pInputPin, &_FxInputPinR));
	
	hr = pFxPinManager->Create(&_FxPin[1], &_pFxOutputPin);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	
	/*! Get the IFxParam to create the Fx public parameters */
	hr = _pFx->FxGetInterface(IFX_PARAM, (Void**)&_pFxParam);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	
	/*! Create the Fx public parameter */
	Uint8 bDefault = 0;
	Uint8 bDefaultMax = 1;
	FX_PARAM FxParamSync = {"Synchronize", "On/Off", &bDefault, &bDefault, &bDefaultMax, UINT8_TYPE, 1};
	hr = _pFxParam->AddFxParam(&FxParamSync);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	
	/* Create User interface */
	_pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(_pWindow), "Merger property");
	gtk_window_set_default_size(GTK_WINDOW(_pWindow), 320, 50);
	gtk_container_set_border_width(GTK_CONTAINER(_pWindow), 4);

	_pMainVBox = gtk_vbox_new(TRUE, 0);
	gtk_container_add(GTK_CONTAINER(_pWindow), _pMainVBox);
	
	_pFrame = gtk_frame_new("Synchronize input (On/Off)");
	_pSlider = gtk_hscale_new_with_range((Float)bDefault, (Float)bDefaultMax, 1); 
	gtk_container_add(GTK_CONTAINER(_pFrame), _pSlider);
	gtk_range_set_value(GTK_RANGE(_pSlider), bDefaultMax);
	g_signal_connect(G_OBJECT(_pSlider), "value_changed", G_CALLBACK(CFxMToSMerger::SliderWrapper), this);
	gtk_box_pack_start(GTK_BOX(_pMainVBox), _pFrame, FALSE, FALSE, 0);
	
	g_signal_connect(G_OBJECT(_pWindow), "delete_event", G_CALLBACK(quit), _pWindow);

	/*! We can release the PinManager */
	SAFE_RELEASE_INTERFACE(pFxPinManager);

	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(_FxState = FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	SliderCallback()
	Retrieves the slider value.
*//*-----------------------------------------------------------------------------*/
Void CFxMToSMerger::SliderCallback()
{
	Uint8 bValue = (Uint8)gtk_range_get_value(GTK_RANGE(_pSlider));
	/* Set Fx parameter */
	_pFxParam->SetFxParamValue("Synchronize", &bValue);

	return;
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFxParam()
	Update Fx Parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToSMerger::UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter)
{
	/*! Update HMI with the new value */
	Uint8 bValue;
	_pFxParam->GetFxParamValue("Synchronize", &bValue);
	gtk_range_set_value(GTK_RANGE(_pSlider), (float)bValue);
	return FX_OK;  
}

/*-----------------------------------------------------------------------------*//*!
	DisplayFxPropertyPage()
	Displays the Fx property page.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToSMerger::DisplayFxPropertyPage(Pvoid pvWndParent)
{
	gtk_widget_show_all(_pWindow);
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToSMerger::StartFx()
{
	/*! Synchronize or not */
	Uint8 bValue;
	_pFxParam->GetFxParamValue("Synchronize", &bValue);
	_IsSynchronous = (bValue == 1) ? TRUE : FALSE;
	
	/*! Reset pin states */
	_FxInputPinL.Reset();
	_FxInputPinR.Reset();
	
	if(!_Thread) {
		/*! Start thread */
		_ShouldCloseThread = FALSE;
		/*! Start new thread */
		if(pthread_create (
		        &_Thread, NULL,
		        CFxMToSMerger::ThreadFctWrapper, (void *)this) != 0){
			return FX_ERROR;
		}
	}
	
	if(_FxState != FX_START_STATE) {
		_pFxState->FxPublishState(_FxState = FX_START_STATE);
	}

    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	RunFx()
	Run the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToSMerger::RunFx()
{
    if(_FxState != FX_RUN_STATE) {
        _pFxState->FxPublishState(_FxState = FX_RUN_STATE);
	}

    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToSMerger::StopFx()
{
	/*! Stop Thread */
	if(_Thread){
		_ShouldCloseThread = TRUE;
		pthread_cond_signal (&_ThreadCond); //!< Unlock wait
		pthread_join (_Thread, NULL);
		_Thread = NULL;
	}
	if(_FxState != FX_STOP_STATE) {
        _pFxState->FxPublishState(_FxState = FX_STOP_STATE);
	}

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CFxMToSMerger::ReleaseFx()
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
Void CFxMToSMerger::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
{
	if( (_FxInputPinL._PinState == PIN_CONNECTED) && (_FxInputPinR._PinState == PIN_CONNECTED) && (_OutPinState == PIN_CONNECTED) ) {
		if(PinState == PIN_NOT_CONNECTED) {
			_pFxState->FxPublishState(_FxState = FX_DISCONNECT_STATE);
		}
	}

	FX_PIN_TYPE FxPinType;
	pFxPin->GetPinType(&FxPinType);
	if(FxPinType == PIN_IN) {
		_FxInputPin[pFxPin]->_PinState = PinState;
		if(PinState == PIN_NOT_CONNECTED){
			_FxInputPin[pFxPin]->_FxPcmFmt.dwSamplingRate = 0; //!< Reset pin stream format
		}
	}
	else {
		_OutPinState = PinState;
		if(PinState == PIN_CONNECTED && _FxOutPcmFmt.dwSamplingRate != 0)
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
			pIFxPcmFormat->SetPcmFormat(&_FxOutPcmFmt);
			pIFxMedia->SetFxMediaName("Stream connexion");

			/*! Deliver the FxMedia to the next Fx */
			_pFxOutputPin->InitStream(pIFxMedia);	
		}
	}
	
	/*! Reset output stream format */
	if(_FxInputPinL._PinState == PIN_NOT_CONNECTED && _FxInputPinR._PinState == PIN_NOT_CONNECTED)
		_FxOutPcmFmt.dwSamplingRate = 0; 

	if( (_FxInputPinL._PinState == PIN_CONNECTED) && (_FxInputPinR._PinState == PIN_CONNECTED) && (_OutPinState == PIN_CONNECTED) ) {
        _pFxState->FxPublishState(_FxState = FX_CONNECT_STATE);		
	}
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxMedia()
	Decides the Media buffer size and number.
*//*-----------------------------------------------------------------------------*/
Void CFxMToSMerger::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
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
Int32 CFxMToSMerger::FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
{
	Int32 hr = FX_OK;
	FX_PCM_FORMAT FxPcmFmt;
	
	if(StreamState == STREAM_FLUSH) {
		_FxInputPin[pFxPin]->_IsFlushed = TRUE;
		_FxInputPin[pFxPin]->_IsActive = FALSE;
		/*! Unblock thread */
		pthread_cond_signal (&_ThreadCond); //!< Unlock wait
		return hr;
	}
	else
		_FxInputPin[pFxPin]->_IsFlushed = FALSE;	
	
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
		/*! Save input Fs */		
		_FxInputPin[pFxPin]->_FxPcmFmt = FxPcmFmt;
		
		/*! Both input frequencies must be the same */
		if(_FxInputPinL._FxPcmFmt.dwSamplingRate != 0 && _FxInputPinR._FxPcmFmt.dwSamplingRate != 0)
			if(_FxInputPinL._FxPcmFmt.dwSamplingRate != _FxInputPinR._FxPcmFmt.dwSamplingRate){
				_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
				return hr;
			}
		
		if(_FxOutPcmFmt.dwSamplingRate == 0){
			/*! publish format to the downstream */
			IFxMedia* pDstFxMedia;
			if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pDstFxMedia, INFINITE_TIME)) ) {
				_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
				return hr;
			}
			pDstFxMedia->Copy(pIFxMedia);
			IFxPcmFormat* pFxPcmFormat;
			if(FEF_FAILED(hr = pDstFxMedia->GetFormatInterface(PCM, (Void**)&pFxPcmFormat))) {
				_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
				return hr;
			}
			_FxOutPcmFmt.dwSamplingRate = FxPcmFmt.dwSamplingRate;
			pFxPcmFormat->SetPcmFormat(&_FxOutPcmFmt);
			_pFxOutputPin->InitStream(pDstFxMedia);
			_pFxState->FxPublishState(FX_STREAM_INIT_STATE);
		}
	}
	
	/*! Retrieve the media name */
	pIFxMedia->GetFxMediaName(_strMediaName);
	
	/*! Get input data */
	Uint32 dwInputDatalength;
	pIFxMedia->GetDataLength(&dwInputDatalength);
	/*! We suppose that we receive data modulus of sample size */
	if((dwInputDatalength % (4 * _FxInputPin[pFxPin]->_FxPcmFmt.wChannels)) > 0) {
		_pFxState->FxPublishState(_FxState = FX_UNDERRUN_STATE);
		dwInputDatalength -= dwInputDatalength % (4 * _FxInputPin[pFxPin]->_FxPcmFmt.wChannels);
	}
	
	/*! */
	if(dwInputDatalength)
		_FxInputPin[pFxPin]->_IsActive = TRUE;
		
	/*! Get media pointer */
	Uint8 *pSrcData;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaPointer(&pSrcData)) ) {
		_pFxState->FxPublishState(_FxState = FX_UNDERRUN_STATE);
		return hr;
	}
	pthread_mutex_lock(&_ThreadMutex);
	/*! Copy data in internal pin buffer */
	if(_FxInputPin[pFxPin]->_FxPcmFmt.wChannels == 1){
		/*! Prevent overrun */
		Uint32 dwFreeSize = INPUT_BUFFER_SIZE - _FxInputPin[pFxPin]->_dwWriteIdx;
		if(dwFreeSize < dwInputDatalength) {
			Uint32 dwDelta = dwInputDatalength - dwFreeSize;
			_FxInputPin[pFxPin]->_dwWriteIdx -= dwDelta;
			memmove(_FxInputPin[pFxPin]->_pbData, _FxInputPin[pFxPin]->_pbData + dwDelta, _FxInputPin[pFxPin]->_dwWriteIdx);
			_pFxState->FxPublishState(FX_OVERRUN_STATE);
		}
		memcpy(_FxInputPin[pFxPin]->_pbData + _FxInputPin[pFxPin]->_dwWriteIdx, pSrcData, dwInputDatalength);
		_FxInputPin[pFxPin]->_dwWriteIdx += dwInputDatalength;
	}
	else { //!< Stereo input
		Float *pfSrcData, *pfDstData;
		pfSrcData = ((Float*)pSrcData);
		/*! Prevent overrun */
		Uint32 dwFreeSize = INPUT_BUFFER_SIZE - _FxInputPin[pFxPin]->_dwWriteIdx;
		if(dwFreeSize < dwInputDatalength / 2) {
			Uint32 dwDelta = dwInputDatalength / 2 - dwFreeSize;
			_FxInputPin[pFxPin]->_dwWriteIdx -= dwDelta;
			memmove(_FxInputPin[pFxPin]->_pbData, _FxInputPin[pFxPin]->_pbData + dwDelta, _FxInputPin[pFxPin]->_dwWriteIdx);
			_pFxState->FxPublishState(FX_OVERRUN_STATE);
		}	
		
		pfDstData = (Float*)(_FxInputPin[pFxPin]->_pbData + _FxInputPin[pFxPin]->_dwWriteIdx);
		Uint32 dwSampleToProcess = dwInputDatalength / 4;
		for(Uint32 Idx = 0; Idx < dwSampleToProcess; Idx += 2)
			*pfDstData++ = (*pfSrcData++ + *pfSrcData++) / 2;	
			
		_FxInputPin[pFxPin]->_dwWriteIdx += dwInputDatalength / 2;
	}
	
	pthread_mutex_unlock(&_ThreadMutex);
	
	/*! Unblock thread */
	pthread_cond_signal (&_ThreadCond); //!< Unlock wait
	
	return hr;
}

Void CFxMToSMerger::GetInputAvailableSize(Uint32* pdwAvailableL, Uint32* pdwAvailableR)
{
	*pdwAvailableL = 0;
	*pdwAvailableR = 0;
	
	/*! Synchronize data */
	if(_IsSynchronous){
		if(!_FxInputPinL._dwWriteIdx || !_FxInputPinR._dwWriteIdx)
			return;
		else	
			_IsSynchronous = FALSE;
	}
	
	if( (_FxInputPinL._dwWriteIdx && _FxInputPinR._IsFlushed) ||
		(_FxInputPinR._dwWriteIdx && _FxInputPinL._IsFlushed) ||
		(_FxInputPinL._dwWriteIdx && !_FxInputPinR._IsActive) ||
		(_FxInputPinR._dwWriteIdx && !_FxInputPinL._IsActive) ||
		(_FxInputPinL._dwWriteIdx && _FxInputPinR._dwWriteIdx) ){
		/*! Set common input size */
		if(_FxInputPinL._dwWriteIdx == 0 || _FxInputPinR._dwWriteIdx == 0){
			*pdwAvailableL = _FxInputPinL._dwWriteIdx;
			*pdwAvailableR = _FxInputPinR._dwWriteIdx;
		}
		else {
			/*! return only the common part */
			*pdwAvailableL = std::min(_FxInputPinL._dwWriteIdx, _FxInputPinR._dwWriteIdx);
			*pdwAvailableR = *pdwAvailableL;
		}
	}
	return;
}

Void* CFxMToSMerger::ThreadFct()
{
	Uint32 dwAvailableL, dwAvailableR;
	pthread_mutex_lock(&_ThreadMutex);
	
	Int32 hr;
	while(!_ShouldCloseThread)
	{
		pthread_cond_wait (&_ThreadCond, &_ThreadMutex);
		pthread_mutex_unlock(&_ThreadMutex);
				
		/*! Get available input sizes */
		GetInputAvailableSize(&dwAvailableL, &dwAvailableR);
				
		Float *pfDstData, *pfSrcDataL, *pfSrcDataR;
		while( dwAvailableL > 0 || dwAvailableR > 0 ) {
			pfSrcDataL = (Float*)_FxInputPinL._pbData;
			pfSrcDataR = (Float*)_FxInputPinR._pbData;
			/*! Get output data buffer to fill */
			IFxMedia* pDstFxMedia;
			if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pDstFxMedia, INFINITE_TIME)) ) {
				_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
			}
			Uint8 *pDstData;
			if( FEF_FAILED(hr = pDstFxMedia->GetMediaPointer(&pDstData)) ) {
				/*! Release Media buffer */
				pDstFxMedia->Release();
				_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
			}
			Uint32 dwOutputSize;
			pDstFxMedia->GetSize(&dwOutputSize);
			memset(pDstData, 0, dwOutputSize);
			
			/* L channel */
			Uint32 dwDataToProcessL = ((dwAvailableL * 2) > dwOutputSize) ?
								dwOutputSize / 2:
								dwAvailableL;
			dwDataToProcessL -= dwDataToProcessL%4; //!< Must be modulus of sample
			Uint32 dwSampleToProcess = dwDataToProcessL / 4;
			pfDstData = ((Float*)pDstData);
			for(Uint32 Idx = 0; Idx < dwSampleToProcess; Idx++){
				*pfDstData = *pfSrcDataL++;
				pfDstData += 2;
			}
			/*! Update input data buffer */
			pthread_mutex_lock(&_ThreadMutex);
			_FxInputPinL._dwWriteIdx -= dwDataToProcessL;
			memmove(_FxInputPinL._pbData, _FxInputPinL._pbData + dwDataToProcessL, _FxInputPinL._dwWriteIdx);
			pthread_mutex_unlock(&_ThreadMutex);
			dwAvailableL -= dwDataToProcessL;
			/* R channel */
			Uint32 dwDataToProcessR = ((dwAvailableR * 2) > dwOutputSize) ?
								dwOutputSize / 2:
								dwAvailableR;
			dwDataToProcessR -= dwDataToProcessR%4; //!< Must be modulus of sample
			dwSampleToProcess = dwDataToProcessR / 4;
			pfDstData = (((Float*)pDstData) + 1);
			for(Uint32 Idx = 0; Idx < dwSampleToProcess; Idx++){
				*pfDstData = *pfSrcDataR++;
				pfDstData += 2;
			}
			/*! Update input data buffer */
			pthread_mutex_lock(&_ThreadMutex);
			_FxInputPinR._dwWriteIdx -= dwDataToProcessR;
			memmove(_FxInputPinR._pbData, _FxInputPinR._pbData + dwDataToProcessR, _FxInputPinR._dwWriteIdx);
			pthread_mutex_unlock(&_ThreadMutex);
			dwAvailableR -= dwDataToProcessR;
			
			/*! Update the valid data length */
			pDstFxMedia->SetDataLength(std::max(dwDataToProcessL, dwDataToProcessR) * 2);

			/*! Set data properties */
			pDstFxMedia->SetFxMediaName(_strMediaName);

			/*! Set Media PCM format */
			IFxPcmFormat* pFxPcmDestFormat;
			if(FEF_FAILED(hr = pDstFxMedia->GetFormatInterface(PCM, (Void**)&pFxPcmDestFormat))) {
				/*! Release Media buffer */
				pDstFxMedia->Release();
				_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
			}
			pFxPcmDestFormat->SetPcmFormat(&_FxOutPcmFmt);

			/*! Deliver the FxMedia to the next Fx */
			_pFxOutputPin->DeliverMedia(pDstFxMedia);
		}
				
		/*! Propagate the flush ? */
		if(dwAvailableL == 0 && dwAvailableR == 0){
			if( (_FxInputPinL._IsFlushed || !_FxInputPinL._IsActive) && (_FxInputPinR._IsFlushed || !_FxInputPinR._IsActive) ){
				_pFxState->FxPublishState(_FxState = FX_FLUSH_STATE);
				_pFxOutputPin->Flush();
			}
		}
				
	}
	pthread_mutex_unlock(&_ThreadMutex);
	return NULL;
}

