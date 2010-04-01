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
#include "FxUtils/FxUtils.h"
#include "FxMedia/FxMedia.h"

//#ifdef  _MSC_VER
//#pragma pack(push,8)
//#else
//#pragma pack(8)
//#endif

#include <queue>
#include <list>

#include <boost/utility.hpp>
#include <boost/thread/condition.hpp> 
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

namespace FEF {

#define MEDIA_SIZE	0x2800	//<! 10Ko
#define MEDIA_NUMBER 0x14	//<! 20

class CFxMediaPool;

typedef std::queue<IFxMedia*> IFxMediaQ;
typedef std::list<IFxMedia*> IFxMediaL;

class CFxMediaPool
{
// Implementation
public:
	CFxMediaPool(Uint32 dwSize = MEDIA_SIZE, Uint32 dwNumber = MEDIA_NUMBER);
	~CFxMediaPool();

    Int32 UpdateMedia(Uint32 dwSize, Uint32 dwNumber);
	Int32 GetFreeMediaNumber(Uint32* pdwFreeMediaNumber);
	Int32 GetDeliveryMedia(IFxMedia** ppCFxMedia, Uint32 dwTimeOut);
    Int32 StopWaiting();
	Int32 ReleaseMedia(IFxMedia* pCFxMedia);
	Int32 ReleaseAllMedia();
	
private:
	//! Media pool
    IFxMediaL	_FreeFxMediaL;
	IFxMediaL   _UsedFxMediaL;
			
private:
	Uint32 _dwSize;
	Uint32 _dwNumber;

private:
	//! Critical Section
	boost::mutex _CS;	
	
	boost::condition MediaPool_not_empty;
};

} //namespace FEF



