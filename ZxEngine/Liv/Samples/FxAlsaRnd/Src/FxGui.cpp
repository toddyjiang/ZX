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
   @file  FxGui.cpp
   @brief The Fx Gui. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/

#include <gtk/gtk.h>

#include <stdio.h>
#include <string.h>

#include "FxGui.h"

/*! Defaults GUI values */
Uint16 g_wDefaultPreBuffering = 200; //!< 200ms
Uint16 g_wMinPreBuffering  = 5; //!< 5ms
Uint16 g_wMaxPreBuffering  = 300; //!< 200ms

Uint16 g_wDefaultBufferLength = 50; //!< 50ms
Uint16 g_wMinBufferLength  = 5; //!< 5ms
Uint16 g_wMaxBufferLength  = 100; //!< 100ms

CFxGui::CFxGui(std::vector<DevicePair> vectorDevice, IFxParam *pFxParam)
{
	/*! Save Fx param interface */
	_pFxParam = pFxParam;
	
	/*! main GTK components */
    GtkWidget *pVBox;
	GtkWidget *pVBoxDevice;
	GtkWidget *pVBoxParam;
	GtkWidget *pVBoxStream;
    GtkWidget *pFrameDevice;
    GtkWidget *pFrameParam;
	GtkWidget *pFrameStream;

	GtkWidget *pHBoxDevice;
	GtkWidget *pHBoxPreBuf;
	GtkWidget *pHBoxBuf;

    /*! Windows creation */
    _pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(_pWindow), "Fx property page");
	gtk_window_set_default_size(GTK_WINDOW(_pWindow), 320, 50);
	gtk_container_set_border_width(GTK_CONTAINER(_pWindow), 4);
    pVBox = gtk_vbox_new(FALSE, 8);
    gtk_container_add(GTK_CONTAINER(_pWindow), pVBox);

    /*! Alsa Device frame */
    pFrameDevice = gtk_frame_new("Alsa device");
    gtk_box_pack_start(GTK_BOX(pVBox), pFrameDevice, FALSE, FALSE, 0);
	pVBoxDevice = gtk_vbox_new(TRUE, 8);
    gtk_container_add(GTK_CONTAINER(pFrameDevice), pVBoxDevice);
	pHBoxDevice = gtk_hbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER(pVBoxDevice), pHBoxDevice);
	_pComboDevice = gtk_combo_box_new_text(); 
    g_signal_connect(G_OBJECT(_pComboDevice), "changed", G_CALLBACK(ControlComboDeviceWrapper), this);
    gtk_box_pack_start (GTK_BOX (pHBoxDevice), _pComboDevice, TRUE, TRUE, 8);
    std::vector<DevicePair>::iterator It = vectorDevice.begin();
    Uint16 wDeviceIdx = 0;
    while(It != vectorDevice.end())
    {
    	gtk_combo_box_append_text (GTK_COMBO_BOX (_pComboDevice), It->first.c_str());
        _mapDevice.insert(make_pair(It->second.c_str(), wDeviceIdx++));
        It++;
    }
    gtk_combo_box_set_active (GTK_COMBO_BOX (_pComboDevice), 0);
    
	/*! Fx Settings */
	pFrameParam = gtk_frame_new("Stream settings");
    gtk_box_pack_start(GTK_BOX(pVBox), pFrameParam, FALSE, FALSE, 0);
	pVBoxParam = gtk_vbox_new(TRUE, 8);
    gtk_container_add(GTK_CONTAINER(pFrameParam), pVBoxParam);
	
	pHBoxPreBuf = gtk_hbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER(pVBoxParam), pHBoxPreBuf);
	GtkWidget* _pLabelBuffering = gtk_label_new("Pre Buffering time (ms):");
    gtk_box_pack_start(GTK_BOX(pHBoxPreBuf), _pLabelBuffering, FALSE, FALSE, 6);
	_pSliderPreBuffering = gtk_hscale_new_with_range(g_wMinPreBuffering, g_wMaxPreBuffering, 1); 
    gtk_range_set_value(GTK_RANGE(_pSliderPreBuffering), g_wDefaultPreBuffering);
    _SliderIdPreBuffering = g_signal_connect(G_OBJECT(_pSliderPreBuffering), "value_changed", G_CALLBACK(ControlSliderPreBufferingWrapper), this);
	gtk_box_pack_start(GTK_BOX(pHBoxPreBuf), _pSliderPreBuffering, TRUE, TRUE, 6);

	pHBoxBuf = gtk_hbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER(pVBoxParam), pHBoxBuf);
	GtkWidget* _pLabelBuffer = gtk_label_new("Waiting for Buffer length (ms):");
    gtk_box_pack_start(GTK_BOX(pHBoxBuf), _pLabelBuffer, FALSE, FALSE, 6);
	_pSliderBufferLength = gtk_hscale_new_with_range(g_wMinBufferLength, g_wMaxBufferLength, 1); 
    gtk_range_set_value(GTK_RANGE(_pSliderBufferLength), g_wDefaultBufferLength);
    _SliderIdBufferLength = g_signal_connect(G_OBJECT(_pSliderBufferLength), "value_changed", G_CALLBACK(ControlSliderBufferLengthWrapper), this);
	gtk_box_pack_start(GTK_BOX(pHBoxBuf), _pSliderBufferLength, TRUE, TRUE, 6);	

	pFrameStream = gtk_frame_new("Stream properties"); 
    gtk_box_pack_start(GTK_BOX(pVBox), pFrameStream, FALSE, FALSE, 0);
	pVBoxStream = gtk_vbox_new(TRUE, 8);
    gtk_container_add(GTK_CONTAINER(pFrameStream), pVBoxStream);
	_pLabelStreamFormat = gtk_label_new("Stream format: Unknown");
    gtk_box_pack_start(GTK_BOX(pVBoxStream), _pLabelStreamFormat, FALSE, FALSE, 0);
	_pLabelPlayedTime = gtk_label_new("Played time: 0 ms");
    gtk_box_pack_start(GTK_BOX(pVBoxStream), _pLabelPlayedTime, FALSE, FALSE, 0);
    	
	g_signal_connect(G_OBJECT(_pWindow), "delete_event", G_CALLBACK(HideWrapper), this);
	
}

