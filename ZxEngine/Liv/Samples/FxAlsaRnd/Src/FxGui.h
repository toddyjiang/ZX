/* __________________________________________________________________________

	Fx sample code. 

	This material has been supplied as part of the FxEngine Framework.
	Under copyright laws, this material may not be duplicated in whole
	or in part, except for personal use, without the express written consent
	of SMProcess. Refer to the license agreement contained with this Framework
	before using any part of this material.

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	PURPOSE.

	Email:  info@SMProcess.com

	Copyrights (c) 2009 SMProcess, all rights reserved.

   __________________________________________________________________________
*//*!
   @file  FxGui.h
   @brief The Fx Gui. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/
#pragma once

#include "IFxBase.h"
using namespace FEF; //!< Using FxEngineFramework

#include <vector>
#include <map>
using namespace std;

typedef std::pair<std::string, std::string> DevicePair;

class CFxGui
{
public:
    CFxGui(std::vector<DevicePair> vectorDevice, IFxParam *pFxParam); 
    virtual ~CFxGui(void);

public:
    Void ShowUI() {gtk_widget_show_all(_pWindow); return;}
    static gint HideWrapper(GtkWidget *widget, GdkEvent *event, CFxGui* pUser) {return pUser->Hide();}
    gint Hide();
    Void UpdateFxParameters();
    static void ControlSliderPreBufferingWrapper(GtkWidget *widget, CFxGui* pUser) {return pUser->ControlSliderPreBuffering();}
    Void ControlSliderPreBuffering();
	static void ControlSliderBufferLengthWrapper(GtkWidget *widget, CFxGui* pUser) {return pUser->ControlSliderBufferLength();}
    Void ControlSliderBufferLength();
	static void ControlComboDeviceWrapper(GtkWidget *widget, CFxGui* pUser) {return pUser->ControlComboDevice();}
	Void ControlComboDevice();
	Void UpdateStreamFormat(Uint32 dwSamplingRate, std::string strType, Uint16 wChannels);
	Void UpdatePlayedTime(Uint64 qTime);
	Void UpdateMaxPreBuffering(Uint16 wMaxPreBuffering);
	gboolean UpdatePlayedTimeCallback();
	static gboolean UpdatePlayedTimeWrapper(void* data) {return ((CFxGui*)data)->UpdatePlayedTimeCallback();}

private:
	GtkWidget* _pWindow;
	IFxParam  *_pFxParam;
    GtkWidget* _pLabelStreamFormat;
    GtkWidget* _pLabelPlayedTime;
	Uint64 _qTime;
    std::map<std::string, Uint16> _mapDevice;
	GtkWidget* _pComboDevice;
	GtkWidget* _pSliderPreBuffering;
	Uint16     _wPreBuffering;
	GtkWidget* _pSliderBufferLength;
	Uint16     _wBufferLength;
    gulong _SliderIdPreBuffering, _SliderIdBufferLength;
};


