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
#include <boost/thread/mutex.hpp>

namespace FEF {
#define BOOST_LIB_VERSION_1_35_0
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

/**
 * This class takes care of the annoying gruntwork
 * of having to zero-out and set the size parameter
 * of our Windows and DirectX structures.
 */
template <class T>
struct sAutoZero : public T
{
	sAutoZero()
	{
		memset( this, 0, sizeof(T) );
		((T*)this)->dwSize = sizeof(T);
	}
};

/**
 * The Right Way to release our COM interfaces.
 * If they're still valid, release them, then
 * invalidate them and null them.
 */
template <class T>
inline void SafeRelease( T& iface )
{
	if( iface )
	{
		iface->Release();
		iface = NULL;
	}
}

/*! Deprecated functions */
//#ifdef WIN32
//#define sprintf_s(a, b, c, d) sprintf_s(a, b, c, d)
//#else
//#define sprintf_s(a, b, c, d) sprintf_s(a, c, d)
//#endif

} //namespace FEF
