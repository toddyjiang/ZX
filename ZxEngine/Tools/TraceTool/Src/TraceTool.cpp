/* __________________________________________________________________________
	
	FxEngine Framework. 
	Copyright (c) 2005, 2009 Sylvain Machel, SMProcess.

	This file is part of FxEngine Framework.
    TraceTool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    TraceTool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with TraceTool. If not, see <http://www.gnu.org/licenses/>.

   ____________________________________________http://www.SMProcess.com______ */
#ifdef WIN32
#include "windows.h"
#endif
#include "stdio.h"
#ifdef WIN32
#include <io.h>
#include <conio.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif
#include <stdlib.h>
#include <ios>        // for std::ios_base, etc.
#include <iostream>   // for std::cerr, std::cout
#include <ostream>    // for std::endl

#include <string>
using namespace std;

#define FXTRACETOOL_VERSION        "v1.4.0.0"	//<! Y: compnent interface, Z: component code
#ifdef WIN32
#define FX_TRACE_DIR        ".\\"
#define REMOVEDIR(a) RemoveDirectory(a)
#define CREATEDIR(a, b) CreateDirectory(a, b)
#else
#define FX_TRACE_DIR        "./"
#define REMOVEDIR(a) rmdir(a)
#define CREATEDIR(a, b) mkdir(a, 0777)
#endif

#define MAX_PATH    260
/*!Test Cervisia */

void EnableTraces();
void DesableTraces();
void ClearTraces();

std::string GetTemporaryDir()
{
	char    strTmpPath[MAX_PATH];
    std::string strTemporaryDir = FX_TRACE_DIR;
    char* pstrTmp;
#ifdef WIN32
    if(0 != ::GetTempPath (MAX_PATH, strTmpPath))
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
#ifdef WIN32
					if (_access("/tmp", 0) == 0)
#else
					if (access("/tmp", 0) == 0)
#endif
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

int main()
{
	int cmd;
	cout << "******************************************************************************" << endl;
	cout << "                        FxEngine Trace Tool " << FXTRACETOOL_VERSION << endl;
	cout << "******************************************************************************" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << "0 - Exit Trace Tool" << endl;
	cout << "" << endl;
	cout << "1 - Enable  FxEngine Traces" << endl;
	cout << "2 - Clear   FxEngine Traces" << endl;
    cout << "3 - Desable FxEngine Traces" << endl;

	cout << "" << endl;

    cout << "Temporary directory is: " << GetTemporaryDir().c_str() << endl;
    cout << "" << endl;

	int rep;

    do {
        cout << "Choose command: ";
        rep = getchar();
        getchar();
        fflush(stdin); /* vidage du flux stdin */
        switch(rep)
        {
		case '0':
            //Quit
            break;
        case '1':
            //cout << "1" << endl;
            EnableTraces();
            break;
        case '2':
            //cout << "2" << endl;
            ClearTraces();
            break;
        case '3':
            //cout << "3" << endl;
            DesableTraces();
            break;
        default:
			cout << "Command Error !! " << endl;
        }

    } while ( rep != '0');

	return 0;
}

void EnableTraces()
{
	char strFxDirName[MAX_PATH];

    std::string strTemporaryDir = GetTemporaryDir();
#ifdef WIN32
    strTemporaryDir += "\\SMProcess.log";
#else
    strTemporaryDir += "/SMProcess.log";
#endif

	CREATEDIR((char*)strTemporaryDir.c_str(), NULL);

	sprintf(strFxDirName, "%s", strTemporaryDir.c_str());
	CREATEDIR(strFxDirName, NULL);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxPin", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxPin", strTemporaryDir.c_str());
#endif

	CREATEDIR(strFxDirName, NULL);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxParam", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxParam", strTemporaryDir.c_str());
#endif

	CREATEDIR(strFxDirName, NULL);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\Fx", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/Fx", strTemporaryDir.c_str());
#endif

	CREATEDIR(strFxDirName, NULL);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxPinManager", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxPinManager", strTemporaryDir.c_str());
#endif

	CREATEDIR(strFxDirName, NULL);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxState", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxState", strTemporaryDir.c_str());
#endif

	CREATEDIR(strFxDirName, NULL);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxEngine", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxEngine", strTemporaryDir.c_str());
#endif

	CREATEDIR(strFxDirName, NULL);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxRefClock", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxRefClock", strTemporaryDir.c_str());
#endif

	CREATEDIR(strFxDirName, NULL);
}

