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
#include "FxVideoImgFormat.h" 
#include "FxUtils/FxUtils.h"

namespace FEF {

CFxVideoImgFormat::CFxVideoImgFormat(void)
{
	_dwWidth = 0;
	_dwHeight = 0;
}

CFxVideoImgFormat::~CFxVideoImgFormat()
{

}


Int32 CFxVideoImgFormat::GetVideoImgProperties(Uint32* pdwWidth, Uint32* pdwHeight)
{
	AutoLock lock(_CS);

	*pdwWidth = _dwWidth;
	*pdwHeight = _dwHeight;

	return FX_OK;
}

Int32 CFxVideoImgFormat::SetVideoImgProperties(Uint32 dwWidth, Uint32 dwHeight)
{
	AutoLock lock(_CS);

	_dwWidth = dwWidth;
	_dwHeight = dwHeight;

	return FX_OK;
}

 } //namespace FEF
