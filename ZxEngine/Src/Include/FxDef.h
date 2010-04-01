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

   ____________________________________________http://www.SMProcess.com______
*//*!
   @file	 FxDef.h
   @brief This file contains the main definitions of the FxEngine Framework.
*//*_______________________________________________________________________*/
#pragma once

//! STL definition
#include <string>
using namespace std;

#ifdef WIN32
#include <io.h>
#endif
#include <stdlib.h>
#include <stdio.h>

//! FxEngine Framework definitions
#include "FxTypes.h"
#include "FxMediaTypes.h"
#include "FxErr.h"

namespace FEF {

//! FxEngine Framework export definition
#ifdef WIN32
#define FXENGINE_EXP __declspec(dllexport)
#else
#define FXENGINE_EXP
#endif

//! FxEngine Framework calling convention
#ifdef WIN32
#define FXENGINE_API __stdcall
#else
#define FXENGINE_API
#endif

#ifdef WIN32
#define FEF_CALLBACK CALLBACK
#else
#define FEF_CALLBACK
#endif
/*----------------------------------------------------------------------*//*!
	Defines, constants, and global variables
*//*-----------------------------------------------------------------------*/
#define SAFE_DELETE_ARRAY(p)		{if(p) {delete [] (p); (p)=NULL;}}
#define SAFE_DELETE_OBJECT(p)		{if(p) {delete (p); (p)=NULL;}}
#define SAFE_RELEASE(p)				{if(p) {(p)->Release(); (p)=NULL;}}
#define SAFE_RELEASE_INTERFACE(p)	{if(p) {(p)->FxReleaseInterface(); (p)=NULL;}}

#define NOT_IMPLEMENTED				FX_NOERROR

#ifndef MAX_PATH
#define MAX_PATH                   260
#endif

#ifndef PURE
#define PURE						= 0
#endif

#ifndef TO_DEF
#define TO_DEF						return NOT_IMPLEMENTED
#endif

#ifndef INFINITE_TIME
#define INFINITE_TIME 0xFFFFFFFF  //!< Infinite timeout
#endif

#ifndef FEF_FAILED
#define FEF_FAILED(hr) ((hr) < 0)
#endif

#ifndef FEF_SUCCEEDED
#define FEF_SUCCEEDED(hr) ((hr) >=0)
#endif

//! FxEngine and Fx Handle definition
typedef Void* FX_HANDLE;

/*----------------------------------------------------------------------*//*!
	Enums and structures definitions
*//*-----------------------------------------------------------------------*/
//! FxEngine constants types.
typedef enum _FXENGINE_CONST_TYPE{
	FX_STATE_CONST = 0,		//!< States of Fx
	FX_TYPE_CONST,			//!< Types of Fx
	UNITTYPE_CONST,			//!< Unit Type of Fx parameters
	FXENGINE_ERROR_CONST,	//!< Errors of FxEngine
	FX_MAINMEDIATYPE_CONST,	//!< Main Media Types of Fx Media
	FX_SUBMEDIATYPE_CONST,	//!< Sub Media Types of Fx Media
	FX_PINTYPE_CONST,		//!< Pin Types of Fx
    FX_SCOPE_CONST			//!< Scopes of Fx
}FXENGINE_CONST_TYPE;

//! Fx Types
typedef enum _FX_TYPE{
	FX_NOT_DEFINED = 0,
	FX_SOURCE,			//!< Source Fx
	FX_RENDERER,		//!< Renderer Fx
	FX_ANALYSER,		//!< Analyser Fx
	FX_PROCESS,			//!< Process Fx
	FX_SPLITTER,		//!< Splitter Fx
	FX_MIXER,			//!< Mixer Fx
	FX_USER				//!< Free user type
}FX_TYPE;

//! Fx Scopes
typedef enum _FX_SCOPE{
	FX_SCOPE_NOT_DEFINED = 0,
	FX_SCOPE_AUDIO,		//!< Audio waveform FX scope
	FX_SCOPE_VIDEO,		//!< Video / Image  FX scope
	FX_SCOPE_TEXT,		//!< Text  FX scope
	FX_SCOPE_DATA,		//!< Raw  FX scope (Vector/Matrix)
    FX_SCOPE_NETWORK,		//!< Network FX scope
    FX_SCOPE_ALL = 1000,		//!< All FX scopes
    FX_SCOPE_USER = 2000 //! Private user scope
}FX_SCOPE;

//! Fx States
typedef enum _FX_STATE{
	FX_UNDEFINED_STATE = 0,
	FX_LOADING_STATE,		//!< Fx is loading
	FX_RELEASE_STATE,		//!< Fx is released
	FX_INIT_STATE,			//!< Fx is initialized
	FX_CONNECT_STATE,		//!< Fx is connected.
	FX_DISCONNECT_STATE,	//!< Fx is disconnected.
	FX_STOP_STATE,			//!< Fx is stopped
	FX_PAUSE_STATE,			//!< Fx is paused
	FX_START_STATE,			//!< Fx is started
	FX_RUN_STATE,			//!< Fx is running
	FX_IDLE_STATE,			//!< Fx is idle
	FX_FLUSH_STATE,			//!< Fx flushing is done
	FX_PARAM_UPDATE,		//!< Fx parameters are updated
	FX_PIN_UPDATE,			//!< Fx Pins are updated
    FX_FRAME_UPDATE,		//!< Fx Frame is updated
	FX_STREAM_INIT_STATE,	//!< Fx received a new stream format

	FX_UNDERRUN_STATE = 100,	//!< Underrun data
	FX_OVERRUN_STATE,			//!< Overrun data
	FX_TIMEOUT_STATE,			//!< Timeout state

	FX_ERROR_RELEASE_STATE = 200,		//!< Fx release error
	FX_ERROR_INIT_STATE,			//!< Fx initialization error
	FX_ERROR_CONNECT_STATE,		//!< Fx connection error
	FX_ERROR_DISCONNECT_STATE,	//!< Fx disconnection error
	FX_ERROR_STATE,				//!< General error on Fx
	FX_ERROR_PIN_STATE,			//!< General error on Fx pin
	FX_ERROR_MEDIA_PIN_STATE,	//!< Invalid media format on Fx pin
	FX_ERROR_STOP_STATE,		//!< Error on stop command
	FX_ERROR_PAUSE_STATE,		//!< Error on pause command
	FX_ERROR_START_STATE,		//!< Error on start command
	FX_ERROR_RUN_STATE,			//!< Error on run command
    FX_ERROR_INVALID_PARAM,	    //!< At least one Fx parameter is invalid
	FX_ERROR_SUBMEDIA_PIN_STATE,//!< Invalid sub media format on Fx pin
	FX_ERROR_MEMORY_STATE,      //!< Error on memory allocation

	FX_USER_STATE = 300,		//!< Fx user states
} FX_STATE;

//! FxEngine Unit types
typedef enum _FX_UNIT_TYPE{
	NOT_DEFINED_TYPE = 0,
	UINT8_TYPE,				//!< Unsigned Integer 8bits
	INT8_TYPE,				//!< Signed Integer 8bits
	UINT16_TYPE,			//!< Unsigned Integer 16bits
	INT16_TYPE,				//!< Signed Integer 16bits
	UINT32_TYPE,			//!< Unsigned Integer 32bits
	INT32_TYPE,				//!< Signed Integer 32bits
	INT64_TYPE,				//!< Signed Integer 64bits
	UINT64_TYPE,			//!< Unsigned Integer 64bits
	FLOAT32_TYPE,			//!< Signed Flotting 32bits
	FLOAT64_TYPE,			//!< Signed Flotting 64bits
	COMPLEX_TYPE			//!< Two words of Signed Flotting 32 bits (Real, Imag)
} FX_UNIT_TYPE;

//! Pin types
typedef enum _FX_PIN_TYPE {
    PIN_IN = 0,		//!< Input pin type
	PIN_OUT			//!< Output pin type
} FX_PIN_TYPE;

//! Pin states
typedef enum _FX_PIN_STATE {
	PIN_NOT_CONNECTED = 0,	//!< Fx pin is not connected
	PIN_CONNECTED,			//!< Fx pin is connected
	PIN_ERROR				//!< Error on the Fx pin
} FX_PIN_STATE;

//! Stream process
typedef enum _FX_STREAM_STATE {
	STREAM_PROCESS = 0,	//!< Normal stream processing 
	STREAM_INIT,		//!< New stream format
	STREAM_FLUSH		//!< End of stream 
} FX_STREAM_STATE;

//! Media Markers
//! Media markers are used to tag IFxMedia
typedef enum _FX_MEDIA_MARKER{
	UNDEFINED_MARKER = 0,		//!< Undefined marker.
	DISCONTINUITY_MARKER,		//!< Stream discontinuity marker.
	USER1_MARKER = 100,			//!< User markers.
	USER2_MARKER,
	USER3_MARKER,
	USER4_MARKER,
	USER5_MARKER,
	USER6_MARKER,
	USER7_MARKER,
	USER8_MARKER
}FX_MEDIA_MARKER;

//! Fx Interfaces
typedef enum _FX_INTERFACE {
	IFX_PINMANGER = 0,			//!< Fx Pin Manager interface (see IFxPinManager).
	IFX_PARAM,					//!< Fx parameter interface (see IFxParam).
	IFX_REFCLOCK,				//!< Fx reference clock interface (see IFxRefClock).
	IFX_STATE,					//!< Fx state interface (see IFxRefClock).
} FX_INTERFACE;

//! Fx parameters update
typedef enum _FX_PARAMETER {
	FX_PARAM_ONE = 0,			//!< Specifies one Fx parameter to update only (see FEF_UpdateFxParam).
	FX_PARAM_ALL,				//!< Specifies all Fx parameters to update (see FEF_UpdateFxParam).
} FX_PARAMETER;

//! Fx descriptor structure definition
typedef struct  _FX_DESCRIPTOR {
    std::string strName;            //!< Specifies the Fx name.
	std::string strVersion;         //!< Specifies the Fx version.
    std::string strAuthor;          //!< Specifies the Fx author.
	std::string strCopyright;       //!< Specifies the Fx copyright.
    FX_TYPE FxType;                 //!< Specifies the Fx Type (see FX_TYPE).
    FX_SCOPE FxScope;               //!< Specifies the Fx Scope (see FX_SCOPE).
} FX_DESCRIPTOR, *PFX_DESCRIPTOR;

//! Type Media structure definition
typedef struct  _FX_MEDIA_TYPE {
    FX_MAIN_MEDIA_TYPE MainMediaType;		//!< Specifies the Main Fx Media type of the FxEngine data.
    FX_SUB_MEDIA_TYPE  SubMediaType;		//!< Specifies the Sub Fx Media type of the FxEngine data.
 } FX_MEDIA_TYPE, *PFX_MEDIA_TYPE;

//! Pcm sub media type definition
typedef struct  _FX_PCM_FORMAT {
    Uint16				wChannels;			//!< Specifies the PCM channel number.
    FX_UNIT_TYPE		FormatTag;			//!< Specifies the PCM sample format (see FX_UNIT_TYPE).
	Uint32				dwSamplingRate;		//!< Specifies the PCM sampling rate.
 } FX_PCM_FORMAT, *PFX_PCM_FORMAT;

//! Fx plugin parameters definitions structure
typedef struct _FX_PARAM {
    std::string strParamName;			//!< Specifies the parameter name.
	std::string strParamUnitName;		//!< Specifies the unit parameter name.
    Void* pvDefaultValue;       //!< Specifies the default values according to its type and number (can be null).
    Void* pvMinValue;           //!< Specifies the min parameter value (can be null).
    Void* pvMaxValue;           //!< Specifies the max parameter value (can be null).
    FX_UNIT_TYPE ParamType;		//!< Specifies the parameter's type.
	Uint32 dwParamNumber;		//!< Specifies the number of parameter.
} FX_PARAM, *PFX_PARAM;

//! Fx plugin String parameters definitions structure
typedef struct _FX_PARAM_STRING {
    std::string strParamName;			//!< Specifies the parameter name.
	std::string strDefaultValue;       //!< Specifies the default values .
} FX_PARAM_STRING, *PFX_PARAM_STRING;

} //namespace FEF
