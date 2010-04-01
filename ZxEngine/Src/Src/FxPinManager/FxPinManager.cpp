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
#include "FxTrace/FxTrace.h"
#include "FxPin/FxPin.h"
#include "Core/FxEngineCore.h"
#include "FxPinManager.h"
#include "FxUtils/FxUtils.h"
#include "boost/lexical_cast.hpp"

#include <algorithm>

namespace FEF {

#define FXMANAGER_VERSION        "3.2.0.0"	//<! Y: compnent interface, Z: component code
/* History */
/*	1.1.0.0 (28/05/2006)
	Add FxPinManager::InitFxPins method to initialize pins (pin can be created in real time)
*/
/*	1.2.0.0 (21/08/2006)
	Add asynchronous trace library
	Remove FxMediaPool given to pins( media pool is created in connect phase).
*/
/*	2.0.0.0 (28/07/2007)
	Add Remove pin method
*/
/*	3.0.0.0 (03/10/2007)
	Replace Char by std::string.
*/
/*	3.1.0.0 (29/12/2007)
	Test if pin callback always exists.
*/
/*	3.2.0.0 (15/06/2008)
	Call Quit infinite waiting time for all pin of both Fx in Remove method.
*/

CFxPinManager::CFxPinManager(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance)
{
	_dwPinInstance = 0;

	/*! Save the Fx Instance number */
	_dwFxInstance = dwFxInstance;

	/*! Save the FxEngine Instance number */
	_dwFxEngineInstance = dwFxEngineInstance;

	_FxLogFile = "";
	_strFxName = strFxName;

	_PinsAreInitialized = FALSE;

	std::string strFxTrace = "";
#ifdef WIN32
	strFxTrace =  GetTemporaryDir() + "\\SMProcess.log\\FxPinManager";
#else
	strFxTrace =  GetTemporaryDir() + "/SMProcess.log/FxPinManager";
#endif
#ifdef WIN32
	if (_access(strFxTrace.c_str(), 0) == 0)
#else
    if (access(strFxTrace.c_str(), 0) == 0)
#endif
        
#ifdef WIN32
		_FxLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + ".txt";
#else
		_FxLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + ".txt";
#endif
		_FxLogFile.erase( remove(_FxLogFile.begin(),_FxLogFile.end(),' ') , _FxLogFile.end() );
#ifdef WIN32
        if (_access(_FxLogFile.c_str(), 0) == 0) {
#else
        if (access(_FxLogFile.c_str(), 0) == 0) {
#endif
			std::string strOldLogFile;
#ifdef WIN32
		    strOldLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + ".old";
#else
			strOldLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwFxInstance) + ".old";
#endif
			strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
            remove(strOldLogFile.c_str());
            rename(_FxLogFile.c_str(), strOldLogFile.c_str());
		}
	_pLogTrace = new CFxLog(_FxLogFile, LOG_IS_TRACE);

	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "              FxEngine:        FxPinManager %s",FXMANAGER_VERSION);
	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "");
    _pLogTrace->FxTrace( "====== FxPinManager constructor (instance number is %d) ======", _dwFxInstance);
}

CFxPinManager::~CFxPinManager()
{
	ClearFxPinList();

	_pLogTrace->FxTrace( "===== FxPinManager destructor (instance number is %d) =====", _dwFxInstance);
	
	SAFE_DELETE_OBJECT(_pLogTrace);
}

