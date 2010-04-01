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
   @file	 IFxBase.h
   @brief This file contains the plugin interface of the FxEngine Framework.

   This file defines the Fx export interface. Every Fx plugin have to inherit
   from it and implement the IFxBase methods.
*//*_______________________________________________________________________*/
#pragma once
//! Fx Interfaces and definitions
#include "IFx.h"

namespace FEF {

//!  The IFXBase class is the plugin interface used by the FxEngine API.
class FXENGINE_EXP IFxBase
{
public:
	/*----------------------------------------------------------------------*//*!
	GetFxInfo().

	This method is called to get the main definitions of the Fx.
    The Fx is in charge of memory allocations (i.e. strName, strVersion,
    strAuthor and strCopyright).
    @param ppFxDescriptor:    Address of a variable that receives a pointer to 
                              the Fx descriptor structure (see FX_DESCRIPTOR structure).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxInfo(/* [out] */const FX_DESCRIPTOR** ppFxDescriptor) PURE;

	/*----------------------------------------------------------------------*//*!
	InitFx().

	The InitFx method is called to initialize the Fx.
	This method has to create an IFx instance, define Fx pins (In or/and Out),
	Fx parameters and all other Fx features.
	WARNING :This method can't be a worker thread !!.
    @param ppFx:    Pointer to a variable that receives the address of the IFx
					interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 InitFx(/* [out] */IFx **ppFx)			PURE;

	/*----------------------------------------------------------------------*//*!
	DisplayFxPropertyPage().

	This method is called to show the Fx property page.

    @param pvWndParent:	Handle to the parent window (Can be null).

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 DisplayFxPropertyPage(/* [in] */Pvoid pvWndParent);

    /*----------------------------------------------------------------------*//*!
	GetFxFrame().

	This method is called to return the current Fx Frame.
    Fx frame is a XPM image format (see http://koala.ilog.fr/lehors/xpm.html)
    and allows to a Framework front-end to render the Fx with a picture.
    Fx can update at any moment its frame and informs Fx observer with the
    FX_FRAME_UPDATE state sending.

    @param ppbFxFrame: Pointer to a variable that receives the XPM data.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxFrame(/* [out] */const Char** ppbFxFrame);

    /*----------------------------------------------------------------------*//*!
	UpdateFxParam().

	The UpdateFxParam is called to update the public parameters when they are
    changed from a FxEngine application.
	See the FEF_UpdateFxParam function.

    @param strParamName:	Variable that contains the parameter name.
    @param FxParameter:     Variable that contains the updating mode.
                            If FxParameter is equal to FX_PARAM_ALL, strParamName
                            is ignored.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
    virtual Int32 UpdateFxParam(/* [in] */const std::string strParamName,
                                /* [in] */FX_PARAMETER FxParameter);

	/*----------------------------------------------------------------------*//*!
	GetFxUserInterface().

	The GetFxUserInterface is called to get an user interface if it exists.
	See the FxEngine API GetFxUserInterface function.
    @param ppvUserInterface:	Address of a variable that receives a pointer
								to the user interface.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxUserInterface(/* [out] */Pvoid* ppvUserInterface);

	/*----------------------------------------------------------------------*//*!
	GetFxSubFxEngine().

	The GetFxSubFxEngine is called to get a FxEngine Handle if it exists.
	Fx can contain a sub FxEngine system with several FXs.
	See the FxEngine API GetFxSubFxEngine function.
    @param phFxEngine:	Pointer to a variable that receives the FxEngine handle.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 GetFxSubFxEngine(	/* [out] */FX_HANDLE* phFxEngine);

	/*----------------------------------------------------------------------*//*!
	StartFx().

	This StartFx function is called to start the Fx.
    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 StartFx()						PURE;

	/*----------------------------------------------------------------------*//*!
	RunFx().

	This RunFx function is called to run the Fx. RunFx is called just after the
	StartFx function.
    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 RunFx();

	/*----------------------------------------------------------------------*//*!
	PauseFx().

	This PauseFx method is called to pause the Fx.
    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 PauseFx();

	/*----------------------------------------------------------------------*//*!
	StopFx().

	This StopFx method is called to stop the Fx.
    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 StopFx()						PURE;

	/*----------------------------------------------------------------------*//*!
	ReleaseFx().

	This ReleaseFx method is called to release the Fx.
	This method has to release all Fx components like interfaces and
	internal values.
    @param None.

	@return	FX_OK if success, otherwise an FX error code.
    *//*-----------------------------------------------------------------------*/
	virtual Int32 ReleaseFx()					PURE;
};
 } //namespace FEF

/*----------------------------------------------------------------------*//*!
	FEF_GetFxBase().

	This function exports the IFxBase interface of the Fx.
    @param None.

	@return	The IFxBase interface of the Fx instance.
    *//*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
namespace FEF {
	FXENGINE_EXP IFxBase* FXENGINE_API FEF_GetFxBase();
 } //namespace FEF
#ifdef __cplusplus
}
#endif

