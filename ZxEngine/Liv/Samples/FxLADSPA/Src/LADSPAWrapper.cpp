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
   @file  LADSPAWrapper.cpp
   @brief The Fx LADSPA allows to load and to use LADSPA plugins. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/

#include <gtk/gtk.h>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include "LADSPAWrapper.h"

/*! Defaults GUI values */
const float gfMin = 0;
const float gfMax = 1000;
const float gfDefault = 1;
const char  gstrDefault[] = "1";
const float gfInc = 0.1f;

/*! Pin Types */
FX_MEDIA_TYPE gPinMediatype = {AUDIO_TYPE, PCM};

/*! Defaults audio values */
unsigned long glDefaultFs = 44100;
unsigned long glAvailableFs[] = {8000, 16000, 22050, 44100, 48000, 96000};
unsigned short gnAvailableFs = 6;

CLADSPAWrapper::CLADSPAWrapper(IFx *pFx):
_pFx(pFx)
{
	/*! main GTK components */
    GtkWidget *pVBox;
    GtkWidget *pVBox2;
    GtkWidget *pVControlBox;
    GtkWidget *phBoxPath;
    GtkWidget *pEntryButton;
    GtkWidget *phBoxParam;
    GtkWidget *pFramePath;
    GtkWidget *pFrameParam;

    /*! Windows creation */
    _pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(_pWindow), "Fx LADSPA wrapper");
	gtk_window_set_default_size(GTK_WINDOW(_pWindow), 320, 50);
	gtk_container_set_border_width(GTK_CONTAINER(_pWindow), 4);
    pVBox = gtk_vbox_new(FALSE, 8);
    gtk_container_add(GTK_CONTAINER(_pWindow), pVBox);


    /* Lapsa path frame */
    pFramePath = gtk_frame_new("LADSPA plugin path");
    gtk_box_pack_start(GTK_BOX(pVBox), pFramePath, FALSE, FALSE, 0);
   
    pVBox2 = gtk_vbox_new(FALSE, 8);
    gtk_container_add(GTK_CONTAINER(pFramePath), pVBox2);
    phBoxPath = gtk_hbox_new(FALSE, 8);
    gtk_container_add(GTK_CONTAINER(pVBox2), phBoxPath);
    _pEntryPath = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(phBoxPath), _pEntryPath, TRUE, TRUE, 3);
    pEntryButton = gtk_button_new_with_label("Plugin File");
    g_signal_connect (pEntryButton, "clicked",
			G_CALLBACK (ChooseLADSPAWrapper), this);
    gtk_box_pack_start(GTK_BOX(phBoxPath), pEntryButton, FALSE, FALSE, 3);
    _pLabelName = gtk_label_new("Name :");
    gtk_box_pack_start(GTK_BOX(pVBox2), _pLabelName, FALSE, FALSE, 0);
    _pLabelAuthor = gtk_label_new("Author :");
    gtk_box_pack_start(GTK_BOX(pVBox2), _pLabelAuthor, FALSE, FALSE, 0);

    /* Lapsa parameters */
    pFrameParam = gtk_frame_new("LADSPA parameters");
    gtk_box_pack_start(GTK_BOX(pVBox), pFrameParam, FALSE, FALSE, 3);
    pVControlBox = gtk_vbox_new(FALSE, 3);
    gtk_container_add(GTK_CONTAINER(pFrameParam), pVControlBox);
	phBoxParam = gtk_hbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(pVControlBox), phBoxParam);
    _pComboControl = gtk_combo_box_new_text(); 
    g_signal_connect(G_OBJECT(_pComboControl), "changed", G_CALLBACK(ControlComboWrapper), this);
    gtk_box_pack_start (GTK_BOX (phBoxParam), _pComboControl, FALSE, FALSE, 8);
    phBoxParam = gtk_hbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(pVControlBox), phBoxParam);
    _pEntryControl = gtk_entry_new();
	gtk_entry_set_text (GTK_ENTRY (_pEntryControl), gstrDefault);
    _EntryId = g_signal_connect(G_OBJECT(_pEntryControl), "changed", G_CALLBACK(ControlEditWrapper), this);
    gtk_box_pack_start(GTK_BOX(phBoxParam), _pEntryControl, FALSE, TRUE, 6);
    _pSliderControl = gtk_hscale_new_with_range(gfMin, gfMax, gfInc); 
    gtk_range_set_value(GTK_RANGE(_pSliderControl), gfDefault);
    _SliderId = g_signal_connect(G_OBJECT(_pSliderControl), "value_changed", G_CALLBACK(ControlSliderWrapper), this);
	gtk_box_pack_start(GTK_BOX(phBoxParam), _pSliderControl, TRUE, TRUE, 6);
    	
	g_signal_connect(G_OBJECT(_pWindow), "delete_event", G_CALLBACK(HideWrapper), this);

    /*! init LADSPA */
    _pvPluginHandle = NULL;

	/*! Thread and mutex */
	_ShouldCloseThread = FALSE;
	_Thread = NULL;
	pthread_mutex_init( &_ThreadMutex, NULL );
    pthread_cond_init(&_ThreadCond, NULL);

	/*! Get the IFxState */
	_pFx->FxGetInterface(IFX_STATE, (Void**)&_pFxState);
}

CLADSPAWrapper::~CLADSPAWrapper(void)
{
	/*! Free current LADSPA */
    if(_pvPluginHandle){
		ResetLADSPA();
        unloadLADSPAPluginLibrary(_pvPluginHandle);
        _pvPluginHandle = NULL;
    }

	/*! Stop Thread */
	if(_Thread){
		_ShouldCloseThread = TRUE;
		pthread_cond_signal (&_ThreadCond); //!< Unlock wait
		pthread_join (_Thread, NULL);
		_Thread = NULL;
	}
	SAFE_RELEASE_INTERFACE(_pFxState);
	gtk_widget_destroy(_pWindow);
}

/*-----------------------------------------------------------------------------*//*!
	ChooseLADSPA()
	Get and load LADSPA.
*//*-----------------------------------------------------------------------------*/
void CLADSPAWrapper::ChooseLADSPA()
{
    GtkWidget *FileChooserWidget;
	FileChooserWidget = gtk_file_chooser_dialog_new("Open LADSPA File",
										 NULL,
										 GTK_FILE_CHOOSER_ACTION_OPEN,
										 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
										 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										 NULL);

	if(gtk_dialog_run(GTK_DIALOG(FileChooserWidget)) == GTK_RESPONSE_ACCEPT) {
		char *strFileName;
		strFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(FileChooserWidget));
		
		/*! Stop current plugin first */
		StopWrapper();
		/*! load plugin */
        if(LoadLADSPA(strFileName) != 0)
		    /* Show file name */
		    gtk_entry_set_text (GTK_ENTRY (_pEntryPath), "Loading Error");
        
        g_free(strFileName);
	}
	gtk_widget_destroy(FileChooserWidget);
    return;
}

