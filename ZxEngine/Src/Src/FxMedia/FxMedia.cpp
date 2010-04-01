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
#include "FxMediaPool/FxMediaPool.h"
#include "FxMedia.h"
#include "FxUtils/FxUtils.h"

namespace FEF {

#ifdef  _MSC_VER
#pragma pack(push,1)
#else
#pragma pack(1)
#endif  /* _MSC_VER */


CFxMedia::CFxMedia(Uint32 dwMediaSize, CFxMediaPool* pFxMediaPool)
{
	_pFxMediaType = NULL;
	_pbMediaData = NULL;

	_dwValidDataSize = 0;
	_dwMediaSize = dwMediaSize;

	_pFxMediaPool = pFxMediaPool;

	_dwUserParam1 = (FX_PTR)NULL;
	_dwUserParam2 = (FX_PTR)NULL;

	_qTimeStamp = 0;

	_MediaMarker = UNDEFINED_MARKER;

	_pFxMediaType = new FX_MEDIA_TYPE;

#ifdef  _MSC_VER
	_pbMediaData = (Uint8*)GlobalAlloc(GPTR, dwMediaSize);
#else
	_pbMediaData = (Uint8*)new Uint8[dwMediaSize];
#endif

	_strFxMedia = "";

    _IsInitMedia = FALSE;
}

CFxMedia::~CFxMedia()
{
	AutoLock lock(_CS);
	SAFE_DELETE_OBJECT(_pFxMediaType);

#ifdef  _MSC_VER
	if(_pbMediaData != NULL)
		GlobalFree(_pbMediaData);
#else
	if(_pbMediaData != NULL)
		SAFE_DELETE_ARRAY(_pbMediaData);
#endif

	lock.unlock();
}

Int32 CFxMedia::Copy(IFxMedia* const pIFxMedia)
{
	if(pIFxMedia == NULL)
		return FX_INVALPARAM;

	Int32 hr = FX_OK;

	/*! Set media */
	FX_MEDIA_TYPE MediaType;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaType(&MediaType)))
		return hr;
	if( FEF_FAILED(hr = this->SetMediaType(&MediaType)))
		return hr;	

	/*! Buffer size */
	Uint32 dwDstSize, dwSrcSize;
	if( FEF_FAILED(hr = this->GetSize(&dwDstSize)))
		return hr; 
	if( FEF_FAILED(hr = pIFxMedia->GetSize(&dwSrcSize)))
		return hr;
	if(dwDstSize != dwSrcSize)
		if( FEF_FAILED(hr = this->SetSize(dwSrcSize)))
			return hr;

	/*! Data lenght */
	Uint32 dwLength;
	if( FEF_FAILED(hr = pIFxMedia->GetDataLength(&dwLength)))
		return hr;
	if( FEF_FAILED(hr = this->SetDataLength(dwLength)))
		return hr;

	/*! Copy data */
	Uint8 *pbDstMediaData = NULL;
	Uint8 *pbSrcMediaData = NULL;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaPointer(&pbSrcMediaData)))
		return hr;
	if( FEF_FAILED(hr = this->GetMediaPointer(&pbDstMediaData)))
		return hr;
	if(pbSrcMediaData && pbDstMediaData)
		memcpy(pbDstMediaData, pbSrcMediaData, dwLength);

	/*! Copy timestamp */
	Uint64 qTimeStamp;
	if( FEF_FAILED(hr = pIFxMedia->GetTimeStamp(&qTimeStamp)))
		return hr;
	if( FEF_FAILED(hr = this->SetTimeStamp(qTimeStamp)))
		return hr;

	/*! Copy media marker */
	FX_MEDIA_MARKER MediaMarker;
	if( FEF_FAILED(hr = pIFxMedia->GetMediaMarker(&MediaMarker)))
		return hr;
	if( FEF_FAILED(hr = this->SetMediaMarker(MediaMarker)))
		return hr;

	/*! Copy user param */
	FX_PTR dwUserParam1, dwUserParam2;
	if( FEF_FAILED(hr = pIFxMedia->GetUserParams(&dwUserParam1, &dwUserParam2)))
		return hr;
	if( FEF_FAILED(hr = this->SetUserParams(dwUserParam1, dwUserParam2)))
		return hr;
	
	/*! Copy media name */
	std::string strFxMediaName;
	if( FEF_FAILED(hr = pIFxMedia->GetFxMediaName(strFxMediaName)))
		return hr;
	if( FEF_FAILED(hr = this->SetFxMediaName(strFxMediaName)))
		return hr;

    /*! Copy init stream */
    this->SetInitMedia(static_cast<CFxMedia*>(pIFxMedia)->IsInitMedia());
    
	/*! Copy media interface */
	Void* pSrcFormatInterface = NULL;
	Void* pDstFormatInterface = NULL;
	if( FEF_FAILED(hr = pIFxMedia->GetFormatInterface(MediaType.SubMediaType, &pSrcFormatInterface)))
		return hr;
	if( FEF_FAILED(hr = this->GetFormatInterface(MediaType.SubMediaType, &pDstFormatInterface)))
		return hr;
	if((pSrcFormatInterface != NULL) && (pDstFormatInterface != NULL)) {
		switch(MediaType.SubMediaType)
		{
			case PCM:
				if(MediaType.MainMediaType == AUDIO_TYPE) {
					IFxPcmFormat* pIFxSrcPcmFormat = static_cast<IFxPcmFormat*>(pSrcFormatInterface);
					IFxPcmFormat* pIFxDstPcmFormat = static_cast<IFxPcmFormat*>(pDstFormatInterface);
					FX_PCM_FORMAT PcmFormat;
					if( FEF_FAILED(hr = pIFxSrcPcmFormat->GetPcmFormat(&PcmFormat)))
						return hr;
					if( FEF_FAILED(hr = pIFxDstPcmFormat->SetPcmFormat(&PcmFormat)))
						return hr;
				}
				else
					return FX_MEDIANOTSUPPORTED;
				break;

			case BGR:
			case RGB:
			case R_COLOR:
			case G_COLOR:
			case B_COLOR:
			case UYVY:
			case YUY2:
			case IYUV:
			case YV12:
			case NV12:
			case NV21:
			case Y800:
				if(MediaType.MainMediaType == VIDEO_TYPE) {
					IFxVideoImgFormat* pIFxSrcVideoImgFormat = static_cast<IFxVideoImgFormat*>(pSrcFormatInterface);
					IFxVideoImgFormat* pIFxDstVideoImgFormat = static_cast<IFxVideoImgFormat*>(pDstFormatInterface);
					Uint32 dwWidth, dwHeight;
					if( FEF_FAILED(hr = pIFxSrcVideoImgFormat->GetVideoImgProperties(&dwWidth, &dwHeight)))
						return hr;
					if( FEF_FAILED(hr = pIFxDstVideoImgFormat->SetVideoImgProperties(dwWidth, dwHeight)))
						return hr;
				}
				else
					return FX_MEDIANOTSUPPORTED;
				break;

			case VECTOR:
				if(MediaType.MainMediaType == DATA_TYPE) {
					IFxVectorFormat* pIFxSrcVectorFormat = static_cast<IFxVectorFormat*>(pSrcFormatInterface);
					IFxVectorFormat* pIFxDstVectorFormat = static_cast<IFxVectorFormat*>(pDstFormatInterface);
					FX_UNIT_TYPE UnitType;
					if( FEF_FAILED(hr = pIFxSrcVectorFormat->GetUnitType(&UnitType)))
						return hr;
					if( FEF_FAILED(hr = pIFxDstVectorFormat->SetUnitType(UnitType)))
						return hr;
					Uint32 dwM;
					if( FEF_FAILED(hr = pIFxSrcVectorFormat->GetVectorProperties(&dwM)))
						return hr;
					if( FEF_FAILED(hr = pIFxDstVectorFormat->SetVectorProperties(dwM)))
						return hr;
				}
				else
					return FX_MEDIANOTSUPPORTED;
				break;

			case MATRIX:
				if(MediaType.MainMediaType == DATA_TYPE) {
					IFxMatrixFormat* pIFxSrcMatrixFormat = static_cast<IFxMatrixFormat*>(pSrcFormatInterface);
					IFxMatrixFormat* pIFxDstMatrixFormat = static_cast<IFxMatrixFormat*>(pDstFormatInterface);
					FX_UNIT_TYPE UnitType;
					if( FEF_FAILED(hr = pIFxSrcMatrixFormat->GetUnitType(&UnitType)))
						return hr;
					if( FEF_FAILED(hr = pIFxDstMatrixFormat->SetUnitType(UnitType)))
						return hr;

					Uint32 dwN, dwM;
					if( FEF_FAILED(hr = pIFxSrcMatrixFormat->GetMatrixProperties(&dwN, &dwM)))
						return hr;
					if( FEF_FAILED(hr = pIFxDstMatrixFormat->SetMatrixProperties(dwN, dwM)))
						return hr;
				}
				else
					return FX_MEDIANOTSUPPORTED;
				break;
			default:
				break;
		}
	}
	
	return FX_OK;
}
/*----------------------------------------------------------------------*//*!
	CheckMediaType().

	The CheckMediaType function checks the current pin media to the media given in parameter.
    @param	IN pPinMediaType:    The media to compare.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxMedia::CheckMediaType(PFX_MEDIA_TYPE pFxMediaType)
{
	AutoLock lock(_CS);

	if( (!_pFxMediaType) || (!pFxMediaType) )
		return FX_ERROR;

	if( (pFxMediaType->MainMediaType  == _pFxMediaType->MainMediaType) &&
		(pFxMediaType->SubMediaType   == _pFxMediaType->SubMediaType) )
		return FX_OK;

	return FX_ERROR;
}

/*----------------------------------------------------------------------*//*!
	SetPinMediaType().

	The SetPinMediaType function set the current pin media with the media given in parameter.
    @param	IN pPinMediaType:  The Pin media to apply.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxMedia::SetMediaType(PFX_MEDIA_TYPE pFxMediaType)
{
	AutoLock lock(_CS);

	if( (!_pFxMediaType) || (!pFxMediaType) )
		return FX_ERROR;

	memcpy(_pFxMediaType, pFxMediaType, sizeof(FX_MEDIA_TYPE));

	return FX_OK;
}

/*----------------------------------------------------------------------*//*!
	GetMediaType().

	The GetMediaType function return the current pin media.
    @param	IN pPinMediaType:  The current Pin media.

	@return	FX_OK if success, otherwise value < 0.
*//*-----------------------------------------------------------------------*/
Int32 CFxMedia::GetMediaType(PFX_MEDIA_TYPE pFxMediaType)
{
	AutoLock lock(_CS);

	if( (!_pFxMediaType) || (!pFxMediaType) )
		return FX_ERROR;

	memcpy(pFxMediaType, _pFxMediaType, sizeof(FX_MEDIA_TYPE));

	return FX_OK;
}

