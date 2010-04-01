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
#include "FxPin.h"
#include "FxMediaPool/FxMediaPool.h"
#include "boost/lexical_cast.hpp"

namespace FEF {

#define FXPIN_VERSION        "4.0.0.0"	//<! W: Fx/CFx interface, X: Fx/CFx code, Y: compnent interface, Z: component code
#define MAX(a, b) ((a) > (b) ? (a) : (b))
/* History */
/*	2.0.0.0
	Add IFxPin::GetMediaTypeCount
	Add IFxPin::GetMediaType
*/
/*	2.1.0.0
	Add test if pinobserver already exist
*/
/*	3.0.0.0 (21/08/06)
	Use Boost thread library
	Add asynchronous trace library
	Add timeout to GetDeliveryMedia method
	Use Boost timer library
*/
/*	3.0.1.0 (02/12/06)
	Rename IFxPin::GetConnexionMediaType to IFxPin::GetConnectionMediaType
*/
/*	3.0.2.0 (17/12/06)
	Add AutoLock lock(_PinCS) to flush and WaitForIFxMedia methods
*/
/*  3.0.2.1 (05/05/07)
	Add AutoLock lock(_PinCS) in DeliverMedia(IFxMedia* pIFxMedia)
*/
/*  3.1.0.0 (18/07/07)
    Update DetachObservers, dettach all when param is NULL
    Add AutoLock lock(_PinCS) in APC Call back
*/
/*  3.2.0.0 (28/07/07)
    Create media pool for each pins
	Update connection: resize and change number buffer according to the connexion
*/
/*  3.2.0.1 (16/08/07)
    Unlock mutex before call pin callbacks (FxPin and FxWait) fix asynchronous pin callback
*/
/*  3.3.0.0 (03/10/07)
    Replace Char by std::string.
    Remove methods re-entrance.
*/
/*  3.4.0.0 (29/12/07)
    Remove all pIFxPinCallBack non NULL tests.
	Rename the all pIFxPinCallBack callback to lockCallBack (avoid re-entrance)
*/
/*  3.5.0.0 (05/03/08)
    Add waiting for end of APC state and APC  pin Data (using StartAPC and StopAPC).
	Release all media data in disconnect method
    Added IFxPin::UnLinkFxPin method (release pin links when pin is removed)
*/
/*  3.6.0.0 (29/04/08)
    Release Mediapool by call 
*/
/*  3.7.0.0 (15/06/08)
	Create QuitInfiniteTime() to release GetDelivery with INFINITE waiting
	Repeat FxMedia when pin state is equal to pause.
*/
/*  3.8.0.0 (18/07/08)
	Unlock mutex before to call SendFxPinStateEx(PIN_NOT_CONNECTED) in DisconnectPin method.
*/
/*  3.9.0.0 (18/07/08)
	Can transmit flush even if pin is stopped.
*/
/*  4.0.0.0 (23/01/09)
	Added InitStream to initialize stream.
	Added GetTxRxBytes to get TxRx bytes of pin
	Updated IFxPinCallback::FxPin method. Added FX_STREAM_STATE.
	Updated GetMediaType method. Remove ** and put * instead.
*/

Int32 IFxPinCallback::FxPin(IFxPin* pFxPin,	IFxMedia* pIFxMedia, FX_STREAM_STATE StreamState)
{
	return FX_OK;
}

Void IFxPinCallback::FxPinState( IFxPin* pFxPin, FX_PIN_STATE PinState)
{return;}

Void IFxPinCallback::FxWaitForIFxMedia(IFxPin* pFxPin, Uint32 dwTimeStamp, FX_PTR dwUser)
{
	FX_PIN_TYPE fxPinType;
	FX_MEDIA_TYPE MediaTypeOut, MediaTypeIn;
	FX_PIN_STATE PinState;
	PIN_RUN_STATE PinRunState;
	if(pFxPin)
	{
		/*! Get Out Pin infos */
		pFxPin->GetConnectionMediaType(&MediaTypeOut);
		pFxPin->GetPinType(&fxPinType);
		if(fxPinType == PIN_OUT)
		{
			CFxPin* pFxPinClass = NULL;
			pFxPinClass = static_cast<CFxPin*>(pFxPin);

			CFxPinIter Iter;
			/*! for each Pin in list */
			for (Iter = pFxPinClass->_lpFxPinIn.begin(); (Iter != pFxPinClass->_lpFxPinIn.end()) && !(pFxPinClass->_lpFxPinIn.empty()); Iter++)
			{
				(*Iter)->GetPinState(&PinState);
				if(PinState == PIN_CONNECTED)
				{
					(*Iter)->GetPinRunState(&PinRunState);
					if(PinRunState == PIN_START)
					{
						(*Iter)->GetConnectionMediaType(&MediaTypeIn);
						if( (MediaTypeOut.MainMediaType  == MediaTypeIn.MainMediaType) &&
							(MediaTypeOut.SubMediaType   == MediaTypeIn.SubMediaType) )
							(*Iter)->WaitForIFxMedia(dwTimeStamp, dwUser);
					}
				}
			}
		}
	}
	return;
}

Void IFxPinCallback::FxMedia(IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	*pdwFxMediaSize = 0;
	*pdwFxMediaNumber = 0;

	return;
}

CFxPin::CFxPin(PFX_PIN pPinInfo, std::string& strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance, Uint32 dwFxPinInstance, Int32& hr)
{
	memset(&_ConnectionMediaType, 0, sizeof(FX_MEDIA_TYPE));

	_IsPinConnected = FALSE;
	_IsConnectionMediaTypeSet = FALSE;

	_PinRunState = PIN_STOP;

	_ShoudStopCallBack = FALSE;

	_pConnectedPin = NULL;

	_dwFxProcessTime = 0;


	_pLogTrace = NULL;
	_pLogDump = NULL;

    _strDumpFilePath = "";

	_DumpState = DUMP_OFF;

	_pFxMediaPool		= NULL;

	_hFxMember		= NULL;

	_dwFxEngineInstance = dwFxEngineInstance;
	_dwFxInstance = dwFxInstance;
	_dwThisInstance = dwFxPinInstance;

	_pPinCallbackCS = NULL;

	_pqTxRxByte = 0;

	hr = FX_OK;

	_pPinInfo = NULL;
	_pPinInfo = new FX_PIN;
	if(_pPinInfo && pPinInfo)
	{
		_pPinInfo->strPinName = pPinInfo->strPinName;
		_pPinInfo->pMediaTypes = NULL;

		_pPinInfo->PinType = pPinInfo->PinType;
        _pPinInfo->pMediaTypes = NULL;
		_pPinInfo->pMediaTypes = new FX_MEDIA_TYPE[pPinInfo->wMediaTypeCount];
		if(_pPinInfo->pMediaTypes)
		{
			for(Int16 Idx = 0; Idx < pPinInfo->wMediaTypeCount; Idx++)
			{
				memcpy(&_pPinInfo->pMediaTypes[Idx], &pPinInfo->pMediaTypes[Idx], sizeof(FX_MEDIA_TYPE));
			}
		}
		_pPinInfo->wMediaTypeCount = pPinInfo->wMediaTypeCount;
		_pPinInfo->pIFxPinCallBack = pPinInfo->pIFxPinCallBack;

		_strFxName = strFxName;
		_FxLogFile = "";

		std::string strFxTrace = "";
#ifdef WIN32
		strFxTrace =  GetTemporaryDir() + "\\SMProcess.log\\FxPin";
#else
		strFxTrace =  GetTemporaryDir() + "/SMProcess.log/FxPin";
#endif

#ifdef WIN32
        if (_access(strFxTrace.c_str(), 0) == 0)
#else
        if (access(strFxTrace.c_str(), 0) == 0)
#endif
        
#ifdef WIN32
		_FxLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Pin_" + _pPinInfo->strPinName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#else
		_FxLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Pin_" + _pPinInfo->strPinName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#endif
		_FxLogFile.erase( remove(_FxLogFile.begin(),_FxLogFile.end(),' ') , _FxLogFile.end() );
#ifdef WIN32
        if (_access(_FxLogFile.c_str(), 0) == 0) {
#else
        if (access(_FxLogFile.c_str(), 0) == 0) {
#endif
				std::string strOldLogFile;
#ifdef WIN32
				strOldLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Pin_" + _pPinInfo->strPinName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#else
				strOldLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Pin_" + _pPinInfo->strPinName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#endif
				strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
				remove(strOldLogFile.c_str());
				rename(_FxLogFile.c_str(), strOldLogFile.c_str());
		}
		
		_pLogTrace = new CFxLog(_FxLogFile, LOG_IS_TRACE);

		_pLogTrace->FxTrace("********************************************************************************");
		_pLogTrace->FxTrace("              FxEngine:        FxPin %s",FXPIN_VERSION);
		_pLogTrace->FxTrace("********************************************************************************");
		_pLogTrace->FxTrace("");
		_pLogTrace->FxTrace("====== FxPin constructor (instance number is %d) ======", _dwThisInstance);

		/*! Create internal mediapool */
        _pFxMediaPool = new CFxMediaPool(MEDIA_SIZE, MEDIA_NUMBER);
		if(_pFxMediaPool == NULL)
		{
			_pLogTrace->FxTrace("FxPin::ConnectPin FEF_FAILED, Cannot create MediaPool");
			hr = FX_NOMEM;
			return;
		}

		/*! Launch the Fx thread */
		_pAPC = NULL;
		_pAPC = new CFxAPC(this);
		if(_pAPC == NULL)
		{
			_pLogTrace->FxTrace("FxPin::ConnectPin FEF_FAILED, Cannot create APC");
			hr = FX_NOMEM;
			return;
		}
	}
	else
	{
		hr = FX_ERROR;
		return;
	}
}

CFxPin::~CFxPin()
{
    _PinRunState = PIN_STOP;
	_ShoudStopCallBack = FALSE;
    
    _pAPC->StopAPC();
    _PinState._pAPC->StopAPC();

	AutoLock lock(_PinCS);

	/*! APC Thread */
	SAFE_DELETE_OBJECT(_pAPC);

	CFxPinObserverIter Iter;
	for (Iter = _FxPinObservers.begin(); (Iter != _FxPinObservers.end()) && !(_FxPinObservers.empty()); Iter++)
		SAFE_DELETE_OBJECT((*Iter));

	_FxPinObservers.clear();

	SAFE_DELETE_ARRAY(_pPinInfo->pMediaTypes);
	SAFE_DELETE_OBJECT(_pPinInfo);

	SAFE_DELETE_OBJECT(_pFxMediaPool);

	_lpFxPinIn.clear();
	_lpFxPinOut.clear();

	lock.unlock();

	if(_pLogTrace)
		_pLogTrace->FxTrace("===== FxPin destructor (instance number is %d) =====", _dwThisInstance);

	SAFE_DELETE_OBJECT(_pLogTrace);
	SAFE_DELETE_OBJECT(_pLogDump);
}

Int32 CFxPin::UpdateMediaPool(Uint32 dwSize, Uint32 dwNumber)
{
    _pFxMediaPool->UpdateMedia(dwSize, dwNumber);
    return FX_OK;
}

Void CFxPin::SetPinCallBackCS(CSPair* pPinCS)
{
	_pPinCallbackCS = pPinCS;
	return;
}

Int32 CFxPin::GetPinName(std::string& strPinName)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetPinName entry");
	if(_pPinInfo)
	{
		strPinName = _pPinInfo->strPinName;
		return FX_OK;
	}

	_pLogTrace->FxTrace("\tFxPin::GetPinName FEF_FAILED !!");
	return FX_ERROR;
}

Int32 CFxPin::GetPinType(FX_PIN_TYPE* pPinType)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetPinType entry");

