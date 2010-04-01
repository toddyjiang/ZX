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
//#pragma warning(disable:4786)

#include <string>
#include <iostream>

#include <boost/thread/thread.hpp>

using namespace std ;

#include "FxDef.h"

#include <map>

namespace FEF {

typedef std::map<Uint32, std::string> FxConst2StringMap;
typedef FxConst2StringMap::value_type vtTString;
typedef FxConst2StringMap::iterator FxConst2StringMapIter;
#define FX_CONST2STRING_INSERT( map, a ) (map).insert( vtTString((Uint32)a, (#a)))

class CFxConst2String {
typedef std::map<Uint32, FxConst2StringMap*> FxConst2Map;
public:
	CFxConst2String(void);
	virtual ~CFxConst2String();

public:
	string GetConstToString(FXENGINE_CONST_TYPE FxEngineConstType, Int32 sdwFxEngineConst);

private:
	FxConst2Map		  _StringofFx;
	FxConst2StringMap _StringOfFxState;
	FxConst2StringMap _StringOfFxType;
    FxConst2StringMap _StringOfFxScope;
	FxConst2StringMap _StringOfFxUnitType;
	FxConst2StringMap _StringOfFxError;
	FxConst2StringMap _StringOfFxMainMediaType;
	FxConst2StringMap _StringOfFxSubMediaType;
	FxConst2StringMap _StringOfFxPinType;

private:
	boost::mutex _CS;
};
} //namespace FEF
