/*!
   @file  AudioRender.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/

#include "SDL_config.h"
#include "SDL.h"
#include "SDL_audio.h"

#include "IFxBase.h"
#include "pthread.h"

#include <vector>
#include <boost/circular_buffer.hpp>

#include "AudioRender.h"

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FEF::FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

Uint32 g_dwFs = 44100;                           /* stream rate */
Uint32 g_dwChannels = 2;                         /* count of channels */
unsigned int g_dwBufferTime = 1000000;           /* ring buffer length in us */
unsigned int g_dwPeriodTime;					 /* periode time */
unsigned int g_dwPrebufferingTime;				 /* prebuffering time in us */


//! The Fx names
const Char g_strFxName[] = "AudioRender";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2009)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";

//! TimeOut
const Uint32 g_dwTimeOut = 3000000; //!< us

//! Defaults values
uint16_t g_wDefaultPreBuffering ; //!< 400ms
uint16_t g_wMinPreBuffering; //!< 5ms
uint16_t g_wMaxPreBuffering; //!< 1000ms

uint16_t g_wDefaultBufferLength; //!< 50ms
uint16_t g_wMinBufferLength; //!< 5ms
uint16_t g_wMaxBufferLength; //!< 250ms

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
	return (IFxBase*)(new CAudioRender);
}
#ifdef __cplusplus
}
#endif

void SDLCALL
CAudioRender::fillerup(void *unused, Uint8 * stream, int len)
{
	Uint8*	waveptr		= NULL;
	int		waveleft	= 0;

	/* Set up the pointers */
	if (_MediaBuffers.size() == 0)
	{
		stream[0] = 0;
		len = 0;
		return;
	}
	ZX_Media_Buffer_t buffer = _MediaBuffers[0];
 	waveptr		= buffer.pData;
	waveleft	= buffer.nData;

	/* Go! */
// 	while (waveleft <= len) {
// 		SDL_memcpy(stream, waveptr, waveleft);
// 		stream += waveleft;
// 		len -= waveleft;
// 		waveptr = wave.sound;
// 		waveleft = wave.soundlen;
// 		wave.soundpos = 0;
//	}
	if (waveleft <= len)
	{
		SDL_memcpy(stream, waveptr, waveleft);
		len = waveleft;
	}
	_MediaBuffers.pop_front();
}

boost::circular_buffer<ZX_Media_Buffer_t> CAudioRender::_MediaBuffers(3);

CAudioRender::CAudioRender():
_FxState(FX_STOP_STATE),
_pFxState(NULL),
_pFxParam(NULL),
_pFxDescriptor(NULL),
_pFxInputPin(NULL),
_dwAskedSample(0)
{
	memset(&_Thread, 0, sizeof(pthread_t));

	pthread_mutex_init( &_ThreadMutex, NULL );
}

