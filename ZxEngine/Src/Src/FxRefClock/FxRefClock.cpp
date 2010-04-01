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
#include "FxDef.h"
#include "FxRefClock.h"
#include "FxUtils/FxUtils.h"
#include "boost/lexical_cast.hpp"

namespace FEF {

#define FXREFCLOCK_VERSION        "2.0.0.0"	//<! Y: compnent interface, Z: component code
/* History */
/*	1.1.0.0 (21/08/06)
	Use Boost thread library
	Add asynchronous trace library
*/

/*  2.0.0.0 (03/10/07)
	Replace Char by std::string.
    Add const in GetFxParam method.
*/


//Int32 IFxRefClock::GetFxEngineRefClock(Uint64* pqRefClock)
//{return NOT_IMPLEMENTED;}
//Int32 IFxRefClock::GetFxRefClock(Uint64* pqRefClock, Uint32 dwId)
//{return NOT_IMPLEMENTED;}
//Int32 IFxRefClock::SetFxRefClock(Uint64 qRefClock, Uint32 dwId)
//{return NOT_IMPLEMENTED;}
//Int32 IFxRefClock::SetFxRefClockError()
//{return NOT_IMPLEMENTED;}
//Int32 IFxRefClock::Release()
//{return NOT_IMPLEMENTED;}

// initialise the static instance count.
//Uint32 CFxRefClock::_dwInstanceCount = 0;

CFxRefClock::CFxRefClock(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance)
{
	_FxRefClockObserver.pObserver = NULL;

	_dwThisInstance = dwFxInstance; //!< For message trace only

	_strFxName = strFxName;

	std::string strFxTrace = "";
#ifdef WIN32
	strFxTrace =  GetTemporaryDir() + "\\SMProcess.log\\FxRefClock";
#else
	strFxTrace =  GetTemporaryDir() + "/SMProcess.log/FxRefClock";
#endif

#ifdef WIN32
	if (_access(strFxTrace.c_str(), 0) == 0)
#else
    if (access(strFxTrace.c_str(), 0) == 0)
#endif

#ifdef WIN32
		_FxLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#else
		_FxLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#endif
		_FxLogFile.erase( remove(_FxLogFile.begin(),_FxLogFile.end(),' ') , _FxLogFile.end() );
#ifdef WIN32
        if (_access(_FxLogFile.c_str(), 0) == 0) {
#else
        if (access(_FxLogFile.c_str(), 0) == 0) {
#endif
            std::string strOldLogFile;
#ifdef WIN32
			_FxLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#else
			_FxLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#endif
			strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
            remove(strOldLogFile.c_str());
            rename(_FxLogFile.c_str(), strOldLogFile.c_str());
        }
	
	_pLogTrace = new CFxLog(_FxLogFile, LOG_IS_TRACE);

	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "              FxEngine:        FxRefClock %s",FXREFCLOCK_VERSION);
	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "");
    _pLogTrace->FxTrace( "====== FxRefClock constructor (instance number is %d) ======", _dwThisInstance);
}

CFxRefClock::~CFxRefClock()
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "===== FxRefClock destructor (instance number is %d) =====", _dwThisInstance);
	
	SAFE_DELETE_OBJECT(_pLogTrace);

	lock.unlock();
}

// fxEngine put its observer here.
Int32 CFxRefClock::AttachObserver(CFxRefClockObserver* pFxRefClockObserver, FX_PTR qUserParam)
{
	AutoLock lock(_CS);

	_FxRefClockObserver.pObserver = pFxRefClockObserver;
	_FxRefClockObserver.qUserParam = qUserParam;

	return FX_OK;
}

Int32 CFxRefClock::SetFxRefClock(Uint64 qRefClock, Uint32 dwId)
{
	AutoLock lock(_CS);

	Int32 hr;
	if(_FxRefClockObserver.pObserver)
	{
		hr = _FxRefClockObserver.pObserver->SetFxRefClock(qRefClock, dwId);
		if(FEF_FAILED(hr))
		{
			_pLogTrace->FxTrace(  "FxEngine::SetFxRefClock FEF_FAILED !!, hr=%d", hr);
			return FX_ERROR;
		}
	}
	else
	{
		_pLogTrace->FxTrace( "CFxRefClock::SetFxRefClock FEF_FAILED !!(No observer defined)");
		return FX_ERROR;
	}
	return FX_OK;
}

Int32 CFxRefClock::GetFxRefClock(Uint64* pqRefClock, Uint32 dwId)
{
	AutoLock lock(_CS);

	Int32 hr;
	if(_FxRefClockObserver.pObserver)
	{
		hr = _FxRefClockObserver.pObserver->GetFxRefClock(pqRefClock, dwId);
		if(FEF_FAILED(hr))
		{
			_pLogTrace->FxTrace(  "FxEngine::GetFxRefClock FEF_FAILED !!, hr=%d", hr);
			return FX_ERROR;
		}
	}
	else
	{
		_pLogTrace->FxTrace( "CFxRefClock::GetFxRefClock FEF_FAILED !!(No observer defined)");
		return FX_ERROR;
	}
	return FX_OK;
}

Int32 CFxRefClock::GetFxEngineRefClock(Uint64* pqRefClock)
{
	AutoLock lock(_CS);

	if(_FxRefClockObserver.pObserver)
		_FxRefClockObserver.pObserver->GetFxEngineRefClock(pqRefClock);
	else
	{
		_pLogTrace->FxTrace( "CFxRefClock::GetFxEngineRefClock FEF_FAILED !!(No observer defined)");
		return FX_ERROR;
	}
	return FX_OK;
}

 } //namespace FEF
