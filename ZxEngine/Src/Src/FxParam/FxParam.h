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
#include "IFxParam.h"
#include "FxConst2String/FxConst2String.h"

#include <boost/thread/thread.hpp>

#include "FxTrace/FxTrace.h"

#include <list>

namespace FEF {

//! Fx Parameters structure
typedef struct __FX_PARAMtag{
	PFX_PARAM pFxParam;					//!< Fx Parameters types
	Void*  pvFxParam;					//!< Fx Parameter values
} FX_PARAM_INFO, *PFX_PARAM_INFO;

typedef std::list<PFX_PARAM_INFO>    lFxParams;
typedef lFxParams::iterator			 lFxParamIter;

//! Fx string Parameters structure
typedef struct __FX_PARAMStringtag{
	PFX_PARAM_STRING pFxParam;					//!< Fx Parameters info
    std::string  strFxParam;					//!< Fx Parameter values
} FX_PARAM_STRING_INFO, *PFX_PARAM_STRING_INFO;

typedef std::list<PFX_PARAM_STRING_INFO>    lFxParamsString;
typedef lFxParamsString::iterator			lFxParamStringIter;

class CFxParam : public IFxParam,
				 public CFxConst2String {

public:
	CFxParam(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance);
	virtual ~CFxParam();

public:
	//!
	//virtual Int32 Release();
	virtual Int32 AddFxParam(const PFX_PARAM pFxParam);
    virtual Int32 AddFxParam(const PFX_PARAM_STRING pFxParam);
    virtual Int32 RemoveFxParam(const std::string strParamName);

	virtual Int32 GetFxParamSize(const std::string strParamName, Uint32* pdwParamSize);

	virtual Int32 SetFxParamValue(const std::string strParamName, const Void* pvParamValue);
    virtual Int32 SetFxParamValue(const std::string strParamName, const std::string strParamValue);

	virtual Int32 GetFxParamValue(const std::string strParamName, Void* pvParamValue);
    virtual Int32 GetFxParamValue(const std::string strParamName, std::string& strParamValue);

	virtual Int32 GetFxParamCount(Uint16* pwParamCount);
    virtual Int32 GetFxParamStringCount(Uint16* pwParamCount);
	virtual Int32 GetFxParam(const FX_PARAM** ppFxParam, const Uint16 wFxParamIndex);
    virtual Int32 GetFxParam(const FX_PARAM_STRING** ppFxParam, const Uint16 wFxParamIndex);

private:
	lFxParams	_lParams;
    lFxParamsString _lParamsString;

private:
	Int32 FindParamter(const std::string strParamName, FX_PARAM_INFO** ppFxParamInfo);
    Int32 FindParamter(const std::string strParamName, FX_PARAM_STRING_INFO** ppFxParamInfo);

private:
	boost::mutex _CS;

private:
	std::string _strFxName;

private:
	std::string _FxLogFile;
	CFxLog* _pLogTrace;

	// Instance value to manage the right log messages.
	Uint32 _dwThisInstance;				// current instance
};

} //namespace FEF

