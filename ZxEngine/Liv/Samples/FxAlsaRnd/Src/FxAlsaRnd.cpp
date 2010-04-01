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
   @file  FxAlsaRnd.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/
#include <gtk/gtk.h>

#include "FxAlsaRnd.h"

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

//static snd_pcm_format_t g_SampleFormat = SND_PCM_FORMAT_S16;    /* sample format */
Uint32 g_dwFs = 44100;                           /* stream rate */
Uint32 g_dwChannels = 2;                         /* count of channels */
unsigned int g_dwBufferTime = 1000000;           /* ring buffer length in us */
snd_pcm_sframes_t g_dwBufferSize;
unsigned int g_dwPeriodTime;					 /* periode time */
unsigned int g_dwPrebufferingTime;				 /* prebuffering time in us */
snd_pcm_uframes_t g_dwPrebufferingSample;        /* prebuffering time in sample */



//! The Fx names
const Char g_strFxName[] = "FxAlsaRnd";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2009)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";

//! TimeOut
const Uint32 g_dwTimeOut = 3000000; //!< us

//! Defaults values
extern Uint16 g_wDefaultPreBuffering ; //!< 400ms
extern Uint16 g_wMinPreBuffering; //!< 5ms
extern Uint16 g_wMaxPreBuffering; //!< 1000ms

extern Uint16 g_wDefaultBufferLength; //!< 50ms
extern Uint16 g_wMinBufferLength; //!< 5ms
extern Uint16 g_wMaxBufferLength; //!< 250ms

static Int16 __inline FloatToLin16 (Float fValue)
{
	Int16 swValue;
    if (fValue > 1.0f)  fValue=1.0f;
    if (fValue < -1.0f) fValue=-1.0f;
    if (fValue < 0.0f)  swValue = (Int16)((fValue * 32767.0f) - 0.5f);
    else                swValue = (Int16)((fValue * 32767.0f) + 0.5f);
	return swValue;
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
	return (IFxBase*)(new CFxAlsaRnd);
}
#ifdef __cplusplus
}
#endif

CFxAlsaRnd::CFxAlsaRnd()
{
	/*! Initialize the Fx components */
	_FxState		= FX_STOP_STATE;
	
	_pFxState		= NULL;

	_pFxParam 		= NULL;

    _pFxDescriptor  = NULL;

	_pFxInputPin	= NULL;

	_pFxGui = NULL;
	
	_pAlsaHandle = NULL;
	
	_pSwParams = NULL;
	_pHwParams = NULL;
	
	_Thread = NULL;
	_dwAskedSample = 0;
	pthread_mutex_init( &_ThreadMutex, NULL );
	
	
	_vectorDevice.push_back(make_pair(std::string("default"), std::string("default")));
}

