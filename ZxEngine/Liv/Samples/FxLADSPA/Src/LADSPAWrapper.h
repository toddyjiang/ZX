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
   @file  LADSPAWrapper.h
   @brief The Fx LADSPA allows to load and to use LADSPA plugins. 
   This Fx is for demonstration purposes only and does not represent
   the quality of SMProcess commercial products. 
   This Fx is provided as an example in the FxEngine Framework.
*//*_______________________________________________________________________*/
#pragma once

/*! From ladspa sdk */
#include "ladspa.h"
#include "utils.h"

#include "IFxBase.h"
using namespace FEF; //!< Using FxEngineFramework

#include <map>
#include <vector>
using namespace std;

#include <pthread.h>

extern unsigned long glDefaultFs;
#define INPUT_BUFFER_SIZE (176400) /*! We are able to save temporary 1s of data (Mono, 32 bits, 44100Hz) */
//! LADSPA types
typedef enum _LADSPA_TYPE {
    LADSPA_SRC = 0,		//!< Source type
	LADSPA_PRC,			//!< Process type
	LADSPA_DST			//!< Dest type
} LADSPA_TYPE;

class CAutoLock
{
public:
	CAutoLock(pthread_mutex_t* pThreadMutex){_pThreadMutex = pThreadMutex; pthread_mutex_lock(_pThreadMutex);}
	~CAutoLock(void){pthread_mutex_unlock(_pThreadMutex);}
private:
	pthread_mutex_t* _pThreadMutex;
};

class CAudioPort
{
public:
	CAudioPort(const std::string strPortName, unsigned long lIndexPort, unsigned long dwFs, pthread_mutex_t* pThreadMutex, pthread_cond_t* pThreadCond)
			  {_strPortName = strPortName; _lIndexPort = lIndexPort; _dwCurrentFs = dwFs; _pThreadMutext = pThreadMutex; _pThreadCond = pThreadCond;}
	virtual ~CAudioPort(void){};
protected:
    std::string _strPortName;
	unsigned long _lIndexPort;
	IFxPin* _pFxPin;			/*!< The Fx pin */
	IFxState* _pFxState;        /*!< The Fx state */
	pthread_mutex_t* _pThreadMutext;
	pthread_cond_t*  _pThreadCond;
	unsigned long _dwCurrentFs;
public:
	Int32 RemovePin(IFx* pFx);
	std::string GetPortName() {return _strPortName;}
	IFxPin* GetIFxPin() {return _pFxPin;}
	unsigned long GetCurrentFs() {return _dwCurrentFs;}
};

class CInAudioPort : public IFxPinCallback,
					 public CAudioPort
{
public:
    CInAudioPort(const std::string strControlName, unsigned long lIndexPort, unsigned long dwFs, pthread_mutex_t* pThreadMutex, pthread_cond_t* pThreadCond);
	virtual ~CInAudioPort(void){};
public:
	virtual Void FxPinState(	/* [in] */IFxPin* pFxPin,
		/* [in] */FX_PIN_STATE PinState) {Reset();} 
	virtual Int32 FxPin(/* [in] */IFxPin* pFxPin,
						/* [in] */IFxMedia* pIFxMedia,
						/* [in] */FX_STREAM_STATE StreamState);
	virtual Void FxMedia(/* [in] */IFxPin* pFxPin, /* [in/out] */Uint32* pdwFxMediaSize, /* [in/out] */Uint32* pdwFxMediaNumber);
private:
    LADSPA_Data _pfInputData[INPUT_BUFFER_SIZE];
	Uint32 _dwWriteIdx;
	Bool _HasReceivedFlush;
	Bool _HasReceivedInitStream;
public:
	Void ConnectPort(const LADSPA_Descriptor *pLADSPADescriptor, void* pvLADSPAHandle);
	Void Reset();
	Void Rewind(Uint32 dwRewindSize);
	Int64 GetInputSize(Int64 qPrevInputSize) {if(qPrevInputSize < 0) return _dwWriteIdx; else return (qPrevInputSize < _dwWriteIdx) ? qPrevInputSize : _dwWriteIdx;}
	Int32 CreatePin(IFx* pFx);
	Bool HasReceivedFlush() {return _HasReceivedFlush;}
	Bool HasReceivedInitStream() {return _HasReceivedInitStream;} 
};

