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

#include "stdafx.h"
#include <list>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

#include "../Src/FxUtils/FxUtils.h"
#include "FxEngine.h"

#include <boost/lexical_cast.hpp>
#include "Fx.h"

#define FX_STATE_SIZE (4)
#define PIN_SIZE (8)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFx::CFx(FX_HANDLE hFxEngine, FX_HANDLE hFx, FX_HANDLE hFxState, std::string strPath, Uint32 dwDefaultX, Uint32 dwDefaultY, std::string strFxName)
{
	Int32 hr;

	_ShouldDrawMarker = FALSE;

	_hFxEngine	= hFxEngine;
	_hFx		= hFx;
	_hFxState	= hFxState;

    _IsSizesInit = FALSE;

	/*! Get Fx Infos */
    const FX_DESCRIPTOR* pFxDescriptor = NULL;
	hr = FEF_GetFxInfo(hFxEngine, hFx, &pFxDescriptor);
	if(FEF_FAILED(hr))
	{
		return;
	}
    if(strFxName.empty())
        _strFxName = pFxDescriptor->strName;
    else
        _strFxName = strFxName;

    _strFxPath = strPath;
    _strVersion = pFxDescriptor->strVersion;
    _FxType = pFxDescriptor->FxType;
    _FxScope = pFxDescriptor->FxScope;

    /*! Initialize Fx pin */
    UpdateFxPin();

    /*! Initialize Fx Frame */
    _FrameHeight = 0;
    _FrameWidth = 0;

    const char* pbFxFrame = NULL;
    hr = FEF_GetFxFrame(_hFxEngine, _hFx, &pbFxFrame);
    if(FEF_FAILED(hr))
	{
		return;
	}
}

CFx::~CFx()
{
	/*! Draw link point */
    _PinTypeMap.clear();
    _PinNameMap.clear();
    _PinVector.clear();
}

Void CFx::SetFxName(std::string strFxName)
{
    _strFxName = strFxName;

    UpdateFxPin();
    return;
}

Void CFx::InitFxPin()
{
    return;
}

Bool CFx::UpdateFxPin()
{
    AutoLock lock(_CSDraw);
	Int32 hr;
	FX_PIN_TYPE PinType;
    std::string strPinName;

    Uint16 wCurrentInPin = _wInPinNb;
    Uint16 wCurrentOutPin = _wOutPinNb;

    _wOutPinNb = 0;
    _wInPinNb = 0;

    Uint16 wPinCount;
	hr = FEF_GetFxPinCount( /* [in] */_hFxEngine,
						/* [in] */_hFx,
						/* [out] */&wPinCount);
	if(FEF_FAILED(hr))
	{
        return FALSE;
	}

	IFxPin* pIFxPin;
	Int16 Idx;

    _PinTypeMap.clear();
    _PinNameMap.clear();
    _PinVector.clear();
    for(Idx = 0; Idx < wPinCount; Idx++)
	{
		hr = FEF_QueryFxPinInterface(/* [in] */_hFxEngine,
							/* [in] */_hFx,
							/* [out] */&pIFxPin,
							/* [in] */Idx);
		if(FEF_FAILED(hr))
		{
            return FALSE;
		}

		pIFxPin->GetPinType(/* [out] */&PinType);
        if(PinType == PIN_IN) _wInPinNb++; else _wOutPinNb++;
        pIFxPin->GetPinName(strPinName);
		_PinTypeMap.insert(make_pair(pIFxPin, PinType));
        _PinNameMap.insert(make_pair(pIFxPin, strPinName));
        _PinVector.push_back(pIFxPin);
	}

    _IsSizesInit = FALSE;

    /*! Detect new pin */
    if( (wCurrentInPin < _wInPinNb) || (wCurrentOutPin < _wOutPinNb) )
        return TRUE;

    return FALSE;
}

Void CFx::UpdateFxFrame()
{
    AutoLock lock(_CSDraw);
    Int32 hr;

    _FrameHeight = 0;
    _FrameWidth = 0;
    const char* pbFxFrame = NULL;
    hr = FEF_GetFxFrame(_hFxEngine, _hFx, &pbFxFrame);
    if(FEF_FAILED(hr))
	{
        return;
	}

    return;
}