Int32 CFxPinManager::Create(PFX_PIN pPinInfo, IFxPin** ppFxPin)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxPinManager::Create entry");

    if(ppFxPin == NULL)
	{
		_pLogTrace->FxTrace(  "CFxPinManager::Create FEF_FAILED !! (Invalid Parameter: ppFxPin)");
		return FX_INVALPARAM;
	}

	*ppFxPin = NULL;
	CFxPin* pFxPin = NULL;
	if(pPinInfo)
	{
	    if(pPinInfo->pMediaTypes == NULL)
		{
			_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Invalid Media Type parameter)");
			return FX_INVALPARAM;
		}
		
		if(pPinInfo->pIFxPinCallBack == NULL)
		{
			_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Invalid pIFxPinCallBack parameter)");
			return FX_INVALPARAM;
		}

		if(pPinInfo->wMediaTypeCount == 0)
		{
			_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Invalid wMediaTypeCount parameter)");
			return FX_INVALPARAM;
		}

        _pLogTrace->FxTrace( "FxPinManager::Create --> Try to create Pin(%s, Type: %s, MainType: %s, SubType: %s)",	pPinInfo->strPinName.c_str(),
																		(GetConstToString(FX_PINTYPE_CONST, (Int32)pPinInfo->PinType)).c_str(),
																		(GetConstToString(FX_MAINMEDIATYPE_CONST, (Int32)pPinInfo->pMediaTypes->MainMediaType)).c_str(),
																		(GetConstToString(FX_SUBMEDIATYPE_CONST, (Int32)pPinInfo->pMediaTypes->SubMediaType)).c_str());

		/*! Check main and sub type */
		switch(pPinInfo->pMediaTypes->MainMediaType)
		{
		case AUDIO_TYPE:
			{
				if( (pPinInfo->pMediaTypes->SubMediaType < PCM) || (pPinInfo->pMediaTypes->SubMediaType > WMA) )
				{
					_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Check sub Media Type of AUDIO_TYPE)");
					return FX_SUBMEDIANOTSUPPORTED;
				}
			}
			break;
		case VIDEO_TYPE:
			{
				if( (pPinInfo->pMediaTypes->SubMediaType < MPV) || (pPinInfo->pMediaTypes->SubMediaType > XVID) )
				{
					_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Check sub Media Type of VIDEO_TYPE)");
					return FX_SUBMEDIANOTSUPPORTED;
				}
			}
			break;
		case TEXT_TYPE:
			{
				if( (pPinInfo->pMediaTypes->SubMediaType < UTF_8) || (pPinInfo->pMediaTypes->SubMediaType > ASCII) )
				{
					_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Check sub Media Type of DATA_TYPE)");
					return FX_SUBMEDIANOTSUPPORTED;
				}
			}
			break;
		case DATA_TYPE:
			{
				if( (pPinInfo->pMediaTypes->SubMediaType < VECTOR) || (pPinInfo->pMediaTypes->SubMediaType > MATRIX) )
				{
					_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Check sub Media Type of DATA_TYPE)");
					return FX_SUBMEDIANOTSUPPORTED;
				}
			}
			break;
		default:
			_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !! (Unknown main Media Type)");
			return FX_MEDIANOTSUPPORTED;
			break;
		}

		Int32 hr;
		pFxPin = new CFxPin(pPinInfo, _strFxName, _dwFxEngineInstance, _dwFxInstance, _dwPinInstance++, hr);
		if(FEF_FAILED(hr))
		{
			_pLogTrace->FxTrace( "FxPinManager::Create Pin FEF_FAILED !!");
			return hr;
		}

		/*! Send In/Out pin liste to the created Pin */
		if(pFxPin)
		{
			CFxPinIter Iter;
			FX_PIN_TYPE PinType;
			for (Iter = _lpFxPin.begin(); (Iter != _lpFxPin.end()) && !(_lpFxPin.empty()); Iter++)
				(*Iter)->LinkFxPin(pFxPin, pPinInfo->PinType);
			for (Iter = _lpFxPin.begin(); (Iter != _lpFxPin.end()) && !(_lpFxPin.empty()); Iter++)
			{
				(*Iter)->GetPinType(&PinType);
				pFxPin->LinkFxPin((*Iter), PinType);
			}
			_lpFxPin.push_back(pFxPin);
		}
		else
		{
			_pLogTrace->FxTrace( "FxPinManager::Create Pin memory FEF_FAILED !!");
			return FX_NOMEM;
		}

		/*! Initialize the callback CS */
		boost::mutex *pCSPinIn, *pCSPinOut, *pCSPinState;

		CSPinMap::iterator it;
		it = _CSPinMap.find( pPinInfo->pIFxPinCallBack );

		if( it == _CSPinMap.end() )
		{
			pCSPinIn = new boost::mutex;
			pCSPinOut = new boost::mutex;
			pCSPinState = new boost::mutex;

			//CSPair CSPin;
			CSPair* pCSPin = new CSPair(make_pair(pCSPinIn, pCSPinOut), pCSPinState);
			//CSPin = make_pair(make_pair(pCSPinIn, pCSPinOut), pCSPinState);
			_CSPinMap.insert(make_pair(pPinInfo->pIFxPinCallBack, pCSPin));
			pFxPin->SetPinCallBackCS(pCSPin);
		}
		else
		{
			pFxPin->SetPinCallBackCS(it->second);
		}

		/*! Init the new FxPin */
		if(_PinsAreInitialized == TRUE)
		{
			pFxPin->SetFxMember(_hFx);
			if( FEF_FAILED(hr = pFxPin->AttachObservers(_pFxPinObservers, (FX_PTR)_hFx) ))
				return hr;
		}

		*ppFxPin = static_cast<IFxPin*>(pFxPin);

		_pLogTrace->FxTrace( "FxPinManager::Create --> Pin(%s, Type: %s, MainType: %s, SubType: %s) OK ",	pPinInfo->strPinName.c_str(),
																		(GetConstToString(FX_PINTYPE_CONST, (Int32)pPinInfo->PinType)).c_str(),
																		(GetConstToString(FX_MAINMEDIATYPE_CONST, (Int32)pPinInfo->pMediaTypes->MainMediaType)).c_str(),
																		(GetConstToString(FX_SUBMEDIATYPE_CONST, (Int32)pPinInfo->pMediaTypes->SubMediaType)).c_str());
		return FX_OK;
	}

	_pLogTrace->FxTrace( "FxPinManager::Create FEF_FAILED !!");
	return FX_ERROR;
}

