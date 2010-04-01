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
   @file  FxSndFileSrc.cpp
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

#include "FxSndFileSrc.h"


/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

//! The Fx names
const Char g_strFxName[] = "FxSndFileSrc";
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
	return (IFxBase*)(new CFxSndFileSrc);
}
#ifdef __cplusplus
}
#endif

CFxSndFileSrc::CFxSndFileSrc()
{
	/*! Initialize the Fx components */
	_FxState = FX_STOP_STATE;

	_pFxState		= NULL;

	_pFxParam 		= NULL;

    _pFxDescriptor  = NULL;

	_OutPinState	= PIN_NOT_CONNECTED;

	_pFxOutputPin	= NULL;

	_sndfile = NULL;
}

CFxSndFileSrc::~CFxSndFileSrc()
{
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
        _pFxDescriptor->FxType = FX_SOURCE;
        _pFxDescriptor->FxScope = FX_SCOPE_AUDIO;
    }

    *ppFxDescriptor = _pFxDescriptor;
			
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	InitFx()
	Initializes the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::InitFx(IFx **ppFxComponent)
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
	if(FEF_FAILED(hr)) {
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	FX_PIN _FxPin[] =
	{ 
        "Sound Output"				//!< Pin Name
		, PIN_OUT					//!< Pin Type
		, &g_FxMediaType			//!< Media Type
		, 1							//!< Media Type number
		, (IFxPinCallback*)this		//!< Pin Callback
	};

	/*! Create the output Fx pin */
	hr = pFxPinManager->Create(_FxPin, &_pFxOutputPin);
	if(FEF_FAILED(hr)) {
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! We can release the PinManager */
	SAFE_RELEASE_INTERFACE(pFxPinManager);

	/*! Get the IFxParam to create the Fx public parameters */
	hr = _pFx->FxGetInterface(IFX_PARAM, (Void**)&_pFxParam);
	if(FEF_FAILED(hr)) {
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
Int32 CFxSndFileSrc::DisplayFxPropertyPage(Pvoid pvWndParent)
{
	/*! Get sound file Fx parameter */
	std::string strFileNameParam;
	_pFxParam->GetFxParamValue("Sound File", strFileNameParam);

// 	GtkWidget *FileChooserWidget;
// 	FileChooserWidget = gtk_file_chooser_dialog_new("Open File",
// 										 NULL,
// 										 GTK_FILE_CHOOSER_ACTION_OPEN,
// 										 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
// 										 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
// 										 NULL);
// 
// 	gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (FileChooserWidget), strFileNameParam.c_str());
// 	gtk_widget_queue_draw(FileChooserWidget);
// 
// 	if(gtk_dialog_run(GTK_DIALOG(FileChooserWidget)) == GTK_RESPONSE_ACCEPT) {
// 		std::string strFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileChooserWidget));
// 		
		std::string strFileName = "C:\\WINDOWS\\Media\\notify.wav";
 		/* Set sound file Fx parameter */
 		_pFxParam->SetFxParamValue("Sound File", std::string(strFileName));
		
		/*! Open file */
		OpenSoundFile();		
		
// 	}
// 	gtk_widget_destroy(FileChooserWidget);
	
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFxParam()
	Update Fx Parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter)
{
	/*! Open file */
	OpenSoundFile();
	
	return FX_OK;  
}

/*-----------------------------------------------------------------------------*//*!
	OpenSoundFile()
	Open sound file and send stream format.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::OpenSoundFile()
{
	Int32 hr = FX_OK;
	
	/*! In all cases, close sound file */
	if(_sndfile) {
		sf_close (_sndfile);
		_sndfile = NULL;
	}
	
	/*! Get sound file Fx parameter */
	std::string strFileName;
	_pFxParam->GetFxParamValue("Sound File", strFileName);
	
	/*! Initialize downstream */
	if (! (_sndfile = sf_open (strFileName.c_str(), SFM_READ, &_sfinfo))) {
		_pFxState->FxPublishState(_FxState = FX_ERROR_INVALID_PARAM);
		return FX_INVALPARAM;
	}	
	
	/*! Get new FxMedia to fill */
	IFxMedia* pIFxMedia;
	if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pIFxMedia)) ) /*!< The Media type of connection is set !! */  
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
	    return hr;
	}
	
	/*! Get Media PCM format */
	IFxPcmFormat* pIFxPcmFormat;
	if(FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
		/*! Release Media buffer */
	    pIFxMedia->Release();
		_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
		return hr;
	}	
	pIFxMedia->SetDataLength(0);
	/*! Set media properties */
	FX_PCM_FORMAT FxPcmFmt;
	FxPcmFmt.wChannels = _sfinfo.channels;			
	FxPcmFmt.FormatTag = FLOAT32_TYPE;			
	FxPcmFmt.dwSamplingRate = _sfinfo.samplerate;
	pIFxPcmFormat->SetPcmFormat(&FxPcmFmt);
	pIFxMedia->SetFxMediaName(strFileName);
	/*! Deliver the FxMedia to the next Fx */
	_pFxOutputPin->InitStream(pIFxMedia);	
		
	_pFxState->FxPublishState(_FxState = FX_IDLE_STATE);
	
	return hr;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::StartFx()
{
	Int32 hr = FX_OK;
	
	/*! Open file */
	if(_sndfile == NULL) {
		/*! Get sound file Fx parameter */
		std::string strFileName;
		_pFxParam->GetFxParamValue("Sound File", strFileName);
	
		/*! Try open */
		if (! (_sndfile = sf_open (strFileName.c_str(), SFM_READ, &_sfinfo))) {
			_pFxState->FxPublishState(_FxState = FX_ERROR_INVALID_PARAM);
			return FX_INVALPARAM;
		}
	}
	else {
		/*! Rewind file */
		if (sf_seek(_sndfile, 0, SEEK_SET) < 0){
			_pFxState->FxPublishState(_FxState = FX_ERROR_START_STATE);
			return FX_INVALPARAM;
		}
	}
	
	if(_FxState != FX_START_STATE) {
		_pFxState->FxPublishState(_FxState = FX_START_STATE);
	}
	
    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	RunFx()
	Run the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::RunFx()
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
Int32 CFxSndFileSrc::StopFx()
{
	/*! Flush downstream */
	_pFxState->FxPublishState(_FxState = FX_FLUSH_STATE);
	_pFxOutputPin->Flush();
			
	if(_FxState != FX_STOP_STATE) {
        _pFxState->FxPublishState(_FxState = FX_STOP_STATE);
	}

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::ReleaseFx()
{
	/*! In all cases, close sound file */
	if(_sndfile) {
		sf_close (_sndfile);
		_sndfile = NULL;
	}

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
Void CFxSndFileSrc::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
{
    _OutPinState = PinState;

	if(_OutPinState == PIN_CONNECTED) {
		_pFxState->FxPublishState(_FxState = FX_CONNECT_STATE);
		/*! Initialize downstream on connexion only */
		if(_sndfile) {
			/*! Get sound file Fx parameter */
			std::string strFileName;
			_pFxParam->GetFxParamValue("Sound File", strFileName);
			/*! Get new FxMedia to fill */
			Int32 hr = FX_OK;
			IFxMedia* pIFxMedia;
			if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pIFxMedia, INFINITE_TIME)) ) /*!< The Media type of connection is set !! */  
			{
				_pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
				return;
			}

			/*! Get Media PCM format */
			IFxPcmFormat* pIFxPcmFormat;
			if(FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
				/*! Release Media buffer */
				pIFxMedia->Release();
				_pFxState->FxPublishState(_FxState = FX_ERROR_MEDIA_PIN_STATE);
				return;
			}	
			pIFxMedia->SetDataLength(0);
			/*! Set media properties */
			FX_PCM_FORMAT FxPcmFmt;
			FxPcmFmt.wChannels = _sfinfo.channels;			
			FxPcmFmt.FormatTag = FLOAT32_TYPE;			
			FxPcmFmt.dwSamplingRate = _sfinfo.samplerate;
			pIFxPcmFormat->SetPcmFormat(&FxPcmFmt);
			pIFxMedia->SetFxMediaName(strFileName);

			/*! Deliver the FxMedia to the next Fx */
			_pFxOutputPin->InitStream(pIFxMedia);
		}
	}
    else
        _pFxState->FxPublishState(_FxState = FX_DISCONNECT_STATE);
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxMedia()
	Decides the Media buffer size and number.
*//*-----------------------------------------------------------------------------*/
Void CFxSndFileSrc::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	/*! Define 10 buffers of 50ms (44100, stereo, 32 bits float pcm data) */ 
	*pdwFxMediaNumber = 10;
	*pdwFxMediaSize = (44100 * 4 * 2)/20;
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	SendData()
	Send sound PCM data on its output pin.
*//*-----------------------------------------------------------------------------*/
Int32 CFxSndFileSrc::SendData(Uint32 dwTimeStamp)
{
    Int32 hr = FX_OK;

	
	/*! How many sample to send ? */
	Uint32 dwSizeRequest = dwTimeStamp * sizeof(Float) * _sfinfo.channels;
		
	while(dwSizeRequest > 0) {
	
		/*! Get new FxMedia to fill */
		IFxMedia* pIFxMedia;
		if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pIFxMedia, INFINITE_TIME)) ) /*!< The Media type of connection is set !! */  
		{
		    _pFxState->FxPublishState(FX_ERROR_STATE);
		    return hr;
		}

		/*! Get Media PCM format */
		IFxPcmFormat* pIFxPcmFormat;
		if(FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
			/*! Release Media buffer */
		    pIFxMedia->Release();
			_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
			return hr;
		}
		FX_PCM_FORMAT FxPcmFmt;

		/*! Get media pointer, Fill and send data */
		Uint8* pData;
		if( FEF_FAILED(hr = pIFxMedia->GetMediaPointer(&pData)) )
		{
		    /*! Release Media buffer */
		    pIFxMedia->Release();
		    _pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
		    return hr;
		}
		Uint32 dwBufferSize;
		pIFxMedia->GetSize(&dwBufferSize);
		dwBufferSize -= dwBufferSize%(sizeof(Float) * _sfinfo.channels); //!< Must be modulus sample size

		/*! How many sample to read ? */
		Uint32 dwItemToRead = (dwBufferSize < dwSizeRequest) ? 
							     dwBufferSize / sizeof(Float) : 
							     dwSizeRequest / sizeof(Float) ;
		/*! Read sound file */
		Uint32 dwItemRead = sf_read_float(_sndfile, (Float*)pData, dwItemToRead);
		Uint32 dwReadSize = dwItemRead * sizeof(Float);
		pIFxMedia->SetDataLength(dwReadSize);
		if(dwReadSize > 0) {
			/*! Set media properties */
			FxPcmFmt.wChannels = _sfinfo.channels;			
			FxPcmFmt.FormatTag = FLOAT32_TYPE;			
			FxPcmFmt.dwSamplingRate = _sfinfo.samplerate;
			pIFxPcmFormat->SetPcmFormat(&FxPcmFmt);
			pIFxMedia->SetFxMediaName(_strFileName);

			/*! Deliver the FxMedia to the next Fx */
			_pFxOutputPin->DeliverMedia(pIFxMedia);		
		}
		else { //!< Flush downstream
			pIFxMedia->Release();
			/*! Flush downstream */
			_pFxState->FxPublishState(_FxState = FX_FLUSH_STATE);
			_pFxOutputPin->Flush();
			return FX_OK; //<! Exit loop
		}
		dwSizeRequest -= dwReadSize;
	}

    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	FxWaitForIFxMedia()
	Send sound data when the downstream asks it.
*//*-----------------------------------------------------------------------------*/
void CFxSndFileSrc::FxWaitForIFxMedia(IFxPin* pFxPin, Uint32 dwTimeStamp, FX_PTR dwUser)
{
	if( (_FxState == FX_RUN_STATE) || (_FxState == FX_START_STATE) )
		if(_sndfile) {
			if(FEF_FAILED(SendData(dwTimeStamp))) //!< Send requested samples on default pin
		    {
		        _pFxState->FxPublishState(_FxState = FX_ERROR_STATE);
		    }
		}

	return;
}