/*-----------------------------------------------------------------------------*//*!
	LoadLADSPA()
	Load LADSPA plugin.
*//*-----------------------------------------------------------------------------*/     
Int32 CLADSPAWrapper::LoadLADSPA(const char* strLADSPAFile, const char* strPluginLabel, Uint32 dwFs)
{
	const LADSPA_Descriptor *psDescriptor;
    LADSPA_Descriptor_Function pfDescriptorFunction;
	unsigned long lPluginIndex;
    unsigned long lPortIndex, lControlPort, lAudioPort;
	Int32 hr;

	/*! Free current LADSPA */
    if(_pvPluginHandle){
		ResetLADSPA();
        unloadLADSPAPluginLibrary(_pvPluginHandle);
        _pvPluginHandle = NULL;
    }
    
    _pvPluginHandle = loadLADSPAPluginLibrary(strLADSPAFile);

    dlerror();
    pfDescriptorFunction 
    = (LADSPA_Descriptor_Function)dlsym(_pvPluginHandle, "ladspa_descriptor");
    if (!pfDescriptorFunction) {
        const char * pcError = dlerror();
        if (pcError) 
            fprintf(stderr,
	        "Unable to find ladspa_descriptor() function in plugin file "
	        "\"%s\": %s.\n"
	        "Are you sure this is a LADSPA plugin file?\n", 
	        strLADSPAFile,
	        pcError);
            return 1;
    }
	
    /*! Get plugin */
    std::map<int, std::string> mapDescriptor;
    std::map<int, std::string>::iterator It;

    for (lPluginIndex = 0;; lPluginIndex++) {
        psDescriptor = pfDescriptorFunction(lPluginIndex);
        if (!psDescriptor)
            break;
        if (strPluginLabel != NULL) {
            if (strcmp(strPluginLabel, psDescriptor->Label) != 0)
                continue;
            else {
                /*! Clear list */
                mapDescriptor.clear();
                break;
            }
        }
        else
        {
            /*! Add to list */
            mapDescriptor.insert(make_pair(lPluginIndex,std::string(psDescriptor->Name)));
        }
    }

	/*! Choose module */
    if(mapDescriptor.size() > 1) {
        static GtkWidget *window = NULL;
        GtkWidget *vbox, *frame, *box, *combo, *entry;
        window = gtk_dialog_new_with_buttons ("LADSPA selection",
					NULL,
					GTK_DIALOG_MODAL,
					GTK_STOCK_OK,
					GTK_RESPONSE_OK,
                    "Cancel",
                    GTK_RESPONSE_CANCEL,
					NULL);
        g_signal_connect (window, "destroy",
                          G_CALLBACK (gtk_widget_destroyed),
                          &window);
        
        vbox = gtk_vbox_new (FALSE, 2);
        gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), vbox);

        frame = gtk_frame_new ("LADSPA modules");
        gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
    
        box = gtk_vbox_new (FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (box), 5);
        gtk_container_add (GTK_CONTAINER (frame), box);
    
        combo = gtk_combo_box_new_text();
        It = mapDescriptor.begin();
        while(It != mapDescriptor.end())
        {
            gtk_combo_box_append_text (GTK_COMBO_BOX (combo), It->second.c_str());
            It++;
        }
        gtk_container_add (GTK_CONTAINER (box), combo);
        gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
        gtk_widget_show_all (window);
        gint response = gtk_dialog_run (GTK_DIALOG (window));

        if (response == GTK_RESPONSE_OK)
        {
            gint nIndex = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
            psDescriptor = pfDescriptorFunction(nIndex);
            
        }
        else
            psDescriptor = pfDescriptorFunction(mapDescriptor.begin()->first); //!< Get first by default

        gtk_widget_destroy (window);
    }
	else if(mapDescriptor.size() == 1)
		psDescriptor = pfDescriptorFunction(mapDescriptor.begin()->first); //!< Get first by default
	    
    if(psDescriptor == NULL)
        return -1;

    if(psDescriptor->PortCount == 0) {
        fprintf(stderr, "%s has no ports\n", psDescriptor->Name);
        return -1;
    }

	/*! Save descriptor for futur use */
	_psDescriptor = psDescriptor;
	
	/*! Instanciate plugin with all available Fs */
	if(!_psDescriptor->instantiate)
		return -1;
	for(unsigned short i = 0; i < gnAvailableFs; i++) {
		_CurrentInstanceHandle = _psDescriptor->instantiate(psDescriptor, glAvailableFs[i]);
		if(_CurrentInstanceHandle == NULL)
			return -1;
		_mapInstanceHandle.insert(make_pair(glAvailableFs[i], _CurrentInstanceHandle));
	}
	_CurrentInstanceHandle = _mapInstanceHandle[dwFs];
	
	_dwCurrentFs = dwFs;
	
    /*! Initialize control ports */
    for (lPortIndex = 0, lControlPort = 0; lPortIndex < _psDescriptor->PortCount; lPortIndex++) {
        if (LADSPA_IS_PORT_CONTROL(_psDescriptor->PortDescriptors[lPortIndex])) {
                CLADSPAControl* pLADSPAControl = new CLADSPAControl(_psDescriptor->PortNames[lPortIndex], lPortIndex, (Bool)LADSPA_IS_PORT_OUTPUT(_psDescriptor->PortDescriptors[lPortIndex]));
				pLADSPAControl->SetRange(_psDescriptor, _dwCurrentFs);
				for(unsigned short i = 0; i < gnAvailableFs; i++)
					pLADSPAControl->ConnectPort(_psDescriptor, _mapInstanceHandle[glAvailableFs[i]]);
                _mapControl.insert(make_pair(lControlPort++, pLADSPAControl));
                /*! Add it to combo */
                gtk_combo_box_append_text (GTK_COMBO_BOX (_pComboControl), _psDescriptor->PortNames[lPortIndex]);
        }
    }
	
    /*! Call first control port */
    if(_mapControl.size() > 0)
        gtk_combo_box_set_active (GTK_COMBO_BOX (_pComboControl), 0);
    ControlCombo();
	
    /*! Initialize audio ports */
    for (lPortIndex = 0, lAudioPort = 0; lPortIndex < _psDescriptor->PortCount; lPortIndex++, lAudioPort++) {
        if(LADSPA_IS_PORT_AUDIO(_psDescriptor->PortDescriptors[lPortIndex])) {
            if (LADSPA_IS_PORT_INPUT(_psDescriptor->PortDescriptors[lPortIndex])) {
				CInAudioPort* pInAudioPort = new CInAudioPort(_psDescriptor->PortNames[lPortIndex], lPortIndex, _dwCurrentFs, &_ThreadMutex, &_ThreadCond);
				hr = pInAudioPort->CreatePin(_pFx);
				if(FEF_FAILED(hr))
					return hr;
				_vectorInAudio.push_back(pInAudioPort);
				/*! Connect port */
				for(unsigned short i = 0; i < gnAvailableFs; i++)
					pInAudioPort->ConnectPort(_psDescriptor, _mapInstanceHandle[glAvailableFs[i]]);
            }
            else if(LADSPA_IS_PORT_OUTPUT(_psDescriptor->PortDescriptors[lPortIndex])) {
				COutAudioPort* pOutAudioPort = new COutAudioPort(_psDescriptor->PortNames[lPortIndex], lPortIndex, _dwCurrentFs, &_ThreadMutex, &_ThreadCond);
				hr = pOutAudioPort->CreatePin(_pFx);
				if(FEF_FAILED(hr))
					return hr;
				_vectorOutAudio.push_back(pOutAudioPort);
            }
        }
    }
		
	/*! Get the IFxState to inform observers*/
    _pFxState->FxPublishState(FX_PIN_UPDATE);
	
	/*! Update Fx parameter */
	IFxParam *pFxParam;
	hr = _pFx->FxGetInterface(IFX_PARAM, (Void**)&pFxParam);
	if(FEF_FAILED(hr)) {
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}
	pFxParam->SetFxParamValue("LADSPA plugin", std::string(strLADSPAFile));
	pFxParam->SetFxParamValue("LADSPA plugin label", std::string((psDescriptor->Label == NULL) ? "Default" : psDescriptor->Label));
	SAFE_RELEASE_INTERFACE(pFxParam);
	
	/*! Set LADSPA type */
	if(_vectorInAudio.size() > 0 && _vectorOutAudio.size() > 0)
		_LadspaType = LADSPA_PRC;
	else if (_vectorInAudio.size() > 0)
		_LadspaType = LADSPA_DST;
	else if (_vectorOutAudio.size() > 0)
		_LadspaType = LADSPA_SRC;
	else {
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		gtk_entry_set_text (GTK_ENTRY (_pEntryPath), "Invalid LADSPA file");
	}

	/*! Stop wrapper first */
	StopWrapper();
			
	/* Show file name */
	gtk_entry_set_text (GTK_ENTRY (_pEntryPath), strLADSPAFile);

    /*! Set LADSPA name and author */
    char strTemp[255];
    sprintf(strTemp, "Name: %s - %s", _psDescriptor->Name, (psDescriptor->Label == NULL) ? "Default" : psDescriptor->Label);
    gtk_label_set_text (GTK_LABEL (_pLabelName), strTemp);
    sprintf(strTemp, "Author: %s", _psDescriptor->Maker);
    gtk_label_set_text (GTK_LABEL (_pLabelAuthor), strTemp);
    
    return 0;
}

