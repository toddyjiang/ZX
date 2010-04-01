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
   @file  FxLADSPA.cpp
   @brief The Fx LADSPA allows to load and to use LADSPA plugins. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework. 
*//*_______________________________________________________________________*/
#include <gtk/gtk.h>
#include "FxLADSPA.h"

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
FX_MEDIA_TYPE g_FxMediaType = {AUDIO_TYPE, PCM};

//! The Fx names
const Char g_strFxName[] = "LADSPA Wrapper";
const Char g_strFxVersion[] = FX_VERSION;
const Char g_strFxAuthor[] = "SMProcess (2009)";
const Char g_strFxCopyright[] = "Permission is granted to copy, distribute and/or modify this Fx.";

/*! Defaults audio values */
extern unsigned long glDefaultFs;
extern unsigned long glAvailableFs[];
extern unsigned short gnAvailableFs;

/*-----------------------------------------------------------------------------*//*!
	FEF_GetFxBase()
	The Fx entry point.
*//*-----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
FXENGINE_EXP IFxBase* FXENGINE_API FEF_GetFxBase()
{
	return (IFxBase*)(new CFxLADSPA);
}
#ifdef __cplusplus
}
#endif

CFxLADSPA::CFxLADSPA()
{
	/*! Initialize the Fx components */
	_FxState = FX_STOP_STATE;

	_pFxState		= NULL;

	_pFxParam 		= NULL;

    _pFxDescriptor  = NULL;

	_pWrapper = NULL;
}

CFxLADSPA::~CFxLADSPA()
{
	SAFE_DELETE_OBJECT(_pWrapper);
	SAFE_RELEASE(_pFx);
    SAFE_DELETE_OBJECT(_pFxDescriptor);
}

/*-----------------------------------------------------------------------------*//*!
	GetFxInfo()
	Returns the Fx infos.
*//*-----------------------------------------------------------------------------*/
Int32 CFxLADSPA::GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor)
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
Int32 CFxLADSPA::InitFx(IFx **ppFxComponent)
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

	/*! Get the IFxParam to create the Fx public parameters */
	hr = _pFx->FxGetInterface(IFX_PARAM, (Void**)&_pFxParam);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! Create the Fx public parameters */
    FX_PARAM_STRING FxParamLADSPAFile = {"LADSPA plugin", "./"};
	hr = _pFxParam->AddFxParam(&FxParamLADSPAFile);
	if(FEF_FAILED(hr)) {
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	FX_PARAM_STRING FxParamLADSPALabel = {"LADSPA plugin label", "NULL"};
	hr = _pFxParam->AddFxParam(&FxParamLADSPALabel);
	if(FEF_FAILED(hr)) {
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}
	//! Defaults values
	FX_PARAM FxParamFs = {"Sampling Frequency", "Hz", &glDefaultFs, &glAvailableFs[0], &glAvailableFs[gnAvailableFs - 1], UINT32_TYPE, 1};
	hr = _pFxParam->AddFxParam(&FxParamFs);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(_pFxParam);
		_pFxState->FxPublishState(_FxState = FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! Create UI */
	_pWrapper = new CLADSPAWrapper(_pFx);

	/*! Publish the init state to the observers */
	_pFxState->FxPublishState(_FxState = FX_INIT_STATE);

	return FX_OK;    
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFxParam()
	Update Fx Parameters.
*//*-----------------------------------------------------------------------------*/
Int32 CFxLADSPA::UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter)
{
	/*! Update UI with the new value */
	std::string strLADSPAFile;
	_pFxParam->GetFxParamValue("LADSPA plugin", strLADSPAFile);
	std::string strLADSPALabel;
	_pFxParam->GetFxParamValue("LADSPA plugin label", strLADSPALabel);
	Uint32 dwFs;
	_pFxParam->GetFxParamValue("Sampling Frequency", &dwFs);
	_pWrapper->LoadLADSPA(strLADSPAFile.c_str(), (strLADSPALabel == "NULL") ? NULL : strLADSPALabel.c_str(), dwFs);
	return FX_OK;  
}

/*-----------------------------------------------------------------------------*//*!
	DisplayFxPropertyPage()
	Displays the Fx property page.
*//*-----------------------------------------------------------------------------*/
Int32 CFxLADSPA::DisplayFxPropertyPage(Pvoid pvWndParent)
{
	_pWrapper->ShowUI();
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StartFx()
	Starts the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxLADSPA::StartFx()
{
	_pFxState->FxPublishState(_FxState = FX_START_STATE);
	
	Int32 hr;
	if(FEF_FAILED( hr = _pWrapper->StartWrapper()))
		return hr;

    return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	RunFx()
	Run the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxLADSPA::RunFx()
{
    Int32 hr = FX_OK;
    
    _pFxState->FxPublishState(_FxState = FX_RUN_STATE);
	
    return hr;
}

/*-----------------------------------------------------------------------------*//*!
	StopFx()
	Stops the Fx.
*//*-----------------------------------------------------------------------------*/
Int32 CFxLADSPA::StopFx()
{
	_pFxState->FxPublishState(_FxState = FX_STOP_STATE);
	
	Int32 hr;
	if(FEF_FAILED( hr = _pWrapper->StopWrapper()))
		return hr;

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ReleaseFx()
	Releases the Fx and its interfaces.
*//*-----------------------------------------------------------------------------*/
Int32 CFxLADSPA::ReleaseFx()
{
	/*! Stop and release Fx components */
	StopFx();

	_pFxState->FxPublishState(FX_RELEASE_STATE);	/*!< Update the Fx state to "Release" */

	_pFxState->FxReleaseInterface();

	/*! Delete the Fx instance */
	delete this;
	return FX_OK;
}
