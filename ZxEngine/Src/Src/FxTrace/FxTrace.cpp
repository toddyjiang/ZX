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
#include "FxUtils/FxUtils.h"
#include "FxTrace/FxTrace.h"
#include "FxUtils/NotifyObjects.h"
//#pragma warning(disable : 4995)	//!< stdio.h
//#pragma warning(disable : 4996) //!< stdlib.h

#include <time.h>
#include <sys/timeb.h>

#include <stdio.h>
#include <stdarg.h>

namespace FEF {

//
// FX_TRACE
//
Void FX_TRACE(const Char* strFile, const Char *strFormat, ...)
{
    va_list vaAP;

    FILE* fd = NULL;

    va_start(vaAP, strFormat);

    if (!strFile) return;

    // Open log file
#ifdef WIN32
    errno_t hr;
    hr = fopen_s(&fd, strFile, "at");
    if (hr != 0) return;
#else
	fd = fopen(strFile, "at");
	if(fd == NULL) return;
#endif

#ifdef WIN32
    struct tm time;
	struct _timeb timebuffer;
#else
    struct tm* time;
    struct timeb timebuffer;
#endif

	/*! Get current time */
#ifdef WIN32
	_ftime64_s( &timebuffer );
	/*! Convert time to struct tm form*/
	 _localtime64_s(&time, &timebuffer.time );
#else
	ftime( &timebuffer );
	/*! Convert time to struct tm form*/
	time = localtime(&timebuffer.time );
#endif
#ifdef WIN32
	fprintf(fd,"%02d/%02d %02d:%02d:%02d:%03d - ",
		time.tm_mday,
		time.tm_mon + 1,
		time.tm_hour,
		time.tm_min,
		time.tm_sec,
		timebuffer.millitm);
#else
	fprintf(fd, "%02d/%02d %02d:%02d:%02d:%03d - ",
		time->tm_mday,
		time->tm_mon + 1,
		time->tm_hour,
		time->tm_min,
		time->tm_sec,
		timebuffer.millitm);
#endif

    vfprintf(fd, strFormat, vaAP);
    fprintf(fd, "\n");

    va_end(vaAP);

    // Close log file
    fclose(fd);
}

Void FX_DUMP(const Char* strFile, Uint8* pData, Uint32 dwDataSize)
{
	if (!strFile) return;

    // Open log file
	FILE* fd = NULL;

#ifdef WIN32
    errno_t hr;
    hr = fopen_s(&fd, strFile, "ab");
    if (hr != 0) return;
#else
	fd = fopen(strFile, "ab");
	if (fd == NULL) return;
#endif


	fwrite(pData, sizeof(Uint8), dwDataSize, fd);

	// Close log file
    fclose(fd);
}

std::string GetTemporaryDir()
{
    char    strTmpPath[MAX_PATH];
    std::string strTemporaryDir = FX_TRACE_DIR;
    char* pstrTmp;
#ifdef WIN32
    if(0 != GetTempPath (MAX_PATH, strTmpPath))
	    strTemporaryDir = strTmpPath;
    else
#endif
    {
        pstrTmp = getenv("TMPDIR");
        if (pstrTmp == NULL)
        {
            pstrTmp = getenv("TMP");
            if (pstrTmp == NULL)
            {
                pstrTmp = getenv("TEMP");
                if (pstrTmp == NULL){
					if (access("/tmp", 0) == 0)
						strTemporaryDir = "/tmp/";
				}
				else
					strTemporaryDir = pstrTmp;
            }
            else
                strTemporaryDir = pstrTmp;
        }
        else
            strTemporaryDir = pstrTmp;
    }
    return strTemporaryDir;
}

void threadAPC(CFxLog* p) {
	AutoLock lkThread(p->_CSThread);
	FILE* fd = NULL;
	void* pObject;
	do {
		AutoLock lk(p->_CS);
		while (p->_dwObjectNumber == 0)
            p->APCBuff_not_empty.wait(lk);
		if((p->_ShouldQuit == true) && (p->_APCBuff.front() == NULL)) {
			p->_APCBuff.erase(p->_APCBuff.begin());
            --p->_dwObjectNumber;
			break;
		}

		pObject = p->_APCBuff.front();
		p->_APCBuff.erase(p->_APCBuff.begin());
		--p->_dwObjectNumber;
		lk.unlock();
		if ((p->_ShouldTrace != FALSE) && (pObject != NULL))
		{
			if(p->_LogType == LOG_IS_TRACE)
			{
				CNotifyLogTrace* pLog = (CNotifyLogTrace*)pObject;
#ifdef WIN32
                errno_t hr;
                hr = fopen_s(&fd, p->_strLogPath.c_str(), "at");
                if(hr == 0)
#else
				fd = fopen(p->_strLogPath.c_str(), "at");
				if(fd != NULL)
#endif

				{
					fprintf(fd, pLog->_str.c_str());
					fprintf(fd, "\n");
					fclose(fd);
				}
				pLog->Release();
			}
			else	//!< LOG_IS_DUMP
			{
				CNotifyLogDump* pLog = (CNotifyLogDump*)pObject;
#ifdef WIN32
                errno_t hr;
                hr = fopen_s(&fd, p->_strLogPath.c_str(), "ab");
                if(hr == 0)
#else
				fd = fopen(p->_strLogPath.c_str(), "ab");
				if(fd != NULL)
#endif

				{
					if(pLog->_pData)
						fwrite(pLog->_pData, sizeof(Uint8), pLog->_dwDataSize, fd);
					fclose(fd);
				}
				pLog->Release();
			}
		}
    } while (1); // -1 indicates end of buffer

	return;
}

CFxLog::CFxLog(std::string &strLogPath, FX_LOG LogType) :
_strLogPath(strLogPath),
_LogType(LogType),
_dwObjectNumber(0),
_ShouldQuit(false),
_pthreadAPC(NULL),
_ShouldTrace(FALSE)
{
	/*! Valid log file */
	FILE* fd = NULL;

#ifdef WIN32
    errno_t hr;
    hr = fopen_s(&fd, _strLogPath.c_str(), "at");
    if(hr == 0)
#else
    fd = fopen(_strLogPath.c_str(), "at");
    if(fd != NULL)
#endif
	{
		_ShouldTrace = TRUE;
		fclose(fd);
	}
	if(_ShouldTrace)
		_pthreadAPC = new boost::thread(boost::bind(&threadAPC, this));
}

CFxLog::CFxLog(Char* strLogPath, FX_LOG LogType) :
_strLogPath(""),
_LogType(LogType),
_dwObjectNumber(0),
_ShouldQuit(false),
_pthreadAPC(NULL),
_ShouldTrace(FALSE)
{
	/*! Valid log file */
	if(strLogPath == NULL)
		_ShouldTrace = FALSE;
	else{
		_strLogPath = strLogPath;
		FILE* fd = NULL;

#ifdef WIN32
        errno_t hr;
        hr = fopen_s(&fd, _strLogPath.c_str(), "at");
        if(hr == 0)
#else
		fd = fopen(_strLogPath.c_str(), "at");
		if(fd != NULL)
#endif
		{
			_ShouldTrace = TRUE;
			fclose(fd);
		}
	}

    if(_ShouldTrace)
		_pthreadAPC = new boost::thread(boost::bind(&threadAPC, this));
}

CFxLog::~CFxLog()
{
	/*! Quit thread */
	_ShouldQuit = true;
	AddAPCObject(NULL);
    if(_pthreadAPC)
	    _pthreadAPC->join();
	AutoLock lkThread(_CSThread);
	if(_pthreadAPC)
		delete (_pthreadAPC);

	/*! Empty APCqueue */
	AutoLock lk(_CS);
	Void* pObject = NULL;
	while( (pObject = GetAPCObject(FALSE)) != NULL )
	{
		if(_LogType == LOG_IS_TRACE)
		{
			CNotifyLogTrace* pLog = (CNotifyLogTrace*)pObject;
			pLog->Release();
		}
		else	//!< LOG_IS_DUMP
		{
			CNotifyLogDump* pLog = (CNotifyLogDump*)pObject;
			pLog->Release();
		}
	}

	_APCBuff.clear();

	lk.unlock();

	lkThread.unlock();
}

void CFxLog::AddAPCObject(void* pObject)
{
	AutoLock lk(_CS);
	_APCBuff.push_back(pObject);
	++_dwObjectNumber;
	lk.unlock();
	APCBuff_not_empty.notify_one();

	return;
}

void* CFxLog::GetAPCObject(Bool ShouldLock)
{
	void* pObject;

    if(ShouldLock == TRUE)
        AutoLock lk(_CS);
	if(_APCBuff.empty() == TRUE)
		return NULL;
	pObject = _APCBuff.front();
	_APCBuff.erase(_APCBuff.begin());
	--_dwObjectNumber;

	return pObject;
}

void CFxLog::FxDump(Uint8* pData, Uint32 dwDataSize)
{
	if(_ShouldTrace == FALSE)
		return;

	AutoLock lk(_CS);

	CNotifyLogDump* pLog = CNotifyLogDump::Create(pData, dwDataSize);

	_APCBuff.push_back(pLog);
	++_dwObjectNumber;
	APCBuff_not_empty.notify_one();

	return;
}

void CFxLog::FxTrace(const Char *strFormat, ...)
{
	if(_ShouldTrace == FALSE)
		return;

	AutoLock lk(_CS);

	va_list vaAP;

	char tmpbuf[MAX_PATH];
    std::string str = "";

#ifdef WIN32
    struct tm time;
	struct _timeb timebuffer;
#else
    struct tm* time;
    struct timeb timebuffer;
#endif


	/*! Get current time */
#ifdef WIN32
	_ftime64_s( &timebuffer );
	/*! Convert time to struct tm form*/
	 _localtime64_s(&time, &timebuffer.time );
#else
	ftime( &timebuffer );
	/*! Convert time to struct tm form*/
	time = localtime(&timebuffer.time );
#endif
#ifdef WIN32
	sprintf_s(tmpbuf, MAX_PATH, "%02d/%02d %02d:%02d:%02d:%03d - ",
		time.tm_mday,
		time.tm_mon + 1,
		time.tm_hour,
		time.tm_min,
		time.tm_sec,
		timebuffer.millitm);
#else
	sprintf(tmpbuf, "%02d/%02d %02d:%02d:%02d:%03d - ",
		time->tm_mday,
		time->tm_mon + 1,
		time->tm_hour,
		time->tm_min,
		time->tm_sec,
		timebuffer.millitm);
#endif

	str += tmpbuf;

	va_start(vaAP, strFormat);
#ifdef WIN32
	vsprintf_s(tmpbuf, MAX_PATH, strFormat, vaAP);
#else
	vsprintf(tmpbuf, strFormat, vaAP);
#endif
    va_end(vaAP);
	str += tmpbuf;

	CNotifyLogTrace* pLog = CNotifyLogTrace::Create(str);
	_APCBuff.push_back(pLog);
	++_dwObjectNumber;
	APCBuff_not_empty.notify_one();

	return;
}

 } //namespace FEF
