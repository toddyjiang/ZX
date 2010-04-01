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

#include "FxColor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFxColor::CFxColor()
{
	_FxSource = wxColour( 255, 158, 0);
	_FxRenderer = wxColour( 206, 207, 255);
	_FxAnalyser = wxColour( 255, 199, 148);
	_FxProcess = wxColour( 0, 199, 255);
	_FxSplitter = wxColour( 156, 207, 0);
	_FxMixer = wxColour( 49, 207, 206);
	_FxUser = wxColour( 255, 255, 255);

    _FxStateGreen = wxColour( 124, 194, 66);
    _FxStateOrange = wxColour( 254, 190, 16);
    _FxStateRed = wxColour( 238, 58, 66);
    _FxStateUser = wxColour( 0, 0, 0);
}

CFxColor::~CFxColor()
{

}

wxColour CFxColor::GetFxColor(FX_TYPE& FxType)
{
	switch(FxType)
	{
	case FX_SOURCE:
		return _FxSource;
	case FX_RENDERER:
		return _FxRenderer;
	case FX_ANALYSER:
		return _FxAnalyser;
	case FX_PROCESS:
		return _FxProcess;
	case FX_SPLITTER:
		return _FxSplitter;
	case FX_MIXER:
		return _FxMixer;
	case FX_USER:
		return _FxUser;
	default:
		return _FxUser;

	}
}

wxColour CFxColor::GetFxStateColor(FX_STATE& FxState)
{
    if(FxState < FX_UNDERRUN_STATE)
        return _FxStateGreen;
    else if(FxState < FX_ERROR_RELEASE_STATE)
        return _FxStateOrange;
    else if(FxState < FX_USER_STATE)
        return _FxStateRed;
    else 
        return _FxStateUser;
}
