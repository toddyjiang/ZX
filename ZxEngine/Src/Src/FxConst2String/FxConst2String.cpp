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
#include "FxConst2String.h"
#include "FxUtils/FxUtils.h"
//#include "IFxPin.h"
//#pragma warning(disable : 4995)	//!< stdio.h
//#pragma warning(disable : 4996) //!< stdlib.h

namespace FEF {

CFxConst2String::CFxConst2String(void)
{
	/*! Fx States */
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_UNDEFINED_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_LOADING_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_RELEASE_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_INIT_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_CONNECT_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_DISCONNECT_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_STOP_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_PAUSE_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_START_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_RUN_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_IDLE_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_FLUSH_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_PARAM_UPDATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_PIN_UPDATE);
    FX_CONST2STRING_INSERT(_StringOfFxState, FX_FRAME_UPDATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_STREAM_INIT_STATE);

    FX_CONST2STRING_INSERT(_StringOfFxState, FX_UNDERRUN_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_OVERRUN_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_TIMEOUT_STATE);

    FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_RELEASE_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_INIT_STATE);
    FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_CONNECT_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_DISCONNECT_STATE);
    FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_PIN_STATE);
    FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_MEDIA_PIN_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_STOP_STATE);
    FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_PAUSE_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_START_STATE);
    FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_RUN_STATE);
    FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_INVALID_PARAM);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_SUBMEDIA_PIN_STATE);
	FX_CONST2STRING_INSERT(_StringOfFxState, FX_ERROR_MEMORY_STATE);

    FX_CONST2STRING_INSERT(_StringOfFxState, FX_USER_STATE);

	/*! Fx Type */
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_NOT_DEFINED);
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_SOURCE);
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_RENDERER);
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_ANALYSER);
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_PROCESS);
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_SPLITTER);
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_MIXER);
	FX_CONST2STRING_INSERT(_StringOfFxType,	  FX_USER);

    /*! Fx Scopes */
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_NOT_DEFINED);
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_AUDIO);
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_VIDEO);
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_TEXT);
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_DATA	);
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_NETWORK);
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_ALL	);
    FX_CONST2STRING_INSERT(_StringOfFxScope,  FX_SCOPE_USER	);

    /*! Fx Pin types */
	FX_CONST2STRING_INSERT(_StringOfFxPinType, PIN_IN);
	FX_CONST2STRING_INSERT(_StringOfFxPinType, PIN_OUT);

	/*! Fx Unit Type */
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, NOT_DEFINED_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, UINT8_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, INT8_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, UINT16_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, INT16_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, UINT32_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, INT32_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, INT64_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, UINT64_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, FLOAT32_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, FLOAT64_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxUnitType, COMPLEX_TYPE);

	/*! Fx Error */
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_OK);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_ERROR);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_INVALPARAM);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_INVALHANDLE);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_NOMEM);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_MEDIANOTSUPPORTED);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_FMTNOTSUPPORTED);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_ERRORSTATE);
	FX_CONST2STRING_INSERT(_StringOfFxError, FX_NOINTERFACE);

	/*! Fx Main Media Type */
	FX_CONST2STRING_INSERT(_StringOfFxMainMediaType, MAIN_TYPE_UNDEFINED);
	FX_CONST2STRING_INSERT(_StringOfFxMainMediaType, AUDIO_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxMainMediaType, VIDEO_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxMainMediaType, TEXT_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxMainMediaType, DATA_TYPE);
	FX_CONST2STRING_INSERT(_StringOfFxMainMediaType, USER_TYPE);

	/*! Fx Sub Media Type */
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, SUB_TYPE_UNDEFINED);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, PCM);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, PCMU);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, PCMA);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, G721);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, G722);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, G723);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, G726);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, G728);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, G729);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, GSM);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, LPC);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, QCELP);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, DVI4_8K);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, DVI4_11K);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, DVI4_16K);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, DVI4_22K);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, MPEGA);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, MPA);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, DOLBY_AC3);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, AAC);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, WMA);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, MPV);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, CELB);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, JPEG);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, BGR);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, RGB);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, R_COLOR);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, G_COLOR);
    FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, B_COLOR);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, UYVY);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, YUY2);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, IYUV);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, YV12);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, NV12);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, NV21);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, Y800);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, H261);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, H263);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, H263_1998);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, MP2T);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, WMV);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, DIVX);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, XVID);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, UTF_8);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, UTF_16);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, ASCII);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, VECTOR);
	FX_CONST2STRING_INSERT(_StringOfFxSubMediaType, MATRIX);

	_StringofFx.insert(make_pair(FX_STATE_CONST, &_StringOfFxState));
	_StringofFx.insert(make_pair(FX_TYPE_CONST, &_StringOfFxType));
    _StringofFx.insert(make_pair(FX_SCOPE_CONST, &_StringOfFxScope));
	_StringofFx.insert(make_pair(UNITTYPE_CONST, &_StringOfFxUnitType));
	_StringofFx.insert(make_pair(FXENGINE_ERROR_CONST, &_StringOfFxError));
	_StringofFx.insert(make_pair(FX_MAINMEDIATYPE_CONST, &_StringOfFxMainMediaType));
	_StringofFx.insert(make_pair(FX_SUBMEDIATYPE_CONST, &_StringOfFxSubMediaType));
	_StringofFx.insert(make_pair(FX_PINTYPE_CONST, &_StringOfFxPinType));
}

CFxConst2String::~CFxConst2String()
{
	AutoLock lock(_CS);
	_StringofFx.clear();
	_StringOfFxState.clear();
	_StringOfFxType.clear();
    _StringOfFxScope.clear();
	_StringOfFxUnitType.clear();
	_StringOfFxError.clear();
	_StringOfFxMainMediaType.clear();
	_StringOfFxSubMediaType.clear();
	_StringOfFxPinType.clear();

	lock.unlock();
}

std::string CFxConst2String::GetConstToString(FXENGINE_CONST_TYPE FxEngineConstType, Int32 sdwFxEngineConst)
{
	AutoLock lock(_CS);

	std::string str;

	FxConst2Map::iterator it1;
	FxConst2StringMap::iterator it2;

    it1 = _StringofFx.find( FxEngineConstType );
    if( it1 == _StringofFx.end() )
    {
        str = "UNKNOWN CONST";
    }
    else
    {
		it2 = it1->second->find( sdwFxEngineConst );
		if( it2 == it1->second->end() )
		{
			str = "UNKNOWN CONST";
		}
		else
		{
			str = it2->second;
		}
	}

	return str;
}

 } //namespace FEF