	if( (!_pPinInfo) || (!pPinType) )
	{
		_pLogTrace->FxTrace("\tFxPin::GetPinType FEF_FAILED");
		return FX_ERROR;
	}

	*pPinType = _pPinInfo->PinType;
	return FX_OK;
}

Int32 CFxPin::GetMediaTypeCount(Uint16* pwMediaTypeCount)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPinManager::GetMediaTypeCount entry");
	*pwMediaTypeCount = _pPinInfo->wMediaTypeCount;

	return FX_OK;
}

Int32 CFxPin::GetMediaType(PFX_MEDIA_TYPE pMediaType, Uint16 wMediaTypeIndex)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPinManager::GetMediaType entry");

	if(pMediaType == NULL)
	{
		return FX_ERROR;
	}

	if(_pPinInfo->wMediaTypeCount == 0)
	{
		pMediaType = NULL;
		return FX_ERROR;
	}

	if(wMediaTypeIndex > (_pPinInfo->wMediaTypeCount - 1))
	{
		_pLogTrace->FxTrace("\tFxPin::GetMediaType FEF_FAILED !! (wMediaTypeIndex too big)");
		return FX_ERROR;
	}

	/*! Copy address of Media Type */
	for(Int16 Idx = 0; Idx < _pPinInfo->wMediaTypeCount; Idx++)
	{
		if(Idx == wMediaTypeIndex)
		{
			pMediaType->MainMediaType  = _pPinInfo->pMediaTypes[Idx].MainMediaType;
			pMediaType->SubMediaType   = _pPinInfo->pMediaTypes[Idx].SubMediaType;
            break;
		}
	}

	return FX_OK;
}