CFxAlsaRnd::~CFxAlsaRnd()
{
	SAFE_DELETE_OBJECT(_pFxGui);
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
        _pFxDescriptor->FxType = FX_RENDERER;
        _pFxDescriptor->FxScope = FX_SCOPE_AUDIO;
    }

    *ppFxDescriptor = _pFxDescriptor;
			
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	InitFx()
	Initializes the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::InitFx(IFx **ppFxComponent)
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

	_pFxState->FxPublishState(FX_LOADING_STATE);

	/*! Get the IFxPinManager to create the Fx pins */
	IFxPinManager* pFxPinManager = NULL;
	hr = _pFx->FxGetInterface(IFX_PINMANGER, (Void**)&pFxPinManager);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	FX_PIN _FxPin[] =
    {
		"Input audio stream"		//!< Pin Name
		, PIN_IN					//!< Pin Type
		, &g_FxMediaType			//!< Media Type
		, 1							//!< Media Type number
		, (IFxPinCallback*)this		//!< Pin Callback
	};

	/*! Create the Fx pins */
	hr = pFxPinManager->Create(_FxPin, &_pFxInputPin);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}
	
	/*! We can release the PinManager */
	SAFE_RELEASE_INTERFACE(pFxPinManager);

	/*! Get the IFxParam to create the Fx public parameters */
	hr = _pFx->FxGetInterface(IFX_PARAM, (Void**)&_pFxParam);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! Create the Fx public parameter */
    FX_PARAM_STRING FxParamSoundDevice = {"Sound device", "default"};
	hr = _pFxParam->AddFxParam(&FxParamSoundDevice);
	if(FEF_FAILED(hr)) {
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	FX_PARAM FxParamPreBuffering = {"Prebuffering Time", "ms", &g_wDefaultPreBuffering, &g_wMinPreBuffering, &g_wMaxPreBuffering, UINT16_TYPE, 1};
	hr = _pFxParam->AddFxParam(&FxParamPreBuffering);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	FX_PARAM FxParamBufferlength = {"Buffer length", "ms", &g_wDefaultBufferLength, &g_wMinBufferLength, &g_wMaxBufferLength, UINT16_TYPE, 1};
	hr = _pFxParam->AddFxParam(&FxParamBufferlength);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! Get available alsa devices and alloc hardware/sofware params */
	EnumDevice();
	    
    /*! Set default audio format */
    _FxPcmFmt.wChannels = g_dwChannels;
    _FxPcmFmt.dwSamplingRate = 0;
    _FxPcmFmt.FormatTag = FLOAT32_TYPE;
	
	/*! Create UI */
	_pFxGui = new CFxGui(_vectorDevice, _pFxParam);
	
	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(_FxState = FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFxParam()
	Update Fx Parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter)
{
	/*! Update UI with the new values */
	_pFxGui->UpdateFxParameters();
	
	return FX_OK;  
}

/*-----------------------------------------------------------------------------*//*!
	DisplayFxPropertyPage()
	Displays the Fx property page.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::DisplayFxPropertyPage(Pvoid pvWndParent)
{
	_pFxGui->ShowUI();
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::StartFx()
{
	Int32 hr;
	
	/*! Stop Thread */
	if(_Thread){
		_ShouldCloseThread = TRUE;
		pthread_join (_Thread, NULL);
		_Thread = NULL;
	}
	_pFxState->FxPublishState(_FxState = FX_STOP_STATE);
	
	if(FEF_FAILED(hr = OpenDevice()))
		return hr;
		
	_pFxState->FxPublishState(_FxState = FX_START_STATE);

    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	RunFx()
	Run the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::RunFx()
{
	/*! Restart thread */
	_ShouldCloseThread = FALSE;

	/*! Start new thread */
	if(pthread_create (
            &_Thread, NULL,
            CFxAlsaRnd::ThreadFctWrapper, (void *)this) != 0){
		return FX_ERROR;
	}
	
    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::StopFx()
{
	/*! Stop playing */
	if(_pAlsaHandle)
		snd_pcm_drop(_pAlsaHandle);
		
	/*! Stop Thread */
	if(_Thread){
		_ShouldCloseThread = TRUE;
		pthread_join (_Thread, NULL);
		_Thread = NULL;
	}
	
	if(_pAlsaHandle) {
		snd_pcm_drop(_pAlsaHandle);
		snd_pcm_close(_pAlsaHandle);
		_pAlsaHandle = NULL;
	}
	_pFxState->FxPublishState(_FxState = FX_STOP_STATE);
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::ReleaseFx()
{
	/*! Stop and release Fx components */
	StopFx();
	
	_pFxState->FxPublishState(FX_RELEASE_STATE);	/*!< Update the Fx state to "Release" */
	
	Int32 hr = FX_OK;
	_pFxState->FxReleaseInterface();
	
	/*! Delete the Fx instance */
	delete this;
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	FxPinState()
	Receives the Fx Pins states.
*//*-----------------------------------------------------------------------------*/
Void CFxAlsaRnd::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
{
	if(PinState == PIN_CONNECTED)
		_pFxState->FxPublishState(FX_CONNECT_STATE);
    else
        _pFxState->FxPublishState(FX_DISCONNECT_STATE);
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxPin()
	Receives the new Fx media on input pin.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
{
	Int32 hr = FX_OK;
	
	CAutoLock Lock(&_ThreadMutex);
	
	if(StreamState == STREAM_FLUSH) {
		snd_pcm_drain(_pAlsaHandle);
		_pFxState->FxPublishState(_FxState = FX_FLUSH_STATE);
		return FX_OK;
	}
	
	/*! Check media type */
	if(FEF_FAILED(hr = pIFxMedia->CheckMediaType(&g_FxMediaType))) {
		_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
		return FX_MEDIANOTSUPPORTED;		
	}
	
	if(StreamState == STREAM_INIT) {
		/*! Get Media PCM format */
		IFxPcmFormat* pFxPcmFormat;
		if(FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (Void**)&pFxPcmFormat))) {
			_pFxState->FxPublishState(FX_ERROR_SUBMEDIA_PIN_STATE);
			return hr;
		}
		FX_PCM_FORMAT FxPcmFmt;
		pFxPcmFormat->GetPcmFormat(&FxPcmFmt);
	
		/*! Float audio samples are accepted only */
		if( (FxPcmFmt.FormatTag != FLOAT32_TYPE) && (FxPcmFmt.FormatTag != UINT16_TYPE) ) {
			_pFxState->FxPublishState(FX_ERROR_SUBMEDIA_PIN_STATE);
			return FX_FMTNOTSUPPORTED;
		}
		hr = FX_OK;Int32 hr = FX_OK;
		if( FxPcmFmt.wChannels < 1 ) {
			_pFxState->FxPublishState(FX_ERROR_SUBMEDIA_PIN_STATE);
			return FX_FMTNOTSUPPORTED;
		}
		/*! Update alsa format */
		_pFxGui->UpdateStreamFormat(FxPcmFmt.dwSamplingRate, (FxPcmFmt.FormatTag == FLOAT32_TYPE) ? "FLOAT32" : "INT16", FxPcmFmt.wChannels);
		if( (FxPcmFmt.wChannels != _FxPcmFmt.wChannels) || (FxPcmFmt.dwSamplingRate != _FxPcmFmt.dwSamplingRate) ) {
			_FxPcmFmt = FxPcmFmt;
			if(_FxState == FX_START_STATE) {
				Lock.Unlock();
				/*! Restart device */
				if(FEF_FAILED(hr = StartFx())) //!< Stop thread and open device 
					return hr;
				if(FEF_FAILED(hr = RunFx())) //!< Start device
					return hr;
				Lock.Lock();
			}
			else { //!< Just try to open it to check pcm format 
				if(FEF_FAILED(hr = OpenDevice())){
					_pFxState->FxPublishState(FX_ERROR_SUBMEDIA_PIN_STATE);
					return FX_FMTNOTSUPPORTED;
				}
				if(_pAlsaHandle) {
					snd_pcm_close(_pAlsaHandle);
					_pAlsaHandle = NULL;
				}
				
			}
		}
		_pFxState->FxPublishState(FX_STREAM_INIT_STATE);
	}
	
	if(_FxState == FX_FLUSH_STATE) //!< Fx must be restarted
		return FX_OK;
	
	if(_pAlsaHandle == NULL){
		return FX_OK;
	}
	
	/*! Recovery if needed */
	snd_pcm_sframes_t sdwAvailable = snd_pcm_avail_update(_pAlsaHandle);
	if (sdwAvailable < 0) {
	    hr = Recovery(sdwAvailable);
	    if (hr < 0)
	    	printf("avail update failed: %s\n", snd_strerror(hr));
	}
	
	Uint32 dwInputDatalength;
	pIFxMedia->GetDataLength(&dwInputDatalength);
	Uint32 dwSampleNumber = dwInputDatalength / (((_FxPcmFmt.FormatTag == FLOAT32_TYPE) ? sizeof(FEF::Float) : sizeof(FEF::Int16)) * _FxPcmFmt.wChannels);
	if(dwSampleNumber == 0)
		return FX_OK;
	
	/*! Get media pointer */
	Uint8 *pSrcData;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaPointer(&pSrcData)) ) {
		_pFxState->FxPublishState(FX_ERROR_PIN_STATE);
		return hr;
	}	

	/*! Check Alsa buffer size */
	if(dwSampleNumber > sdwAvailable){
		_pFxState->FxPublishState(FX_OVERRUN_STATE); 
		dwSampleNumber = sdwAvailable;
	}
	
	/*! Update sample format */ 
	if((_FxPcmFmt.FormatTag == FLOAT32_TYPE) && !_IsPcmFloatAccepted){ 
		Float *pfSample = (Float*)pSrcData;
		Int16 *pswSample = (Int16*)pSrcData;
		for(Uint32 IdxSample = 0; IdxSample < dwSampleNumber; IdxSample++){
			for(Uint32 IdxChannel = 0; IdxChannel < _FxPcmFmt.wChannels; IdxChannel++){
				pswSample[IdxChannel] = FloatToLin16 (pfSample[IdxChannel]);	
			}
			pswSample += _FxPcmFmt.wChannels;			
			pfSample += _FxPcmFmt.wChannels;		
		}
	}
	
	/*! Write data */
	hr = snd_pcm_writei(_pAlsaHandle, pSrcData, dwSampleNumber); 
	if (hr < 0) {
            printf("Write error: %s\n", snd_strerror(hr));
            return FX_ERROR;
    }
    if (hr != dwSampleNumber) {
            printf("Write error: written %i expected %li\n", hr, dwSampleNumber);
            return FX_ERROR;
    }
    /*! Update written sample */
    _qWrittenSample += dwSampleNumber;
    
    /*! Update incomming size */
    _dwAskedSample = (_dwAskedSample < dwSampleNumber) ? 0: _dwAskedSample - dwSampleNumber; //!< Prevent Underrun
			
	return hr;
}

/*-----------------------------------------------------------------------------*//*!
	ThreadFct()
	Audio thread processing.
*//*-----------------------------------------------------------------------------*/
Void* CFxAlsaRnd::ThreadFct() 
{
	Int32 hr;
	snd_pcm_state_t state;
	Bool ShouldStart = TRUE;

	snd_pcm_sframes_t sdwSampleDelay;
	Uint32 dwWaitingTime = 0;
	_dwAskedSample = 0;
	_qWrittenSample = 0;
	Uint64 qPlayedSample = 0;
	Uint64 qWrittenTime = 0;
	_dPlayedTime = 0;
			
	while(!_ShouldCloseThread) {
					
		pthread_mutex_lock(&_ThreadMutex);
		
		/*! Update played time */
		if (!snd_pcm_delay(_pAlsaHandle, &sdwSampleDelay))
			qPlayedSample = (_qWrittenSample < sdwSampleDelay) ? 0 : _qWrittenSample - sdwSampleDelay; //!< Prevent start delay
			
		_dPlayedTime = 1000 * ((Double)qPlayedSample / _FxPcmFmt.dwSamplingRate); //! Sample number to time in ms
		_pFxGui->UpdatePlayedTime(_dPlayedTime);
		
		/*! Fx State must be FX_START_STATE */
		if(_FxState != FX_FLUSH_STATE){
			/*! Alsa state */
			state = snd_pcm_state(_pAlsaHandle);
		    if (state == SND_PCM_STATE_XRUN) {
		    	_dwAskedSample = 0;
				dwWaitingTime = 0;
		    	_pFxState->FxPublishState(FX_UNDERRUN_STATE);
		        hr = Recovery(-EPIPE);
		        if (hr < 0) {
		                printf("XRUN recovery failed: %s\n", snd_strerror(hr));
		        }
		    }
		    else if (state == SND_PCM_STATE_SUSPENDED) {
		    	_dwAskedSample = 0;
				dwWaitingTime = 0;
		        hr = Recovery(-ESTRPIPE);
		        if (hr < 0) {
		        	printf("SUSPEND recovery failed: %s\n", snd_strerror(hr));
		        }
		    }
		    else if (state == SND_PCM_STATE_PREPARED && ShouldStart) {
		    	if(_qWrittenSample >= (g_dwPrebufferingSample)) {
		    		hr = snd_pcm_start(_pAlsaHandle);
		    		if (hr < 0) {
						printf("Start error: %s\n", snd_strerror(hr));	        
					}
					else
						ShouldStart = FALSE;
				}
		    }
		
			/*! Get available size to fill */
			snd_pcm_sframes_t sdwAvailable = snd_pcm_avail_update(_pAlsaHandle);
		
			if (sdwAvailable < 0) {
				hr = Recovery(sdwAvailable);
				if (hr < 0) {
					printf("avail update failed: %s\n", snd_strerror(hr));
				}
			}
						
			sdwAvailable -= _dwAskedSample; //!< Do not ask too much
					
			Uint16 wBufferLength;
			_pFxParam->GetFxParamValue("Buffer length", &wBufferLength);
			wBufferLength = ((Float)wBufferLength/1000)*_FxPcmFmt.dwSamplingRate; //!< From Time to frame numbers
			
			/*! get right buffer size */
			if(state == SND_PCM_STATE_RUNNING)
				sdwAvailable -= sdwAvailable%wBufferLength;
			
			if(sdwAvailable > 0) {
				/*! Send sample request according buffer size */
				Uint32 dwSampleToAsk;
				_dwAskedSample += sdwAvailable;
				while(sdwAvailable > 0) {
					dwSampleToAsk = (sdwAvailable < wBufferLength) ? sdwAvailable : wBufferLength;
					sdwAvailable -= dwSampleToAsk;
					pthread_mutex_unlock(&_ThreadMutex);
					_pFxInputPin->WaitForIFxMedia(dwSampleToAsk, NULL);
					pthread_mutex_lock(&_ThreadMutex);
					dwWaitingTime = 0;
				}
			}
			else {
				if(_FxState == FX_START_STATE && dwWaitingTime > g_dwTimeOut) {
					_dwAskedSample = 0;
					dwWaitingTime = 0;
					_pFxState->FxPublishState(FX_UNDERRUN_STATE); 
		    	}
			}
		}
		
		pthread_mutex_unlock(&_ThreadMutex);
		
		/*! Watchdog */
        usleep(g_dwPeriodTime);
        dwWaitingTime += g_dwPeriodTime;        	
	}
	return NULL;
}

/*-----------------------------------------------------------------------------*//*!
	EnumDevice()
	Enumerate sound device.
*//*-----------------------------------------------------------------------------*/
Void CFxAlsaRnd::EnumDevice( void )
{
	snd_ctl_t *handle;
	int card, err, dev, idx;
	snd_ctl_card_info_t *info;
	snd_pcm_info_t *pcminfo;
	snd_ctl_card_info_alloca(&info);
	snd_pcm_info_alloca(&pcminfo);
	Char strDeviceName[64];

	card = -1;
	if (snd_card_next(&card) < 0 || card < 0) {
		printf(("no soundcards found..."));
		return;
	}
	
	while (card >= 0) {
		char name[64];
		sprintf(name, "hw:%d", card);
		if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
			printf("control open (%i): %s", card, snd_strerror(err));
			goto next_card;
		}
		if ((err = snd_ctl_card_info(handle, info)) < 0) {
			printf("control hardware info (%i): %s", card, snd_strerror(err));
			snd_ctl_close(handle);
			goto next_card;
		}
		dev = -1;
		while (1) {
			unsigned int count;
			if (snd_ctl_pcm_next_device(handle, &dev)<0)
				printf("snd_ctl_pcm_next_device");
			if (dev < 0)
				break;
			snd_pcm_info_set_device(pcminfo, dev);
			snd_pcm_info_set_subdevice(pcminfo, 0);
			snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
			if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
				if (err != -ENOENT)
					printf("control digital audio info (%i): %s", card, snd_strerror(err));
				continue;
			}
			sprintf(strDeviceName, "hw:%d,%d", card, dev);
			_vectorDevice.push_back(make_pair(std::string(snd_ctl_card_info_get_name(info)), std::string(strDeviceName)));
		}
		snd_ctl_close(handle);
	next_card:
		if (snd_card_next(&card) < 0) {
			printf("snd_card_next");
			break;
		}
	}
}

