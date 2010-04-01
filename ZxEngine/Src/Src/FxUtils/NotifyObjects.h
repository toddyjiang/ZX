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

   ____________________________________________http://www.SMProcess.com______ 
*//*!
   @file     NotifyObjects.h
   @brief This is the main interface file for the Notify Objects.

   This file contains all declarations of the Notify Objects.
   The Notify objects are used to dispatch parameters
   into a static function.

*//*_______________________________________________________________________*/
#ifndef __NOTIFYOBJECTS_H__
#define __NOTIFYOBJECTS_H__

#ifndef WIN32
#include <cstring>
#endif

namespace FEF {

#ifdef WIN32
#define FX_LONPTR unsigned __int64
#else
#define FX_LONPTR unsigned long long
#endif

//! Notify object is a parameter
class CNotifyObjects
{
public:
	static CNotifyObjects* Create( FX_PTR qInstance, FX_PTR qParam1 = NULL, FX_PTR qParam2 = NULL)
	{
		return new CNotifyObjects( qInstance, qParam1, qParam2);
	}

	FEF::Void Release() {delete this;}

	FX_PTR _qInstance;
	FX_PTR _qParam1;
	FX_PTR _qParam2;

private:
	//!Use Create instead
	CNotifyObjects( FX_PTR qInstance, FX_PTR qParam1 = NULL, FX_PTR qParam2 = NULL)
		: _qInstance( qInstance), _qParam1( qParam1), _qParam2( qParam2)
	{}
	//!Use Destroy instead
	~CNotifyObjects()
	{}
};

//! Notify object is a function
class CNotifyFunction
{
public:
	static CNotifyFunction* Create( FX_PTR dwFunctType, FX_PTR dwParam1, FX_LONPTR dwParam2, FX_LONPTR dwParam3)
	{
		return new CNotifyFunction( dwFunctType, dwParam1, dwParam2, dwParam3);
	}

	FEF::Void Release() {delete this;}

	FX_PTR _dwFunctType;
	FX_PTR _dwParam1;
	FX_LONPTR _dwParam2;
	FX_LONPTR _dwParam3;

private:
	//!Use Create instead
	CNotifyFunction( FX_PTR dwFunctType, FX_PTR dwParam1, FX_LONPTR dwParam2, FX_LONPTR dwParam3 )
		: _dwFunctType( dwFunctType), _dwParam1( dwParam1), _dwParam2( dwParam2), _dwParam3( dwParam3)
	{}
	//!Use Destroy instead
	~CNotifyFunction()
	{}
};

//! Notify object is a log message
class CNotifyLogTrace
{
public:
	static CNotifyLogTrace* Create(std::string& str)
	{
		return new CNotifyLogTrace( str );
	}

	FEF::Void Release() {delete this;}

	std::string _str;

private:
	//!Use Create instead
	CNotifyLogTrace( std::string& str )
		: _str(str)
	{}
	//!Use Destroy instead
	~CNotifyLogTrace()
	{}
};

//! Notify object is a media data
class CNotifyLogDump
{
public:
	static CNotifyLogDump* Create(Uint8* pData, Uint32& dwDataSize)
	{
		return new CNotifyLogDump( pData, dwDataSize );
	}

	FEF::Void Release() {delete this;}

	Uint8* _pData;
	Uint32 _dwDataSize;

private:
	//!Use Create instead
	CNotifyLogDump( Uint8* pData, Uint32& dwDataSize )
		: _pData(NULL), _dwDataSize( dwDataSize)
	{
		_pData = new Uint8[_dwDataSize];
		if(_pData)
			memcpy(_pData, pData, _dwDataSize*sizeof(Uint8));
	}
	//!Use Destroy instead
	~CNotifyLogDump()
	{
		if(_pData)
			delete [] _pData;
	}
};

} //namespace FEF


#endif // __NOTIFYOBJECT_H__