Int32 CFxPinManager::Remove(IFxPin* pFxPin)
{
    AutoLock lock(_CS);
	Int32 hr;
	_pLogTrace->FxTrace( "FxPinManager::Remove entry");

    if(pFxPin == NULL)
	{
		_pLogTrace->FxTrace(  "FxPinManager::Remove FEF_FAILED !! (Invalid Parameter: pFxPin is null)");
		return FX_INVALPARAM;
	}

    /*! Find Pin */
    CFxPinIter Iter;
    CFxPin* pFxPinClass;

    /*! Find address of Pins created in list */
	Iter = std::find(_lpFxPin.begin(), _lpFxPin.end(), static_cast<CFxPin*>(pFxPin));
	if( Iter != _lpFxPin.end() )
	{
        pFxPinClass = (*Iter);

		/*! Call before DisconnectPin, because DisconnectPin set to NULL connected pin */
        IFxPin* pFxPinConnected = NULL;
	    pFxPinClass->GetPinConnected(&pFxPinConnected);
		CFxPin* pFxPinConnectedClass = NULL;
		pFxPinConnectedClass = static_cast<CFxPin*>(pFxPinConnected);

        /*! Stop, disconnect and dettach observer pin */
        pFxPinClass->SetPinRunState(PIN_STOP);
        pFxPinClass->_ShoudStopCallBack = TRUE;
        if(pFxPinConnected) {
			pFxPinConnectedClass->SetPinRunState(PIN_STOP);
        }

        /*! Quit infinite waiting time for all pin of both Fx */
		CFxPinIter IterPin;
		for (IterPin = _lpFxPin.begin(); (IterPin != _lpFxPin.end()) && !(_lpFxPin.empty()); IterPin++)
			(*IterPin)->QuitInfiniteTime(); /*< Stop pin callbacks when we remove the FX */
		
		if(pFxPinConnectedClass) {
			FX_HANDLE hFx;
		    FX_COMPONENT* pFx;
			hFx = pFxPinConnectedClass->GetFxMember();
			pFx = static_cast<FX_COMPONENT*>(hFx);
			Uint16 wPinCount;
			if( FEF_FAILED(hr = pFx->pFxPinManager->GetPinCount(wPinCount)) ) {
				_pLogTrace->FxTrace(  "\tCFxPinManager::Remove FEF_FAILED !! (Cannot get pin count)");
				return hr;
			}
			for(Int16 Idx = 0; Idx < wPinCount; Idx++)
			{
				IFxPin* pFxPin = NULL;
				CFxPin* pFxPinClass = NULL;
				pFx->pFxPinManager->GetPin(&pFxPin, Idx);
				if(pFxPin) {
					pFxPinClass = static_cast<CFxPin*>(pFxPin);
					pFxPinClass->QuitInfiniteTime(); /*< Stop pin callbacks when we remove the FX */
				}
			}
		}

        pFxPinClass->WaitForEndDataInQueue();
		if(pFxPinConnected)
			pFxPinConnectedClass->WaitForEndDataInQueue();
       
        pFxPinClass->DisconnectPin();
        pFxPinClass->DetachObservers(NULL);
        if(pFxPinConnected)
	    {
		    pFxPinConnectedClass->DisconnectPin();
			pFxPinConnectedClass->DetachObservers(NULL);
	    }
        
		_lpFxPin.erase(Iter);

        /*! Unlink Pin */
        for (Iter = _lpFxPin.begin(); (Iter != _lpFxPin.end()) && !(_lpFxPin.empty()); Iter++)
            (*Iter)->UnLinkFxPin(pFxPinClass);      

        SAFE_DELETE_OBJECT(pFxPinClass);			/*!< Delete the Pin object */
    }
    else
    {
        _pLogTrace->FxTrace(  "FxPinManager::Remove FEF_FAILED !! (Invalid Parameter: pFxPin not found)");
		return FX_INVALPARAM;
    }

    return FX_OK;
}

