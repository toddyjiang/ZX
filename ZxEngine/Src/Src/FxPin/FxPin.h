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

   ____________________________________________http://www.SMProcess.com______ */
#pragma once
#include "FxDef.h"
#include "IFxPin.h"
#include "IFxPinManager.h"
#include "IFxPinCallback.h"
#include "FxMediaPool/FxMediaPool.h"

#include "FxUtils/FxUtils.h"
#include "FxUtils/NotifyObjects.h"

#include "FxTrace/FxTrace.h"
#include "FxConst2String/FxConst2String.h"

#include "FxUtils/FxAPC.h"

#include <list>

#include <boost/timer.hpp>

using namespace std ;

namespace FEF {

//! Dump states
typedef enum _DUMP_STATE{
	DUMP_OFF = 0,
	DUMP_INIT,
	DUMP_STOP,
	DUMP_START
}DUMP_STATE;

//! Pin states
typedef enum _PIN_RUN_STATE{
	PIN_STOP = 0,
	PIN_PAUSE,
	PIN_START,
}PIN_RUN_STATE;


class CFxPin;
class CFxAPC;

class CFxPinObserver {
public:
	virtual ~ CFxPinObserver(){};
	virtual Void FxPinUpdate(CFxPin* pPin, Int32 sdwHr, FX_STATE FxState, FX_PTR qUserParam ) PURE;
protected:
	CFxPinObserver(void){};
};

class CPinState : public CFxAPCCallBack {
public:
	CPinState(void);
	virtual ~CPinState();
public:
	CFxAPC* _pAPC;
	virtual Int32 APCCallBack(void* pObject);