Int32 CFxMedia::GetDataLength(Uint32* pdwLength) //!< Retrieves the length of the valid data in the buffer.
{
	AutoLock lock(_CS);

	if(pdwLength)
		*pdwLength = _dwValidDataSize;
	else
		return FX_ERROR;

	return FX_OK;
}

Int32 CFxMedia::SetDataLength(Uint32 dwLength) //!<  Sets the length of the valid data in the buffer.
{
	AutoLock lock(_CS);

	_dwValidDataSize = dwLength;
	return FX_OK;
}

Int32 CFxMedia::GetMediaPointer(Uint8** ppbMediaData) //!< Retrieves a read/write pointer to this buffer's memory.
{
	AutoLock lock(_CS);

	if( (!_pbMediaData) || (!ppbMediaData) )
		return FX_ERROR;

	*ppbMediaData = _pbMediaData;

	return FX_OK;
}

Int32 CFxMedia::GetSize(Uint32* pdwSize) //!<Retrieves the size of the buffer.
{
	AutoLock lock(_CS);

	if(pdwSize)
		*pdwSize = _dwMediaSize;
	else
		return FX_ERROR;

	return FX_OK;
}

Int32 CFxMedia::SetSize(Uint32 dwSize)
{
	AutoLock lock(_CS);

#ifdef  _MSC_VER
	_pbMediaData = (Uint8*)GlobalReAlloc((HGLOBAL)_pbMediaData, dwSize, GMEM_MOVEABLE | GMEM_ZEROINIT);
#else
	SAFE_DELETE_ARRAY(_pbMediaData);
	_pbMediaData = (Uint8*) new Uint8[dwSize];
#endif

	if(_pbMediaData == NULL)
		return FX_NOMEM;

	_dwMediaSize = dwSize;

	return FX_OK;

}

