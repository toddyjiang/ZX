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
#include "IFxPinManager.h"
#include "FxConst2String/FxConst2String.h"
#include <map>

namespace FEF {

typedef std::map<IFxPinCallback*, CSPair*> CSPinMap;
//typedef std::pair<IFxPinCallback*, CSPair> Pin_Pair;

class CFxPinManager : public IFxPinManager,
					  public CFxConst2String {
public:
    CFxPinManager(const std::string strFxName, Uint32 dwFxEngineInstance, Uint32 dwFxInstance);
	virtual ~CFxPinManager();

  // Interfaces
public:
    // Interface for Pin management
	virtual Int32 Create(PFX_PIN pPinInfo, IFxPin** ppFxPin);
    virtual Int32 Remove(IFxPin* pFxPin);
	virtual Int32 GetPinCount(Uint16& wPinCount);
	virtual Int32 GetPin(IFxPin** ppIFxPin, Uint16 wPinNumber);
	virtual Int32 GetPinNumber(IFxPin* pIFxPin, Uint16* pwPinNumber);

	// internal methods
public:
	//! Used for created pin after Fx init
	virtual Int32 InitFxPins(CFxPinObserver* pFxPinObservers, FX_HANDLE hFx);
	CFxPinObserver* _pFxPinObservers;
	FX_HANDLE		_hFx;
	Bool _PinsAreInitialized;

private:
	CFxPinL		_lpFxPin;

private:
	CSPinMap	_CSPinMap;

private:
	//! Critical Section
    boost::mutex _CS;

private:
	Void ClearFxPinList();

private:
	std::string _strFxName;

private:
	std::string _FxLogFile;
	CFxLog* _pLogTrace;

	Uint32 _dwPinInstance;				// current instance
	Uint32 _dwFxInstance;				// Fx instance
	Uint32 _dwFxEngineInstance;			// FxEngine instance
};

} //namespace FEF