/*----------------------------------------------------------------------*//*!
	CheckMediaType().

	The CheckMediaType function checks the current pin media to the media given in parameter.
    @param	IN pPinMediaType:    The media to compare.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxPin::CheckMediaType(PFX_MEDIA_TYPE pMediaType)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::CheckMediaType entry");

	if(!_pPinInfo)
		return FX_ERROR;

	if( (!_pPinInfo->pMediaTypes) || (!pMediaType) )
	{
		_pLogTrace->FxTrace("\tFxPin::CheckMediaType FEF_FAILED !!");
		return FX_ERROR;
	}

	for(Int16 Idx = 0; Idx < _pPinInfo->wMediaTypeCount; Idx++)
	{
		if( (pMediaType->MainMediaType  == _pPinInfo->pMediaTypes[Idx].MainMediaType) &&
			(pMediaType->SubMediaType   == _pPinInfo->pMediaTypes[Idx].SubMediaType) )
		{
			_pLogTrace->FxTrace("FxPin::CheckMediaType Match");
			return FX_OK;
		}
	}
	_pLogTrace->FxTrace("\tFxPin::CheckMediaType Not Match");
	return FX_ERROR;
}

/*----------------------------------------------------------------------*//*!
	SetConnectionMediaType().

	The SetConnectionMediaType function set the current pin media with the media given in parameter.
    @param	IN pPinMediaType:  The Pin media to apply.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxPin::SetConnectionMediaType(PFX_MEDIA_TYPE pMediaType)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::SetConnectionMediaType entry");

	if( (!pMediaType) )
	{
		_pLogTrace->FxTrace("\tFxPin::SetConnectionMediaType FEF_FAILED");
		return FX_ERROR;
	}

	memcpy(&_ConnectionMediaType, pMediaType, sizeof(FX_MEDIA_TYPE));

	_IsConnectionMediaTypeSet = TRUE;

	return FX_OK;
}

/*----------------------------------------------------------------------*//*!
	GetConnectionMediaType().

	The GetConnectionMediaType function return the current pin media.
    @param	IN pPinMediaType:  The current Pin media.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxPin::GetConnectionMediaType(PFX_MEDIA_TYPE pMediaType)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetConnectionMediaType entry");

	if( pMediaType == NULL )
	{
		_pLogTrace->FxTrace("\tFxPin::GetConnectionMediaType FEF_FAILED");
		return FX_ERROR;
	}

	if(_IsConnectionMediaTypeSet == FALSE)
	{
		FX_MEDIA_TYPE MediaType;
		MediaType.MainMediaType = MAIN_TYPE_UNDEFINED;
		MediaType.SubMediaType  = SUB_TYPE_UNDEFINED;
		_pLogTrace->FxTrace("\tFxPin::GetConnectionMediaType FEF_FAILED (Pin not connected)");
		return FX_OK;
	}

	memcpy(pMediaType, &_ConnectionMediaType, sizeof(FX_MEDIA_TYPE));

	return FX_OK;
}

/*----------------------------------------------------------------------*//*!
	DeliverMedia().

	The DeliverMedia add buffer to the in/out pin.
    @param	IN pData:  The buffer to add.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxPin:: DeliverMedia(IFxMedia* pIFxMedia)
{
	_pLogTrace->FxTrace("FxPin::DeliverMedia entry");

	if(pIFxMedia == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::DeliverMedia FEF_FAILED !! (Ivalid Parameter pIFxMedia)");
		return FX_ERROR;
	}

	if(_pPinInfo->PinType == PIN_IN)
	{
		Uint32 dwMediaSize;
		pIFxMedia->GetDataLength(&dwMediaSize);
		if(_DumpState == DUMP_START)
		{
			FX_MEDIA_TYPE MediaType;
			pIFxMedia->GetMediaType(&MediaType);
			Uint8* pDataPtr;
			pIFxMedia->GetMediaPointer(&pDataPtr);
			_pLogDump->FxDump(pDataPtr, dwMediaSize);
		}
		if( (_PinRunState == PIN_STOP) || (_IsPinConnected == FALSE) )
		{
            pIFxMedia->Release();
			_pLogTrace->FxTrace("FxPin::DeliverMedia quit");
			return FX_OK;
		}
		_pqTxRxByte += dwMediaSize;
		_pAPC->AddAPCObject(pIFxMedia);
	}
	else
	{
		/*! in pin must not lockable cause deliver and WaitForIFxMedia can be call in the time */
		AutoLock lock(_PinCS);

		if( _PinRunState == PIN_STOP )
		{
			pIFxMedia->Release();
			_pLogTrace->FxTrace("FxPin::DeliverMedia quit");
			return FX_OK;
		}

		if(_IsPinConnected == FALSE)
		{
            pIFxMedia->Release();
			_pLogTrace->FxTrace("\tFxPin::DeliverMedia, Pin is not connected");
			return FX_OK;
		}

		if(_pConnectedPin == NULL)
        {
            pIFxMedia->Release();
			return FX_OK;
        }

		Uint32 dwMediaSize;
		pIFxMedia->GetDataLength(&dwMediaSize);
		if(_DumpState == DUMP_START)
		{
			FX_MEDIA_TYPE MediaType;
			pIFxMedia->GetMediaType(&MediaType);
			Uint8* pDataPtr;
			pIFxMedia->GetMediaPointer(&pDataPtr);
			_pLogDump->FxDump(pDataPtr, dwMediaSize);
		}
		if(_pLogTrace->ShouldTrace())
		{
			FX_MEDIA_TYPE MediaType;
			pIFxMedia->GetMediaType(&MediaType);
			Uint32 dwMediaSize;
			pIFxMedia->GetDataLength(&dwMediaSize);
			_pLogTrace->FxTrace("-->FxPin::DeliverMedia (MainType: %s, SubType: %s, Mediasize: %d)", (GetConstToString(FX_MAINMEDIATYPE_CONST, MediaType.MainMediaType)).c_str(),
																								(GetConstToString(FX_SUBMEDIATYPE_CONST, MediaType.SubMediaType)).c_str(),
																										dwMediaSize);
		}
		_pqTxRxByte += dwMediaSize;
		_pConnectedPin->DeliverMedia(pIFxMedia);
	}
	_pLogTrace->FxTrace("FxPin::DeliverMedia quit");
	return FX_OK;
}

