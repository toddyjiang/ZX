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
#ifndef _EDITOR_DEF_H
#define _EDITOR_DEF_H

#pragma warning(disable : 4786)

#include <list>
#include <map>
#include <vector>
#include <algorithm>

#include "FxDef.h"
using namespace FEF; //!< Using FxEngineFramework
#include <boost/thread/mutex.hpp>

using namespace std;
using namespace boost;

#ifndef BOOST_LIB_VERSION_1_35_0
typedef boost::mutex::scoped_lock AutoLock;
#else
class AutoLock : public boost::mutex::scoped_lock
{
public:
	AutoLock(boost::mutex& m, Bool ShouldLock):
    boost::mutex::scoped_lock(m, boost::defer_lock)
	{
		if(ShouldLock == TRUE)
			lock();
	}
	AutoLock(boost::mutex& m):
	boost::mutex::scoped_lock(m)
	{}
	~AutoLock()
	{}
};
#endif

/*----------------------------------------------------------------------*//*!
	General FxEngineEditor types
*//*-----------------------------------------------------------------------*/
//wxString to std::string
inline std::string ws2s(wxString as){
    return string(as.mb_str(wxConvISO8859_1));
}

//std::string to wxString
inline wxString s2ws(std::string s){
    return wxString((s).c_str(),wxConvISO8859_1);
}

//C string to wxString
inline wxString cs2ws(const char* s){
    return wxString(s,wxConvISO8859_1);
}
/*----------------------------------------------------------------------*//*!
	Defines, constants, and global variables
*//*-----------------------------------------------------------------------*/
#define SAFE_DELETE_ARRAY(p)		{if(p) {delete [] (p); (p)=NULL;}}
#define SAFE_DELETE_OBJECT(p)		{if(p) {delete (p); (p)=NULL;}}
#define DELETE_SINGLETON(p)			{delete (p);}
#define SAFE_DELETE(p)				{if(p) {delete (p);}}

#define PURE	= 0

#define SIZE (sizeof/sizeof(X[1]))

#define HWORKINGSIZE (1024*4)
#define VWORKINGSIZE (768*4)

#define MAX_HWORKINGSIZE (HWORKINGSIZE)
#define MAX_VWORKINGSIZE (VWORKINGSIZE)

#define MAX_WORKING_H(x) (x > MAX_HWORKINGSIZE) ? MAX_HWORKINGSIZE : x
#define MAX_WORKING_V(x) (x > MAX_VWORKINGSIZE) ? MAX_VWORKINGSIZE : x

#define BACKGROUND_R 101
#define BACKGROUND_G 148
#define BACKGROUND_V 201

#define VPIN_INTERVAL	(8)
#define X_ORIGINE		    100
#define Y_ORIGINE		    100
#define TEXT_SPACE		    4
#define PIN_FONT_HEIGHT		(12)
#define FONT_HEIGHT		    (14)
#define FONT_WIDTH		    (12)
#define PIN_SIZE		    (8)

#define BOXNAME_SIZE    (17)
#define MAX_ZOOM (2)
#define MIN_ZOOM (0.25)

#define NODE_REGION_SIZE	5

//! Types of point
typedef struct {
	int x;
	int y;
	bool hit;
} hitParams;


//! Mouse evnt
typedef enum _MOUSE_EVT{
	MOUSE_NONE = 0,
	MOUSE_LD,
	MOUSE_LU,
	MOUSE_LLD,
	MOUSE_M,
	MOUSE_RD,
	MOUSE_RU,
	MOUSE_RRD
}MOUSE_EVT;

//! Type of point
typedef enum _TYPE_POINT{
	NONE = 0,
	BEGIN_POINT,
	END_POINT,
}TYPE_POINT;

//! Type of point
typedef enum _TYPE_SAVE{
	SAVE = 0,
	SAVE_AS,
}TYPE_SAVE;
#endif