void ClearDir(char* strDirName, char* strExt)
{
#ifdef WIN32
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;

	BOOL fFinished = FALSE;

	char str[MAX_PATH];
	sprintf(str, "%s%s", strDirName,  strExt);

	// Start searching for .TXT files in the current directory.
	hSearch = FindFirstFile(str, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		return;
	}

	// Copy each .TXT file to the new directory
	// and change it to read only, if not already.

	while (!fFinished)
	{
		sprintf(str, "%s%s", strDirName,  FileData.cFileName);
		remove(str);
		if (!FindNextFile(hSearch, &FileData))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
			{
				fFinished = TRUE;
			}
			else
			{
				return;
			}
		}
	}

	// Close the search handle.

	FindClose(hSearch);
#else
    std::string strFxTemporaryDir;
    struct dirent **namelist;
    int n;
    n = scandir(strDirName, &namelist, 0, alphasort);
    if (n < 0)
        return;
    else {
        while(n--) {
            strFxTemporaryDir = strDirName;
            strFxTemporaryDir += namelist[n]->d_name;
            remove(strFxTemporaryDir.c_str());
            free(namelist[n]);
        }
        free(namelist);
    }

#endif
}

void ClearTraces()
{
	char strFxDirName[MAX_PATH];
	sprintf(strFxDirName, "%s", "");

    std::string strTemporaryDir = GetTemporaryDir();
    std::string strFxTemporaryDir;
#ifdef WIN32
    strTemporaryDir += "\\SMProcess.log";
#else
    strTemporaryDir += "/SMProcess.log";
#endif

	ClearDir((char*)strTemporaryDir.c_str(), "*.*");

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxPin\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxPin/", strTemporaryDir.c_str());
#endif
    strFxTemporaryDir = strFxDirName;
	ClearDir((char*)strFxTemporaryDir.c_str(), "*.*");

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxParam\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxParam/", strTemporaryDir.c_str());
#endif

	strFxTemporaryDir = strFxDirName;
	ClearDir((char*)strFxTemporaryDir.c_str(), "*.*");

#ifdef WIN32
    sprintf(strFxDirName, "%s\\Fx\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/Fx/", strTemporaryDir.c_str());
#endif

	strFxTemporaryDir = strFxDirName;
	ClearDir((char*)strFxTemporaryDir.c_str(), "*.*");

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxPinManager\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxPinManager/", strTemporaryDir.c_str());
#endif

	strFxTemporaryDir = strFxDirName;
	ClearDir((char*)strFxTemporaryDir.c_str(), "*.*");

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxState\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxState/", strTemporaryDir.c_str());
#endif

	strFxTemporaryDir = strFxDirName;
	ClearDir((char*)strFxTemporaryDir.c_str(), "*.*");

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxEngine\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxEngine/", strTemporaryDir.c_str());
#endif

	strFxTemporaryDir = strFxDirName;
	ClearDir((char*)strFxTemporaryDir.c_str(), "*.*");

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxRefClock\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxRefClock/", strTemporaryDir.c_str());
#endif
	strFxTemporaryDir = strFxDirName;
	ClearDir((char*)strFxTemporaryDir.c_str(), "*.*");

    return;
}

void DesableTraces()
{
    ClearTraces();

    char strFxDirName[MAX_PATH];

    std::string strTemporaryDir = GetTemporaryDir();
#ifdef WIN32
    strTemporaryDir += "\\SMProcess.log";
#else
    strTemporaryDir += "/SMProcess.log";
#endif

	REMOVEDIR((char*)strTemporaryDir.c_str());

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxPin\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxPin/", strTemporaryDir.c_str());
#endif

	REMOVEDIR(strFxDirName);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxParam\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxParam/", strTemporaryDir.c_str());
#endif

	REMOVEDIR(strFxDirName);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\Fx\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/Fx/", strTemporaryDir.c_str());
#endif

	REMOVEDIR(strFxDirName);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxPinManager\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxPinManager/", strTemporaryDir.c_str());
#endif

	REMOVEDIR(strFxDirName);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxState\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxState/", strTemporaryDir.c_str());
#endif

	REMOVEDIR(strFxDirName);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxEngine\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxEngine/", strTemporaryDir.c_str());
#endif

	REMOVEDIR(strFxDirName);

#ifdef WIN32
    sprintf(strFxDirName, "%s\\FxRefClock\\", strTemporaryDir.c_str());
#else
    sprintf(strFxDirName, "%s/FxRefClock/", strTemporaryDir.c_str());
#endif

	REMOVEDIR(strFxDirName);

    return;
}
