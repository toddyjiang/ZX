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
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <boost/lexical_cast.hpp>

#include "XMLFx.h"
#include "FxUtils/FxUtils.h"

namespace FEF {

map<FX_UNIT_TYPE, int> g_ParamSize;

template < class T >
  struct check_string
  {
    static void from_string( const std::string & src, T & dest )
    {
		std::istringstream iss( src );
		iss >> dest;
    }
  };

  template < >
  struct check_string < std::string >
  {
    static void from_string( const std::string & src, std::string & dest )
    {
      dest = src;
    }
  };

  template < class T >
  void from_string( const std::string & src, T & dest )
  {
    check_string<T>::from_string( src, dest );
  }

template<typename T>
bool getValue(std::string& str, T &Dest )
{
    size_t pfrom, pto;
    pfrom = 0;
    pto   = 0;
    std::string  strTag = "\"";
    pfrom = str.find(strTag,pfrom);
    pto   = str.find(strTag  ,pfrom+1);

    std::string strResult;
    if (pfrom >= 0 && pto > 0 && pfrom!=pto)
    {
      strResult = str.substr(pfrom+strTag.size(),pto-pfrom-strTag.size());
      str = str.erase(pfrom,pto-pfrom+1);
      Dest = boost::lexical_cast<T>(strResult);
      return 0;
    }
    return 1;
}

template<typename T>
void writeValue(std::ofstream& fd, Uint8* pVal )
{
    fd << *((T*)pVal);
}

void writeValue(std::ofstream& fd, FX_UNIT_TYPE UnitType, Uint8 *pVal )
{
    Uint32 dwVal = 0;
    Int32 sdwVal = 0;
    switch(UnitType) {
      case UINT8_TYPE:
          dwVal = Uint32(*pVal);
          fd << "i=\""; writeValue<Uint32>(fd, (Uint8*)&dwVal);
          fd << "\"";
          break;
      case INT8_TYPE:
          sdwVal = Int32(*pVal);
          fd << "i=\""; writeValue<Int32>(fd, (Uint8*)&sdwVal);
          fd << "\"";
          break;
      case UINT16_TYPE:
          fd << "i=\""; writeValue<Uint16>(fd, pVal);
          fd << "\"";
          break;
      case INT16_TYPE:
          fd << "i=\""; writeValue<Int16>(fd, pVal);
          fd << "\"";
          break;
      case UINT32_TYPE:
          fd << "i=\""; writeValue<Uint32>(fd, pVal);
          fd << "\"";
          break;
      case INT32_TYPE:
          fd << "i=\""; writeValue<Int32>(fd, pVal);
          fd << "\"";
          break;
      case INT64_TYPE:
          fd << "i=\""; writeValue<Int64>(fd, pVal);
          fd << "\"";
          break;
      case UINT64_TYPE:
          fd << "i=\""; writeValue<Uint64>(fd, pVal);
          fd << "\"";
          break;
      case FLOAT32_TYPE:
          fd << "i=\""; writeValue<Float>(fd, pVal);
          fd << "\"";
          break;
      case FLOAT64_TYPE:
          fd << "i=\""; writeValue<Double>(fd, pVal);
          fd << "\"";
          break;
      case COMPLEX_TYPE:
          fd << "i=\""; writeValue<Float>(fd, pVal);
          fd << "\"" << " j=\""; writeValue<Float>(fd, pVal+4);
          fd << "\"";
          break;
      case NOT_DEFINED_TYPE:
          break;
    }

    return;
}

void readValue(std::string& str, FX_UNIT_TYPE UnitType, Uint8 *pbVal )
{
    Int16 swVal = 0;
    Uint16 wVal = 0;

    switch(UnitType) {
      case UINT8_TYPE:
          getValue(str, wVal); *pbVal = (Uint8)wVal;
          break;
      case INT8_TYPE:
          getValue(str, wVal); *((Int8*)pbVal) = (Int8)swVal;
          break;
      case UINT16_TYPE:
          getValue(str, wVal); *((Uint16*)pbVal) = wVal;
          break;
      case INT16_TYPE:
          getValue(str, swVal); *((Int16*)pbVal) = swVal;
          break;
      case UINT32_TYPE:
          Uint32 dwVal;
          getValue(str, dwVal); *((Uint32*)pbVal) = dwVal;
          break;
      case INT32_TYPE:
          Int32 sdwVal;
          getValue(str, sdwVal); *((Int32*)pbVal) = sdwVal;
          break;
      case INT64_TYPE:
          Int64 sqVal;
          getValue(str, sqVal); *((Int64*)pbVal) = sqVal;
          break;
      case UINT64_TYPE:
          Uint64 qVal;
          getValue(str, qVal); *((Uint64*)pbVal) = qVal;
          break;
      case FLOAT32_TYPE: {
          Float fVal;
          getValue(str, fVal); *((Float*)pbVal) = fVal;
                         }
          break;
      case FLOAT64_TYPE:
          Double dVal;
          getValue(str, dVal); *((Double*)pbVal) = dVal;
          break;
      case COMPLEX_TYPE: {
          Float fVal;
          getValue(str, fVal); *((Float*)pbVal) = fVal;
          getValue(str, fVal); *((Float*)(pbVal+4)) = fVal;
                         }
          break;
      case NOT_DEFINED_TYPE:
          break;
    }
}

CXMLFxManager::CXMLFxManager(void)
{
    /*! Initialize parameter size */
    g_ParamSize[NOT_DEFINED_TYPE] = 0;
    g_ParamSize[UINT8_TYPE] = 1;
    g_ParamSize[INT8_TYPE] = 1;
    g_ParamSize[UINT16_TYPE] = 2;
    g_ParamSize[INT16_TYPE] = 2;
    g_ParamSize[UINT32_TYPE] = 4;
    g_ParamSize[INT32_TYPE] = 4;
    g_ParamSize[INT64_TYPE] = 8;
    g_ParamSize[UINT64_TYPE] = 8;
    g_ParamSize[FLOAT32_TYPE] = 4;
    g_ParamSize[FLOAT64_TYPE] = 8;
    g_ParamSize[COMPLEX_TYPE] = 8;
}

CXMLFxManager::~CXMLFxManager()
{
}

Int32 CXMLFxManager::ReadObject(std::ifstream& fd, MapFxIdToFxXML& FxIdToFxXMLMap)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    /*! Get line of xml and create the FEF objects */
    std::string strLine = "";