/*-----------------------------------------------------------------------------*//*!
	ControlEdit()
	LADSPA control edit callback.
*//*-----------------------------------------------------------------------------*/
Void CLADSPAWrapper::ControlEdit()
{
	gint lComboActive;
    lComboActive = gtk_combo_box_get_active (GTK_COMBO_BOX (_pComboControl));

    float fValue = atof(gtk_entry_get_text(GTK_ENTRY(_pEntryControl)));
    if(lComboActive == -1) {
        fValue = (fValue < gfMin) ? gfMin : fValue;
        fValue = (fValue > gfMax) ? gfMax : fValue;
    }
    else {
		   	fValue = (fValue < _mapControl[lComboActive]->GetMin()) ? _mapControl[lComboActive]->GetMin() : fValue;
        	fValue = (fValue > _mapControl[lComboActive]->GetMax()) ? _mapControl[lComboActive]->GetMax() : fValue;
        	_mapControl[lComboActive]->SetValue(fValue);
    }
    /*! Set set slider */
    g_signal_handler_block(G_OBJECT(_pSliderControl), _SliderId);
    gtk_range_set_value(GTK_RANGE(_pSliderControl), fValue);
    g_signal_handler_unblock(G_OBJECT(_pSliderControl), _SliderId);
    gtk_widget_queue_draw (_pSliderControl);
   
    return;
}

/*-----------------------------------------------------------------------------*//*!
	ControlSlider()
	LADSPA control slider callback.
*//*-----------------------------------------------------------------------------*/
Void CLADSPAWrapper::ControlSlider()
{
    float fValue = gtk_range_get_value(GTK_RANGE(_pSliderControl));
    
    /*! Set edit box */
    char strValue[255];
    sprintf(strValue, "%f", fValue);
    g_signal_handler_block(G_OBJECT(_pEntryControl), _EntryId);
    gtk_entry_set_text (GTK_ENTRY (_pEntryControl), strValue);
    g_signal_handler_unblock(G_OBJECT(_pEntryControl), _EntryId);

    gint lComboActive;
    lComboActive = gtk_combo_box_get_active (GTK_COMBO_BOX (_pComboControl));
    if(lComboActive == -1)
        return;
    _mapControl[lComboActive]->SetValue(fValue);
		
    return;
}

/*-----------------------------------------------------------------------------*//*!
	ControlCombo()
	LADSPA control combo callback.
*//*-----------------------------------------------------------------------------*/
Void CLADSPAWrapper::ControlCombo()
{
    gint lComboActive;
    lComboActive = gtk_combo_box_get_active (GTK_COMBO_BOX (_pComboControl));
    if(lComboActive == -1)
        return;
    /*! Update texts and values */
    char strValue[255];
    sprintf(strValue, "%f", _mapControl[lComboActive]->GetValue());
	g_signal_handler_block(G_OBJECT(_pEntryControl), _EntryId);
    gtk_entry_set_text (GTK_ENTRY(_pEntryControl), strValue);
	g_signal_handler_unblock(G_OBJECT(_pEntryControl), _EntryId);

	g_signal_handler_block(G_OBJECT(_pSliderControl), _SliderId);
    gtk_range_set_range(GTK_RANGE(_pSliderControl),_mapControl[lComboActive]->GetMin(), _mapControl[lComboActive]->GetMax());
    gtk_range_set_value(GTK_RANGE(_pSliderControl), _mapControl[lComboActive]->GetValue());
	g_signal_handler_unblock(G_OBJECT(_pSliderControl), _SliderId);

	if(_mapControl[lComboActive]->IsReadOnly()) { //!< Case where control is an output port
		gtk_widget_set_sensitive(GTK_WIDGET(_pSliderControl), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(_pEntryControl), FALSE);
	}
	else {
		gtk_widget_set_sensitive(GTK_WIDGET(_pSliderControl), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(_pEntryControl), TRUE);
	}

    return;
}