Int32 CFxPin::GetFreeMediaNumber(Uint32* pdwFreeMediaNumber)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetFreeMediaNumber entry");

	if(pdwFreeMediaNumber)
	{
		if(_pFxMediaPool)
		{
			_pFxMediaPool->GetFreeMediaNumber(pdwFreeMediaNumber);
		}
		else
		{
			_pLogTrace->FxTrace("FxPin::GetFreeMediaNumber FEF_FAILED !! (MediaPool not defined)");
			return FX_ERROR;
		}
	}

	_pLogTrace->FxTrace("FxPin::GetFreeMediaNumber quit");
	return FX_OK;
}

Int32 CFxPin::GetDeliveryMedia(IFxMedia** ppIFxMedia, Uint32 dwTimeOut)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetDeliveryMedia entry, TimeOut:%d", dwTimeOut);

	*ppIFxMedia = NULL;

	Int32 hr;

	if(ppIFxMedia)
	{
        if(_pFxMediaPool) {
            lock.unlock();
			hr = _pFxMediaPool->GetDeliveryMedia(ppIFxMedia, dwTimeOut);
            lock.lock();
        }
		else
		{
			_pLogTrace->FxTrace("\tFxPin::GetDeliveryMedia FEF_FAILED !! (MediaPool not defined)");
			return FX_ERROR;
		}

		if(FEF_FAILED(hr))
		{
			_pLogTrace->FxTrace("\tFxPin::GetDeliveryMedia FEF_FAILED !!, hr: %d", hr);
			return hr;
		}

        if(_IsConnectionMediaTypeSet == FALSE)
		{
			FX_MEDIA_TYPE MediaType;
			lock.unlock();
			if(FEF_FAILED(hr = GetMediaType(&MediaType, 0))) { //!< Get first media type
				_pLogTrace->FxTrace("FxPin::GetDeliveryMedia Undefined Types (Pin is not connected)");
				return hr;
			}
			lock.lock();
			(*ppIFxMedia)->SetMediaType(&MediaType);
			_pLogTrace->FxTrace("FxPin::GetDeliveryMedia Undefined Types (Pin is not connected)");
			return FX_OK;
		}

		(*ppIFxMedia)->SetMediaType(&_ConnectionMediaType);
		_pLogTrace->FxTrace("FxPin::GetDeliveryMedia quit");
		return FX_OK;
	}

	_pLogTrace->FxTrace("\tFxPin::GetDeliveryMedia FEF_FAILED !!(Ivalid Parameter ppIFxMedia)");
	return FX_ERROR;
}

Int32 CFxPin::Flush()
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::Flush entry");

    if(_IsPinConnected == FALSE)
        return FX_OK;
	
	if(_pPinInfo->PinType == PIN_IN)
	{
		//! Check Pin run state.
		if((_PinRunState == PIN_STOP) /*|| _ShoudStopCallBack*/)
			return FX_OK;
		_pAPC->AddAPCObject(NULL);
	}
	else
	{
		/*! Flush downstream can be done even if pin is stopped */
		_pConnectedPin->Flush();
	}
	_pLogTrace->FxTrace("FxPin::Flush quit");
	return FX_OK;
}

Int32 CFxPin::InitStream(IFxMedia* pIFxMedia)
{
    _pLogTrace->FxTrace("FxPin::InitStream entry");

	if(pIFxMedia == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::InitStream FEF_FAILED !! (Ivalid Parameter pIFxMedia)");
		return FX_ERROR;
	}

	if(_pPinInfo->PinType == PIN_IN)
	{
		if( /*(_PinRunState == PIN_STOP) ||*/ (_IsPinConnected == FALSE) )
		{
            pIFxMedia->Release();
			_pLogTrace->FxTrace("FxPin::InitStream quit");
			return FX_OK;
		}

		_pAPC->AddAPCObject(pIFxMedia);
	}
	else
	{
		/*! in pin must not lockable cause deliver and WaitForIFxMedia can be call in the time */
		AutoLock lock(_PinCS);

		if(_IsPinConnected == FALSE)
		{
            pIFxMedia->Release();
			_pLogTrace->FxTrace("\tFxPin::InitStream, Pin is not connected");
			return FX_OK;
		}

		if(_pConnectedPin == NULL)
        {
            pIFxMedia->Release();
			return FX_OK;
        }
		/*! Set up init stream flag to pass over apc thread */
        static_cast<CFxMedia*>(pIFxMedia)->SetInitMedia(TRUE);
        _pConnectedPin->InitStream(pIFxMedia);
	}
	_pLogTrace->FxTrace("FxPin::InitStream quit");
	return FX_OK;
}

