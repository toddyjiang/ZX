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
#include "Fx.h"
#include "FxEngineCore.h"
#include "boost/lexical_cast.hpp"

namespace FEF {

// initialise the static instance count.
Uint32 CFx::_dwInstanceCount = 0;

#define FX_VERSION        "2.2.0.0"  //<! W: Fx/CFx interface, X: Fx/CFx code, Y: compnent interface, Z: component code
/* History */
/*	2.0.0.0 (01/05/2006)
	Add IFxPin::GetMediaTypeCount
	Add IFxPin::GetMediaType
*/
/*	2.1.0.0 (21/08/2006)
	Use Boost thread library
	Add asynchronous trace library
*/
/*  2.2.0.0 (24/04/2007)
    Fix scanf in GetIFxVersion method (replace uint16 by int32)
*/
namespace
{
Int32 CFx::GetIFxVersion(Uint16* pwMajor, Uint16* pwMinor, Uint16* pwBuild, Uint16* pwRev)
{
	AutoLock lock(_CS);

	if(pwMajor == NULL)
	{
		_pLogTrace->FxTrace("Fx::GetIFxVersion FEF_FAILED !! (Invalid Parameter: pwMajor)");
		return FX_INVALPARAM;
	}

	if(pwMinor == NULL)
	{
		_pLogTrace->FxTrace("Fx::GetIFxVersion !! (Invalid Parameter: pwMinor)");
		return FX_INVALPARAM;
	}

	if(pwBuild == NULL)
	{
		_pLogTrace->FxTrace("Fx::GetIFxVersion FEF_FAILED !! (Invalid Parameter: pwBuild)");
		return FX_INVALPARAM;
	}

	if(pwRev == NULL)
	{
		_pLogTrace->FxTrace( "Fx::GetIFxVersion FEF_FAILED !! (Invalid Parameter: pwRev)");
		return FX_INVALPARAM;
	}

	Int32 sdwMajor = 0;
	Int32 sdwMinor = 0;
	Int32 sdwBuild = 0;
	Int32 sdwRev = 0;
#ifdef WIN32
	sscanf_s(FX_VERSION, "%d.%d.%d.%d", &sdwMajor, &sdwMinor, &sdwBuild, &sdwRev); // C4996
#else
	sscanf(FXENGINE_VERSION, "%ld.%ld.%ld.%ld", &sdwMajor, &sdwMinor, &sdwBuild, &sdwRev); // C4996
#endif

	*pwMajor = (Uint16)sdwMajor;
	*pwMinor = (Uint16)sdwMinor;
	*pwBuild = (Uint16)sdwBuild;
	*pwRev = (Uint16)sdwRev;

	return FX_OK;
}

// Create a new database object and return a pointer to it
Int32 CFx::FxGetInterface(FX_INTERFACE FxInterfaceType, Void** ppFxInterface)
{
	AutoLock lock(_CS);

	*ppFxInterface = NULL;

	if(_pFxObject == NULL)
		return FX_NOMEM;

	if(FxInterfaceType == IFX_PINMANGER)
		*ppFxInterface=static_cast<IFxPinManager*>( _pFxObject );
	else if(FxInterfaceType == IFX_PARAM)
		*ppFxInterface=static_cast<IFxParam*>( _pFxObject );
	else if(FxInterfaceType == IFX_REFCLOCK)
		*ppFxInterface = static_cast<IFxRefClock*>( _pFxObject );
	else if(FxInterfaceType == IFX_STATE)
		*ppFxInterface = static_cast<IFxState*>( _pFxObject );
	else
	{
		_pLogTrace->FxTrace( "Fx::GetInterface Error !! (Invalid Interface: %d)",  FxInterfaceType);
		return FX_INVALPARAM;
	}

	_dwRef++;
	switch(FxInterfaceType)
	{
	case IFX_PINMANGER:
        _pLogTrace->FxTrace( "Fx::GetInterface IFX_PINMANGER");
		break;
	case IFX_PARAM:
        _pLogTrace->FxTrace( "Fx::GetInterface IFX_PARAM");
		break;
	case IFX_REFCLOCK:
        _pLogTrace->FxTrace( "Fx::GetInterface IFX_REFCLOCK");
		break;
	case IFX_STATE:
        _pLogTrace->FxTrace( "Fx::GetInterface IFX_STATE");
		break;
	}

	return (_dwRef);
}

Int32 CFx::Release()
{
	AutoLock lock(_CS);

	if(_dwRef)
		_pLogTrace->FxTrace( "Fx::All Interfaces are not released: Ref=%d !!", _dwRef);

	_pLogTrace->FxTrace("====== Fx destructor (instance number is %d) ======", _dwThisInstance);
    
	lock.unlock();

	SAFE_DELETE_OBJECT(_pLogTrace);
	CFx::_dwInstanceCount--;
	delete this; //!< Delete Fx object
	return 0;
}

Int32 CFx::FxReleaseInterface()
{
	AutoLock lock(_CS);

	_dwRef--;

	return _dwRef;
}

Int32 CFx::GetConstToString(FXENGINE_CONST_TYPE FxEngineConstType, Int32 sdwFxEngineConst, std::string& strStateName)
{
	strStateName = _FxConst2String.GetConstToString(FxEngineConstType, sdwFxEngineConst);

	return FX_OK;
}



CFx::CFx(const std::string strFxName):
CFxPinManager(strFxName, CFxEngineCore::_dwInstanceCount, _dwInstanceCount),
CFxParam(strFxName, CFxEngineCore::_dwInstanceCount, _dwInstanceCount),
CFxRefClock(strFxName, CFxEngineCore::_dwInstanceCount, _dwInstanceCount),
CFxState(strFxName, CFxEngineCore::_dwInstanceCount, _dwInstanceCount)
{
	_pFxObject = NULL;

	_dwThisInstance = _dwInstanceCount; //!< For message trace only

	_dwRef = 0;

	if(!strFxName.empty()) 
	    _strFxName = strFxName;
    else
        _strFxName ="FxNullName";
	
	std::string strFxTrace = "";
#ifdef WIN32
	strFxTrace =  GetTemporaryDir() + "\\SMProcess.log\\Fx";
#else
	strFxTrace =  GetTemporaryDir() + "/SMProcess.log\\Fx";
#endif
#ifdef WIN32
	if (_access(strFxTrace.c_str(), 0) == 0)
#else
    if (access(strFxTrace.c_str(), 0) == 0)
#endif
    
#ifdef WIN32
        _FxLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(CFxEngineCore::_dwInstanceCount) + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#else
		_FxLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(CFxEngineCore::_dwInstanceCount) + boost::lexical_cast<std::string>(_dwThisInstance) + ".txt";
#endif
		_FxLogFile.erase( remove(_FxLogFile.begin(),_FxLogFile.end(),' ') , _FxLogFile.end() );
#ifdef WIN32
        if (_access(_FxLogFile.c_str(), 0) == 0) {
#else
        if (access(_FxLogFile.c_str(), 0) == 0) {
#endif
            std::string strOldLogFile;
#ifdef WIN32
            strOldLogFile = strFxTrace + "\\" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(CFxEngineCore::_dwInstanceCount) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#else
			strOldLogFile = strFxTrace + "/" + _strFxName.c_str() + "_Log" + boost::lexical_cast<std::string>(CFxEngineCore::_dwInstanceCount) + boost::lexical_cast<std::string>(_dwThisInstance) + ".old";
#endif
			strOldLogFile.erase( remove(strOldLogFile.begin(),strOldLogFile.end(),' ') , strOldLogFile.end() );
            remove(strOldLogFile.c_str());
            rename(_FxLogFile.c_str(), strOldLogFile.c_str());
        }
		
	_pLogTrace = new CFxLog(_FxLogFile, LOG_IS_TRACE);

	_pLogTrace->FxTrace("********************************************************************************");
	_pLogTrace->FxTrace("              FxEngine:        Fx Core %s", FX_VERSION);
	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "");
	_pLogTrace->FxTrace( "====== Fx constructor (instance number is %d) ======", _dwThisInstance);
	_pLogTrace->FxTrace( "Create Fx. Name: %s", _strFxName.c_str());

	if(_pFxObject == NULL)
        _pFxObject = this;
}
}
 } //namespace FEF

#ifdef __cplusplus
extern "C" {
#endif
namespace FEF {
FXENGINE_EXP Int32 FXENGINE_API FEF_CreateFx(IFx ** ppFx, const std::string strFxName)
{
	CFx::_dwInstanceCount++; //!< For message trace only
	CFx* pFx = new CFx(strFxName);
	if(pFx)
	{
		*ppFx=static_cast<IFx*>(pFx);
		return FX_OK;
	}

	return FX_NOMEM;
}
 } //namespace FEF
#ifdef __cplusplus
}
#endif