/*-----------------------------------------------------------------------------*//*!
	ResetLADSPA()
	Reset plugin components.
*//*-----------------------------------------------------------------------------*/
Void CLADSPAWrapper::ResetLADSPA()
{
	/*! Stop wrapper first */
	StopWrapper();
	
	/*! Clear instances */
	if(_psDescriptor->deactivate)
		_psDescriptor->deactivate(_CurrentInstanceHandle);
	std::map<unsigned long, LADSPA_Handle>::iterator ItHandle = _mapInstanceHandle.begin();
    while(ItHandle != _mapInstanceHandle.end())
    {
		if(_psDescriptor->activate) 
			_psDescriptor->activate(ItHandle->second);
		if(_psDescriptor->deactivate)
			_psDescriptor->deactivate(ItHandle->second);
		if(_psDescriptor->cleanup)
			_psDescriptor->cleanup(ItHandle->second);
        ItHandle++;
    }
	_mapInstanceHandle.clear();
	
	/*! Clear Fx Pins */
	std::vector<CInAudioPort*>::iterator ItInPin;
	for(ItInPin = _vectorInAudio.begin(); ItInPin != _vectorInAudio.end(); ItInPin++)
		(*ItInPin)->RemovePin(_pFx);
	_vectorInAudio.clear();
	
	std::vector<COutAudioPort*>::iterator ItOutPin;
	for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++)
		(*ItOutPin)->RemovePin(_pFx);
	_vectorOutAudio.clear();
	
	/*! Clear controls */
    std::map<int, CLADSPAControl*>::iterator ItCtrl = _mapControl.begin();
    while(ItCtrl != _mapControl.end())
    {
        gtk_combo_box_remove_text(GTK_COMBO_BOX (_pComboControl), 0); 
        delete (ItCtrl->second);
        ItCtrl++;
    }
	_mapControl.clear();
    gtk_widget_queue_draw (_pComboControl);
    gtk_combo_box_set_active (GTK_COMBO_BOX (_pComboControl), -1);
    	
    return;
}

/*-----------------------------------------------------------------------------*//*!
	Hide()
	Hide main window.
*//*-----------------------------------------------------------------------------*/
gint CLADSPAWrapper::Hide() {
	/* Do not close window now */
	gtk_widget_hide(_pWindow);
	return TRUE;
}

/*-----------------------------------------------------------------------------*//*!
	StartWrapper()
	Start LADSPA plugin wrapper.
*//*-----------------------------------------------------------------------------*/
Int32 CLADSPAWrapper::StartWrapper()
{
	/*! A valid plugin must be loaded */
	if(_pvPluginHandle == NULL)
		return FX_OK;

 	/*! Stop Thread */
	if(_Thread) {
		StopWrapper();
	}
	
	/*! Reset all audio ports */
	std::vector<CInAudioPort*>::iterator ItInPin;
	for(ItInPin = _vectorInAudio.begin(); ItInPin != _vectorInAudio.end();ItInPin++)
		(*ItInPin)->Reset();

	std::vector<COutAudioPort*>::iterator ItOutPin;
	for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++)
		(*ItOutPin)->Reset();

	/*! Activate plugin */
	if(_psDescriptor->deactivate)
		_psDescriptor->deactivate(_CurrentInstanceHandle);
	if(_psDescriptor->activate) 
		_psDescriptor->activate(_CurrentInstanceHandle);
		
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	StopWrapper()
	Stop LADSPA plugin wrapper.
*//*-----------------------------------------------------------------------------*/
Int32 CLADSPAWrapper::StopWrapper()
{
	/*! A valid plugin must be loaded */
	if(_pvPluginHandle == NULL)
		return FX_OK;

	/*! Stop Thread */
	if(_Thread){
		_ShouldCloseThread = TRUE;
		pthread_cond_signal (&_ThreadCond); //!< Unlock wait
		pthread_join (_Thread, NULL);
		_Thread = NULL;
	}

	/*! Restart thread */
	_ShouldCloseThread = FALSE;

	/*! Start new thread */
	if(pthread_create (
            &_Thread, NULL,
            CLADSPAWrapper::ThreadFctWrapper, (void *)this) != 0){
		return FX_ERROR;
	}

	/*! Desactivate plugin */
	if(_psDescriptor->deactivate)
		_psDescriptor->deactivate(_CurrentInstanceHandle);

	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	DoSource()
	Apply LADSPA source plugin. (Has Output pin(s) only) 
*//*-----------------------------------------------------------------------------*/
Void CLADSPAWrapper::DoSource(Uint32 dwReqTimeStamp)
{
	std::vector<COutAudioPort*>::iterator ItOutPin;
	IFxMedia* pIFxMedia;
	/*! Send Request TimeStamp */
	while(dwReqTimeStamp && !_ShouldCloseThread) {
		Uint32 dwSizeRequest = dwReqTimeStamp * sizeof(LADSPA_Data);
		/*! Get Dst IFxMedia to send */
		for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++) {
			/*! Get new FxMedia to fill */
			if( FEF_FAILED((*ItOutPin)->GetDeliveryMedia()) ) {/*!< The Media type of connection is set !! */  
			    _pFxState->FxPublishState(FX_UNDERRUN_STATE);
		    	return;
			}
			pIFxMedia = (*ItOutPin)->GetIFxMedia();

			/*! Get Media PCM format */
			IFxPcmFormat* pIFxPcmFormat;
			if(FEF_FAILED(pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
			/*! Release Media buffer */
		    	pIFxMedia->Release();
				_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
				return;
			}
			FX_PCM_FORMAT FxPcmFmt;

			/*! Get media pointer, Fill and send data */
			Uint8* pbDataLocation;
			if( FEF_FAILED(pIFxMedia->GetMediaPointer(&pbDataLocation) ) ) {
		    	/*! Release Media buffer */
		    	pIFxMedia->Release();
		    	_pFxState->FxPublishState(FX_ERROR_PIN_STATE);
		    	return;
			}
			/*! Connect output port */
			(*ItOutPin)->ConnectPort(_psDescriptor, _CurrentInstanceHandle, (LADSPA_Data *)pbDataLocation);

			Uint32 dwBufferSize;
			pIFxMedia->GetSize(&dwBufferSize);
			dwSizeRequest = (dwSizeRequest < dwBufferSize) ? dwSizeRequest : dwBufferSize;
			
			/*! Set media properties */
			FxPcmFmt.wChannels = 1;			
			FxPcmFmt.FormatTag = FLOAT32_TYPE;			
			FxPcmFmt.dwSamplingRate = _dwCurrentFs;
			pIFxPcmFormat->SetPcmFormat(&FxPcmFmt);
			pIFxMedia->SetFxMediaName((*ItOutPin)->GetPortName());
		}
		/*! Call LADSPA */
		dwSizeRequest -= dwSizeRequest%sizeof(LADSPA_Data); //!< Must be modulus sample size
		Uint32 dwSampleToSend = dwSizeRequest/sizeof(LADSPA_Data);
		if(_psDescriptor->run)
			_psDescriptor->run(_CurrentInstanceHandle, dwSampleToSend);

		/*! Update dwReqTimeStamp */
		dwReqTimeStamp -= dwSampleToSend;

		/*! Send IFxMedia */
		for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++) {
			pIFxMedia = (*ItOutPin)->GetIFxMedia();
			pIFxMedia->SetDataLength(dwSizeRequest);
			/*! Deliver the FxMedia to the next Fx */
			(*ItOutPin)->GetIFxPin()->DeliverMedia(pIFxMedia);	
			/*! Update Req sample */
			(*ItOutPin)->UpdateTimeStamp(dwSampleToSend);
		}
	}
	return;
}