Int32 CFxPinManager::GetPinCount(Uint16& wPinCount)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxPinManager::GetPinCount entry");

	wPinCount = (Uint16)_lpFxPin.size();
	_pLogTrace->FxTrace( "FxPinManager::GetPinCount (%d)", wPinCount);
	return FX_OK;
}

Int32 CFxPinManager::GetPin(IFxPin** ppIFxPin, Uint16 wPinNumber)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxPinManager::GetPin entry");

	CFxPinIter Iter;

	if(ppIFxPin == NULL)
	{
		_pLogTrace->FxTrace( "FxPinManager::GetPin FEF_FAILED !! (Invalid Parameters: ppIFxPin)");
		return FX_INVALPARAM;
	}

	if(_lpFxPin.empty() == TRUE)
	{
		*ppIFxPin = NULL;
		return FX_OK;
	}

	if(wPinNumber > ((Uint16)_lpFxPin.size() - 1))
	{
		_pLogTrace->FxTrace( "FxPinManager::GetPin FEF_FAILED !! (Invalid Parameters: wPinNumber too big)");
		return FX_INVALPARAM;
	}

	/*! Copy address of Pins created in list */
	Uint16 Idx;
	*ppIFxPin = NULL;
	for (Iter = _lpFxPin.begin(), Idx = 0; (Iter != _lpFxPin.end()) && !(_lpFxPin.empty()); Iter++, Idx++)
	{
		if(Idx == wPinNumber)
		{
			*ppIFxPin =  static_cast<IFxPin*>(*Iter);			/*!< Copy the Pin object */
			break;
		}
	}

	return FX_OK;
}

Int32 CFxPinManager::GetPinNumber(IFxPin* pIFxPin, Uint16* pwPinNumber)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxPinManager::GetPinNumber entry");

	CFxPinIter Iter;

	if(pwPinNumber == NULL)
	{
		_pLogTrace->FxTrace( "FxPinManager::GetPinNumber FEF_FAILED !! (Invalid Parameters: pwPinNumber)");
		return FX_INVALPARAM;
	}

	if(_lpFxPin.empty() == TRUE)
	{
		return FX_ERROR;
	}

	/*! Find address of Pins created in list */
	Iter = std::find(_lpFxPin.begin(), _lpFxPin.end(), static_cast<CFxPin*>(pIFxPin));
	if( Iter != _lpFxPin.end() )
	{
		*pwPinNumber = (Uint16)distance(_lpFxPin.begin(), Iter); /*!< Copy the Pin object */
		return FX_OK;
	}

	/*
	Uint16 Idx;
		for (Iter = _lpFxPin.begin(), Idx = 0; (Iter != _lpFxPin.end()) && !(_lpFxPin.empty()); Iter++, Idx++)
		{
			if((*Iter) == pIFxPin)
			{
				*pwPinNumber = Idx;			/ *!< Copy the Pin object * /
				return FX_OK;
			}
		}*/

	_pLogTrace->FxTrace( "FxPinManager::GetPinNumber FEF_FAILED !! (Invalid Parameters: pIFxPin)");
	return FX_ERROR;

}