Int32 CFxMedia::GetFormatInterface(FX_SUB_MEDIA_TYPE  SubMediaType, Void** ppFormatInterface)
{
	AutoLock lock(_CS);

	if(ppFormatInterface == NULL)
		return FX_INVALPARAM;

	*ppFormatInterface = NULL;

	switch(SubMediaType)
	{

		case PCM:
			if(_pFxMediaType->MainMediaType == AUDIO_TYPE)
				*ppFormatInterface = static_cast<IFxPcmFormat*>( this );
			else
				return FX_ERROR;
			break;

		case BGR:
		case RGB:
		case R_COLOR:
        case G_COLOR:
        case B_COLOR:
		case UYVY:
		case YUY2:
		case IYUV:
		case YV12:
		case NV12:
		case NV21:
		case Y800:
			if(_pFxMediaType->MainMediaType == VIDEO_TYPE)
				*ppFormatInterface = static_cast<IFxVideoImgFormat*>( this );
			else
			{
				*ppFormatInterface = NULL;
				return FX_ERROR;
			}
			break;

		case VECTOR:
			if(_pFxMediaType->MainMediaType == DATA_TYPE)
				*ppFormatInterface = static_cast<IFxVectorFormat*>( this );
			else
			{
				*ppFormatInterface = NULL;
				return FX_ERROR;
			}
			break;

		case MATRIX:
			if(_pFxMediaType->MainMediaType == DATA_TYPE)
				*ppFormatInterface = static_cast<IFxMatrixFormat*>( this );
			else
			{
				*ppFormatInterface = NULL;
				return FX_ERROR;
			}
			break;

		default:
			*ppFormatInterface = NULL;
			return FX_NOINTERFACE;
		break;
	}
	return FX_OK;
}