/*-----------------------------------------------------------------------------*//*!
	DoDst()
	Apply LADSPA destination plugin.  (Has Input pin(s) only)
*//*-----------------------------------------------------------------------------*/
Void CLADSPAWrapper::DoDst(Uint32 dwInputSize)
{
	dwInputSize -= dwInputSize%sizeof(LADSPA_Data); //!< Must be modulus sample size
	Uint32 dwSampleNumber = dwInputSize/sizeof(LADSPA_Data);
	/*! Call LADSPA */
	if(_psDescriptor->run)
		_psDescriptor->run(_CurrentInstanceHandle, dwSampleNumber);

	/*! Send request timestamp (i.e. the same received timestamp) */
	std::vector<CInAudioPort*>::iterator ItInPin;
	for(ItInPin = _vectorInAudio.begin(); ItInPin != _vectorInAudio.end(); ItInPin++) {
		(*ItInPin)->GetIFxPin()->WaitForIFxMedia(dwSampleNumber, NULL);
		(*ItInPin)->Rewind(dwInputSize);
	}	
	
	return;
}

/*-----------------------------------------------------------------------------*//*!
	DoProc()
	Apply LADSPA procesing plugin.  (Has Output pin(s) and Input pin(s))
*//*-----------------------------------------------------------------------------*/
Void CLADSPAWrapper::DoProc(Uint32 dwInputSize, Uint32 dwReqTimeStamp)
{
	std::vector<COutAudioPort*>::iterator ItOutPin;
	std::vector<CInAudioPort*>::iterator ItInPin;
	if(dwReqTimeStamp > 0) {
		/*! Send request timestamp (i.e. the same received timestamp) */
		for(ItInPin = _vectorInAudio.begin(); ItInPin != _vectorInAudio.end(); ItInPin++) {
			(*ItInPin)->GetIFxPin()->WaitForIFxMedia(dwReqTimeStamp, NULL);
		}	
		/*! Reset request timestamp */
		for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++) {
			(*ItOutPin)->UpdateTimeStamp(dwReqTimeStamp);
		}
	}
	dwInputSize -= dwInputSize%sizeof(LADSPA_Data); //!< Must be modulus sample size
	IFxMedia* pIFxMedia;
	Uint32 dwSizeToprocess;
	Int64 qCurrentReqTimeStamp;
	/*! Send Request TimeStamp */
	while(dwInputSize && !_ShouldCloseThread) {
		dwSizeToprocess = dwInputSize;
		qCurrentReqTimeStamp = -1;
		/*! Get Dst IFxMedia to send */
		for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++) {
			/*! Get requeste sample during loop processing */
			qCurrentReqTimeStamp = (*ItOutPin)->GetReqTimeStamp(qCurrentReqTimeStamp);
			
			/*! Get new FxMedia to fill */
			if( FEF_FAILED((*ItOutPin)->GetDeliveryMedia()) ) {/*!< The Media type of connection is set !! */  
			    _pFxState->FxPublishState(FX_UNDERRUN_STATE);
		    	return;
			}
			pIFxMedia = (*ItOutPin)->GetIFxMedia();

			/*! Get Media PCM format */
			IFxPcmFormat* pIFxPcmFormat;
			if(FEF_FAILED(pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
			/*! Release Media buffer */
		    	pIFxMedia->Release();
				_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
				return;
			}
			FX_PCM_FORMAT FxPcmFmt;

			/*! Get media pointer, Fill and send data */
			Uint8* pbDataLocation;
			if( FEF_FAILED(pIFxMedia->GetMediaPointer(&pbDataLocation) ) ) {
		    	/*! Release Media buffer */
		    	pIFxMedia->Release();
		    	_pFxState->FxPublishState(FX_ERROR_PIN_STATE);
		    	return;
			}
			/*! Connect output port */
			(*ItOutPin)->ConnectPort(_psDescriptor, _CurrentInstanceHandle, (LADSPA_Data *)pbDataLocation);

			Uint32 dwBufferSize;
			pIFxMedia->GetSize(&dwBufferSize);
			dwSizeToprocess = (dwSizeToprocess < dwBufferSize) ? dwSizeToprocess : dwBufferSize;
			
			/*! Set media properties */
			FxPcmFmt.wChannels = 1;			
			FxPcmFmt.FormatTag = FLOAT32_TYPE;			
			FxPcmFmt.dwSamplingRate = _dwCurrentFs;
			pIFxPcmFormat->SetPcmFormat(&FxPcmFmt);
			pIFxMedia->SetFxMediaName((*ItOutPin)->GetPortName());
		}
		/*! Call LADSPA */
		dwSizeToprocess -= dwSizeToprocess%sizeof(LADSPA_Data); //!< Must be modulus sample size
		Uint32 dwSampleToSend = dwSizeToprocess/sizeof(LADSPA_Data);
		if(_psDescriptor->run){
			_psDescriptor->run(_CurrentInstanceHandle, dwSampleToSend); //!< We could call run_adding instead 
		}

		/*! Update dwReqTimeStamp */
		dwInputSize -= dwSizeToprocess;

		/*! Send IFxMedia */
		for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++) {
			(*ItOutPin)->GetIFxMedia()->SetDataLength(dwSizeToprocess);
			/*! Deliver the FxMedia to the next Fx */
			(*ItOutPin)->GetIFxPin()->DeliverMedia((*ItOutPin)->GetIFxMedia());	
			if(qCurrentReqTimeStamp > 0) {
				(*ItOutPin)->UpdateTimeStamp(qCurrentReqTimeStamp);
			}
		}
		/*! Rewind input data */
		for(ItInPin = _vectorInAudio.begin(); ItInPin != _vectorInAudio.end(); ItInPin++) {
			(*ItInPin)->Rewind(dwSizeToprocess);
			if(qCurrentReqTimeStamp > 0) {
				(*ItInPin)->GetIFxPin()->WaitForIFxMedia(qCurrentReqTimeStamp, NULL);
			}
		}
	}
			
	return;
}