CAudioRender::~CAudioRender()
{
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioRender::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
Int32 CAudioRender::InitFx(IFx **ppFxComponent)
{
	Int32 hr = FX_ERROR;

	/*! Create the Fx */
	if(FEF_FAILED(hr = FEF_CreateFx(&(_pFx), g_strFxName)))
		return hr;

	*ppFxComponent = _pFx;

	/*! Get the IFxState and publish the loading state */
	hr = _pFx->FxGetInterface(IFX_STATE, (void**)&_pFxState);
	if(FEF_FAILED(hr))
		return hr;	

	_pFxState->FxPublishState(FX_LOADING_STATE);

	/*! Get the IFxPinManager to create the Fx pins */
	IFxPinManager* pFxPinManager = NULL;
	hr = _pFx->FxGetInterface(IFX_PINMANGER, (void**)&pFxPinManager);
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
	hr = _pFx->FxGetInterface(IFX_PARAM, (void**)&_pFxParam);
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
	    
    /*! Set default audio format */
    _FxPcmFmt.wChannels = g_dwChannels;
    _FxPcmFmt.dwSamplingRate = 0;
    _FxPcmFmt.FormatTag = FLOAT32_TYPE;
		
	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(_FxState = FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFxParam()
	Update Fx Parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioRender::UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter)
{	
	return FX_OK;  
}

/*-----------------------------------------------------------------------------*//*!
	DisplayFxPropertyPage()
	Displays the Fx property page.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioRender::DisplayFxPropertyPage(Pvoid pvWndParent)
{
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioRender::StartFx()
{
	Int32 hr;

	/* Load the SDL library */
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return (1);
	}

	MakeSDLSpec();

	/* Initialize fillerup() variables */
	if (SDL_OpenAudio(&_Spec, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
// 		SDL_FreeWAV(wave.sound);
	}

	g_dwPeriodTime = g_dwBufferTime / 4;
	
	_ShouldCloseThread = TRUE;
	pthread_join (_Thread, NULL);

	_pFxState->FxPublishState(_FxState = FX_STOP_STATE);
			
	_pFxState->FxPublishState(_FxState = FX_START_STATE);

    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	RunFx()
	Run the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioRender::RunFx()
{
	/*! Restart thread */
	_ShouldCloseThread = FALSE;

	if(_FxState != FX_RUN_STATE) {
		_pFxState->FxPublishState(_FxState = FX_RUN_STATE);
	}

	/*! Start new thread */
	if(pthread_create (&_Thread, NULL, CAudioRender::ThreadFctWrapper, (void *)this) != 0){
		return FX_ERROR;
	}

	SDL_PauseAudio(0);
	
    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioRender::StopFx()
{
	SDL_PauseAudio(1);

	_ShouldCloseThread = TRUE;
	pthread_join (_Thread, NULL);

	_pFxState->FxPublishState(_FxState = FX_STOP_STATE);
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioRender::ReleaseFx()
{
	/*! Stop and release Fx components */
	StopFx();
	
// 	SDL_FreeWAV(wave.sound);
	SDL_Quit();

	_pFxState->FxPublishState(FX_RELEASE_STATE);	/*!< Update the Fx state to "Release" */
	
	_pFxState->FxReleaseInterface();
	
	/*! Delete the Fx instance */
	delete this;
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	FxPinState()
	Receives the Fx Pins states.
*//*-----------------------------------------------------------------------------*/
void CAudioRender::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
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
Int32 CAudioRender::FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
{
	Int32 hr = FX_OK;
	
	CAutoLock Lock(&_ThreadMutex);
	
	if(StreamState == STREAM_FLUSH) {
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
		if(FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (void**)&pFxPcmFormat))) {
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
		/*! Update format */
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
			}
		}
		_pFxState->FxPublishState(FX_STREAM_INIT_STATE);
	}
	
	if(_FxState == FX_FLUSH_STATE) //!< Fx must be restarted
		return FX_OK;
	
	Uint32 dwInputDatalength;
	pIFxMedia->GetDataLength(&dwInputDatalength);
	Uint32 dwSampleNumber = dwInputDatalength / (((_FxPcmFmt.FormatTag == FLOAT32_TYPE) ? sizeof(Float) : sizeof(Int16)) * _FxPcmFmt.wChannels);
	if(dwSampleNumber == 0)
		return FX_OK;
	
	/*! Get media pointer */
	Uint8 *pSrcData;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaPointer(&pSrcData)) ) {
		_pFxState->FxPublishState(FX_ERROR_PIN_STATE);
		return hr;
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
	
	/*! TODO: Write data */
	ZX_Media_Buffer_t buffer;
	buffer.pData = pSrcData;
	buffer.nData = dwInputDatalength;
	_MediaBuffers.push_back(buffer);

    /*! Update written sample */
    _qWrittenSample += dwSampleNumber;
    
    /*! Update incoming size */
    _dwAskedSample = (_dwAskedSample < dwSampleNumber) ? 0: _dwAskedSample - dwSampleNumber; //!< Prevent Underrun
			
	return hr;
}

#define SAMPLE_REQUEST (441) 
/*-----------------------------------------------------------------------------*//*!
	ThreadFct()
	Audio thread processing.
*//*-----------------------------------------------------------------------------*/

void* CAudioRender::ThreadFct() 
{
	while(!_ShouldCloseThread) {

 		pthread_mutex_lock(&_ThreadMutex);
// 		while(1) {
// 			pthread_mutex_unlock(&_ThreadMutex);
			_pFxInputPin->WaitForIFxMedia(SAMPLE_REQUEST, NULL);
// 			pthread_mutex_lock(&_ThreadMutex);
// 			dwWaitingTime = 0;
// 		}
 		pthread_mutex_unlock(&_ThreadMutex);

		/*! Watchdog */
		_sleep(g_dwPeriodTime/1000);
	}
	return NULL;
}

void CAudioRender::MakeSDLSpec()
{
	_Spec.callback	= CAudioRender::fillerup;
	_Spec.channels	= _FxPcmFmt.wChannels;
// 	_Spec.format	= _FxPcmFmt.FormatTag;]
	_Spec.format	= AUDIO_S16LSB;
	_Spec.freq		= _FxPcmFmt.dwSamplingRate;
	_Spec.samples	= 4096;
}