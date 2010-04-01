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
//#pragma warning(disable : 4995)	//!< stdio.h
//#pragma warning(disable : 4996) //!< stdlib.h
//#pragma warning(disable : 4786)

#include <vector>
#include <map>
#include <strstream>
#include <iostream>
#include <fstream>
#include <string>

#include "FxDef.h"
using namespace std;

namespace FEF {

#define FX_TAG_OBJECT "<FxList>"
#define FX_TAG_OBJECT_END "</FxList>"
#define FX_TAG_OBJECT_CHILD "<Fx "
#define FX_TAG_OBJECT_CHILD_END "</Fx>"

#define FXPATH_TAG_ELEMENT "<FxPath "
#define FXPARAM_TAG_ELEMENT "<FxParam "
#define FXPARAM_TAG_ELEMENT_END "</FxParam>"
#define FXPARAMVALUE_TAG_ELEMENT "<Val "

class CXMLFx;

typedef struct _FX_XML_PARAM{
    std::string strName;
    FX_UNIT_TYPE UnitType;
	Uint32 dwNumber;
    Uint8* pbValue;
	std::string strValue;
}FX_XML_PARAM;

typedef map<Uint16, CXMLFx*>           MapFxIdToFxXML;
typedef MapFxIdToFxXML::iterator       MapFxIdToFxXMLItr;

typedef vector<FX_XML_PARAM*>           VectorOfFxParam;
typedef VectorOfFxParam::iterator       VectorOfFxParamItr;

class CXMLFxManager
{
public:
    CXMLFxManager(void);
public:
    virtual ~CXMLFxManager();

public:
    Int32 ReadObject(std::ifstream& fd, MapFxIdToFxXML& FxIdToFxXMLMap);
    Int32 WriteObject(std::ofstream& fd, MapFxIdToFxXML& FxIdToFxXMLMap);
};

class CXMLFx
{
public:
    CXMLFx(void);
    CXMLFx(std::ifstream& fd, std::string& strLine);
    virtual ~CXMLFx();

private:
    Uint16 _wId;
    std::string _strFxName;
	VectorOfFxParam _FxParamVector;

public:
	Uint16& GetFxId() {return _wId;}
    Void SetFxId(Uint16 wId) { _wId = wId; return; }
	std::string& GetFxName() {return _strFxName;}
    Void SetFxName(std::string strFxName) { _strFxName = strFxName; return; }
    VectorOfFxParam& GetVectorOfFxParam() {return _FxParamVector;}
};

} //namespace FEF
