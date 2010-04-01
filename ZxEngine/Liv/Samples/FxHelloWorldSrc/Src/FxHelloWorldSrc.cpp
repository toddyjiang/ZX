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
   @file  FxHelloWorldSrc.cpp
   @brief This Fx is an example from the FxEngine Framework.  
*//*_______________________________________________________________________*/
#include "FxHelloWorldSrc.h"

/*! Includes XPM pictures */
#include "FxFrames.h"

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType = {TEXT_TYPE, ASCII};

//! The Fx names
const Char g_strFxName[] = "FxHelloWorldSrc Demo";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2007)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";

/*-----------------------------------------------------------------------------*//*!
	Dll entry point.
*//*-----------------------------------------------------------------------------*/
#ifdef WIN32
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
	return (IFxBase*)(new CFxHelloWorldSrc);
}
#ifdef __cplusplus
}
#endif

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
        _pFxDescriptor->FxScope = FX_SCOPE_TEXT;
    }

    *ppFxDescriptor = _pFxDescriptor;
			
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	InitFx()
	Initializes the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::InitFx(IFx **ppFxComponent)
{
	Int32 hr = FX_ERROR;

	/*! Initialize the Fx components */
	_FxState		= FX_STOP_STATE;
	_pFxState		= NULL;
	_pFxParam 		= NULL;
    _pFxDescriptor  = NULL;
	_OutPinState	= PIN_NOT_CONNECTED;
	_pFxOutputPin	= NULL;
	_qTimeStamp = 0;

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
        "Text Output"				//!< Pin Name
		, PIN_OUT					//!< Pin Type
		, &g_FxMediaType			//!< Media Type
		, 1							//!< Media Type number
		, (IFxPinCallback*)this		//!< Pin Callback
	};

	/*! Create the output Fx pin */
	hr = pFxPinManager->Create(_FxPin, &_pFxOutputPin);
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
   FX_PARAM_STRING FxParamNull = {"Fx Hello Word parameter", "Null parameter value"};
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
	DisplayFxPropertyPage()
	Displays the Fx property page.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::DisplayFxPropertyPage(Pvoid pvWndParent)
{
    /*! Fx has no Fx Property Page */
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	GetFxFrame()
    Returns the current FxFrame.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::GetFxFrame(const Char** ppbFxFrame)
{
    /*! Fill the Fx frame */
	*ppbFxFrame = (const Char*)HelloWorld_xpm;
    
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::StartFx()
{
	if(_FxState != FX_START_STATE)
	{
        _FxState = FX_START_STATE;
		
        _pFxState->FxPublishState(_FxState);
	}

    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	RunFx()
	Run the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::RunFx()
{
    Int32 hr = FX_OK;
    
    if(_FxState != FX_RUN_STATE)
	{
        _FxState = FX_RUN_STATE;
		
        _pFxState->FxPublishState(_FxState);
	}

    /*! Send Hello world on each start-run call */
    if(FEF_FAILED(SendhelloWorld()))
    {
        _FxState = FX_ERROR_STATE;
        _pFxState->FxPublishState(_FxState);
    }

	return hr;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::StopFx()
{
	/*! Flush downstream */
	_pFxState->FxPublishState(FX_FLUSH_STATE);
	_pFxOutputPin->Flush();

	if(_FxState != FX_STOP_STATE)
	{
        _FxState = FX_STOP_STATE;
		_pFxState->FxPublishState(_FxState);
	}

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::ReleaseFx()
{
	SAFE_DELETE_OBJECT(_pFxDescriptor);
	_pFxState->FxPublishState(FX_RELEASE_STATE);	/*!< Update the Fx state to "Release" */
	_pFxState->FxReleaseInterface();
	
	SAFE_RELEASE(_pFx);

	delete this; //!< Delete FxBase object
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	FxPinState()
	Receives the Fx Pins states.
*//*-----------------------------------------------------------------------------*/
Void CFxHelloWorldSrc::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState )
{
    _OutPinState = PinState;

	if(_OutPinState == PIN_CONNECTED)
		_pFxState->FxPublishState(FX_CONNECT_STATE);
    else
        _pFxState->FxPublishState(FX_DISCONNECT_STATE);
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxMedia()
	Decides the Media buffer size and number.
*//*-----------------------------------------------------------------------------*/
Void CFxHelloWorldSrc::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	/*! Returns the default values */ 

	return;
}

/*-----------------------------------------------------------------------------*//*!
	SendhelloWorld()
	Send Hello World text on its output pin.
*//*-----------------------------------------------------------------------------*/
Int32 CFxHelloWorldSrc::SendhelloWorld()
{
    Int32 hr = FX_OK;
	
	/*! Get new FxMedia to fill */
	IFxMedia* pFxMedia;
	if( FEF_FAILED(hr = _pFxOutputPin->GetDeliveryMedia(&pFxMedia)) ) /*!< The Media type of connection is set !! */  
    {
        _FxState = FX_ERROR_STATE;
        _pFxState->FxPublishState(_FxState);
        return hr;
    }

	/*! Get media pointer, Fill and send data */
	Uint8* pData;
	if( FEF_FAILED(hr = pFxMedia->GetMediaPointer(&pData)) )
	{
        /*! Release Media buffer */
        pFxMedia->Release();
        _FxState = FX_ERROR_STATE;
        _pFxState->FxPublishState(_FxState);
        return hr;
    }
    std::string strHelloWorld = "Hello World";
    sprintf((Char*)pData, strHelloWorld.c_str(), strHelloWorld.size());

    /*! Update the valid data length */
	pFxMedia->SetDataLength(strHelloWorld.size());

    /*! Set media propperties */
    pFxMedia->SetFxMediaName(strHelloWorld);

	/*! Set timestamp */
    pFxMedia->SetTimeStamp(_qTimeStamp++);
    
    /*! Deliver the FxMedia to the next Fx */
	_pFxOutputPin->DeliverMedia(pFxMedia);

    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	FxWaitForIFxMedia()
	Send Hello World text when the downstream asks it.
*//*-----------------------------------------------------------------------------*/
void CFxHelloWorldSrc::FxWaitForIFxMedia(IFxPin* pFxPin, Uint32 dwTimeStamp, FX_PTR dwUser)
{
	if(_FxState == FX_RUN_STATE)
    {
        if(FEF_FAILED(SendhelloWorld()))
        {
            _FxState = FX_ERROR_STATE;
            _pFxState->FxPublishState(_FxState);
        }
    }

	return;
}