	//! Critical Section
	boost::mutex _PinStateCS;
};

typedef struct _FX_OBSERVER {
	CFxPinObserver*	pObserver;
	FX_PTR			qUserParam;
} FX_OBSERVER, *PFX_OBSERVER;

typedef std::list<FX_OBSERVER*>  CFxPinObserverL;
typedef CFxPinObserverL::iterator   CFxPinObserverIter;

typedef std::list<CFxPin*>  CFxPinL;
typedef CFxPinL::iterator   CFxPinIter;

typedef std::pair< pair<boost::mutex*, boost::mutex*>, boost::mutex*> CSPair;

class CFxPin : public IFxPin,
			   public CFxConst2String,
			   public CFxAPCCallBack	{

public:
	CFxPin(PFX_PIN pPinInfo, std::string& strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance, Uint32 dwFxPinInstance, Int32& hr);

public:
	virtual ~CFxPin();

public:
	PFX_PIN _pPinInfo;

private:
	Bool _IsPinConnected;
	Bool _IsConnectionMediaTypeSet;

public:
	virtual Void SendFxPinState(FX_PIN_STATE PinState);
    virtual Void SendFxPinStateEx(FX_PIN_STATE PinState);

private:
	IFxPin*	 _pConnectedPin;	//!< The attached Pin to the Pin
	FX_MEDIA_TYPE _ConnectionMediaType;

private:
	CFxMediaPool* _pFxMediaPool;
	
private:
	std::string _strFxName;

private:
	std::string _FxLogFile;
	CFxLog* _pLogTrace;
	CFxLog* _pLogDump;

	Uint32 _dwFxEngineInstance;			// current FxEngine instance
	Uint32 _dwFxInstance;				// current Fx instance

	// Instance value to manage the right log messages.
	Uint32 _dwThisInstance;				// current instance

private:
    std::string _strDumpFilePath;
	DUMP_STATE	_DumpState;

//! Interface IFxPin
public:

    virtual Int32 GetPinName(std::string& strPinName);						//!< Get the Pin Name.
	virtual Int32 GetPinType(FX_PIN_TYPE* pPinType);				//!< Get Pin type.

private:
	//! Critical Section
	boost::mutex _PinCS;

	CSPair*			_pPinCallbackCS;
public:
	virtual Void	SetPinCallBackCS(CSPair* pPinCS);

private:
	Uint32 _dwFxProcessTime;
	boost::timer _tElapse;

public:

	virtual Int32 ConnectPin(IFxPin*	 pFxPin);					//!< Connect Pin.
	virtual Int32 DisconnectPin();									//!< DisConnect Pin.
	virtual Int32 GetPinState(FX_PIN_STATE* pPinState);				//!< Current Pin Sate.
	virtual Int32 GetPinConnected(IFxPin** ppFxPin);

	virtual Int32 CheckMediaType(PFX_MEDIA_TYPE pMediaType);	//!< Check Pin media.
	virtual Int32 SetConnectionMediaType(PFX_MEDIA_TYPE pMediaType);	//!< Set Pin media of the connection.
	virtual Int32 GetMediaTypeCount(Uint16* pwMediaTypeCount);			//!< Return the number of mediatype for this Pin.
	virtual Int32 GetMediaType(PFX_MEDIA_TYPE pMediaType, Uint16 wMediaTypeIndex); //!< Get media types
	virtual Int32 GetConnectionMediaType(PFX_MEDIA_TYPE pMediaType);	//!< Get Pin media of the connecxion.

	virtual Int32 GetFreeMediaNumber(Uint32* pdwFreeMediaNumber);
	virtual Int32 DeliverMedia(IFxMedia* pIFxMedia);					//!< Add buffer to the next input pin queue.
	virtual Int32 GetDeliveryMedia(IFxMedia** ppIFxMedia, Uint32 dwTimeOut = 0);

	virtual Int32 GetProcessTime(Uint32* pdwProcessingTime);

	virtual Int32 Flush();
    virtual Int32 InitStream(IFxMedia* pIFxMedia);

    virtual Int32 InitDumpData(const std::string strFilePath);
	virtual Int32 StartDumpData();
	virtual Int32 StopDumptData();

	virtual Int32 GetPinRunState(PIN_RUN_STATE* pPinRunState);
	virtual Int32 SetPinRunState(PIN_RUN_STATE PinRunState, Bool ShouldProtect = TRUE);

    virtual Int32 UpdateMediaPool(Uint32 dwSize, Uint32 dwNumber);

	virtual Void LinkFxPin(CFxPin* pFxPin, FX_PIN_TYPE PinType);
    virtual Void UnLinkFxPin(CFxPin* pFxPin);

	virtual Void SetFxMember(FX_HANDLE hFx) { _hFxMember = hFx; }
	virtual FX_HANDLE GetFxMember() const   { return (_hFxMember); }

	virtual Int32 WaitForIFxMedia(Uint32 dwTimeStamp, FX_PTR dwUser);

	virtual Void QuitInfiniteTime();
	virtual Void WaitForEndDataInQueue();

	virtual Int32 GetTxRxBytes(/* [out] */Uint64* pqTxRxByte) {*pqTxRxByte = _pqTxRxByte; return FX_OK;}

public:
	virtual Int32 AttachObservers(CFxPinObserver* pFxPinObservers, FX_PTR qUserParam);
	virtual Int32 DetachObservers(CFxPinObserver* pFxPinObservers);
	virtual Void NotifyObservers(Int32 sdwHr, FX_STATE FxState);

private:
	CFxPinObserverL _FxPinObservers;

private:
	FX_HANDLE _hFxMember;

public:
	CFxPinL		_lpFxPinIn;
	CFxPinL		_lpFxPinOut;

public:
	Bool		_ShoudStopCallBack;

private:
	PIN_RUN_STATE	_PinRunState;

private:
	Uint64  _pqTxRxByte;

//! Pin callback
public:
	CFxAPC* _pAPC;
	virtual Int32 APCCallBack(void* pObject);

//! Pin State callback
	friend class CPinState;
	CPinState _PinState;
};

} //namespace FEF
