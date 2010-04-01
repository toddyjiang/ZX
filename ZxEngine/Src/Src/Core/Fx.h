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

#include "IFxBase.h"
#include "IFx.h"
#include "FxPin/FxPin.h"
#include "FxState/FxState.h"
#include "FxPinManager/FxPinManager.h"
#include "FxParam/FxParam.h"
#include "FxRefClock/FxRefClock.h"

#include "FxTrace/FxTrace.h"
#include "FxConst2String/FxConst2String.h"

namespace FEF {
namespace
{

	class CFx :	
				public IFx,
				public CFxPinManager,
				public CFxParam,
				public CFxRefClock,
				public CFxState
				
{
	public:
        CFx(const std::string strFxName);
		virtual ~CFx(){};

	// Interface
	public:
		virtual Int32 GetIFxVersion(Uint16* pwMajor, Uint16* pwMinor, Uint16* pwBuild, Uint16* pwRev);
		virtual Int32 FxGetInterface(FX_INTERFACE FxInterfaceType, Void** ppFxInterface);
		virtual Int32 Release();
        virtual Int32 GetConstToString(FXENGINE_CONST_TYPE FxEngineConstType, Int32 sdwFxEngineConst, std::string& strStateName);

	private:
		virtual Int32 FxReleaseInterface();

	private:
		Uint32		_dwRef;
		CFx* _pFxObject;

	private:
		std::string	_strFxName;

	private:
		std::string _FxLogFile;
		CFxLog* _pLogTrace;

		// Instance value to manage the right log messages.
		Uint32 _dwThisInstance;				// current instance
	public:
		Uint32 GetFxInstance() const {return _dwThisInstance;}
	public:
		static Uint32 _dwInstanceCount;		// total instances

	private:
		CFxConst2String  _FxConst2String;

	private:
		boost::mutex _CS;
	};
}
} //namespace FEF
