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
#include "FxPcmFormat.h"
#include "FxUtils/FxUtils.h"

namespace FEF {

CFxPcmFormat::CFxPcmFormat(void)
{
	memset(&_PcmFormat, 0, sizeof(FX_PCM_FORMAT));

	_BitPerSampleMap.insert(make_pair(NOT_DEFINED_TYPE, 0));
	_BitPerSampleMap.insert(make_pair(UINT8_TYPE, 8));
	_BitPerSampleMap.insert(make_pair(INT8_TYPE, 8));
	_BitPerSampleMap.insert(make_pair(UINT16_TYPE, 16));
	_BitPerSampleMap.insert(make_pair(INT16_TYPE, 16));
	_BitPerSampleMap.insert(make_pair(UINT32_TYPE, 32));
	_BitPerSampleMap.insert(make_pair(INT32_TYPE, 32));
	_BitPerSampleMap.insert(make_pair(INT64_TYPE, 64));
	_BitPerSampleMap.insert(make_pair(UINT64_TYPE, 64));
	_BitPerSampleMap.insert(make_pair(FLOAT32_TYPE, 32));
	_BitPerSampleMap.insert(make_pair(FLOAT64_TYPE, 64));
	_BitPerSampleMap.insert(make_pair(FLOAT64_TYPE, 64));
	_BitPerSampleMap.insert(make_pair(COMPLEX_TYPE, 32)); /* 2*32 */
}

CFxPcmFormat::~CFxPcmFormat()
{

}

Int32 CFxPcmFormat::GetPcmFormat(PFX_PCM_FORMAT pPcmFormat)
{
	AutoLock lock(_CS);

	if(pPcmFormat == NULL)
		return FX_INVALPARAM;

	memcpy(pPcmFormat, &_PcmFormat, sizeof(FX_PCM_FORMAT));

	return FX_OK;
}

Int32 CFxPcmFormat::SetPcmFormat(PFX_PCM_FORMAT pPcmFormat)
{
	AutoLock lock(_CS);

	if(pPcmFormat == NULL)
		return FX_INVALPARAM;

	memcpy(&_PcmFormat, pPcmFormat, sizeof(FX_PCM_FORMAT));

	return FX_OK;
}

Int32 CFxPcmFormat::GetBitsPerSample(Uint16* pwBitsPerSample)
{
	AutoLock lock(_CS);

	if(pwBitsPerSample == NULL)
		return FX_INVALPARAM;

	BitPerSampleMap::iterator it;

	it = _BitPerSampleMap.find( _PcmFormat.FormatTag );
    if( it == _BitPerSampleMap.end() )
    {
		*pwBitsPerSample = 0;
        return FX_ERROR;
    }
    else
    {
		*pwBitsPerSample = it->second;
	}
	return FX_OK;
}

 } //namespace FEF