/*----------------------------------------------------------------------*//*!
	ConnectPin().

	The ConnectPin function connect two pins.
    @param	IN pInPin:  The Pin to connect to the current Pin.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxPin::ConnectPin(IFxPin* pFxPin)
{
    AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::ConnectPin entry");

	if(_IsPinConnected == TRUE)
	{
		_pLogTrace->FxTrace("FxPin::ConnectPin, already connected");
		return FX_OK;
	}

	if(!pFxPin)
	{
		_pLogTrace->FxTrace("\tFxPin::ConnectPin FEF_FAILED !! (Invalid Parameter: pFxPin)");
		lock.unlock();
		NotifyObservers(FX_INVALPARAM, FX_ERROR_CONNECT_STATE);
		return FX_INVALPARAM;
	}

    /*! Check pin address*/
    if(pFxPin == static_cast<CFxPin*>(this))
    {
        _pLogTrace->FxTrace("\tFxPin::ConnectPin FEF_FAILED !! (Same pin address)");
		lock.unlock();
		NotifyObservers(FX_INVALHANDLE, FX_ERROR_CONNECT_STATE);
		return FX_INVALHANDLE;
	}

	_pConnectedPin = NULL;

    /*! Check pin type*/
	FX_PIN_TYPE fxExternPinType;
	pFxPin->GetPinType(&fxExternPinType);
	if(_pPinInfo->PinType == fxExternPinType)
	{
	    _pLogTrace->FxTrace("\tFxPin::ConnectPin FEF_FAILED !! (Equal pin types)");
	    lock.unlock();
	    NotifyObservers(FX_INVALPINTYPE, FX_ERROR_CONNECT_STATE);
		return FX_INVALPINTYPE;
	}

    if(_pPinInfo->PinType == PIN_OUT)
	{
		/*! Get media buffer properties */
		Uint32 dwSize1 = MEDIA_SIZE;
		Uint32 dwSize2 = MEDIA_SIZE;
		Uint32 dwNumber1 = MEDIA_NUMBER;
		Uint32 dwNumber2 = MEDIA_NUMBER;
		
	    lock.unlock();
		_pPinInfo->pIFxPinCallBack->FxMedia(this, &dwSize1, &dwNumber1);
		lock.lock();
		
        CFxPin* pFxPinClass = NULL;
		pFxPinClass = static_cast<CFxPin*>(pFxPin);
		
	    lock.unlock();
		pFxPinClass->_pPinInfo->pIFxPinCallBack->FxMedia(this, &dwSize2, &dwNumber2);
		lock.lock();
		
		/*! Update internal mediapool */
		UpdateMediaPool( (dwSize1 == 0) && (dwSize2 == 0) ? MEDIA_SIZE : MAX(dwSize1, dwSize2),
											   (dwNumber1 == 0) && (dwNumber2 == 0) ? MEDIA_NUMBER : MAX(dwNumber1, dwNumber2));

        pFxPinClass->UpdateMediaPool( (dwSize1 == 0) && (dwSize2 == 0) ? MEDIA_SIZE : MAX(dwSize1, dwSize2),
											   (dwNumber1 == 0) && (dwNumber2 == 0) ? MEDIA_NUMBER : MAX(dwNumber1, dwNumber2));
		_pConnectedPin = pFxPin;

        if(_pLogTrace->ShouldTrace())
		{
			std::string strExternPinName;
			pFxPin->GetPinName(strExternPinName);
			_pLogTrace->FxTrace("FxPin::ConnectPin --> Connect(%s to %s)",_pPinInfo->strPinName.c_str(), strExternPinName.c_str());
		}

	}
	else if(_pPinInfo->PinType == PIN_IN)
	{
	    _pConnectedPin = pFxPin;

		if(_pLogTrace->ShouldTrace())
		{
           	std::string strExternPinName;
			pFxPin->GetPinName(strExternPinName);
			_pLogTrace->FxTrace("FxPin::ConnectPin --> Connect(%s from %s)",_pPinInfo->strPinName.c_str(), strExternPinName.c_str());
		}
	}

    _PinState._pAPC->StartAPC(); //!< Start APC callback
    _pAPC->StartAPC(); //!< Start APC callback
    
    _pqTxRxByte = 0;
	_IsPinConnected = TRUE;
	
	_pLogTrace->FxTrace("FxPin::ConnectPin quit");
	return FX_OK;
}

Int32 CFxPin::DisconnectPin()
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::DisconnectPin entry");

    //_PinRunState = PIN_STOP;
    SetPinRunState(PIN_STOP, FALSE);

	if(_pConnectedPin == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::DisconnectPin Pin: Pin is not connected");
		_pLogTrace->FxTrace("FxPin::DisconnectPin quit");
		return FX_OK;
	}

    _IsPinConnected = FALSE;
    _pConnectedPin = NULL;
	_pqTxRxByte = 0;

	/*! Waiting for end of callback (queue and callback calling) */
    lock.unlock();
    _PinState._pAPC->StopAPC();
    lock.lock();

	/*! Waiting for and of pin thread */
    lock.unlock();
    _pAPC->StopAPC();
    lock.lock();

	lock.unlock();
	SendFxPinStateEx(PIN_NOT_CONNECTED); //!< send message outside the thread
	lock.lock();

    memset(&_ConnectionMediaType, 0, sizeof(FX_MEDIA_TYPE));
	_IsConnectionMediaTypeSet = FALSE;

	/*if(_pPinInfo->PinType == PIN_IN)
	{
		Void* pObject = NULL;
		while( (pObject = _pAPC->GetAPCObject()) != NULL )
		{
			((IFxMedia*)pObject)->Release();
		}
	}*/

    ///*! Waiting for and of pin thread */
    //lock.unlock();
    //_pAPC->StopAPC();
    //lock.lock();

    /*! Release all media data */
    _pFxMediaPool->ReleaseAllMedia();

    _pLogTrace->FxTrace("FxPin::DisconnectPin quit");
	return FX_OK;
}

