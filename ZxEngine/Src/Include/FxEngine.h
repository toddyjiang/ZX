/* __________________________________________________________________________
	
	FxEngine Framework. 
	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.

	This file is part of FxEngine Framework.
    The FxEngine Framework library is free software; you can redistribute it
	and/or modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library.
	if not,  If not, see <http://www.gnu.org/licenses/>.

   ____________________________________________http://www.SMProcess.com______
*//*!
   @file	 FxEngine.h
   @brief This file contains the definitions of the FxEngine Framework API.

	The FxEngine API allows to use the Fx plugins (see IFxBase interface).
*//*_______________________________________________________________________*/
#pragma once 

//! FxEngine Framework definitions
#include "FxDef.h"
#include "IFxBase.h"

#ifdef __cplusplus    //!< If used by C++ code, 
extern "C" {          //!< we need to export the C interface
#endif
namespace FEF {
	/*----------------------------------------------------------------------*//*!
	FEF_GetFxEngineVersion().

	This function gets the the FxEngine API version.
    @param pwMajor:		Pointer to a variable that receives the Major
	of FxEngine API version.
	@param pwMinor:		Pointer to a variable that receives the Minor
	of FxEngine API version.
	@param pwBuild:		Pointer to a variable that receives the Build
	of FxEngine API version.
	@param pwRev:		Pointer to a variable that receives the Revision
	of FxEngine API version.
       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxEngineVersion(	/* [out] */Uint16* pwMajor,
															/* [out] */Uint16* pwMinor,
															/* [out] */Uint16* pwBuild,
															/* [out] */Uint16* pwRev);

	/*----------------------------------------------------------------------*//*!
	FEF_CreateFxEngine().

	The FEF_CreateFxEngine create a FxEngine Instance.
	The FxEngine Instance is identified by its FX_HANDLE value.
    @param phFxEngine:			Pointer to a variable that receives the FxEngine handle.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_CreateFxEngine(	/* [out] */FX_HANDLE* phFxEngine);

	/*----------------------------------------------------------------------*//*!
	FEF_ReleaseFxEngine().

	The FEF_ReleaseFxEngine function releases a FxEngine Instance which was
	created with the FEF_CreateFxEngine function.
	@param hFxEngine:			Handle of the FxEngine instance	to release.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_ReleaseFxEngine(/* [in] */FX_HANDLE hFxEngine);

	/*----------------------------------------------------------------------*//*!
	FEF_AddFx().

	The FEF_AddFx function allows to add a Fx plugin in the FxEngine from a DLL path.
	InitFx function	of each	Fx plugin is called.
	@param hFxEngine:			Handle of the FxEngine instance.
	@param strFx:				Variable that contains the Fx plugin path.
	@param phFx:				Pointer to a variable that receives the
								Fx plugin handle.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_AddFx(	/* [in] */FX_HANDLE hFxEngine,
												/* [in] */const std::string strFx,
												/* [out] */FX_HANDLE* phFx);

	/*----------------------------------------------------------------------*//*!
	FEF_AddFxEx().

	The FEF_AddFxEx function allows to add a Fx in the FxEngine using an
	IFxBase interface. The IFxBase::InitFx function of each Fx is called.
	@param hFxEngine:			Handle of the FxEngine instance.
	@param pIFxBase:			Pointer to a variable that contains the IFxBase interface
								of Fx to add.
	@param phFx:				Pointer to a variable that receives the
								Fx plugin handle.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_AddFxEx(/* [in] */FX_HANDLE hFxEngine,
												/* [in] */IFxBase* pIFxBase,
												/* [out] */FX_HANDLE* phFx);

	/*----------------------------------------------------------------------*//*!
	FEF_RemoveFx().

	The FEF_RemoveFx function allows to remove a Fx plugin in the FxEngine.
	@param hFxEngine:			Handle of the FxEngine instance.
	@param hFx:					Handle of the Fx plugin to remove.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_RemoveFx(	/* [in] */FX_HANDLE hFxEngine,
													/* [in] */FX_HANDLE hFx);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxCount().

	The FEF_GetFxCount returns the number of FXs (N) in the FxEngine.
	@param hFxEngine:			Handle of the FxEngine instance.
	@param pwFxCount:			Pointer to a variable that receives the number
								of Fx plugin.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxCount(	/* [in] */FX_HANDLE hFxEngine,
													/* [out] */Uint16* pwFxCount);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFx().

	The FEF_GetFx returns the Fx plugin handle in the FxEngine by its index.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param phFx:			Pointer to a variable that receives
							the Fx plugin handle.
	@param wFxIndex:		Index of the Fx plugin to get. From 0 to N-1.
							N is given by the GetFxCount function.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFx(	/* [in] */FX_HANDLE hFxEngine,
												/* [out] */FX_HANDLE* phFx,
												/* [in] */Uint16 wFxIndex);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxUserInterface().

	The FEF_GetFxUserInterface gets a user interface if it exists.
	See the IFxBase::GetFxUserInterface method.
	@param hFxEngine:			Handle of the FxEngine instance.
	@param hFx:					Handle of the Fx plugin.
	@param ppvUserInterface:	Address of a variable that receives a pointer
								to the user interface.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxUserInterface(	/* [in] */FX_HANDLE hFxEngine,
															/* [in] */FX_HANDLE hFx,
															/* [out] */Pvoid* ppvUserInterface);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxSubFxEngine().

	The FEF_GetFxSubFxEngine is called to get a FxEngine Handle if it exists.
	Fx can contain a sub FxEngine system with several FXs.
	See the IFxBase::GetFxSubFxEngine method.
	@param hFxEngine:			Handle of the FxEngine instance.
	@param hFx:					Handle of the Fx plugin.
	@param phFxEngine:			Pointer to a variable that receives the FxEngine handle.
	       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxSubFxEngine(	/* [in] */FX_HANDLE hFxEngine,
															/* [in] */FX_HANDLE hFx,
															/* [out] */FX_HANDLE* phFxEngine);

	/*----------------------------------------------------------------------*//*!
	FEF_StartFxEngine().

	The FEF_StartFxEngine function starts the FXs in the FxEngine.
	The IFxBase::StartFx method of each Fx is called. After the IFxBase::StartFx
	functions calling, the IFxBase::RunFx method of each Fx is called.
	@param hFxEngine:		Handle of the FxEngine instance.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_StartFxEngine(/* [in] */FX_HANDLE hFxEngine);

	/*----------------------------------------------------------------------*//*!
	FEF_StartFx().

	The StartFx function starts a Fx individually. In order,
	the IFxBase::StartFx and IFxBase::RunFx functions of Fx are called.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin to start.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_StartFx(/* [in] */FX_HANDLE hFxEngine,
												/* [in] */FX_HANDLE hFx);

	/*----------------------------------------------------------------------*//*!
	FEF_PauseFxEngine().

	The FEF_PauseFxEngine function pauses the FXs in the FxEngine.
	The IFxBase::PauseFx method of each Fx is called ( if it exists).
	@param hFxEngine:		Handle of the FxEngine instance.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_PauseFxEngine(/* [in] */FX_HANDLE hFxEngine);

	/*----------------------------------------------------------------------*//*!
	FEF_PauseFx().

	The FEF_PauseFx function pauses a Fx.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin to stop.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_PauseFx(/* [in] */FX_HANDLE hFxEngine,
												/* [in] */FX_HANDLE hFx);

	/*----------------------------------------------------------------------*//*!
	FEF_StopFxEngine().

	The FEF_StopFxEngine function stops the FXs in the FxEngine.
	The IFxBase::StopFx method of each Fx is called.
	@param hFxEngine:		Handle of the FxEngine instance.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_StopFxEngine(/* [in] */FX_HANDLE hFxEngine);

	/*----------------------------------------------------------------------*//*!
	FEF_StopFx().

	The FEF_StopFx function stops a Fx individually.
	The IFxBase::StopFx of Fx is called.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin to stop.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_StopFx(	/* [in] */FX_HANDLE hFxEngine,
												/* [in] */FX_HANDLE hFx);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxInfo().

	The FEF_GetFxInfo function gets the mains definitions of a Fx plugin.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param ppFxDescriptor:	Address of a variable that receives a pointer to 
                            the Fx descriptor structure (see FX_DESCRIPTOR structure).
	
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxInfo(	/* [in] */FX_HANDLE hFxEngine,
													/* [in] */FX_HANDLE hFx,
													/* [out] */const FX_DESCRIPTOR** ppFxDescriptor);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxState().

	The FEF_GetFxInfo function gets the Fx plugin state (see FX_STATE).
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param pFxState:		Pointer to a variable that receives the Fx plugin
							   state (see FX_STATE).
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxState(	/* [in] */FX_HANDLE hFxEngine,
													/* [in] */FX_HANDLE hFx,
													/* [out] */FX_STATE* pFxState);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxStateName().

	The FEF_GetConstToString function converts a FxEngine constant to a string.
	@param FxEngineConstType:	Variable that contains the Type of the constant.
	@param sdwFxEngineConst:	Variable that contains the constant to convert.
	@param strStateName:		Reference to a variable that receives the FxEngine
                                constant name.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetConstToString(	/* [in] */FXENGINE_CONST_TYPE FxEngineConstType,
															/* [in] */Int32 sdwFxEngineConst,
															/* [out] */std::string& strStateName);

	/*----------------------------------------------------------------------*//*!
	FEF_AttachFxObserver().

	The FEF_AttachFxObserver function attaches a FxState observer to a Fx.
	
    @param hFxEngine:			Handle of the FxEngine instance.
	@param hFx:					Handle of the Fx plugin.
	@param pFxStateCallback:	Pointer to a CFxStateCallback object containing
								the callback method to be called during Fx
								running to process messages related to the
								progress of the Fx states.
	@param dwParam:				First User-supplied callback data.
	@param hObserverId:			Pointer to a Handle that receives the ObserverId.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_AttachFxObserver(	/* [in] */FX_HANDLE hFxEngine,
															/* [in] */FX_HANDLE hFx,
															/* [in] */CFxStateCallback* pFxStateCallback,
															/* [in] */FX_PTR dwParam,
															/* [out] */FX_HANDLE* phObserverId);

	/*----------------------------------------------------------------------*//*!
	FEF_AttachFxObserverEx().

	The FEF_AttachFxObserver method attaches a Fx state observer to a Fx.

    @param hFxEngine:			Handle of the FxEngine instance.
	@param hFx:					Handle of the Fx plugin.
	@param pFxStateCallback:	Pointer to a fixed callback function to be
								called during Fx running to process messages
								related to the progress of the Fx states.
	@param dwParam:				First User-supplied callback data.
	@param hObserverId:			Pointer to a Handle that receives the ObserverId. 

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_AttachFxObserverEx(	/* [in] */FX_HANDLE hFxEngine,
															/* [in] */FX_HANDLE hFx,
															/* [in] */FXSTATECALLBACK* pFxStateCallback,
															/* [in] */FX_PTR dwParam,
															/* [out] */FX_HANDLE* phObserverId);

	/*----------------------------------------------------------------------*//*!
	FEF_DetachFxObserver().

	The FEF_DetachFxObserver method detaches a Fx state observer.

    @param hFxEngine:		Handle of the FxEngine instance.
	@param hObserverId:	Handle that contains the ObserverId to detach. 
	
	
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_DetachFxObserver(	/* [in] */FX_HANDLE hFxEngine,
															/* [in] */FX_HANDLE hObserverId);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxPinCount().

	The FEF_GetFxPinCount function gets the number (N) of pin of a Fx.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param pwPinCount:		Pointer to a variable that receives the number
							of Fx plugin pin.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxPinCount(	/* [in] */FX_HANDLE hFxEngine,
														/* [in] */FX_HANDLE hFx,
														/* [out] */Uint16* pwPinCount);

	/*----------------------------------------------------------------------*//*!
	FEF_QueryFxPinInterface().

	The FEF_QueryFxPinInterface function gets the IFXPin interface of a Fx pin.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param ppIFxPin:		Address of a variable that receives a pointer
							to an IFxPin interface.
	@param wFxIndex:		Index of the Fx pin to get. From 0 to N-1.
							N is given by the GetFxPinCount function.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_QueryFxPinInterface(/* [in] */FX_HANDLE hFxEngine,
															/* [in] */FX_HANDLE hFx,
															/* [out] */IFxPin** ppIFxPin,
															/* [in] */Uint16 wPinIndex);

	/*----------------------------------------------------------------------*//*!
	FEF_QueryFxParamInterface().

	The FEF_QueryFxParamInterface function gets the IFxParam interface of a Fx plugin.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param ppIFxParam:		Address of a variable that receives a pointer
							to an IFxParam interface.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_QueryFxParamInterface(	/* [in] */FX_HANDLE hFxEngine,
																/* [in] */FX_HANDLE hFx,
																/* [out] */IFxParam** ppIFxParam);

    /*----------------------------------------------------------------------*//*!
	FEF_UpdateFxParam().

	The FEF_UpdateFxParam function calls the IFxBase::UpdateFxParam method.
    It allows to a Fx to reload the public parameters.

	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param strParamName:	Variable that contains the parameter name.
    @param FxParameter:     Variable that contains the updating mode.
                            If FxParameter is equal to FX_PARAM_ALL, strParamName
                            is ignored.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_UpdateFxParam(	/* [in] */FX_HANDLE hFxEngine,
														/* [in] */FX_HANDLE hFx,
														/* [in] */const std::string strParamName,
														/* [in] */FX_PARAMETER FxParameter);

	/*----------------------------------------------------------------------*//*!
	FEF_ConnectFxPin().

	The FEF_ConnectFxPin function connects two pins.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param pFxPinIn:		Pointer to the input pin. 
	@param pFxPinOut:		Pointer to the output pin.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_ConnectFxPin(/* [in] */FX_HANDLE hFxEngine,
													 /* [in] */IFxPin* pFxPinIn,
													 /* [in] */IFxPin* pFxPinOut);

	/*----------------------------------------------------------------------*//*!
	FEF_ConnectFxPinEx().

	The FEF_ConnectFxPin function connects two pins with a specified media type.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param pFxPinIn:		Pointer to the input pin. 
	@param pFxPinOut:		Pointer to the output pin.
	@param pMediaType:		Pointer to a variable that contains the media type.							.
		       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_ConnectFxPinEx(	/* [in] */FX_HANDLE hFxEngine,
														/* [in] */IFxPin* pFxPinIn,
														/* [in] */IFxPin* pFxPinOut,
														/* [in] */PFX_MEDIA_TYPE pMediaType);
	

	/*----------------------------------------------------------------------*//*!
	FEF_DisconnectFxPin().

	The FEF_DisconnectFxPin function disconnects pin.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param pFxPin:			Pointer to the pin to disconnect. 
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_DisconnectFxPin(/* [in] */FX_HANDLE hFxEngine,
														/* [in] */IFxPin* pFxPin);

	/*----------------------------------------------------------------------*//*!
	FEF_SetFxEngineRefClock().

	The FEF_SetFxEngineRefClock function sets a reference clock to the Fx engine.
	Fx plugins uses IFxRefClock::GetFxEngineRefClock method to retrieve it.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param qRefClock:		Reference clock to set. 
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_SetFxEngineRefClock(/* [in] */FX_HANDLE hFxEngine,
															/* [in] */Uint64 qRefClock);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxEngineRefClock().

	The FEF_GetFxEngineRefClock function gets a reference clock of the Fx engine.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param pqRefClock:		Pointer to a variable that receives the reference clock.
										       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxEngineRefClock(/* [in] */FX_HANDLE hFxEngine,
															/* [out] */Uint64* pqRefClock);

	/*----------------------------------------------------------------------*//*!
	FEF_GetFxRefClock().

	The FEF_GetFxRefClock method gets the latest reference clock sent by a Fx Plugin.
	@param hFxEngine:		Handle of the FxEngine instance.
    @param qRefClock:		Variable that receives the reference clock.
							Can be null if no reference clock exists.
	@param dwId:			Variable that contains the Fx reference clock ID. 
	
	
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxRefClock(	/* [in] */FX_HANDLE hFxEngine, 
														/* [out] */Uint64* pqRefClock,
														/* [in] */Uint32 dwId);
									
	/*----------------------------------------------------------------------*//*!
	FEF_DisplayFxPropertyPage().

	The FEF_DisplayFxPropertyPage function allows to display the property page of a
	Fx plugin if it exists (see IFxBase::).
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param pvWndParent:		Handle to the parent window (Can be null).
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_DisplayFxPropertyPage(	/* [in] */FX_HANDLE hFxEngine,
																/* [in] */FX_HANDLE hFx,
																/* [in] */Pvoid pvWndParent);

    /*----------------------------------------------------------------------*//*!
	FEF_GetFxFrame().

	The FEF_GetFxFrame function returns the Fx frame if it exists (see IFxbase::. Fx frame
    is a XPM image format (see http://koala.ilog.fr/lehors/xpm.html)
    and allows to a Framework front-end to render the Fx with a picture.
    Fx can update at any moment its frame and informs Fx observer with the 
    FX_FRAME_UPDATE state sending.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param hFx:				Handle of the Fx plugin.
	@param ppbFxFrame:		Address of a variable that receives the XPM data.
			       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_GetFxFrame(	/* [in] */FX_HANDLE hFxEngine,
													/* [in] */FX_HANDLE hFx,
													/* [out] */const Char** ppbFxFrame);

    /*----------------------------------------------------------------------*//*!
	FEF_SaveFxEngine().

	The FEF_SaveFxEngine function allows to save the FxEngine configuration.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param strFilePath:		Variable that contains configuration file path
                            to save.
				       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_SaveFxEngine(/* [in] */FX_HANDLE hFxEngine,
													 /* [in] */const std::string strFilePath );

	/*----------------------------------------------------------------------*//*!
	FEF_LoadFxEngine().

	The FEF_LoadFxEngine function allows to load a FxEngine configuration.
	@param hFxEngine:		Handle of the FxEngine instance.
	@param strFilePath:		Variable that contains the configuration file
                            path to load.
				       
	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	FXENGINE_EXP Int32 FXENGINE_API FEF_LoadFxEngine(/* [in] */FX_HANDLE hFxEngine,
												     /* [in] */const std::string strFilePath );
 } //namespace FEF
#ifdef __cplusplus
}
#endif


