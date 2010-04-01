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
   @file  FxMToSMerger.h
   @brief The Fx Sound File Source merges two mono stream to stereo stream. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/
#include "IFxBase.h"
using namespace FEF; //!< Using FxEngineFramework

#include <map>
using namespace std;

#pragma warning(disable : 4995)	//!< stdio.h
#pragma warning(disable : 4996) //!< stdlib.h

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
#define FX_VERSION        ("1.1.0.0")

#define INPUT_BUFFER_SIZE (176400) /*! We are able to save temporary 1s of data (Mono, 32 bits, 44100Hz) */

//! Pin parameters
typedef struct  _INPUT_PIN {
	_INPUT_PIN(){_pInputPin = NULL; _FxPcmFmt.wChannels = 0; _FxPcmFmt.FormatTag == FLOAT32_TYPE; _FxPcmFmt.dwSamplingRate = 0; _PinState = PIN_NOT_CONNECTED; Reset();}
	IFxPin* _pInputPin;
	FX_PCM_FORMAT   _FxPcmFmt;  	/*!< Current audio format */
    FX_PIN_STATE	_PinState;		/*!< Contains the InPin state */
    Uint8 _pbData[INPUT_BUFFER_SIZE];
	Uint32 _dwWriteIdx;
	Bool _IsFlushed;
	Bool _IsActive;
	Void Reset(){ _dwWriteIdx = 0; _IsFlushed = FALSE; _IsActive = FALSE; memset(_pbData, 0, INPUT_BUFFER_SIZE*sizeof(Uint8));}
 } INPUT_PIN, *PINPUT_PIN;
 
/*! Input Pin parameters */
typedef map<IFxPin*, INPUT_PIN*> mapOfInputPin;
 
class CFxMToSMerger:public IFxBase, 
			    public IFxPinCallback
{
public:
	CFxMToSMerger(void);
	~CFxMToSMerger();

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
	virtual Void FxMedia(IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber);

private:
	FX_STATE		_FxState;		/*!< Contains the Fx state */
	FX_PIN_STATE	_OutPinState;	/*!< Contains the OutPin state */
	FX_MEDIA_TYPE	_PinMediatype;	/*!< Contains the Pin media type */

    FX_DESCRIPTOR   *_pFxDescriptor;  /*!< Contains the Fx description */
	
	IFx *_pFx;					   /*!< The IFx interface */
	FX_PCM_FORMAT   _FxOutPcmFmt;  /*!< Current audio format */

	IFxState        *_pFxState;	/*!< The IFxState interface on Fx */
	IFxParam        *_pFxParam; /*!< The IFxParam interface on Fx */
	
	//! GTK handles
public:
	static Void SliderWrapper(GtkWidget* pWidget, CFxMToSMerger* pFx) {pFx->SliderCallback(); return;}
	Void SliderCallback();
private:
	GtkWidget* _pWindow;
	GtkWidget* _pMainVBox;
	GtkWidget* _pFrame;
	GtkWidget* _pSlider;

private:
	INPUT_PIN _FxInputPinL;			/*!< The Fx input Left pin */
	INPUT_PIN _FxInputPinR;			/*!< The Fx input Right pin */
	mapOfInputPin _FxInputPin;		/*!< The Fx input pins map */
	IFxPin* _pFxOutputPin;			/*!< The Fx output pin */
	Bool _IsSynchronous;			/*!< Synchronous on start */
	std::string _strMediaName;		/*!< Media name */
	Void GetInputAvailableSize(Uint32* pdwAvailableL, Uint32* pdwAvailableR); /*! Get available sizes */
	
private:
	pthread_t _Thread;
	Bool _ShouldCloseThread;
	pthread_mutex_t _ThreadMutex;
	pthread_cond_t  _ThreadCond;
	
public:
	static void *ThreadFctWrapper(Void *p_data) { return ((CFxMToSMerger*)p_data)->ThreadFct(); }
	Void* ThreadFct();
};