    while (getline(fd, strLine))
	{
        if(strLine.find(FX_TAG_OBJECT_CHILD)  < strLine.size())
        {
			CXMLFx* pFx = new CXMLFx(fd, strLine);
			FxIdToFxXMLMap.insert(make_pair(pFx->GetFxId(), pFx));
        }
        if(strLine.find(FX_TAG_OBJECT_END) < strLine.size())
            return FX_OK;
    }
    return FX_OK;
}

Int32 CXMLFxManager::WriteObject(std::ofstream& fd, MapFxIdToFxXML& FxIdToFxXMLMap)
{
    if(fd.is_open() == FALSE)
        return FX_ERROR;

    MapFxIdToFxXMLItr Itr;
    VectorOfFxParamItr ItrVec;
    for(Itr = FxIdToFxXMLMap.begin(); Itr != FxIdToFxXMLMap.end(); Itr++)
    {
        fd << FX_TAG_OBJECT_CHILD << "id=\"" << (*Itr).first << "\">" << endl;
        {
            fd << FXPATH_TAG_ELEMENT << "name=\"" << (*Itr).second->GetFxName() << "\"/>" << endl;

            VectorOfFxParam FxParam = (*Itr).second->GetVectorOfFxParam();
            for(ItrVec = FxParam.begin(); ItrVec != FxParam.end(); ItrVec++)
            {
				if((*ItrVec)->UnitType != NOT_DEFINED_TYPE) { 
					fd << FXPARAM_TAG_ELEMENT <<
						"name=\"" << (*ItrVec)->strName << "\"" <<
						" type=\"" << (*ItrVec)->UnitType << "\"" <<
						" number=\"" << (*ItrVec)->dwNumber << "\">" << endl;
					for(Uint32 i = 0; i < (*ItrVec)->dwNumber; i++)
					{
						fd << FXPARAMVALUE_TAG_ELEMENT << "id=\"" << i << "\" ";
						writeValue(fd, (*ItrVec)->UnitType, &(*ItrVec)->pbValue[i*g_ParamSize[(*ItrVec)->UnitType]]);
						fd << "/>" << endl;
					}
					fd << FXPARAM_TAG_ELEMENT_END << endl;
				}
				else if((*ItrVec)->strValue != "")
				{
					fd << FXPARAM_TAG_ELEMENT <<
						"name=\"" << (*ItrVec)->strName << "\"" << 
						" type=\"" << (*ItrVec)->UnitType << "\">" << endl;
						fd << FXPARAMVALUE_TAG_ELEMENT << "id=\"" << "0" << "\" " << 
						"i=\"" << (*ItrVec)->strValue << "\"" <<
						"/>" << endl;
					fd << FXPARAM_TAG_ELEMENT_END << endl;
				}
                
            }
        }
        fd << FX_TAG_OBJECT_CHILD_END << endl;
    }
    return FX_OK;
 }

