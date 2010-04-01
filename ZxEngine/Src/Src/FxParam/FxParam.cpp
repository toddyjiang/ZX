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
#include "FxParam.h"
#include "FxUtils/FxUtils.h"
#include "boost/lexical_cast.hpp"

namespace FEF {

#define FXPARAM_VERSION        "3.1.0.0"	//<! Y: compnent interface, Z: component code

/*	1.1.0.0 (21/08/2006)
	Add asynchronous trace library
-----------------------------------------------------
    2.0.0.0 (18/07/2007)
	Add IFxParam::RemoveFxParam
-----------------------------------------------------
    3.0.0.0 (02/10/2007)
	Replace Char by std::string.
    Add const in GetFxParam method.
--------------------------------------------
	3.1.0.0 (22/01/2008)
    Add string parameter to IFxParam interface
    Fix RemoveFxParam method, memory leak
*/

// initialise the static instance count.
//Uint32 CFxParam::_dwInstanceCount = 0;

CFxParam::CFxParam(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance)
{
	_dwThisInstance = dwFxInstance; //!< For message trace only

	_strFxName = strFxName;

	std::string strFxTrace = "";
#ifdef WIN32
	strFxTrace =  GetTemporaryDir() + "\\SMProcess.log\\FxParam";
#else
	strFxTrace =  GetTemporaryDir() + "/SMProcess.log/FxParam";
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
	_pLogTrace->FxTrace( "              FxEngine:        FxParam %s",FXPARAM_VERSION);
	_pLogTrace->FxTrace( "********************************************************************************");
	_pLogTrace->FxTrace( "");
    _pLogTrace->FxTrace( "====== FxParam constructor (instance number is %d) ======", _dwThisInstance);

}

CFxParam::~CFxParam()
{
	AutoLock lock(_CS);
	lFxParamIter  lParIter;
	for (lParIter = _lParams.begin(); (lParIter != _lParams.end()) && !(_lParams.empty()); lParIter++)
	{
		FX_PARAM_INFO* pFxParamInfo = (*lParIter);
		SAFE_DELETE_OBJECT((*lParIter)->pFxParam->pvMaxValue);
        SAFE_DELETE_OBJECT((*lParIter)->pFxParam->pvMinValue);
        SAFE_DELETE_OBJECT((*lParIter)->pFxParam->pvDefaultValue);

		SAFE_DELETE_ARRAY((*lParIter)->pvFxParam);
		SAFE_DELETE_OBJECT((*lParIter)->pFxParam);
		SAFE_DELETE_OBJECT((*lParIter));
	}
	_lParams.clear();

    lFxParamStringIter  lParStrIter;
	for (lParStrIter = _lParamsString.begin(); (lParStrIter != _lParamsString.end()) && !(_lParamsString.empty()); lParStrIter++)
	{
		FX_PARAM_STRING_INFO* pFxParamInfo = (*lParStrIter);
		SAFE_DELETE_OBJECT((*lParStrIter)->pFxParam);
		SAFE_DELETE_OBJECT((*lParStrIter));
	}
	_lParamsString.clear();

	lock.unlock();

	_pLogTrace->FxTrace( "===== FxParam destructor (instance number is %d) =====", _dwThisInstance);
	
	SAFE_DELETE_OBJECT(_pLogTrace);
}

template<typename T>
void CreateParam(Void** ppv, Uint32 dwSize, Void* pvDefault = NULL)
{
	*ppv = (T*) new T[dwSize];
    memset(*ppv, 0, sizeof(T)*dwSize);

    /*! Copy default value */
    if(pvDefault)
        memcpy(*ppv, pvDefault, sizeof(T)*dwSize);
}

template<typename T>
void SetParam(Void* pv, const Void* pValue, const Uint32 dwSize)
{
	memcpy(((T*)(pv)), ((T*)(pValue)), dwSize*sizeof(T));
}

template<typename T>
void GetParam(Void* pv, Void* pValue, Uint32 dwSize)
{
	memcpy(((T*)(pValue)), ((T*)(pv)), dwSize*sizeof(T));
}

template<typename T>
void AllocGetParam(Void* pv, Void** ppValue, Uint32 dwSize)
{
	*ppValue = new Uint8[dwSize*sizeof(T)];
	if(*ppValue)
		memcpy(((T*)(*ppValue)), ((T*)(pv)), dwSize*sizeof(T));
}

Int32 CFxParam::AddFxParam(const PFX_PARAM pFxParam)
{
	AutoLock lock(_CS);

	_pLogTrace->FxTrace( "FxParam::AddFxParam entry");

    FX_PARAM_INFO* pFxParamInfo = NULL;
    if(FEF_SUCCEEDED(FindParamter(pFxParam->strParamName, &pFxParamInfo)))
    {
        _pLogTrace->FxTrace( "FxParam::AddFxParam FEF_FAILED !! (Fx Parameter already exists)");
		return FX_INVALPARAM;
    }

    if(pFxParam->dwParamNumber == 0)
	{
		_pLogTrace->FxTrace( "FxParam::AddFxParam FEF_FAILED !! (Invalid Parameter: pFxParam->strParamUnitName)");
		return FX_ERROR;
	}

	pFxParamInfo = (FX_PARAM_INFO*)new FX_PARAM_INFO;
	if(pFxParamInfo == NULL)
		return FX_NOMEM;

	pFxParamInfo->pFxParam = NULL;
	pFxParamInfo->pFxParam = (FX_PARAM*)new FX_PARAM;
	if(pFxParamInfo->pFxParam == NULL)
		return FX_NOMEM;

	pFxParamInfo->pFxParam->strParamName = pFxParam->strParamName;
	
	pFxParamInfo->pFxParam->strParamUnitName = pFxParam->strParamUnitName;
	
	pFxParamInfo->pFxParam->ParamType = pFxParam->ParamType;
	pFxParamInfo->pFxParam->dwParamNumber = pFxParam->dwParamNumber;
    pFxParamInfo->pFxParam->pvMaxValue = NULL;
    pFxParamInfo->pFxParam->pvMinValue = NULL;
    pFxParamInfo->pFxParam->pvDefaultValue = NULL;

	switch(pFxParamInfo->pFxParam->ParamType)
	{
	case UINT8_TYPE:
		{
			CreateParam<Uint8>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Uint8>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Uint8>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
			if(pFxParam->pvDefaultValue){
				CreateParam<Uint8>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Uint8>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case INT8_TYPE:
		{
			CreateParam<Int8>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Int8>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Int8>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Int8>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Int8>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case UINT16_TYPE:
		{
			CreateParam<Uint16>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Uint16>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Uint16>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Uint16>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Uint16>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case INT16_TYPE:
		{
			CreateParam<Int16>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Int16>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Int16>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Int16>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Int16>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case UINT32_TYPE:
		{
			CreateParam<Uint32>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Uint32>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Uint32>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Uint32>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Uint32>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case INT32_TYPE:
		{
			CreateParam<Int32>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Int32>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Int32>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Int32>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Int32>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case INT64_TYPE:
		{
			CreateParam<Int64>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Int64>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Int64>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Int64>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Int64>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case UINT64_TYPE:
		{
			CreateParam<Uint64>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Uint64>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Uint64>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Uint64>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Uint64>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case FLOAT32_TYPE:
		{
			CreateParam<Float>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Float>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Float>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
			    CreateParam<Float>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Float>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case FLOAT64_TYPE:
		{
			CreateParam<Double>(&pFxParamInfo->pvFxParam, pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Double>(&pFxParamInfo->pFxParam->pvMaxValue, 1, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Double>(&pFxParamInfo->pFxParam->pvMinValue, 1, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Double>(&pFxParamInfo->pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Double>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, pFxParamInfo->pFxParam->dwParamNumber);
			}
		}
		break;
	case COMPLEX_TYPE:
		{
			pFxParamInfo->pvFxParam = (Float*) new Float[2*pFxParamInfo->pFxParam->dwParamNumber];
            memset(pFxParamInfo->pvFxParam, 0, sizeof(Float)*2*pFxParamInfo->pFxParam->dwParamNumber);
            if(pFxParam->pvMaxValue) CreateParam<Float>(&pFxParamInfo->pFxParam->pvMaxValue, 2, pFxParam->pvMaxValue);
            if(pFxParam->pvMinValue) CreateParam<Float>(&pFxParamInfo->pFxParam->pvMinValue, 2, pFxParam->pvMinValue);
            if(pFxParam->pvDefaultValue){
				CreateParam<Float>(&pFxParamInfo->pFxParam->pvDefaultValue, 2*pFxParamInfo->pFxParam->dwParamNumber, pFxParam->pvDefaultValue);
				SetParam<Float>(pFxParamInfo->pvFxParam, pFxParam->pvDefaultValue, 2*pFxParamInfo->pFxParam->dwParamNumber);
			}
            break;
		}
	default:
		return FX_ERROR;
	}

	_lParams.push_back(pFxParamInfo);

	_pLogTrace->FxTrace( "FxParam::AddFxParam OK (Name: %s, Unit: %s, Type: %s, Size: %d)",
				pFxParamInfo->pFxParam->strParamName.c_str(),
				pFxParamInfo->pFxParam->strParamUnitName.c_str(),
				(GetConstToString(UNITTYPE_CONST, (Int32)pFxParamInfo->pFxParam->ParamType)).c_str(),
				pFxParamInfo->pFxParam->dwParamNumber);

	return FX_OK;
}

Int32 CFxParam::AddFxParam(const PFX_PARAM_STRING pFxParam)
{
    AutoLock lock(_CS);

	_pLogTrace->FxTrace( "FxParam::AddFxParam entry");

    FX_PARAM_STRING_INFO* pFxParamInfo = NULL;
    if(FEF_SUCCEEDED(FindParamter(pFxParam->strParamName, &pFxParamInfo)))
    {
        _pLogTrace->FxTrace( "FxParam::AddFxParam FEF_FAILED !! (Fx Parameter already exists)");
		return FX_INVALPARAM;
    }

    pFxParamInfo = (FX_PARAM_STRING_INFO*)new FX_PARAM_STRING_INFO;
	if(pFxParamInfo == NULL)
		return FX_NOMEM;

    pFxParamInfo->pFxParam = NULL;
	pFxParamInfo->pFxParam = (FX_PARAM_STRING*)new FX_PARAM_STRING;
	if(pFxParamInfo->pFxParam == NULL)
		return FX_NOMEM;

    pFxParamInfo->pFxParam->strParamName = pFxParam->strParamName;
    pFxParamInfo->pFxParam->strDefaultValue = pFxParam->strDefaultValue;
    pFxParamInfo->strFxParam = pFxParam->strDefaultValue;

    _lParamsString.push_back(pFxParamInfo);

	_pLogTrace->FxTrace( "FxParam::AddFxParam OK (Name: %s)",pFxParamInfo->pFxParam->strParamName.c_str());

	return FX_OK;
}

Int32 CFxParam::RemoveFxParam(const std::string strParamName)
{
    AutoLock lock(_CS);

    /*! Find parameter */
    FX_PARAM_INFO* pFxParamInfo = NULL;
	if(FEF_SUCCEEDED(FindParamter(strParamName, &pFxParamInfo)))
    {
        if(pFxParamInfo)
        {
            SAFE_DELETE_OBJECT(pFxParamInfo->pFxParam->pvMaxValue);
            SAFE_DELETE_OBJECT(pFxParamInfo->pFxParam->pvMinValue);
            SAFE_DELETE_OBJECT(pFxParamInfo->pFxParam->pvDefaultValue);
            SAFE_DELETE_OBJECT(pFxParamInfo->pFxParam);
            SAFE_DELETE_ARRAY(pFxParamInfo->pvFxParam);
		    _lParams.remove(pFxParamInfo);
		    SAFE_DELETE_OBJECT(pFxParamInfo);
        }
        else
        {
		    _pLogTrace->FxTrace( "FxParam::RemoveFxParam FEF_FAILED !! (Invalid Parameter: strParamName)");
		    return FX_INVALPARAM;
	    }
    }
    FX_PARAM_STRING_INFO* pFxParamInfoString = NULL;
	if(FEF_SUCCEEDED(FindParamter(strParamName, &pFxParamInfoString)))
    {
        if(pFxParamInfoString)
        {
            SAFE_DELETE_OBJECT(pFxParamInfoString->pFxParam);
            _lParamsString.remove(pFxParamInfoString);
            SAFE_DELETE_OBJECT(pFxParamInfoString);
        }
        else
        {
		    _pLogTrace->FxTrace( "FxParam::RemoveFxParam FEF_FAILED !! (Invalid Parameter: strParamName)");
		    return FX_INVALPARAM;
	    }
    }
    
    if(!pFxParamInfoString && !pFxParamInfo)
    {
        _pLogTrace->FxTrace( "FxParam::RemoveFxParam FEF_FAILED !! (Invalid Parameter: strParamName)");
		return FX_INVALPARAM;
    }
    
    return FX_OK;
}

Int32 CFxParam::SetFxParamValue(const std::string strParamName, const Void*  pvParamValue)
{
	AutoLock lock(_CS);

	if(pvParamValue == NULL)
	{
		_pLogTrace->FxTrace( "FxParam::SetFxParamValue FEF_FAILED !! (Invalid Parameter: pvParamValue)");
		return FX_INVALPARAM;
	}

	FX_PARAM_INFO* pFxParamInfo = NULL;
	if(FEF_FAILED(FindParamter(strParamName, &pFxParamInfo)))
		return FX_INVALPARAM;

	switch(pFxParamInfo->pFxParam->ParamType)
	{
	case UINT8_TYPE:
		{
			SetParam<Uint8>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT8_TYPE:
		{
			SetParam<Int8>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case UINT16_TYPE:
		{
			SetParam<Uint16>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT16_TYPE:
		{
			SetParam<Int16>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case UINT32_TYPE:
		{
			SetParam<Uint32>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT32_TYPE:
		{
			SetParam<Int32>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case UINT64_TYPE:
		{
			SetParam<Uint64>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT64_TYPE:
		{
			SetParam<Int64>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case FLOAT32_TYPE:
		{
			SetParam<Float>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case FLOAT64_TYPE:
		{
			SetParam<Double>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case COMPLEX_TYPE:
		{
			memcpy(((Float*)(pFxParamInfo->pvFxParam)), ((Float*)(pvParamValue)), 2*pFxParamInfo->pFxParam->dwParamNumber*sizeof(Float));
		}
        break;
	default:
		return FX_ERROR;
	}

	return FX_OK;
}

Int32 CFxParam::SetFxParamValue(const std::string strParamName, const std::string strParamValue)
{
    AutoLock lock(_CS);

	FX_PARAM_STRING_INFO* pFxParamInfo = NULL;
	if(FEF_FAILED(FindParamter(strParamName, &pFxParamInfo)))
		return FX_INVALPARAM;

    pFxParamInfo->strFxParam = strParamValue;
    return FX_OK;
}

Int32 CFxParam::GetFxParamValue(const std::string strParamName, Void* pvParamValue)
{
	AutoLock lock(_CS);

	if(pvParamValue == NULL)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParamValue FEF_FAILED !! (Invalid Parameter: pvParamValue)");
		return FX_INVALPARAM;
	}

	FX_PARAM_INFO* pFxParamInfo = NULL;
	if(FEF_FAILED(FindParamter(strParamName, &pFxParamInfo)))
		return FX_INVALPARAM;

	switch(pFxParamInfo->pFxParam->ParamType)
	{
	case UINT8_TYPE:
		{
			GetParam<Uint8>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT8_TYPE:
		{
			GetParam<Int8>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case UINT16_TYPE:
		{
			GetParam<Uint16>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT16_TYPE:
		{
			GetParam<Int16>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case UINT32_TYPE:
		{
			GetParam<Uint32>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT32_TYPE:
		{
			GetParam<Int32>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case UINT64_TYPE:
		{
			GetParam<Uint64>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case INT64_TYPE:
		{
			GetParam<Int64>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case FLOAT32_TYPE:
		{
			GetParam<Float>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case FLOAT64_TYPE:
		{
			GetParam<Double>(pFxParamInfo->pvFxParam, pvParamValue, pFxParamInfo->pFxParam->dwParamNumber);
		}
		break;
	case COMPLEX_TYPE:
		{
			memcpy(((Float*)(pvParamValue)), ((Float*)(pFxParamInfo->pvFxParam)), 2*pFxParamInfo->pFxParam->dwParamNumber*sizeof(Float));
		}
        break;
	default:
		return FX_ERROR;
	}

	return FX_OK;
}

Int32 CFxParam::GetFxParamValue(const std::string strParamName, std::string& strParamValue)
{
    AutoLock lock(_CS);

	FX_PARAM_STRING_INFO* pFxParamInfo = NULL;
	if(FEF_FAILED(FindParamter(strParamName, &pFxParamInfo)))
		return FX_INVALPARAM;

    strParamValue = pFxParamInfo->strFxParam;

    return FX_OK;
}

Int32 CFxParam::GetFxParamSize(const std::string strParamName, Uint32* pdwParamNumber)
{
	AutoLock lock(_CS);

    if(strParamName.empty())
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParamSize FEF_FAILED !! (Invalid Parameter: strParamName)");
		return FX_INVALPARAM;
	}

	if(pdwParamNumber == NULL)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParamSize FEF_FAILED !! (Invalid Parameter: pdwParamNumber)");
		return FX_INVALPARAM;

	}
	FX_PARAM_INFO* pFxParamInfo = NULL;
	if(FEF_FAILED(FindParamter(strParamName, &pFxParamInfo)))
		return FX_ERROR;

	*pdwParamNumber = pFxParamInfo->pFxParam->dwParamNumber;

	return FX_OK;
}

Int32 CFxParam::GetFxParamCount(Uint16* pwParamCount)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxParam::GetFxParamCount entry");

	if(pwParamCount == NULL)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParamCount FEF_FAILED !! (Invalid Parameter: pwParamCount)");
		return FX_INVALPARAM;

	}

	*pwParamCount = (Uint16)_lParams.size();

	return FX_OK;
}

Int32 CFxParam::GetFxParamStringCount(Uint16* pwParamCount)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxParam::GetFxParamCount entry");

	if(pwParamCount == NULL)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParamCount FEF_FAILED !! (Invalid Parameter: pwParamCount)");
		return FX_INVALPARAM;

	}

	*pwParamCount = (Uint16)_lParamsString.size();

	return FX_OK;
}

Int32 CFxParam::GetFxParam(const FX_PARAM** ppFxParam, const Uint16 wFxParamIndex)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxParam::GetFxParam entry");

	if(ppFxParam == NULL)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParam FEF_FAILED !! (Invalid Parameter: ppFxParam)");
		return FX_INVALPARAM;
	}

	if(_lParams.empty() == TRUE)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParam Empty");
		*ppFxParam = NULL;
		return FX_OK;
	}

	if(wFxParamIndex > ((Uint16)_lParams.size() - 1))
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParam FEF_FAILED !! (Invalid Parameter: wFxParamIndex too big)");
		return FX_INVALPARAM;
	}

	/*! Copy address of Media Type */
	lFxParamIter  lParIter;
	Uint16 Idx;
	*ppFxParam = NULL;
	for (lParIter = _lParams.begin(), Idx=0; lParIter != _lParams.end(); lParIter++, Idx++)
	{
		if(Idx == wFxParamIndex)
		{
			*ppFxParam = (PFX_PARAM)((*lParIter)->pFxParam);

			break;
		}
	}

	return FX_OK;
}

Int32 CFxParam::GetFxParam(const FX_PARAM_STRING** ppFxParam, const Uint16 wFxParamIndex)
{
	AutoLock lock(_CS);
	_pLogTrace->FxTrace( "FxParam::GetFxParam entry");

	if(ppFxParam == NULL)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParam FEF_FAILED !! (Invalid Parameter: ppFxParam)");
		return FX_INVALPARAM;
	}

	if(_lParamsString.empty() == TRUE)
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParam Empty");
		*ppFxParam = NULL;
		return FX_OK;
	}

	if(wFxParamIndex > ((Uint16)_lParamsString.size() - 1))
	{
		_pLogTrace->FxTrace( "FxParam::GetFxParam FEF_FAILED !! (Invalid Parameter: wFxParamIndex too big)");
		return FX_INVALPARAM;
	}

	/*! Copy address of Media Type */
	lFxParamStringIter  lParIter;
	Uint16 Idx;
	*ppFxParam = NULL;
	for (lParIter = _lParamsString.begin(), Idx=0; lParIter != _lParamsString.end(); lParIter++, Idx++)
	{
		if(Idx == wFxParamIndex)
		{
			*ppFxParam = (PFX_PARAM_STRING)((*lParIter)->pFxParam);

			break;
		}
	}

	return FX_OK;
}

Int32 CFxParam::FindParamter(const std::string strParamName, FX_PARAM_INFO** ppFxParamInfo)
{
	lFxParamIter lpParIter;
	for (lpParIter = _lParams.begin(); lpParIter != _lParams.end(); lpParIter++)
	{
		if ( (*lpParIter)->pFxParam->strParamName == strParamName )
		{
			*ppFxParamInfo = (*lpParIter);
			return FX_OK;
		}
	}
	return FX_ERROR;
}

Int32 CFxParam::FindParamter(const std::string strParamName, FX_PARAM_STRING_INFO** ppFxParamInfo)
{
	lFxParamStringIter lpParIter;
	for (lpParIter = _lParamsString.begin(); lpParIter != _lParamsString.end(); lpParIter++)
	{
		if ( (*lpParIter)->pFxParam->strParamName == strParamName )
		{
			*ppFxParamInfo = (*lpParIter);
			return FX_OK;
		}
	}
	return FX_ERROR;
}

 } //namespace FEF
