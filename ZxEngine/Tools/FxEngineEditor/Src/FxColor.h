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
#ifndef _FX_COLOR_H
#define _FX_COLOR_H

#include <wx/colour.h>
#include "FxEngine.h"
using namespace FEF; //!< Using FxEngineFramework

class CFxColor  
{
public:
	CFxColor();
	virtual ~CFxColor();

public:
	wxColour GetFxColor(FX_TYPE& FxType);
    wxColour GetFxStateColor(FX_STATE& FxState);

private:
	wxColour _FxSource;			
	wxColour _FxRenderer;		
	wxColour _FxAnalyser;		
	wxColour _FxProcess;			
	wxColour _FxSplitter;		
	wxColour _FxMixer;			
	wxColour _FxUser;

    wxColour _FxStateGreen; 
    wxColour _FxStateOrange;
    wxColour _FxStateRed; 
    wxColour _FxStateUser;

};
#endif

