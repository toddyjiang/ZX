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
#include "IFxRefClock.h"

#include <boost/thread/thread.hpp>

#include "FxTrace/FxTrace.h"

using namespace std ;

namespace FEF {

class CFxRefClockObserver {
public:
	virtual ~CFxRefClockObserver(){};
	virtual Int32 GetFxRefClock(Uint64* pqRefClock, Uint32 dwId ) PURE;
	virtual Int32 SetFxRefClock(Uint64 qRefClock, Uint32 dwId ) PURE;
	virtual Int32 GetFxEngineRefClock(Uint64* pqRefClock) PURE;
	virtual Int32 SetFxRefClockError(FX_PTR qUserParam) PURE;
protected:
	CFxRefClockObserver(){};
};

typedef struct _FX_REFCLOCKOBSERVER {
	CFxRefClockObserver*	pObserver;
	FX_PTR			   qUserParam;
} FX_REFCLOCKOBSERVER, *PFX_REFCLOCKOBSERVER;

class CFxRefClock : public IFxRefClock {
public:
	CFxRefClock(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance);
	virtual ~CFxRefClock();

public:
	virtual Int32 GetFxEngineRefClock(Uint64* pqRefClock);
	virtual Int32 GetFxRefClock(Uint64* pqRefClock, Uint32 dwId);
	virtual Int32 SetFxRefClock(Uint64 qRefClock, Uint32 dwId);

public:
	virtual Int32 AttachObserver(CFxRefClockObserver* pFxRefClockObserver, FX_PTR qUserParam);

private:
	FX_REFCLOCKOBSERVER _FxRefClockObserver;

private:
	boost::mutex _CS;

private:
	std::string			_strFxName;

private:
	std::string _FxLogFile;
	CFxLog* _pLogTrace;

	// Instance value to manage the right log messages.
	Uint32			_dwThisInstance;		// current instance
};

} //namespace FEF
