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
#ifndef _FX_H
#define _FX_H

#include "FxEngine.h" 

#include <list>
#include <map>
#include <vector>
#include <boost/thread/mutex.hpp>

using namespace FEF; //!< Using FxEngineFramework


class CFx;

typedef std::list<IFxPin*>    lFx;
typedef lFx::iterator	      lFxIter;

typedef std::map<FX_HANDLE, CFx*> FxMap;

typedef std::map<IFxPin*, FX_PIN_TYPE> PinTypeMap;
typedef std::map<IFxPin*, std::string> PinNameMap;
typedef std::vector<IFxPin*> PinVector;

class CFx{
public:
	CFx(FX_HANDLE hFxEngine, FX_HANDLE hFx, FX_HANDLE hFxState, std::string strPath, Uint32 dwDefaultX = 100, Uint32 dwDefaultY = 100, std::string strFxName = "");
	//CFx(const CFx& Fx);
	virtual ~CFx();

private:
	FX_HANDLE _hFxEngine;
	FX_HANDLE _hFx;
	FX_HANDLE _hFxState;

    int _FrameHeight;
    int _FrameWidth;

    std::string	_strFxName;
	std::string	_strVersion;
	std::string _strFxPath;
	FX_TYPE _FxType;
    FX_SCOPE _FxScope;

	int  _wNameH;
    int  _wPinH;
    int  _wNameW;
    int  _wPinW;
    Uint16  _wPinCount;

private:
    PinTypeMap  _PinTypeMap;
    PinNameMap  _PinNameMap;
    PinVector   _PinVector;
    Uint16      _wOutPinNb;
    Uint16      _wInPinNb;

    Bool        _IsSizesInit;

	Bool		_ShouldDrawMarker;

    //! Critical Section
	boost::mutex _CSDraw;

public:
	Bool UpdateFxPin();
    Void UpdateFxFrame();
    Void InitFxPin();
	Void UpdateTxRx();

	FX_HANDLE GethFxState() const   { return (_hFxState); }
    FX_HANDLE GethFxHandle() const   { return (_hFx); }

    Void SetFxName(std::string strFxName);
    const std::string GetFxName() const { return _strFxName; }

	const std::string GetstrFxPath() const   { return _strFxPath; }

private:
};
#endif
