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
   @file  FxWavFileRnd.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/
//#include <gtk/gtk.h>

/*! 
 *  sndfile.h is part of libsndfile libary.
 *  libsndfile was written by Erik de Castro Lopo (erikd AT mega-nerd DOT com).  
 *  The libsndfile home page is at :
 *  http://www.mega-nerd.com/libsndfile/
 */
#include <sndfile.h>

#include "FxWavFileRnd.h"

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
#define SAMPLE_REQUEST (441) //!< 10ms at 44100

FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

//! The Fx names
const Char g_strFxName[] = "FxWavFileRnd";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2008)";
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
	return (IFxBase*)(new CFxWavFileRnd);
}
#ifdef __cplusplus
}
#endif

CFxWavFileRnd::CFxWavFileRnd()
{
	/*! Initialize the Fx components */
	_FxState = FX_STOP_STATE;

	_pFxState		= NULL;

	_pFxParam 		= NULL;

    _pFxDescriptor  = NULL;

	_InPinState		= PIN_NOT_CONNECTED;

	_pFxInputPin	= NULL;

	_sndfile = NULL;
}

CFxWavFileRnd::~CFxWavFileRnd()
{
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxWavFileRnd::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
Int32 CFxWavFileRnd::InitFx(IFx **ppFxComponent)
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
		"Input pin"					//!< Pin Name
		, PIN_IN					//!< Pin Type
		, &g_FxMediaType			//!< Media Type
		, 1							//!< Media Type number
		, (IFxPinCallback*)this		//!< Pin Callback
	};

	/*! Create the Fx pins */
	hr = pFxPinManager->Create(_FxPin, &_pFxInputPin);
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
    FX_PARAM_STRING FxParamSoundFile = {"Sound File", "./"};
	hr = _pFxParam->AddFxParam(&FxParamSoundFile);
	if(FEF_FAILED(hr)) {
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(_FxState = FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	DisplayFxPropertyPage()
	Displays the Fx property page.
*//*-----------------------------------------------------------------------------*/
Int32 CFxWavFileRnd::DisplayFxPropertyPage(Pvoid pvWndParent)
{
	/* Get sound file Fx parameter */
	std::string strFileNameParam;
	_pFxParam->GetFxParamValue("Sound File", strFileNameParam);
	/*! strFileNameParam must exit to appear in edit box of FileChooserWidget */
	
// 	GtkWidget *FileChooserWidget;
// 	FileChooserWidget = gtk_file_chooser_dialog_new("Save File",
// 										 NULL,
// 										 GTK_FILE_CHOOSER_ACTION_SAVE,
// 										 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
// 										 GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
// 										 NULL);
// 	
// 	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (FileChooserWidget), strFileNameParam.c_str());
// 	gtk_widget_queue_draw(FileChooserWidget);
// 
// 	if(gtk_dialog_run(GTK_DIALOG(FileChooserWidget)) == GTK_RESPONSE_ACCEPT) {
// 		std::string strFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileChooserWidget));
// 		
		std::string strFileName = "C:\\notify.wav";
		/* Set sound file Fx parameter */
		_pFxParam->SetFxParamValue("Sound File", strFileName);
		StopFx();
		/*! Create empty file */
		_sfinfo.frames = 0;
		_sfinfo.channels = 1;			
		_sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		_sfinfo.samplerate = 44100;
		_sndfile = sf_open (strFileName.c_str(), SFM_WRITE, &_sfinfo);
		StopFx();
		if(_FxState == FX_RUN_STATE) {
			StartFx();
			RunFx();
		}
		else
			_pFxState->FxPublishState(_FxState = FX_IDLE_STATE);
// 	}
// 	gtk_widget_destroy(FileChooserWidget);
	
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxWavFileRnd::StartFx()
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
Int32 CFxWavFileRnd::RunFx()
{
    Int32 hr = FX_OK;
    
    if(_FxState != FX_RUN_STATE) {
        _pFxState->FxPublishState(_FxState = FX_RUN_STATE);
	}

	/*! Get first samples, at this step, all Fxs in the FxEngine are started */
	_pFxInputPin->WaitForIFxMedia(SAMPLE_REQUEST, NULL);

    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxWavFileRnd::StopFx()
{
	/*! In all cases, close sound file */
	if(_sndfile) {
		sf_close (_sndfile);
		_sndfile = NULL;
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
Int32 CFxWavFileRnd::ReleaseFx()
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
Void CFxWavFileRnd::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
{
	if(PinState == PIN_CONNECTED)
		_pFxState->FxPublishState(_FxState = FX_CONNECT_STATE);
    else
        _pFxState->FxPublishState(_FxState = FX_DISCONNECT_STATE);
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxMedia()
	Decides the Media buffer size and number.
*//*-----------------------------------------------------------------------------*/
Void CFxWavFileRnd::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	/*! Returns the default values */ 
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxPin()
	Receives the new Fx media on input pin.
*//*-----------------------------------------------------------------------------*/
Int32 CFxWavFileRnd::FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
{
	Int32 hr = FX_OK;

	if(StreamState == STREAM_FLUSH) {
		StopFx();
		return hr;
	}
	
	FX_PCM_FORMAT FxPcmFmt;
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
		_pFxState->FxPublishState(_FxState = FX_STREAM_INIT_STATE);
	}	
	
	Uint32 dwInputDatalength;
	pIFxMedia->GetDataLength(&dwInputDatalength);
	
	if(dwInputDatalength == 0)
		return FX_OK;
	
	/*! Get media pointer */
	Uint8 *pSrcData;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaPointer(&pSrcData)) ) {
		_pFxState->FxPublishState(_FxState = FX_ERROR_PIN_STATE);
		return hr;
	}	

	/*! Open file */
	if(_sndfile == NULL) {
		_sfinfo.frames = 0;
		_sfinfo.channels = FxPcmFmt.wChannels;			
		_sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		_sfinfo.samplerate = FxPcmFmt.dwSamplingRate;
		_pFxParam->GetFxParamValue("Sound File", _strFileName);
		if (! (_sndfile = sf_open (_strFileName.c_str(), SFM_WRITE, &_sfinfo))) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_INVALID_PARAM);
			
			return FX_INVALPARAM;
		}	
	}

	/*! Write data */
	Uint32 dwItemToWrite = dwInputDatalength / sizeof(Float);
	if( sf_write_float(_sndfile, (Float*)pSrcData, dwItemToWrite) != dwItemToWrite) {
		_pFxState->FxPublishState(_FxState = FX_ERROR_PIN_STATE);
		return FX_ERROR;
	}
	/*! Do not ask too quickly data !!!! */
	Uint32 dwSampleTime = (Uint32)1000000 * (dwItemToWrite / FxPcmFmt.wChannels) / FxPcmFmt.dwSamplingRate;
//  usleep(dwSampleTime);
	_sleep(dwSampleTime/1000);
	
	/*! Get next sound data */
	if( (_FxState != FX_STOP_STATE) && (_FxState != FX_PAUSE_STATE) )
		_pFxInputPin->WaitForIFxMedia(SAMPLE_REQUEST, NULL);
	
	return hr;
}