CXMLFx::CXMLFx(void)
{
}

CXMLFx::CXMLFx(std::ifstream& fd, std::string& strLine) :
_wId(0),
_strFxName("")
{
    getValue(strLine, _wId);
	if(fd.is_open() == TRUE)
	{
        FX_XML_PARAM* pFxParam;
        std::string strFxParam;
        Uint32 dwNull;
        Uint32 dwParamNb = 0;
        Uint32 dwParamType = 0;
       do{
			if(strLine.find(FXPATH_TAG_ELEMENT)  < strLine.size())
			{
	            getValue(strLine, _strFxName);
			}
			if(strLine.find(FXPARAM_TAG_ELEMENT)  < strLine.size())
			{
                dwParamNb = 0;
                pFxParam = new FX_XML_PARAM;
                pFxParam->UnitType = NOT_DEFINED_TYPE;
                pFxParam->pbValue = NULL;
                pFxParam->dwNumber = 0;
                getValue(strLine, strFxParam);
                
                pFxParam->strName = strFxParam;

                getValue(strLine, dwParamType); pFxParam->UnitType = (FX_UNIT_TYPE)dwParamType;
				if(pFxParam->UnitType != NOT_DEFINED_TYPE) {
					getValue(strLine, pFxParam->dwNumber);
					pFxParam->pbValue = new Uint8[g_ParamSize[pFxParam->UnitType] * pFxParam->dwNumber];
				}
			}
            if(strLine.find(FXPARAMVALUE_TAG_ELEMENT)  < strLine.size())
            {
                if(pFxParam != NULL) {
                    getValue(strLine, dwNull); //!< Get Val id;
					if(pFxParam->UnitType != NOT_DEFINED_TYPE) 
						readValue(strLine, pFxParam->UnitType, &pFxParam->pbValue[dwParamNb*g_ParamSize[pFxParam->UnitType]] );
					else
						getValue(strLine, pFxParam->strValue);
					dwParamNb++;
                }
            }
            if(strLine.find(FXPARAM_TAG_ELEMENT_END)  < strLine.size())
            {
                _FxParamVector.push_back(pFxParam);
            }

			if(strLine.find(FX_TAG_OBJECT_CHILD_END) < strLine.size())
				break;
		} while(getline(fd, strLine));
	}
}

CXMLFx::~CXMLFx()
{
    VectorOfFxParamItr ItrFxParam;
    ItrFxParam = _FxParamVector.begin();
    while(ItrFxParam != _FxParamVector.end())
    {
        SAFE_DELETE_ARRAY((*ItrFxParam)->pbValue);
        SAFE_DELETE_OBJECT((*ItrFxParam));
        ItrFxParam++;
    }
}

 } //namespace FEF