CFxGui::~CFxGui(void)
{
	gtk_widget_destroy(_pWindow);
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFxParameters()
	Update Gui Fx parameters.
*//*-----------------------------------------------------------------------------*/
Void CFxGui::UpdateFxParameters()
{
	/*! Prebuffering Time */
	Uint16 wPrebufferingTime;
	_pFxParam->GetFxParamValue("Prebuffering Time", &wPrebufferingTime);
	
	wPrebufferingTime = (wPrebufferingTime < g_wMinPreBuffering) ? g_wMinPreBuffering : wPrebufferingTime;
	wPrebufferingTime = (wPrebufferingTime > g_wMaxPreBuffering) ? g_wMaxPreBuffering : wPrebufferingTime;
	gtk_range_set_value(GTK_RANGE(_pSliderPreBuffering), wPrebufferingTime);
	
	/*! Buffer Length */
	Uint16 wBufferLength;
	_pFxParam->GetFxParamValue("Buffer length", &wBufferLength);
	wBufferLength = (wBufferLength < g_wMinBufferLength) ? g_wMinBufferLength : wBufferLength;
	wBufferLength = (wBufferLength > g_wMaxBufferLength) ? g_wMaxBufferLength : wBufferLength;
	gtk_range_set_value(GTK_RANGE(_pSliderBufferLength), wBufferLength);
	
	/*! Sound Device */
	std::string strSoundDevice;
	_pFxParam->GetFxParamValue("Sound device", strSoundDevice);
	
	std::map<std::string, Uint16>::iterator ItDevice = _mapDevice.find(strSoundDevice);
    if(ItDevice != _mapDevice.end()) {
    	gtk_combo_box_set_active (GTK_COMBO_BOX (_pComboDevice), ItDevice->second);
    }
    else {
    	_pFxParam->SetFxParamValue("Sound device", "default");
    	gtk_combo_box_set_active (GTK_COMBO_BOX (_pComboDevice), 0);
    }
    	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	ControlSliderPreBuffering()
	PreBuffering slider callback.
*//*-----------------------------------------------------------------------------*/
Void CFxGui::ControlSliderPreBuffering()
{
    _wPreBuffering = (Uint16)gtk_range_get_value(GTK_RANGE(_pSliderPreBuffering));
    _pFxParam->SetFxParamValue("Prebuffering Time", &_wPreBuffering);
    
    return;
}

/*-----------------------------------------------------------------------------*//*!
	ControlSliderBufferLength()
	Buffer Length slider callback.
*//*-----------------------------------------------------------------------------*/
Void CFxGui::ControlSliderBufferLength()
{
    _wBufferLength = (Uint16)gtk_range_get_value(GTK_RANGE(_pSliderBufferLength));
    _pFxParam->SetFxParamValue("Buffer length", &_wBufferLength); 
    	
    return;
}

/*-----------------------------------------------------------------------------*//*!
	ControlComboDevice()
	Alsa device combo callback.
*//*-----------------------------------------------------------------------------*/
Void CFxGui::ControlComboDevice()
{
    gint lComboActive;
    lComboActive = gtk_combo_box_get_active (GTK_COMBO_BOX (_pComboDevice));
    if(lComboActive == -1)
        return;
    
    std::map<std::string, Uint16>::iterator ItDevice = _mapDevice.begin();
    while(ItDevice != _mapDevice.end()) {
    	if(ItDevice->second == lComboActive)
    		_pFxParam->SetFxParamValue("Sound device", ItDevice->first);
    	ItDevice++;
    }
	    
    return;
}

/*-----------------------------------------------------------------------------*//*!
	Hide()
	Hide main window callback.
*//*-----------------------------------------------------------------------------*/
gint CFxGui::Hide() {
	/* Do not close window now */
	gtk_widget_hide(_pWindow);
	return TRUE;
}

/*-----------------------------------------------------------------------------*//*!
	UpdateStreamFormat()
	Update stream format text.
*//*-----------------------------------------------------------------------------*/
Void CFxGui::UpdateStreamFormat(Uint32 dwSamplingRate, std::string strType, Uint16 wChannels){
	char strTemp[255];
    sprintf(strTemp, "Stream format: %d Hz, %s, %d channel(s)", dwSamplingRate, strType.c_str(), wChannels);
    gtk_label_set_text (GTK_LABEL (_pLabelStreamFormat), strTemp);
    
	return;
}

/*-----------------------------------------------------------------------------*//*!
	UpdatePlayedTimeCallback()
	Update played time text callback().
*//*-----------------------------------------------------------------------------*/
gboolean CFxGui::UpdatePlayedTimeCallback()
{
	char strTemp[255];
    sprintf(strTemp, "Played time: %ld ms", _qTime);
    gtk_label_set_text (GTK_LABEL (_pLabelPlayedTime), strTemp);
    
	return FALSE;
}

/*-----------------------------------------------------------------------------*//*!
	UpdatePlayedTime()
	Update played time text.
*//*-----------------------------------------------------------------------------*/
Void CFxGui::UpdatePlayedTime(Uint64 qTime){
	/*! Update time and add callback in queue */
	_qTime = qTime;
    g_idle_add(UpdatePlayedTimeWrapper, this);
        
    return;
}

/*-----------------------------------------------------------------------------*//*!
	UpdateMaxPreBuffering()
	Update the max allowing Pre Buffering time.
*//*-----------------------------------------------------------------------------*/
Void CFxGui::UpdateMaxPreBuffering(Uint16 wMaxPreBuffering)
{
	gtk_range_set_range(GTK_RANGE(_pSliderPreBuffering), g_wMinPreBuffering, wMaxPreBuffering);
	return;
}

