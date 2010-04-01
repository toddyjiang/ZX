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

#include <vector>
#include <boost/utility.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using namespace std;

#include "FxDef.h"

namespace FEF {

#ifdef WIN32
#define FX_TRACE_DIR        ".\\"
#else
#define FX_TRACE_DIR        "./"
#endif

extern std::string GetTemporaryDir();
extern Void FX_TRACE(const Char* strFile, const Char *Pm_format, ...);
extern Void FX_DUMP(const Char* strFile, Uint8* pData, Uint32 dwDataSize);

//! Pin states
typedef enum _FX_LOG {
	LOG_IS_TRACE = 0,	//!< Log is a standard trace
	LOG_IS_DUMP,		//!< Log is a data dump
} FX_LOG;

class CFxLog : private boost::noncopyable
{

public:
    CFxLog(std::string &strLogPath, FX_LOG LogType);
	CFxLog(Char* strLogPath, FX_LOG LogType);
	~CFxLog();

public:
	void FxTrace(const Char *Pm_format, ...);
	Void FxDump(Uint8* pData, Uint32 dwDataSize);
	unsigned long GetObjectNumberInQueue() const {return _dwObjectNumber;}
	void AddAPCObject(void* pObject);
	void* GetAPCObject(Bool ShouldLock = TRUE);
	Bool ShouldTrace() {return _ShouldTrace;}

public:
    unsigned long _dwObjectNumber;
    std::vector<void*> _APCBuff;
	boost::condition APCBuff_not_empty;
	boost::mutex _CSThread;
    boost::mutex _CS;

	bool _ShouldQuit;

	FX_LOG		   _LogType;
	Bool		   _ShouldTrace;

private:
	boost::thread* _pthreadAPC;

public:
	std::string    _strLogPath;

};

 } //namespace FEF
