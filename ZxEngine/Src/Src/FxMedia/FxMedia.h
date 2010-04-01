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

#include "FxDef.h"
#include "FxUtils/FxUtils.h"
#include "IFxMedia.h"

#include "FxPcmFormat/FxPcmFormat.h"
#include "FxMatrixFormat/FxMatrixFormat.h"
#include "FxVectorFormat/FxVectorFormat.h"
#include "FxVideoImgFormat/FxVideoImgFormat.h"

#include <boost/thread/thread.hpp>

#include "IFxPinManager.h"

namespace FEF {

class CFxMediaPool;

class CFxMedia : public IFxMedia,
					public CFxPcmFormat,
					public CFxMatrixFormat,
					public CFxVectorFormat,
					public CFxVideoImgFormat
{

public:
	CFxMedia(Uint32 dwMediaSize, CFxMediaPool* pFxMediaPool);
	virtual ~CFxMedia();

private:
	//! Critical Section
	boost::mutex _CS;

private:
	Uint32 _dwValidDataSize;
	Uint32 _dwMediaSize;

private:
	CFxMediaPool* _pFxMediaPool;

#ifdef  _MSC_VER
#pragma pack(push,1)
#else
#pragma pack(1)
#endif  /* _MSC_VER */
	Uint8* _pbMediaData;
#ifdef  _MSC_VER
#pragma pack(pop)
#else
#pragma pack()
#endif  /* _MSC_VER */

private:
	FX_PTR _dwUserParam1;
	FX_PTR _dwUserParam2;

private:
	Uint64 _qTimeStamp;

private:
	FX_MEDIA_MARKER _MediaMarker;

private:
	PFX_MEDIA_TYPE _pFxMediaType;

private:
	std::string _strFxMedia;

private:
    Bool _IsInitMedia;

private:
    virtual Int32 Copy(IFxMedia* const pIFxMedia);
	virtual Int32 CheckMediaType(PFX_MEDIA_TYPE pMediaType); //!< Check media.
	virtual Int32 SetMediaType(PFX_MEDIA_TYPE pMediaType);	//!< Set media.
	virtual Int32 GetMediaType(PFX_MEDIA_TYPE pMediaType);	//!< Get media.

	virtual Int32 GetFormatInterface(FX_SUB_MEDIA_TYPE SubMediaType, Void** ppFormatInterface);

	virtual Int32 GetDataLength(Uint32* pdwLength); //!< Retrieves the length of the valid data in the buffer.
	virtual Int32 SetDataLength(Uint32 dwLength); //!<  Sets the length of the valid data in the buffer.
	virtual Int32 GetMediaPointer(Uint8** ppbMediaData); //!< Retrieves a read/write pointer to this buffer's memory.
	virtual Int32 GetSize(Uint32* pdwSize); //!<Retrieves the size of the buffer.
	virtual Int32 SetSize(Uint32 dwSize); //!< Re-allocate the memory block.
	virtual Int32 Release();

    virtual Int32 GetTimeStamp(Uint64* pqTimeStamp);
	virtual Int32 SetTimeStamp(Uint64 qTimeStamp);

	virtual Int32 GetMediaMarker(FX_MEDIA_MARKER* pMediaMarker);
	virtual Int32 SetMediaMarker(FX_MEDIA_MARKER MediaMarker);

	virtual Int32 SetUserParams(FX_PTR dwUserParam1, FX_PTR dwUserParam2) ;
	virtual Int32 GetUserParams(FX_PTR* pdwUserParam1, FX_PTR* pdwUserParam2) ;

	virtual Int32 SetFxMediaName(const std::string strFxMediaName);
    virtual Int32 GetFxMediaName(std::string& strFxMediaName);

public:
    virtual Bool IsInitMedia() {return _IsInitMedia;}
    virtual Void SetInitMedia(Bool IsInitMedia) {_IsInitMedia = IsInitMedia;}

};
} //namespace FEF

