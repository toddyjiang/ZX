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
   @file  FxPcmGain.h
   @brief The Fx Sound File Source applies a gain to sound data on its input pin. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/
#include "IFxBase.h"
using namespace FEF; //!< Using FxEngineFramework

#pragma warning(disable : 4995)	//!< stdio.h
#pragma warning(disable : 4996) //!< stdlib.h

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
#define FX_VERSION        ("1.2.0.0")

class CFxPcmGain:public IFxBase, 
			    public IFxPinCallback
{
public:
	CFxPcmGain(void);
	~CFxPcmGain();

public:
	//! The IFxBase classe implementation
	virtual Int32 GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor);
	virtual Int32 InitFx(IFx **ppFx);
	virtual Int32 UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter);
	virtual Int32 DisplayFxPropertyPage(Pvoid pvWndParent);
    virtual Int32 StartFx();
    virtual Int32 RunFx();
	virtual Int32 StopFx();
	virtual Int32 ReleaseFx();

	//! The IFxPinCallback interface implementation
	virtual Int32 FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState);
	virtual Void FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState);
	virtual Void FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber);
    
	//! GTK handles
public:
	static Void SliderWrapper(GtkWidget* pWidget, CFxPcmGain* pFx) {pFx->SliderCallback(); return;}
	Void SliderCallback();
private:
	GtkWidget* _pWindow;
	GtkWidget* _pMainVBox;
	GtkWidget* _pFrame;
	GtkWidget* _pSlider;

private:
	FX_STATE		_FxState;		/*!< Contains the Fx state */
	FX_PIN_STATE	_OutPinState;	/*!< Contains the OutPin state */
	FX_PIN_STATE	_InPinState;	/*!< Contains the InPin state */
	FX_MEDIA_TYPE	_PinMediatype;	/*!< Contains the Pin media type */

    FX_DESCRIPTOR   *_pFxDescriptor;  /*!< Contains the Fx description */
	
	IFx *_pFx;					/*!< The IFx interface */
	IFxParam        *_pFxParam; /*!< The IFxParam interface on Fx */

	IFxState*		_pFxState;	/*!< The IFxState interface on Fx */

	FX_PCM_FORMAT _FxPcmFmt; /*!< Current PCM format */
	Bool 		  _IsFormatSet;

private:
	IFxPin* _pFxInputPin;			/*!< The Fx input pin */
	IFxPin* _pFxOutputPin;			/*!< The Fx output pin */
};