class COutAudioPort : public IFxPinCallback,
					  public CAudioPort
{
public:
    COutAudioPort(const std::string strControlName, unsigned long lIndexPort, unsigned long dwFs, pthread_mutex_t* pThreadMutex, pthread_cond_t* pThreadCond);
	virtual ~COutAudioPort(void){};
public:
	virtual Void FxWaitForIFxMedia(/* [in] */IFxPin* pFxPin,
									/* [in] */Uint32 dwTimeStamp,
									/* [in] */FX_PTR dwUser) {CAutoLock Lock(_pThreadMutext); _dwReqTimeStamp += dwTimeStamp; pthread_cond_signal (_pThreadCond);}
	virtual Void FxPinState(	/* [in] */IFxPin* pFxPin,
		/* [in] */FX_PIN_STATE PinState);
private:
    Uint32 _dwReqTimeStamp;
	IFxMedia* _pIFxMedia;
public:
	Int32 CreatePin(IFx* pFx);
	Void Reset() {CAutoLock Lock(_pThreadMutext); _dwReqTimeStamp = 0; return;}
	Int64 GetReqTimeStamp(Int64 qPrevReqTimeStamp);
	Void UpdateTimeStamp(Uint32 dwTimeStamp) {CAutoLock Lock(_pThreadMutext); _dwReqTimeStamp -= (_dwReqTimeStamp < dwTimeStamp) ? _dwReqTimeStamp : dwTimeStamp; return; }
	Void ConnectPort(const LADSPA_Descriptor *pLADSPADescriptor, Void* pvLADSPAHandle, LADSPA_Data *pDataLocation);
	Int32 GetDeliveryMedia() {return _pFxPin->GetDeliveryMedia(&_pIFxMedia, INFINITE_TIME);}
	IFxMedia* GetIFxMedia() {return _pIFxMedia;}
};

class CLADSPAControl
{
public:
    CLADSPAControl(const std::string strControlName, unsigned long lIndexPort, Bool IsReadOnly = FALSE);
    virtual ~CLADSPAControl(void){};
private:
    std::string _strControlName;
	unsigned long _lIndexPort;
    float _fControlValue;
    float _fControlMin;
    float _fControlMax;
    float _fControlDefault;
	Bool _IsReadOnly;
public:
	Void SetRange(const LADSPA_Descriptor *pLADSPADescriptor, unsigned long lFs);
	Void ConnectPort(const LADSPA_Descriptor *pLADSPADescriptor, LADSPA_Handle pvLADSPAHandle);
    std::string GetControlName() {return _strControlName;}
    float GetValue() {return _fControlValue;}
    float GetMin() {return _fControlMin;}
    float GetMax() {return _fControlMax;}
    float GetDefault() {return _fControlDefault;}
    Void SetValue(float fControlValue) {_fControlValue = fControlValue;}
	Bool IsReadOnly() {return _IsReadOnly;}
};

class CLADSPAWrapper
{
public:
    CLADSPAWrapper(IFx *pFx);
    virtual ~CLADSPAWrapper(void);

public:
    Void ShowUI() {gtk_widget_show_all(_pWindow); return;}
    static void ChooseLADSPAWrapper(GtkButton *button, CLADSPAWrapper* pUser) {pUser->ChooseLADSPA(); return;}
    Void ChooseLADSPA();
    static gint HideWrapper(GtkWidget *widget, GdkEvent *event, CLADSPAWrapper* pUser) {return pUser->Hide();}
    gint Hide();
    static void ControlEditWrapper(GtkWidget *widget, CLADSPAWrapper* pUser) {return pUser->ControlEdit();}
    Void ControlEdit();
    static void ControlSliderWrapper(GtkWidget *widget, CLADSPAWrapper* pUser) {return pUser->ControlSlider();}
    Void ControlSlider();
    static void ControlComboWrapper(GtkWidget *widget, CLADSPAWrapper* pUser) {return pUser->ControlCombo();}
    Void ControlCombo();
    
    Int32 LoadLADSPA(const char* strLADSPAFile, const char* strPluginLabel = NULL, Uint32 dwFs = glDefaultFs);

	Int32 StartWrapper();
	Int32 StopWrapper();
	
private:
    Void ResetLADSPA();

private:
	GtkWidget* _pWindow;
    GtkWidget* _pEntryPath;
    GtkWidget* _pLabelName;
    GtkWidget* _pLabelAuthor;
    GtkWidget* _pComboControl;
    GtkWidget* _pEntryControl;
    GtkWidget* _pSliderControl;
    gulong _SliderId;
    gulong _EntryId;

private:
	IFx *_pFx;
	IFxState* _pFxState;
    
private:
    Void* _pvPluginHandle;
	std::map<unsigned long, LADSPA_Handle> _mapInstanceHandle;
	LADSPA_Handle _CurrentInstanceHandle;
	unsigned long _dwCurrentFs;
    LADSPA_Descriptor const *_psDescriptor;
    std::map<int, CLADSPAControl*> _mapControl;
	std::vector<CInAudioPort*> _vectorInAudio;
	std::vector<COutAudioPort*> _vectorOutAudio;
	LADSPA_TYPE _LadspaType;

private:
	pthread_t _Thread;
	Bool _ShouldCloseThread;
	pthread_mutex_t _ThreadMutex;
	pthread_cond_t  _ThreadCond;
	Void DoSource(Uint32 dwReqSize);
	Void DoDst(Uint32 dwInputSize);
	Void DoProc(Uint32 dwInputSize, Uint32 dwReqSize);
	Int32 UpdateStreamFormat(unsigned long dwFs);

public:
	static void *ThreadFctWrapper(Void *p_data) { return ((CLADSPAWrapper*)p_data)->ThreadFct(); }
	Void* ThreadFct();
};