/*-----------------------------------------------------------------------------*//*!
	ThreadFct()
	Audio thread processing.
*//*-----------------------------------------------------------------------------*/
Void* CLADSPAWrapper::ThreadFct()
{
	unsigned long dwInFs;
	Int64 qInputSize;
	Int64 qReqTimeStamp;
	Bool ShouldFlush;
	Bool ShouldChangeStreamFormat;
	std::vector<CInAudioPort*>::iterator ItInPin;
	std::vector<COutAudioPort*>::iterator ItOutPin;
	std::map<int, CLADSPAControl*>::iterator ItCtrl;

	pthread_mutex_lock(&_ThreadMutex);
	while(!_ShouldCloseThread)
	{
		/*! Initialize loop values */
		qInputSize = -1;
		qReqTimeStamp = -1;
		if(_vectorInAudio.size() != 0) { //! Apply on when input is not null
			ShouldFlush = TRUE;
			ShouldChangeStreamFormat = TRUE;
		}
		else {
			ShouldFlush = FALSE;
			ShouldChangeStreamFormat = FALSE;
		}
		
		/*! Input pins processing */
		dwInFs = _dwCurrentFs;
		
		for(ItInPin = _vectorInAudio.begin(); ItInPin != _vectorInAudio.end(); ItInPin++) {
			 dwInFs = (*ItInPin)->GetCurrentFs(); //!< We suppose that all input pins receive the same Fs
			 qInputSize = (*ItInPin)->GetInputSize(qInputSize); //!< Get common input size
			 ShouldFlush &= (*ItInPin)->HasReceivedFlush();
			 ShouldChangeStreamFormat &= (*ItInPin)->HasReceivedInitStream();
		}
		if( (dwInFs != _dwCurrentFs) || ShouldChangeStreamFormat) //!< Get right instance according to Fs
			UpdateStreamFormat(dwInFs);
		
		/*! Output pins processing */
		for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++) {
			qReqTimeStamp = (*ItOutPin)->GetReqTimeStamp(qReqTimeStamp);
		}
		
		if( qInputSize < 1 && qReqTimeStamp < 1 ) {//!< Waiting for data
			/*! Has to flush ? */
			if(ShouldFlush == TRUE) { //! must have at least one input pin
				for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++)
					(*ItOutPin)->GetIFxPin()->Flush();
				for(ItInPin = _vectorInAudio.begin(); ItInPin != _vectorInAudio.end(); ItInPin++)
					(*ItInPin)->Reset();
				_pFxState->FxPublishState(FX_FLUSH_STATE);		
			} 
			pthread_cond_wait (&_ThreadCond, &_ThreadMutex);
		}
		else {
			pthread_mutex_unlock(&_ThreadMutex);
			if(_LadspaType == LADSPA_SRC)
				DoSource(qReqTimeStamp);
			else if (_LadspaType == LADSPA_DST)
				DoDst(qInputSize);
			else {
				DoProc(qInputSize, qReqTimeStamp);
			}
			pthread_mutex_lock(&_ThreadMutex);
		}
	}
	pthread_mutex_unlock(&_ThreadMutex);
	return NULL;
}

/*-----------------------------------------------------------------------------*//*!
	UpdateFs()
	Set new Fs.
*//*-----------------------------------------------------------------------------*/
Int32 CLADSPAWrapper::UpdateStreamFormat(unsigned long dwFs)
{
	Int32 hr;
	std::vector<COutAudioPort*>::iterator ItOutPin;
	std::map<int, CLADSPAControl*>::iterator ItCtrl;
	
	if(dwFs != _dwCurrentFs) {
		/*! Update plugin */
		if(_psDescriptor->deactivate)
			_psDescriptor->deactivate(_CurrentInstanceHandle);
		_CurrentInstanceHandle = _mapInstanceHandle[dwFs];
		if(_psDescriptor->activate)
			_psDescriptor->activate(_CurrentInstanceHandle);
		ItCtrl = _mapControl.begin();
		while(ItCtrl != _mapControl.end())
		{
			(ItCtrl->second)->SetRange(_psDescriptor, dwFs);
			ItCtrl++;
		}
		ControlCombo();
		_dwCurrentFs = dwFs;
	}
	
	for(ItOutPin = _vectorOutAudio.begin(); ItOutPin != _vectorOutAudio.end();ItOutPin++) {
		/*! Get new FxMedia to fill */
		IFxMedia* pIFxMedia;
		if( FEF_FAILED(hr = (*ItOutPin)->GetDeliveryMedia()) ) /*!< The Media type of connection is set !! */  
		{
		    _pFxState->FxPublishState(FX_ERROR_STATE);
		    return hr;
		}
		pIFxMedia = (*ItOutPin)->GetIFxMedia();
		/*! Get Media PCM format */
		IFxPcmFormat* pIFxPcmFormat;
		if(FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
			/*! Release Media buffer */
		    pIFxMedia->Release();
			_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
			return hr;
		}	
		pIFxMedia->SetDataLength(0);
		/*! Set media properties */
		FX_PCM_FORMAT FxPcmFmt;
		FxPcmFmt.wChannels = 1;			
		FxPcmFmt.FormatTag = FLOAT32_TYPE;			
		FxPcmFmt.dwSamplingRate = dwFs;
		pIFxPcmFormat->SetPcmFormat(&FxPcmFmt);
		pIFxMedia->SetFxMediaName("NULL");

		/*! Deliver the FxMedia to the next Fx */
		(*ItOutPin)->GetIFxPin()->InitStream((*ItOutPin)->GetIFxMedia());
	}
	_pFxState->FxPublishState(FX_STREAM_INIT_STATE);
		
}

CLADSPAControl::CLADSPAControl(const std::string strControlName, unsigned long lIndexPort, Bool IsReadOnly):
_strControlName(strControlName),
_lIndexPort(lIndexPort),
_IsReadOnly(IsReadOnly)
{}

/*-----------------------------------------------------------------------------*//*!
	SetRange()
	Set LADSPA control range.
*//*-----------------------------------------------------------------------------*/
Void CLADSPAControl::SetRange(const LADSPA_Descriptor *pLADSPADescriptor, unsigned long lFs)
{
	/*! Get min, max and default */
    _fControlMin = gfMin;
    _fControlMax = gfMax;
    _fControlDefault = gfDefault;
    LADSPA_PortRangeHintDescriptor iHintDescriptor;
    iHintDescriptor = pLADSPADescriptor->PortRangeHints[_lIndexPort].HintDescriptor;
	
    if (LADSPA_IS_HINT_BOUNDED_BELOW(iHintDescriptor))
	    _fControlMin = pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound;
    else
        _fControlMin = gfMin;
       	  
    if (LADSPA_IS_HINT_BOUNDED_ABOVE(iHintDescriptor))
        _fControlMax = pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound;
    else
        _fControlMax = gfMax;
	if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor)) {
		_fControlMin *= lFs;
		_fControlMax *= lFs;
	}
	
	switch (iHintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
	case LADSPA_HINT_DEFAULT_NONE:
	  break;
	case LADSPA_HINT_DEFAULT_MINIMUM:
	  _fControlDefault = pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound;
	  break;
	case LADSPA_HINT_DEFAULT_LOW:
	  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
	    _fControlDefault 
	      = exp(log(pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound) 
		    * 0.75f
		    + log(pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound) 
		    * 0.25f);
	  }
	  else {
	    _fControlDefault 
	      = (pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound
		 * 0.75f
		 + pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound
		 * 0.25f);
	  }
	  if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
		_fControlDefault *= lFs;
      break;
	case LADSPA_HINT_DEFAULT_MIDDLE:
	  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
	    _fControlDefault 
	      = sqrt(pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound
		     * pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound);
	  }
	  else {
	    _fControlDefault 
	      = 0.5f * (pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound
		       + pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound);
      }
	  if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
		_fControlDefault *= lFs;
	  break;
	case LADSPA_HINT_DEFAULT_HIGH:
	  if (LADSPA_IS_HINT_LOGARITHMIC(iHintDescriptor)) {
	    _fControlDefault 
	      = exp(log(pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound) 
		    * 0.25f
		    + log(pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound) 
		    * 0.75f);
	  }
	  else {
	    _fControlDefault 
	      = (pLADSPADescriptor->PortRangeHints[_lIndexPort].LowerBound
		 * 0.25f
		 + pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound
		 * 0.75f);
	  }
	  if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
		_fControlDefault *= lFs;
	  break;
	case LADSPA_HINT_DEFAULT_MAXIMUM:
	  _fControlDefault = pLADSPADescriptor->PortRangeHints[_lIndexPort].UpperBound;
	  if (LADSPA_IS_HINT_SAMPLE_RATE(iHintDescriptor))
		_fControlDefault *= lFs;
	  break;
    case LADSPA_HINT_DEFAULT_0:
	  _fControlDefault = 0;
	  break;
	case LADSPA_HINT_DEFAULT_1:
	  _fControlDefault = 1;
	  break;
	case LADSPA_HINT_DEFAULT_100:
	  _fControlDefault = 100;
	  break;
	case LADSPA_HINT_DEFAULT_440:
	  _fControlDefault = 440;
	  break;
	default:
	  break;
	}
    _fControlValue = _fControlDefault;
	return;
}