Int32 CFxPin::GetPinConnected(IFxPin** ppFxPin)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetPinConnected entry");

	if(ppFxPin == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::GetPinConnected FEF_FAILED !! (Invalid Parameter: ppFxPin)");
		return FX_INVALPARAM;
	}

	if(_IsPinConnected == TRUE)
	{
		*ppFxPin = _pConnectedPin;
		return FX_OK;
	}

	*ppFxPin = NULL;
	return FX_OK;
}

Int32 CFxPin::GetPinState(FX_PIN_STATE* pPinState)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetPinState entry");

	if(pPinState == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::GetPinState FEF_FAILED !! (Invalid Parameter: pPinState)");
		return FX_INVALPARAM;
	}

	*pPinState = (_IsPinConnected == TRUE) ? PIN_CONNECTED : PIN_NOT_CONNECTED;

	return FX_OK;
}

Int32 CFxPin::GetPinRunState(PIN_RUN_STATE* pPinRunState)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetPinRunState entry");

	if(pPinRunState == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::GetPinRunState FEF_FAILED !! (Invalid Parameter: pPinRunState)");
		return FX_INVALPARAM;
	}

	*pPinRunState = _PinRunState;

	return FX_OK;
}

Int32 CFxPin::SetPinRunState(PIN_RUN_STATE PinRunState, Bool ShouldProtect/* = TRUE*/)
{
	AutoLock lock(_PinCS, ShouldProtect);
	_pLogTrace->FxTrace("FxPin::SetPinRunState entry: %d", PinRunState);

    _PinRunState = PinRunState;

    return FX_OK;
}


Int32 CFxPin::GetProcessTime(Uint32* pdwProcessingTime)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::GetProcessTime entry");

	if(pdwProcessingTime == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::GetProcessTime FEF_FAILED !! (Invalid Parameter: pdwProcessingTime)");
		return FX_INVALPARAM;
	}

	if(_pPinInfo->PinType == PIN_IN)
		*pdwProcessingTime = _dwFxProcessTime;
	else
		*pdwProcessingTime = 0;

	return FX_OK;
}

Int32 CFxPin::InitDumpData(const std::string strFilePath)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::InitDumpData entry");

    _strDumpFilePath = strFilePath;

	/*! Open log file */
	FILE* fd = NULL;

#ifdef WIN32
    errno_t hr;
    hr = fopen_s(&fd, _strDumpFilePath.c_str(), "ab");
    if (hr != 0)
#else
    fd = fopen(_strDumpFilePath.c_str(), "ab");
    if(fd == NULL)
#endif
	{
		_pLogTrace->FxTrace("\tFxPin::InitDumpData FEF_FAILED !! (Invalid Parameter: strFilePath)");
		_DumpState = DUMP_OFF;
		return FX_INVALPARAM;
	}

	// Close log file
    fclose(fd);
#ifdef WIN32
	if (_access(_strDumpFilePath.c_str(), 0) == 0) {
#else
    if (access(_strDumpFilePath.c_str(), 0) == 0) {
#endif
		remove(_strDumpFilePath.c_str());
	}

	Char strTemp[MAX_PATH];
#ifdef WIN32
	sprintf_s(strTemp, MAX_PATH, _strDumpFilePath.c_str());
#else
	sprintf(strTemp, _strDumpFilePath.c_str());
#endif
	std::string str = strTemp;

	_pLogDump = new CFxLog(str, LOG_IS_DUMP);

	_DumpState = DUMP_INIT;

	_pLogTrace->FxTrace("FxPin::InitDumpData quit");
	return FX_OK;
}

Int32 CFxPin::StartDumpData()
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::StartDumpData entry");

	if(_DumpState == DUMP_OFF)
	{
		_pLogTrace->FxTrace("\tFxPin::StartDumpData FEF_FAILED !! (Not Initialized)");
		return FX_ERROR;
	}

	_DumpState = DUMP_START;

	return FX_OK;
}

Int32 CFxPin::StopDumptData()
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::StopDumptData entry");

    if(_DumpState == DUMP_OFF)
	{
		_pLogTrace->FxTrace("\tFxPin::StopDumptData FEF_FAILED !! (Not Initialized)");
		return FX_ERROR;
	}

	_DumpState = DUMP_STOP;

	return FX_OK;
}

Int32 CFxPin::AttachObservers(CFxPinObserver* pFxPinObservers, FX_PTR qUserParam)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::AttachObservers entry");

	if(pFxPinObservers == NULL)
	{
		_pLogTrace->FxTrace("\tFxPin::AttachObservers FEF_FAILED !! (Invalid Parameter: pFxPinObservers)");
		return FX_INVALPARAM;
	}

	/* Is pFxPinObserver already exist ? */
	Bool IsFound = FALSE;
	CFxPinObserverIter Iter;
	for (Iter = _FxPinObservers.begin(); (Iter != _FxPinObservers.end()) && !(_FxPinObservers.empty()); Iter++)
	{
		if( (*Iter)->pObserver == pFxPinObservers )
		{
			SAFE_DELETE_OBJECT((*Iter));
			IsFound = TRUE;
			break;
		}
	}

	if(IsFound == FALSE)
	{
		FX_OBSERVER* pFxObserver = NULL;
		pFxObserver = new FX_OBSERVER;
		if(pFxObserver == NULL)
		{
			_pLogTrace->FxTrace("\tFxPin::AttachObservers FEF_FAILED !! (No free memory)");
			return FX_NOMEM;
		}

		pFxObserver->pObserver = pFxPinObservers;
		pFxObserver->qUserParam = qUserParam;

		_FxPinObservers.push_back(pFxObserver);
	}

	_pLogTrace->FxTrace("FxPin::AttachObservers quit");
	return FX_OK;
}

