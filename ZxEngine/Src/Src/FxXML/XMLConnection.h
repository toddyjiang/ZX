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
//#pragma warning(disable : 4995) //!< stdio.h
//#pragma warning(disable : 4996) //!< stdlib.h
//#pragma warning(disable : 4786)

#include <vector>
#include <map>
#include <strstream>
#include <iostream>
#include <fstream>
#include <string>

#include "FxDef.h"

namespace FEF {

#define CONNECTION_TAG_OBJECT "<ConnectionList>"
#define CONNECTION_TAG_OBJECT_END "</ConnectionList>"
#define CONNECTION_TAG_OBJECT_CHILD "<Connection "
#define CONNECTION_TAG_OBJECT_CHILD_END "</Connection>"

#define FX_TAG_ELEMENT "<Fx "
#define MEDIA_TAG_ELEMENT "<Media "
#define FXLINKPOINT_TAG_ELEMENT "<FxLinkPoint "

using namespace std;

class CXMLConnection;

typedef struct _FXLINKPOINT{
	Uint32 dwX;
	Uint32 dwY;
}FXLINKPOINT;

typedef map<Uint16, CXMLConnection*>           MapIdToConnectionXML;
typedef MapIdToConnectionXML::iterator       MapIdToConnectionXMLItr;

typedef vector<FXLINKPOINT*>           VectorOfFxLinkPoint;
typedef VectorOfFxLinkPoint::iterator       VectorOfFxLinkPointItr;

class CXMLConnectionManager
{
public:
    CXMLConnectionManager(void);
public:
    virtual ~CXMLConnectionManager();

public:
    Int32 ReadObject(std::ifstream& fd, MapIdToConnectionXML& IdToConnectionXMLMap);
    Int32 WriteObject(std::ofstream& fd, MapIdToConnectionXML& IdToConnectionXMLMap);
};

class CXMLConnection
{
public:
    CXMLConnection(void);
    CXMLConnection(std::ifstream& fd, std::string& strLine);

private:
    Uint16 _wId;
    Uint16 _wFxId[2];
	Uint16 _wFxPinId[2];
	FX_MAIN_MEDIA_TYPE _MainMedia;
	FX_SUB_MEDIA_TYPE _SubMedia;
    VectorOfFxLinkPoint _FxLinkPointVector;

public:
	Uint16& GetId() {return _wId;}
    Void SetId(Uint16 wId) { _wId = wId; return; }
	Uint16& GetFxId(Uint32 Idx) {return _wFxId[Idx];}
    Void SetFxId(Uint16 wId, Uint32 Idx) { _wFxId[Idx] = wId; return; }
	Uint16& GetFxPinId(Uint32 Idx) {return _wFxPinId[Idx];}
    Void SetFxPinId(Uint16 wId, Uint32 Idx) { _wFxPinId[Idx] = wId; return; }
	FX_MAIN_MEDIA_TYPE& GetMainMedia() {return _MainMedia;}
    Void SetMainMedia(FX_MAIN_MEDIA_TYPE MainMedia) { _MainMedia = MainMedia; return; }
	FX_SUB_MEDIA_TYPE& GetSubMedia() {return _SubMedia;}
    Void SetSubMedia(FX_SUB_MEDIA_TYPE SubMedia) { _SubMedia = SubMedia; return; }
    VectorOfFxLinkPoint& GetVectorOfFxLinkPoint() {return _FxLinkPointVector;}

};

} //namespace FEF