Int32 CFxPinManager::InitFxPins(CFxPinObserver* pFxPinObservers, FX_HANDLE hFx)
{
	Int32 hr;
	/*! Initialize FxPins */
	Uint16 wPinCount;
	if( FEF_FAILED(hr = GetPinCount(wPinCount)) )
		wPinCount = 0;

	for(Int16 Idx = 0; Idx < wPinCount; Idx++)
	{
		IFxPin* pFxPin = NULL;
		CFxPin* pFxPinClass = NULL;
		GetPin(&pFxPin, Idx);
		if(pFxPin)
		{
            pFxPinClass = static_cast<CFxPin*>(pFxPin);
			pFxPinClass->SetFxMember(hFx);
			if( FEF_FAILED(hr = pFxPinClass->AttachObservers(pFxPinObservers, (FX_PTR)hFx) ))
				return hr;
		}
	}

	_pFxPinObservers	= pFxPinObservers;
	_hFx				= hFx;

	/*! Set PinsAreInitialized to TRUE */
	_PinsAreInitialized = TRUE;
	return FX_OK;
}

Void CFxPinManager::ClearFxPinList()
{
	_pLogTrace->FxTrace( "FxPinManager::ClearFxPinList entry");
	/*! Remove Pins */
	CFxPin* pFxPinClass = NULL;
	Uint16 wPinCount = 0;
	if( FEF_FAILED(GetPinCount(wPinCount)) )
	    wPinCount = 0;
    do
    {
		
        IFxPin* pFxPin = NULL;
		GetPin(&pFxPin, 0);
		if(pFxPin) {
			/*! Disable callback for pin state first */
			pFxPinClass = static_cast<CFxPin*>(pFxPin);
			pFxPinClass->_ShoudStopCallBack = TRUE;
            Remove(pFxPin);
		}

        if( FEF_FAILED(GetPinCount(wPinCount)) )
		    wPinCount = 0;
    }while(wPinCount > 0);


	//CFxPinIter Iter;
 //   CFxPin* pFxPinClass;

	///*! Clear Pin created in list */
 //   for (Iter = _lpFxPin.begin(); (Iter != _lpFxPin.end()) && !(_lpFxPin.empty()); Iter++){
 //       
 //       pFxPinClass = (*Iter);

        /*! All pin are disconnected before in release Fx */

     //   /*! Stop, disconnect and dettach observer pin */
     //   pFxPinClass->SetPinRunState(PIN_STOP);
     //   pFxPinClass->WaitForEndDataInQueue();

	    //IFxPin* pFxPinConnected = NULL;
	    //pFxPinClass->GetPinConnected(&pFxPinConnected);

     //   CFxPin* pFxPinConnectedClass = NULL;
	    //if(pFxPinConnected)
	    //{
		   // pFxPinConnectedClass = static_cast<CFxPin*>(pFxPinConnected);
     //       /*! Stop, disconnect and dettach observer pin */
     //       pFxPinConnectedClass->SetPinRunState(PIN_STOP);
     //       pFxPinConnectedClass->WaitForEndDataInQueue();
		   // pFxPinConnectedClass->DisconnectPin();
	    //}

     //   pFxPinClass->DisconnectPin();

  //      pFxPinClass->DetachObservers(NULL);
		//SAFE_DELETE_OBJECT(pFxPinClass);			/*!< Delete the Pin object */
  //  }

	AutoLock lock(_CS);

	_lpFxPin.clear();

	CSPinMap::iterator it;
	/*! Clear CS callback Pin created in map */
	for ( it = _CSPinMap.begin( ); it != _CSPinMap.end( ); it++ )
	{
		SAFE_DELETE_OBJECT(it->second->first.first);
		SAFE_DELETE_OBJECT(it->second->first.second);
		SAFE_DELETE_OBJECT(it->second->second);
		SAFE_DELETE_OBJECT(it->second);
	}
	_CSPinMap.clear();

	_pLogTrace->FxTrace( "FxPinManager::ClearFxPinList quit");

	return;
}

 } //namespace FEF