Int32 CFxPin::DetachObservers(CFxPinObserver* pFxPinObservers)
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::DetachObservers entry");

    CFxPinObserverIter Iter;
	if(pFxPinObservers != NULL)
	{
		Bool IsFound = FALSE;
        for (Iter = _FxPinObservers.begin(); (Iter != _FxPinObservers.end()) && !(_FxPinObservers.empty()); Iter++)
        {
	        if( (*Iter)->pObserver == pFxPinObservers )
	        {
		        IsFound = TRUE;
		        break;
	        }
        }

        if(IsFound == TRUE)
        {
            SAFE_DELETE_OBJECT((*Iter));
	        _FxPinObservers.erase((Iter));
        }
    }
    else
    {
        Iter = _FxPinObservers.begin();
        while(Iter != _FxPinObservers.end())
        {
            SAFE_DELETE_OBJECT((*Iter));
	        Iter = _FxPinObservers.erase((Iter));
        }
        _FxPinObservers.clear();
    }

	_pLogTrace->FxTrace("FxPin::DetachObservers quit");

	return FX_OK;
}

/*! Call only with AutoLock before */
Void CFxPin::NotifyObservers(Int32 sdwHr, FX_STATE FxState)
{
	_pLogTrace->FxTrace("FxPin::NotifyObservers entry");

    CFxPinObserverIter Iter;
	for (Iter = _FxPinObservers.begin(); (Iter != _FxPinObservers.end()) && !(_FxPinObservers.empty()); Iter++)
		(*Iter)->pObserver->FxPinUpdate(this, sdwHr, FxState, (*Iter)->qUserParam);

	return;
}

Void CFxPin::LinkFxPin(CFxPin* pFxPin, FX_PIN_TYPE PinType)
{
    AutoLock lock(_PinCS);

	if(PinType == PIN_IN)
		_lpFxPinIn.push_back(pFxPin);
	else
		_lpFxPinOut.push_back(pFxPin);

	return;
}

Void CFxPin::UnLinkFxPin(CFxPin* pFxPin)
{
    AutoLock lock(_PinCS);

    CFxPinIter Iter;
    Iter = std::find(_lpFxPinIn.begin(), _lpFxPinIn.end(), static_cast<CFxPin*>(pFxPin));
	if( Iter != _lpFxPinIn.end() )
        _lpFxPinIn.erase(Iter);

    Iter = std::find(_lpFxPinOut.begin(), _lpFxPinOut.end(), static_cast<CFxPin*>(pFxPin));
	if( Iter != _lpFxPinOut.end() )
        _lpFxPinOut.erase(Iter);

    return;
}

Void CFxPin::SendFxPinState(FX_PIN_STATE PinState)
{
	if(_ShoudStopCallBack == TRUE)       //!< _ShoudStopCallBack is true when pinmanager is destroyed
		return;						     //!< to avoid send message when IFx is released
		
	CNotifyObjects* pParam = CNotifyObjects::Create( (FX_PTR)this, (FX_PTR)PinState, (FX_PTR)NULL);
	_PinState._pAPC->AddAPCObject(pParam);

	return;
}

/*! Call APC callback outside the thread */
Void CFxPin::SendFxPinStateEx(FX_PIN_STATE PinState)
{
	if(_ShoudStopCallBack == TRUE)       //!< _ShoudStopCallBack is true when pinmanager is destroyed
		return;							 //!< to avoid send message when IFx is released

    CNotifyObjects* pParam = CNotifyObjects::Create( (FX_PTR)this, (FX_PTR)PinState, (FX_PTR)NULL);
    _PinState._pAPC->_pAPCCallBack->APCCallBack(pParam);
    
	return;
}

Int32 CFxPin::WaitForIFxMedia(Uint32 dwTimeStamp, FX_PTR dwUser)
{
	Int32 hr = FX_OK;

	_pLogTrace->FxTrace("FxPin::WaitForIFxMedia entry");

	FX_PIN_TYPE fxPinType = _pPinInfo->PinType;

	if(fxPinType == PIN_IN)
	{
		AutoLock lock(_PinCS);
		if(_PinRunState == PIN_STOP)
		{
			_pLogTrace->FxTrace("\tFxPin::WaitForIFxMedia, Pin is stopped");
			return FX_OK;
		}

		if(_IsPinConnected == FALSE)
		{
			_pLogTrace->FxTrace("\tFxPin::WaitForIFxMedia, Pin is not connected");
			return FX_ERROR;
		}

		if(_pConnectedPin == NULL)
		{
			_pLogTrace->FxTrace("\tFxPin::WaitForIFxMedia, Pin is not connected");
			return FX_ERROR;
		}

		_pConnectedPin->WaitForIFxMedia(dwTimeStamp, dwUser);
	}
	else
	{
		if(_PinRunState == PIN_STOP)
		{
			_pLogTrace->FxTrace("FxPin::WaitForIFxMedia, Pin is stopped");
			return FX_OK;
		}

		if(_IsPinConnected == FALSE)
		{
			_pLogTrace->FxTrace("FxPin::WaitForIFxMedia, Pin is not connected");
			return FX_ERROR;
		}

		if(_pConnectedPin == NULL)
		{
			_pLogTrace->FxTrace("FxPin::WaitForIFxMedia, Pin is not connected");
			return FX_ERROR;
		}

		CNotifyObjects* pParam = CNotifyObjects::Create( (FX_PTR)this, (FX_PTR)dwTimeStamp, (FX_PTR)dwUser);
		_pAPC->AddAPCObject((Void*)pParam);

	}
    _pLogTrace->FxTrace("FxPin::WaitForIFxMedia quit");
	return hr;
}

Void CFxPin::QuitInfiniteTime()
{
	AutoLock lock(_PinCS);
    /*! Free getdelivry media in case where we are in infinite or ms time */
    _pFxMediaPool->StopWaiting();
	return;
}

Void CFxPin::WaitForEndDataInQueue()
{
	AutoLock lock(_PinCS);
	_pLogTrace->FxTrace("FxPin::WaitForEndDataInQueue entry (APC size: %ld)", _pAPC->GetObjectNumberInQueue());
    lock.unlock(); 

	//_ShoudStopCallBack = TRUE;

    /*! Waiting for the end of data in queue */
    /*! We use PeekMessage to free all meesages in queue */
#ifdef WIN32
    MSG   msg;
#endif
	while( _pAPC->GetObjectNumberInQueue() > 0 ) {
#ifdef WIN32
        if(PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ))
        {
            TranslateMessage( &msg ); 
            DispatchMessage( &msg ); 
        }
#endif
        boost::thread::yield(); 
    }
    
    lock.lock();
    _pLogTrace->FxTrace("FxPin::WaitForEndDataInQueue quit");
	return;
}