/*-----------------------------------------------------------------------------*//*!
	ConnectPort()
	Connect LADSPA control port.
*//*-----------------------------------------------------------------------------*/
Void CLADSPAControl::ConnectPort(const LADSPA_Descriptor *pLADSPADescriptor, LADSPA_Handle pvLADSPAHandle)
{
	/*! Connect to port */
	if(pLADSPADescriptor->connect_port)
		pLADSPADescriptor->connect_port(pvLADSPAHandle, _lIndexPort, &_fControlValue);
	return;
}

/*-----------------------------------------------------------------------------*//*!
	RemovePin()
	Remove Fx audio pin.
*//*-----------------------------------------------------------------------------*/
Int32 CAudioPort::RemovePin(IFx* pFx)
{
	/*! Get the IFxPinManager to create the Fx pins */
	IFxPinManager* pFxPinManager = NULL;
	Int32 hr = pFx->FxGetInterface(IFX_PINMANGER, (Void**)&pFxPinManager);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	pFxPinManager->Remove(_pFxPin);
	/*! We can release interfaces */
	SAFE_RELEASE_INTERFACE(pFxPinManager);
	SAFE_RELEASE_INTERFACE(_pFxState);
	return FX_OK;
}

CInAudioPort::CInAudioPort(const std::string strPortName, unsigned long lIndexPort, unsigned long dwFs, pthread_mutex_t* pThreadMutex, pthread_cond_t* pThreadCond):
CAudioPort(strPortName, lIndexPort, dwFs, pThreadMutex, pThreadCond)
{
	/*! Initialize values */
	_dwWriteIdx = 0;
	memset(_pfInputData, 0, INPUT_BUFFER_SIZE * sizeof(LADSPA_Data));
}

