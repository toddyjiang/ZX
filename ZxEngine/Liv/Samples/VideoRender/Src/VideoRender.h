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


using namespace std;
using namespace FEF;

#pragma warning(disable : 4995)	//!< stdio.h
#pragma warning(disable : 4996) //!< stdlib.h

#ifndef BOOL
#define BOOL uint8_t
#endif

/*-----------------------------------------------------------------------------*//*!
	FX constants
*//*-----------------------------------------------------------------------------*/
#define FX_VERSION        ("1.1.0.0")

typedef struct ZX_Media_Buffer_
{
	uint8_t*	pData;
	uint8_t		nData;
}ZX_Media_Buffer_t;

class CAutoLock
{
public:
	CAutoLock(pthread_mutex_t* pThreadMutex){_pThreadMutex = pThreadMutex; pthread_mutex_lock(_pThreadMutex); _IsLocked = TRUE;}
	~CAutoLock(void){if(_IsLocked) pthread_mutex_unlock(_pThreadMutex);}
	void Unlock() {if(_IsLocked){ pthread_mutex_unlock(_pThreadMutex); _IsLocked = FALSE;} return;}
	void Lock() {if(!_IsLocked){ pthread_mutex_lock(_pThreadMutex); _IsLocked = TRUE;} return;}
private:
	pthread_mutex_t* _pThreadMutex;
	BOOL _IsLocked;
};

class CVideoRender:public IFxBase, 
			    public IFxPinCallback
{
public:
	CVideoRender(void);
	~CVideoRender();

public:
	//! The IFxBase class implementation
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
	virtual void FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState);

private:
	void MakeSDLSpec();
	    
private:
	FX_STATE		_FxState;			/*!< Contains the Fx state */
	FX_MEDIA_TYPE	_PinMediatype;		/*!< Contains the Pin media type */

    FX_DESCRIPTOR   *_pFxDescriptor;	/*!< Contains the Fx description */
	
	IFx 			*_pFx;				/*!< The IFx interface */
	IFxParam        *_pFxParam;			/*!< The IFxParam interface on Fx */

	IFxState*		_pFxState;			/*!< The IFxState interface on Fx */

	IFxPin*			_pFxInputPin;		/*!< The Fx input pin */

	std::string		_strFileName;		/*!< Stream name */
	
	FX_PCM_FORMAT	_FxPcmFmt;
	SDL_VideoInfo	_Spec;

	pthread_t		_Thread;
	BOOL			_ShouldCloseThread;
	pthread_mutex_t _ThreadMutex;
	Uint32			_dwAskedSample;
	Uint64			_qWrittenSample;
	Double			_dPlayedTime;

	BOOL			_IsPcmFloatAccepted;

	SDL_Surface *	_Screen;
	static boost::circular_buffer<ZX_Media_Buffer_t>	_MediaBuffers;
	
public:
	static void *ThreadFctWrapper(void *p_data) { return ((CVideoRender*)p_data)->ThreadFct(); }
	void* ThreadFct();
	static void SDLCALL fillerup(void *unused, Uint8 * stream, int len);
};

