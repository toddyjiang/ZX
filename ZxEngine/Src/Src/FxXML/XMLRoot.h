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

#include "FxDef.h"
#include "XMLFx.h"
#include "XMLConnection.h"

using namespace std;

namespace FEF {

class CXMLRoot
{
public:
    CXMLRoot(const std::string strFilePath);
    Uint16 GetMajor() {return _wMajor;}
    Uint16 GetMinor() {return _wMinor;}
private:
    Uint16 _wMajor;
    Uint16 _wMinor;
    std::string _strFilePath;
public:
    virtual ~CXMLRoot();

    Int32 ReadXML(MapFxIdToFxXML& FxIdToFxXMLMap, MapIdToConnectionXML& IdToConnectionXMLMap);
    Int32 WriteXML(MapFxIdToFxXML& FxIdToFxXMLMap, MapIdToConnectionXML& IdToConnectionXMLMap);
};

 } //namespace FEF