signed long int CFxPin::APCCallBack(void* pObject)
{
	Int32 hr = FX_OK;

	_pLogTrace->FxTrace("FxPin::APC entry");

    AutoLock lock(_PinCS);

	if(_pPinInfo->PinType == PIN_IN)
	{
		Uint32	dwLocalElapseTime = 0;

		IFxMedia* pIFxMedia = (IFxMedia*)pObject;

		AutoLock lockCallBack(*_pPinCallbackCS->first.first);

		if(pIFxMedia) //!< Normal media to process
		{
			if(_IsPinConnected == FALSE || _ShoudStopCallBack)
            {
				pIFxMedia->Release();
				_pLogTrace->FxTrace("FxPin::APC, Pin is not connected");
				return 0;
			}
            Bool IsInitStream = static_cast<CFxMedia*>(pIFxMedia)->IsInitMedia();
			FX_STREAM_STATE StreamState = STREAM_INIT;
            if(IsInitStream == FALSE) {
			    //! Check Pin run state.
			    if((_PinRunState == PIN_STOP) /*|| _ShoudStopCallBack*/)
			    {
				    pIFxMedia->Release();
				    _pLogTrace->FxTrace("FxPin::APC, Pin is stopped");
				    return 0;
			    }    

			    if((_PinRunState == PIN_PAUSE) /*|| _ShoudStopCallBack*/)
			    {
				    _pLogTrace->FxTrace("FxPin::APC, Pin in pause, repeat FxMedia");
				    boost::thread::yield(); 
				    return 1;
			    } 
				StreamState = STREAM_PROCESS;
            }

			if(_pLogTrace->ShouldTrace())
			{
				FX_MEDIA_TYPE MediaType;
				pIFxMedia->GetMediaType(&MediaType);
				Uint32 dwMediaSize;
				pIFxMedia->GetDataLength(&dwMediaSize);
				_pLogTrace->FxTrace("Received Media(MainType: %s, SubType: %s, Mediasize: %d)", (GetConstToString(FX_MAINMEDIATYPE_CONST, MediaType.MainMediaType)).c_str(),
									 (GetConstToString(FX_SUBMEDIATYPE_CONST, MediaType.SubMediaType)).c_str(),
									 dwMediaSize);
			}

			_tElapse.restart();
			lock.unlock();
			hr = _pPinInfo->pIFxPinCallBack->FxPin(static_cast<IFxPin*>(this), pIFxMedia, StreamState);
            lock.lock();
			dwLocalElapseTime = (Uint32)(_tElapse.elapsed()*1000.f);
			
			if(FEF_FAILED(hr))
				_pLogTrace->FxTrace("\tFxPin function FEF_FAILED !!, hr=%d", hr);
			else
			{
				//! Time processing is computed here.
				_dwFxProcessTime = dwLocalElapseTime;
			}

			if(hr == FX_REPEATFXMEDIA)
			{
				_pLogTrace->FxTrace("FxPin::APC, FX_REPEATFXMEDIA");
				boost::thread::yield(); 
				return 1;
			}

			pIFxMedia->Release();
		}
		else //!< Flush pin
		{
			lock.unlock();
            if(_ShoudStopCallBack)
                return 0;
			hr = _pPinInfo->pIFxPinCallBack->FxPin(static_cast<IFxPin*>(this), NULL, STREAM_FLUSH);
            lock.lock();
			if(FEF_FAILED(hr))
				_pLogTrace->FxTrace("\tFxPin Flush function FEF_FAILED !!, hr=%d", hr);
			if(hr == FX_REPEATFXMEDIA) {
				boost::thread::yield(); 
				return 1;
			}
		}
	}
	else
	{
		CNotifyObjects* pParam = (CNotifyObjects*)pObject;

		AutoLock lockCallBack(*_pPinCallbackCS->first.second);

		if(_IsPinConnected == FALSE || _ShoudStopCallBack)
        {
			pParam->Release();
			_pLogTrace->FxTrace("FxPin::APC, Pin is not connected");
			return 0;
		}

		//! Check Pin run state.
		if((_PinRunState == PIN_STOP) /*|| _ShoudStopCallBack*/)
		{
			pParam->Release();
			_pLogTrace->FxTrace("FxPin::APC, Pin is stopped");
			return 0;
		}        

		if((_PinRunState == PIN_PAUSE) /*|| _ShoudStopCallBack*/)
		{
			_pLogTrace->FxTrace("FxPin::APC, Pin in pause, repeat FxMedia");
			boost::thread::yield(); 
			return 1;
		}  

		_pLogTrace->FxTrace("Wait Media: %d", (Uint32)pParam->_qParam1);
		lock.unlock();
		_pPinInfo->pIFxPinCallBack->FxWaitForIFxMedia(static_cast<IFxPin*>(this), (Uint32)pParam->_qParam1, (Uint32)pParam->_qParam2);
		lock.lock();
		pParam->Release();
	}
	_pLogTrace->FxTrace("FxPin::APC quit");
	return 0;
}

CPinState::CPinState()
{
	_pAPC = NULL;
	_pAPC = new CFxAPC(this);
}

CPinState::~CPinState()
{
	/*! APC Thread */
	SAFE_DELETE_OBJECT(_pAPC);
}

signed long int CPinState::APCCallBack(void* pObject)
{
	Int32 hr = FX_OK;

	CNotifyObjects* pParam = (CNotifyObjects*)pObject;
	CFxPin* pPin = ((CFxPin*)(pParam->_qInstance));

    if(pPin->_ShoudStopCallBack == FALSE)
	{
	    AutoLock lockCallBack(*pPin->_pPinCallbackCS->second);
	
		pPin->_pPinInfo->pIFxPinCallBack->FxPinState(static_cast<IFxPin*>(pPin), (FX_PIN_STATE)pParam->_qParam1);
	}

	pParam->Release();

	return hr;
}

 } //namespace FEF
