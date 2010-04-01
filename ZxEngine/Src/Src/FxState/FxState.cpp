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
#include "FxDef.h"
#include "FxState.h"
#include "boost/lexical_cast.hpp"

namespace FEF {

#define FXSTATE_VERSION        "4.0.0.0"	//<! Y: compnent interface, Z: component code
/* History */
/*	2.0.0.0
	Use Boost thread library
	Add asynchronous trace library
*/
/*	2.0.0.0 (09/07/2007)
	Add CState::_StateObserverCS
*/
/*	3.0.0.0 (03/10/2007)
	Replace Char by std::string.
*/
/*	4.0.0.0 (03/01/2009)
	Added FxGetState method.
	Added FxPublishState.
*/
// initialise the static instance count.
//Uint32 CFxState::_dwInstanceCount = 0;

CFxState::CFxState(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance)
{
	_pStateObjt = NULL;
	_LastFxState = FX_UNDEFINED_STATE;

	_dwThisInstance = dwFxInstance;	//!< For message trace only

	_strFxName = strFxName;

	std::string strFxTrace = "";
#ifdef WIN32
	strFxTrace =  GetTemporaryDir() + "\\SMProcess.log\\FxState";
#else
	strFxTrace =  GetTemporaryDir() + "/SMProcess.log/FxState";
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
			strOldLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#else
			strOldLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(dwFxEngineInstance) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#endif
			strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
            remove(strOldLogFile.c_str());
            rename(strOldLogFile.c_str(), strOldLogFile.c_str());
		}
	_pLogTrace = new CFxLog(_FxLogFile, LOG_IS_TRACE);

	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "              FxEngine:        FxState %s",FXSTATE_VERSION);
	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "");
    _pLogTrace->FxTrace( "====== FxState constructor (instance number is %d) ======", dwFxInstance);
}

CFxState::~CFxState()
{
	AutoLock lock(_CS);
	_pStateObjt = NULL;

	_pLogTrace->FxTrace( "===== FxState destructor (instance number is %d) =====", _dwThisInstance);
	
	SAFE_DELETE_OBJECT(_pLogTrace);
	lock.unlock();
}

Int32 CFxState::AttachFxObserver(CState* pStateObjt)
{
	AutoLock lock(_CS);

	if(pStateObjt == NULL)
	{
		return FX_INVALPARAM;
	}

	_pStateObjt = pStateObjt;
	if(_LastFxState > FX_UNDEFINED_STATE)
		_pStateObjt->PublishFxState(_LastFxState);

	return FX_OK;
}

Int32 CFxState::DetachFxObserver()
{
	AutoLock lock(_CS);

	_pStateObjt = NULL;
	_LastFxState = FX_UNDEFINED_STATE;

	return FX_OK;
}

Int32 CFxState::FxPublishState( /* [in] */FX_STATE FxState)
{
	AutoLock lock(_CS);

	if(_pStateObjt != NULL)
	{
		_pStateObjt->PublishFxState(FxState);
	}
	else
	{
		_LastFxState = FxState;
	}

	_pLogTrace->FxTrace( "Fx state:%s", (GetFxStringState(FxState)).c_str());

	return FX_OK;
}

Int32 CFxState::FxGetState( /* [out] */FX_STATE *pFxState) 
{
	AutoLock lock(_CS);
	if(pFxState == NULL)
		return FX_INVALPARAM;

	*pFxState = _LastFxState;

	return FX_OK;
}

std::string CFxState::GetFxStringState(/* [in] */FX_STATE FxState)
{
	return GetConstToString(FX_STATE_CONST, FxState);
}

 } //namespace FEF
