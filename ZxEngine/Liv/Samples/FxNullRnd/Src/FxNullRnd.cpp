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
   @file  FxNullRnd.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/
#include "FxNullRnd.h"

/*! Includes XPM pictures */
#include "FxFrames.h"

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType[] = {
                                    {AUDIO_TYPE, PCM},
                                    {VIDEO_TYPE, RGB},
                                    {TEXT_TYPE, ASCII}
                                };

//! The Fx names
const Char g_strFxName[] = "FxNullRnd Demo";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2007)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";

#ifdef WIN32
/*-----------------------------------------------------------------------------*//*!
	Dll entry point.
*//*-----------------------------------------------------------------------------*/
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			/*! Useful optimization for multithreaded  */
			DisableThreadLibraryCalls((HMODULE)hModule);
			break;
		}
	}
	return TRUE;
}
#endif

/*-----------------------------------------------------------------------------*//*!
	FEF_GetFxBase()
	The Fx entry point.
*//*-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
FXENGINE_EXP IFxBase* FXENGINE_API FEF_GetFxBase()
{
	return (IFxBase*)(new CFxNullRnd);
}
#ifdef __cplusplus
}
#endif

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxNullRnd::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
        _pFxDescriptor->FxScope = FX_SCOPE_ALL;
    }

    *ppFxDescriptor = _pFxDescriptor;
			
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	InitFx()
	Initializes the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxNullRnd::InitFx(IFx **ppFxComponent)
{
	Int32 hr = FX_ERROR;

	/*! Initialize the Fx components */
	_FxState		= FX_STOP_STATE;
	_pFxState		= NULL;
	_pFxParam 		= NULL;
    _pFxDescriptor  = NULL;
	_InPinState		= PIN_NOT_CONNECTED;
	_pFxInputPin	= NULL;

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
	{     "Null Input"			//!< Pin Name
		, PIN_IN				//!< Pin Type
		, g_FxMediaType			//!< Media Type
		, 3						//!< Media Type number
		, (IFxPinCallback*)this	//!< Pin Callback
	};

	/*! Create the input Fx pin */
	hr = pFxPinManager->Create(_FxPin, &_pFxInputPin);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(pFxPinManager);
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
   Char strDefault[MAX_PATH] = "Null parameter value";
	FX_PARAM FxParamNull = {"Fx Null parameter", "Null", &strDefault, NULL, NULL, UINT8_TYPE, MAX_PATH};
	hr = _pFxParam->AddFxParam(&FxParamNull);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	GetFxFrame()
    Returns the current FxFrame.
*//*-----------------------------------------------------------------------------*/
Int32 CFxNullRnd::GetFxFrame(const Char** ppbFxFrame)
{
    /*! Fill the FxFrame*/
	*ppbFxFrame = (const Char*)FxNullRnd_xpm;
    
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxNullRnd::StartFx()
{
	if(_FxState != FX_START_STATE)
	{
        _FxState = FX_START_STATE;
		
        _pFxState->FxPublishState(FX_START_STATE);
	}

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxNullRnd::StopFx()
{
	if(_FxState != FX_STOP_STATE)
	{
        _FxState = FX_STOP_STATE;
		
        _pFxState->FxPublishState(FX_STOP_STATE);
	}

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CFxNullRnd::ReleaseFx()
{
	/*! Stop and release Fx components */
	StopFx();

	SAFE_RELEASE_INTERFACE(_pFxParam);
	SAFE_DELETE_OBJECT(_pFxDescriptor);
	_pFxState->FxPublishState(FX_RELEASE_STATE);	//!< Update the Fx state to "Release"
	SAFE_RELEASE_INTERFACE(_pFxState);
	SAFE_RELEASE(_pFx);
	
	delete this; //!< Delete FxBase object
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	FxPinState()
	Receives the Fx Pins states.
*//*-----------------------------------------------------------------------------*/
Void CFxNullRnd::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
{
    _InPinState = PinState;
	if(_InPinState == PIN_CONNECTED)
		_pFxState->FxPublishState(FX_CONNECT_STATE);
	else
		_pFxState->FxPublishState(FX_DISCONNECT_STATE);

	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxMedia()
	Decides the Media buffer size and number.
*//*-----------------------------------------------------------------------------*/
Void CFxNullRnd::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	/*! Returns the default values */ 

	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxPin()
	Receives the new Fx media on input pin.
*//*-----------------------------------------------------------------------------*/
Int32 CFxNullRnd::FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
{
	/*! I'm a Null Fx ;) */
	return FX_OK;
}

