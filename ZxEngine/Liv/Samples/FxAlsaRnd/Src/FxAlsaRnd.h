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
   @file  FxAlsaRnd.h
   @brief Sound Stream Renderer with alsa. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/
#include "IFxBase.h"
using namespace FEF;

#include "FxGui.h"

#include <alsa/asoundlib.h>

#include <vector>
using namespace std;

#pragma warning(disable : 4995)	//!< stdio.h
#pragma warning(disable : 4996) //!< stdlib.h

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
#define FX_VERSION        ("1.1.0.0")

class CAutoLock
{
public:
	CAutoLock(pthread_mutex_t* pThreadMutex){_pThreadMutex = pThreadMutex; pthread_mutex_lock(_pThreadMutex); _IsLocked = TRUE;}
	~CAutoLock(void){if(_IsLocked) pthread_mutex_unlock(_pThreadMutex);}
	Void Unlock() {if(_IsLocked){ pthread_mutex_unlock(_pThreadMutex); _IsLocked = FALSE;} return;}
	Void Lock() {if(!_IsLocked){ pthread_mutex_lock(_pThreadMutex); _IsLocked = TRUE;} return;}
private:
	pthread_mutex_t* _pThreadMutex;
	Bool _IsLocked;
};

class CFxAlsaRnd:public IFxBase, 
			    public IFxPinCallback
{
public:
	CFxAlsaRnd(void);
	~CFxAlsaRnd();

public:
	//! The IFxBase classe implementation
	virtual Int32 GetFxInfo(const FX_DESCRIPTOR** ppFxDescriptor);
	virtual Int32 InitFx(IFx **ppFx);
	virtual Int32 DisplayFxPropertyPage(Pvoid pvWndParent);
	virtual Int32 UpdateFxParam(const std::string strParameter, FX_PARAMETER FxParameter);
    virtual Int32 StartFx();
    virtual Int32 RunFx();
	virtual Int32 StopFx();
	virtual Int32 ReleaseFx();

	//! The IFxPinCallback interface implementation
	virtual Int32 FxPin(IFxPin* pFxPin, IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState);
	virtual Void FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState);
	
private:
	Void EnumDevice(void);
	    
private:
	FX_STATE		_FxState;		/*!< Contains the Fx state */
	FX_MEDIA_TYPE	_PinMediatype;	/*!< Contains the Pin media type */

    FX_DESCRIPTOR   *_pFxDescriptor;  /*!< Contains the Fx description */
	
	IFx 			*_pFx;		/*!< The IFx interface */
	IFxParam        *_pFxParam; /*!< The IFxParam interface on Fx */

	IFxState*		_pFxState;	/*!< The IFxState interface on Fx */

private:
	IFxPin* _pFxInputPin;		/*!< The Fx input pin */

private:
	std::string _strFileName; /*!< Stream name */

private:
	CFxGui* _pFxGui;  /*!< The Fx GUI */
	
	//! Alsa part
private:
	FX_PCM_FORMAT _FxPcmFmt;
	std::vector<DevicePair> _vectorDevice;
	snd_pcm_t *_pAlsaHandle;
    snd_pcm_hw_params_t *_pHwParams;
    snd_pcm_sw_params_t *_pSwParams;
    Int32 OpenDevice();
    Int32 SetHw();
    Int32 SetSw();
    Int32 Recovery(Int32 hr);
    Bool _IsPcmFloatAccepted;
	
private:
	pthread_t _Thread;
	Bool _ShouldCloseThread;
	pthread_mutex_t _ThreadMutex;
	Uint32 _dwAskedSample;
	Uint64 _qWrittenSample;
	Double _dPlayedTime;
	
public:
	static void *ThreadFctWrapper(Void *p_data) { return ((CFxAlsaRnd*)p_data)->ThreadFct(); }
	Void* ThreadFct();
};