Int32 CFxMedia::Release()
{
    AutoLock lock(_CS);
	if(_pFxMediaPool != NULL){
	    /*! SetUserParams */
	    _dwUserParam1 = (FX_PTR)NULL;
        _dwUserParam2 = (FX_PTR)NULL;

        /*! SetDataLength */
        _dwValidDataSize = 0;

		/*! SetMediaMarker*/
		_MediaMarker = UNDEFINED_MARKER;

		/*! SetTimeStamp */
		_qTimeStamp = 0;

		/*! SetFxMediaName */
		_strFxMedia = "";
		
		/*! STREAM_INIT */
		_IsInitMedia = FALSE;

		_pFxMediaPool->ReleaseMedia(this);
	}
	return FX_OK;
}

Int32 CFxMedia::SetUserParams(FX_PTR dwUserParam1, FX_PTR dwUserParam2)
{
	AutoLock lock(_CS);

	_dwUserParam1 = dwUserParam1;
	_dwUserParam2 = dwUserParam2;
	return FX_OK;
}

Int32 CFxMedia::GetUserParams(FX_PTR* pdwUserParam1, FX_PTR* pdwUserParam2)
{
	AutoLock lock(_CS);

	if( (pdwUserParam1 == NULL) || (pdwUserParam2 == NULL) )
		return FX_ERROR;

	*pdwUserParam1 = _dwUserParam1;
	*pdwUserParam2 = _dwUserParam2;

	return FX_OK;
}

Int32 CFxMedia::GetTimeStamp(Uint64* pqTimeStamp)
{
	AutoLock lock(_CS);

	if(pqTimeStamp == NULL)
		return FX_ERROR;

	*pqTimeStamp = _qTimeStamp;

	return FX_OK;
}

Int32 CFxMedia::SetTimeStamp(Uint64 qTimeStamp)
{
	AutoLock lock(_CS);

	_qTimeStamp = qTimeStamp;

	return FX_OK;
}

Int32 CFxMedia::GetMediaMarker(FX_MEDIA_MARKER* pMediaMarker)
{
	AutoLock lock(_CS);

	if(pMediaMarker == NULL)
		return FX_ERROR;

	*pMediaMarker = _MediaMarker;

	return FX_OK;
}

Int32 CFxMedia::SetMediaMarker(FX_MEDIA_MARKER MediaMarker)
{
	AutoLock lock(_CS);

	_MediaMarker = MediaMarker;

	return FX_OK;
}

Int32 CFxMedia::SetFxMediaName(const std::string strFxMediaName)
{
	AutoLock lock(_CS);

	_strFxMedia = strFxMediaName;

	return FX_OK;
}

Int32 CFxMedia::GetFxMediaName(std::string& strFxMediaName)

{
	AutoLock lock(_CS);

    strFxMediaName = _strFxMedia;

	return FX_OK;
}

#ifdef  _MSC_VER
#pragma pack(pop)
#else
#pragma pack()
#endif  /* _MSC_VER */

 } //namespace FEF
