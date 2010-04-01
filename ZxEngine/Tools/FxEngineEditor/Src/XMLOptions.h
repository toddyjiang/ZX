/* __________________________________________________________________________
	
	FxEngine Framework. 
	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.

	This file is part of FxEngine Editor.
    FxEngine Editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FxEngine Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FxEngine Editor. If not, see <http://www.gnu.org/licenses/>.

   ____________________________________________http://www.SMProcess.com______ */
#ifndef _XML_OPTIONS_H
#define _XML_OPTIONS_H

#pragma warning(disable : 4995)	//!< stdio.h
#pragma warning(disable : 4996) //!< stdlib.h
#pragma warning(disable : 4786)

#include <vector>
#include <map>
#include <strstream>
#include <iostream>
#include <fstream>
#include <string>

#include "FxDef.h"
using namespace FEF; //!< Using FxEngineFramework

#define OPTION_TAG_OBJECT "<EditorOptions>"
#define OPTION_TAG_OBJECT_END "</EditorOptions>"

#define OPTION_TAG_ELEMENT_LOCK "<LockOnStart "

using namespace std;

class CXMLOptionsManager
{
public:
    CXMLOptionsManager();
public:
    virtual ~CXMLOptionsManager();

public:
    Int32 ReadObject(std::ifstream& fd, Bool& isLockOnStart);
    Int32 WriteObject(std::ofstream& fd, Bool& isLockOnStart);
};
#endif