/*-----------------------------------------------------------------------------*//*!
	OpenDevice()
	Open Alsa device.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::OpenDevice()
{
	Int32 hr = FX_OK;
	snd_pcm_info_t *pDeviceInfo;
	
	/*! Close if needed */
	if(_pAlsaHandle) {
		snd_pcm_close(_pAlsaHandle);
		_pAlsaHandle = NULL;
	}
	/*! Open Sound Device */
	snd_pcm_info_alloca(&pDeviceInfo);
	std::string strSoundDevice;
	_pFxParam->GetFxParamValue("Sound device", strSoundDevice);
	int err;
	if ((err = snd_pcm_open(&_pAlsaHandle, strSoundDevice.c_str(), SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    	printf("Playback open error: %s\n", snd_strerror(err));
    	_pFxState->FxPublishState(FX_ERROR_START_STATE);
        return FX_INVALPARAM;
     }
     if ((err = snd_pcm_info(_pAlsaHandle, pDeviceInfo)) < 0) { //!< Check sound card
		printf("Info error: %s\n", snd_strerror(err));
		return FX_INVALPARAM;
	}
	
	if ((err = snd_pcm_nonblock(_pAlsaHandle, 1)) < 0) {
		printf("nonblock setting error: %s", snd_strerror(err)); //!< Force non blocking mode
		return FX_INVALPARAM;
	}
	/*! Update alsa format */
	if(FEF_FAILED(hr = SetHw())) {
		_pFxState->FxPublishState(FX_ERROR_SUBMEDIA_PIN_STATE);
		return FX_FMTNOTSUPPORTED;
	}
	
	if(FEF_FAILED(hr = SetSw())) {
		_pFxState->FxPublishState(FX_ERROR_SUBMEDIA_PIN_STATE);
		return FX_FMTNOTSUPPORTED;
	}
		
	return hr;
}

/*-----------------------------------------------------------------------------*//*!
	SetHw()
	Set hardware device parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::SetHw()
{
	Int32 hr = FX_OK;
	/* choose all parameters */
	snd_pcm_hw_params_alloca(&_pHwParams);
	hr = snd_pcm_hw_params_any(_pAlsaHandle, _pHwParams);
    if (hr < 0) {
            printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(hr));
            return hr;
    }
    
    /* set the interleaved read/write format */
    hr = snd_pcm_hw_params_set_access(_pAlsaHandle, _pHwParams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (hr < 0) {
            printf("Access type not available for playback: %s\n", snd_strerror(hr));
            return hr;
    }    
    	
    snd_pcm_format_t SampleFormat = (_FxPcmFmt.FormatTag == FLOAT32_TYPE) ? SND_PCM_FORMAT_FLOAT : SND_PCM_FORMAT_S16;    /* sample format */
    /* set the sample format */
    _IsPcmFloatAccepted = FALSE;
    hr = snd_pcm_hw_params_set_format(_pAlsaHandle, _pHwParams, SampleFormat);
    if (hr < 0) {
        printf("Sample format not available for playback: %s\n", snd_strerror(hr));
        if(_FxPcmFmt.FormatTag == FLOAT32_TYPE) {
        	SampleFormat = SND_PCM_FORMAT_S16; //!< Try the format that should work on almost all devices  
			hr = snd_pcm_hw_params_set_format(_pAlsaHandle, _pHwParams, SampleFormat);
			if (hr < 0) {
				printf("Sample format not available for playback: %s\n", snd_strerror(hr));
				return hr;
			}
	    }
	    else
	    	return hr;
    }
    else {
    	if(_FxPcmFmt.FormatTag == FLOAT32_TYPE)
        	_IsPcmFloatAccepted = TRUE;
    }
    
    /* set the count of channels */
    unsigned int channels = _FxPcmFmt.wChannels;
    hr = snd_pcm_hw_params_set_channels(_pAlsaHandle, _pHwParams, channels);
    if (hr < 0) {
            printf("Channels count (%i) not available for playbacks: %s\n", _FxPcmFmt.wChannels, snd_strerror(hr));
            return hr;
    }
    /* set the stream rate */
    unsigned int dwReqFs = _FxPcmFmt.dwSamplingRate;
    hr = snd_pcm_hw_params_set_rate_near(_pAlsaHandle, _pHwParams, &dwReqFs, 0);
    if (hr < 0) {
            printf("Rate %iHz not available for playback: %s\n", _FxPcmFmt.dwSamplingRate, snd_strerror(hr));
            return hr;
    }
    if (dwReqFs != _FxPcmFmt.dwSamplingRate) {
            printf("Rate doesn't match (requested %iHz, get %iHz)\n", _FxPcmFmt.dwSamplingRate, hr);
            return -EINVAL;
    }
    /* set the buffer time */
    unsigned int dwMaxBufferTime;
    hr =  snd_pcm_hw_params_get_buffer_time_max(_pHwParams, &dwMaxBufferTime, 0);
	if (hr < 0) {
            printf("Error on snd_pcm_hw_params_get_buffer_time_max\n");
            return hr;
    }
    g_dwBufferTime = (g_dwBufferTime > dwMaxBufferTime) ? dwMaxBufferTime : g_dwBufferTime;
    g_dwBufferTime -= g_dwBufferTime%4;
    int dir;
    hr = snd_pcm_hw_params_set_buffer_time_near(_pAlsaHandle, _pHwParams, &g_dwBufferTime, &dir);
    if (hr < 0) {
            printf("Unable to set buffer time %i for playback: %s\n", g_dwBufferTime, snd_strerror(hr));
            return hr;
    }
    
    snd_pcm_uframes_t size;
    hr = snd_pcm_hw_params_get_buffer_size(_pHwParams, &size);
    if (hr < 0) {
            printf("Unable to get buffer size for playback: %s\n", snd_strerror(hr));
            return hr;
    }
    g_dwBufferSize = size;
    /* set the period time */
    g_dwPeriodTime = g_dwBufferTime / 4;
    hr = snd_pcm_hw_params_set_period_time_near(_pAlsaHandle, _pHwParams, &g_dwPeriodTime, &dir);
    if (hr < 0) {
            printf("Unable to set period time %i for playback: %s\n", g_dwPeriodTime, snd_strerror(hr));
            return hr;
    }
    
    /* write the parameters to device */
    hr = snd_pcm_hw_params(_pAlsaHandle, _pHwParams);
    if (hr < 0) {
            printf("Unable to set hw params for playback: %s\n", snd_strerror(hr));
            return hr;
    }
    
    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	SetSw()
	Set hardware device parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::SetSw()
{
	Int32 hr = FX_OK;
	/* get the current swparams */
	snd_pcm_sw_params_alloca(&_pSwParams);
	hr = snd_pcm_sw_params_current(_pAlsaHandle, _pSwParams);
    if (hr < 0) {
            printf("Unable to determine current swparams for playback: %s\n", snd_strerror(hr));
            return hr;
    }
    
    /* start with prebuffering time: */
    Uint16 wPrebufferingTime;
	_pFxParam->GetFxParamValue("Prebuffering Time", &wPrebufferingTime);
	Uint32 dwPrebufferingTime = wPrebufferingTime * 1000; //!< To us
	dwPrebufferingTime = (dwPrebufferingTime > g_dwBufferTime) ? g_dwBufferTime : dwPrebufferingTime;
	g_dwPrebufferingSample = ((Float)dwPrebufferingTime / 1000000) * _FxPcmFmt.dwSamplingRate;
	hr = snd_pcm_sw_params_set_start_threshold(_pAlsaHandle, _pSwParams, g_dwPrebufferingSample);
    if (hr < 0) {
            printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(hr));
            return hr;
    }
    snd_pcm_sw_params_get_start_threshold(_pSwParams, &g_dwPrebufferingSample);
    
    /* write the parameters to the playback device */
    hr = snd_pcm_sw_params(_pAlsaHandle, _pSwParams);
    if (hr < 0) {
            printf("Unable to set sw params for playback: %s\n", snd_strerror(hr));
            return hr;
    }
    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	Recovery()
	Underrun and suspend recovery.
*//*-----------------------------------------------------------------------------*/
Int32 CFxAlsaRnd::Recovery(Int32 hr)
{
	if (hr == -EPIPE) {    /* under-run */
        hr = snd_pcm_prepare(_pAlsaHandle);
        if (hr < 0)
            printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(hr));
        return 0;
    } else if (hr == -ESTRPIPE) {
        while ((hr = snd_pcm_resume(_pAlsaHandle)) == -EAGAIN)
        	sleep(1);       /*! wait until the suspend flag is released */
        if (hr < 0) {
            hr = snd_pcm_prepare(_pAlsaHandle);
            if (hr < 0)
            	printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(hr));
        }
        return 0;
    }
    return hr;
}