/*-----------------------------------------------------------------------------*//*!
	CreatePin()
	Create Fx audio Input pin.
*//*-----------------------------------------------------------------------------*/
Int32 CInAudioPort::CreatePin(IFx* pFx)
{
	/*! Get the IFxState and publish the loading state */
	Int32 hr = pFx->FxGetInterface(IFX_STATE, (Void**)&_pFxState);
	if(FEF_FAILED(hr))
		return hr;
	
	/*! Get the IFxPinManager to create the Fx pins */
	IFxPinManager* pFxPinManager = NULL;
	hr = pFx->FxGetInterface(IFX_PINMANGER, (Void**)&pFxPinManager);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	FX_PIN _FxPin =
	{	_strPortName			//!< Pin Name
		, PIN_IN				//!< Pin Type
		, &gPinMediatype		//!< Media Type
		, 1						//!< Media Type number
		, (IFxPinCallback*)this	//!< Pin Callback
	};

	/*! Create the Fx pin */
	hr = pFxPinManager->Create(&_FxPin, &_pFxPin);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(pFxPinManager);
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! We can release interfaces */
	SAFE_RELEASE_INTERFACE(pFxPinManager);
	
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	Reset()
	Reset pin internal buffer.
*//*-----------------------------------------------------------------------------*/
Void CInAudioPort::Reset()
{
	/*! Initialize value */
	_dwWriteIdx = 0;
	_HasReceivedInitStream = FALSE;
	_HasReceivedFlush = FALSE;
	memset(_pfInputData, 0, INPUT_BUFFER_SIZE * sizeof(LADSPA_Data));
}

/*-----------------------------------------------------------------------------*//*!
	Rewind()
	Rewind internal buffer.
*//*-----------------------------------------------------------------------------*/
Void CInAudioPort::Rewind(Uint32 dwRewindSize)
{
	CAutoLock Lock(_pThreadMutext);
	_dwWriteIdx -= dwRewindSize;
	memmove(_pfInputData, _pfInputData + dwRewindSize, _dwWriteIdx);
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxMedia()
	Decides the Media buffer size and number.
*//*-----------------------------------------------------------------------------*/
Void CInAudioPort::FxMedia( IFxPin* pFxPin, Uint32* pdwFxMediaSize, Uint32* pdwFxMediaNumber)
{
	/*! Define 10 buffers of 50ms (44100, stereo, 32 bits float pcm data) */ 
	*pdwFxMediaNumber = 10;
	*pdwFxMediaSize = (44100 * 4 * 2)/20;
		
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxPin()
	Fx input pin callback.
*//*-----------------------------------------------------------------------------*/
Int32 CInAudioPort::FxPin(/* [in] */IFxPin* pFxPin,
						/* [in] */IFxMedia* pIFxMedia,
						/* [in] */FX_STREAM_STATE StreamState)
{
	CAutoLock Lock(_pThreadMutext);
	Int32 hr;
	_HasReceivedInitStream = FALSE;
	_HasReceivedFlush = FALSE;
	if(pIFxMedia != NULL)
	{
		/*! have we new format */
		if(StreamState == STREAM_INIT){
			_HasReceivedInitStream = TRUE;
			/*! Check the FxMedia type */
			if(FEF_FAILED(hr = pIFxMedia->CheckMediaType(&gPinMediatype)) )
			{
				_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
				return FX_MEDIANOTSUPPORTED;
			}
		
			/*! Check the FxMedia PCM format */
			IFxPcmFormat* pFxPcmFormat;
			if( FEF_FAILED(hr = pIFxMedia->GetFormatInterface(PCM, (Void**)&pFxPcmFormat)))
				return hr;
			
			FX_PCM_FORMAT FxPcmFmt;
			pFxPcmFormat->GetPcmFormat(&FxPcmFmt);

			/*! Accept uncompressed Float audio sample only */
			if( FxPcmFmt.FormatTag != FLOAT32_TYPE )
			{
				_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
				return FX_FMTNOTSUPPORTED;
			}

			/*! Accept sampling rates 8000,22050,44100,48000 and 96000*/
			if( (FxPcmFmt.dwSamplingRate != 8000) && 
				(FxPcmFmt.dwSamplingRate != 22050) &&
				(FxPcmFmt.dwSamplingRate != 44100) &&
				(FxPcmFmt.dwSamplingRate != 48000) &&
				(FxPcmFmt.dwSamplingRate != 96000) )
			{
				_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
				return FX_FMTNOTSUPPORTED;
			}
				
			/*! Save current Fs if needed */
			if(_dwCurrentFs != FxPcmFmt.dwSamplingRate) {
				Reset();
				_dwCurrentFs = FxPcmFmt.dwSamplingRate;
			}
			/*! LADSPA has one channel only per port */
			if(FxPcmFmt.wChannels != 1)
			{
				_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
				return FX_FMTNOTSUPPORTED;
			}
			_pFxState->FxPublishState(FX_STREAM_INIT_STATE);
		}		
		/*! Get length and data address */
		Uint32 dwSrcDataLength = 0;
		Uint8* pDataSrc;
		pIFxMedia->GetDataLength(&dwSrcDataLength);
		pIFxMedia->GetMediaPointer(&pDataSrc);
		/*! Get size to copy */
		Uint32 dwDataLengthToCopy = ((_dwWriteIdx + dwSrcDataLength) > INPUT_BUFFER_SIZE) ? INPUT_BUFFER_SIZE - _dwWriteIdx : dwSrcDataLength; 
		if( dwDataLengthToCopy < dwSrcDataLength)
			_pFxState->FxPublishState(FX_OVERRUN_STATE);
		/*! Save data */
		memcpy(_pfInputData + _dwWriteIdx, pDataSrc, dwDataLengthToCopy);
		//memset(_pfInputData + _dwWriteIdx, 0, dwDataLengthToCopy);
		_dwWriteIdx += dwDataLengthToCopy;
	}

	/*! Perform the Echo flushing */
	if(StreamState == STREAM_FLUSH) {
		_HasReceivedFlush = TRUE;
	}
	
	pthread_cond_signal(_pThreadCond);
	
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ConnectPort()
	Connect input audio pin.
*//*-----------------------------------------------------------------------------*/
Void CInAudioPort::ConnectPort(const LADSPA_Descriptor *pLADSPADescriptor, Void* pvLADSPAHandle)
{
	/*! Connect to port */
	if(pLADSPADescriptor->connect_port)
		pLADSPADescriptor->connect_port(pvLADSPAHandle, _lIndexPort, _pfInputData);
	return;
}

COutAudioPort::COutAudioPort(const std::string strPortName, unsigned long lIndexPort, unsigned long dwFs, pthread_mutex_t* pThreadMutex, pthread_cond_t* pThreadCond):
CAudioPort(strPortName, lIndexPort, dwFs, pThreadMutex, pThreadCond)
{
	/*! Initialize value */
	_dwReqTimeStamp = 0;
}

/*-----------------------------------------------------------------------------*//*!
	CreatePin()
	Create Fx audio Output pin.
*//*-----------------------------------------------------------------------------*/
Int32 COutAudioPort::CreatePin(IFx* pFx)
{
	/*! Get the IFxState and publish the loading state */
	Int32 hr = pFx->FxGetInterface(IFX_STATE, (Void**)&_pFxState);
	if(FEF_FAILED(hr))
		return hr;
	
	/*! Get the IFxPinManager to create the Fx pins */
	IFxPinManager* pFxPinManager = NULL;
	hr = pFx->FxGetInterface(IFX_PINMANGER, (Void**)&pFxPinManager);
	if(FEF_FAILED(hr))
	{
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	FX_PIN _FxPin =
	{	_strPortName			//!< Pin Name
		, PIN_OUT				//!< Pin Type
		, &gPinMediatype		//!< Media Type
		, 1						//!< Media Type number
		, (IFxPinCallback*)this	//!< Pin Callback
	};

	/*! Create the Fx pin */
	hr = pFxPinManager->Create(&_FxPin, &_pFxPin);
	if(FEF_FAILED(hr))
	{
		SAFE_RELEASE_INTERFACE(pFxPinManager);
		_pFxState->FxPublishState(FX_ERROR_INIT_STATE);
		return hr;
	}

	/*! We can release interfaces */
	SAFE_RELEASE_INTERFACE(pFxPinManager);
	
	return FX_OK;
}

/*-----------------------------------------------------------------------------*//*!
	ConnectPort()
	Connect output audio pin.
*//*-----------------------------------------------------------------------------*/
Void COutAudioPort::ConnectPort(const LADSPA_Descriptor *pLADSPADescriptor, Void* pvLADSPAHandle, LADSPA_Data *pDataLocation)
{
	/*! Connect to port */
	if(pLADSPADescriptor->connect_port)
		pLADSPADescriptor->connect_port(pvLADSPAHandle, _lIndexPort, pDataLocation);
	return;
}

/*-----------------------------------------------------------------------------*//*!
	FxPinState()
	Pin state callback.
*//*-----------------------------------------------------------------------------*/
Void COutAudioPort::FxPinState(IFxPin* pFxPin, FX_PIN_STATE PinState)
{
	/*! Initialize downstream on connexion only */
	if(PinState == PIN_CONNECTED) {
		
		/*! Get new FxMedia to fill */
		IFxMedia* pIFxMedia;
		if( FEF_FAILED(_pFxPin->GetDeliveryMedia(&pIFxMedia)) ) /*!< The Media type of connection is set !! */  
		{
			_pFxState->FxPublishState(FX_ERROR_STATE);
			return;
		}

		/*! Get Media PCM format */
		IFxPcmFormat* pIFxPcmFormat;
		if(FEF_FAILED(pIFxMedia->GetFormatInterface(PCM, (Void**)&pIFxPcmFormat))) {
			/*! Release Media buffer */
			pIFxMedia->Release();
			_pFxState->FxPublishState(FX_ERROR_MEDIA_PIN_STATE);
			return;
		}	
		pIFxMedia->SetDataLength(0);
		/*! Set media properties */
		FX_PCM_FORMAT FxPcmFmt;
		FxPcmFmt.wChannels = 1;			
		FxPcmFmt.FormatTag = FLOAT32_TYPE;			
		FxPcmFmt.dwSamplingRate = _dwCurrentFs;
		pIFxPcmFormat->SetPcmFormat(&FxPcmFmt);
		pIFxMedia->SetFxMediaName("Stream connexion");

		/*! Deliver the FxMedia to the next Fx */
		_pFxPin->InitStream(pIFxMedia);	
	}

	return;
}

Int64 COutAudioPort::GetReqTimeStamp(Int64 qPrevReqTimeStamp)
{
	if(qPrevReqTimeStamp < 0 || qPrevReqTimeStamp == 0)
		return _dwReqTimeStamp;
	else { //!< Return the min greater than 0
		if (_dwReqTimeStamp == 0)
			return qPrevReqTimeStamp;
		else
			return (qPrevReqTimeStamp < _dwReqTimeStamp) ? qPrevReqTimeStamp : _dwReqTimeStamp;
	} 
}

