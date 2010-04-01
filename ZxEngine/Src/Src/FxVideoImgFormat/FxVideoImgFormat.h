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
#include "IFxVideoImgFormat.h"

#include <boost/thread/thread.hpp>

namespace FEF {

class CFxVideoImgFormat : public IFxVideoImgFormat {

public:
	CFxVideoImgFormat(void);
	virtual ~CFxVideoImgFormat();

public:
	virtual Int32 GetVideoImgProperties(Uint32* pdwWidth, Uint32* pdwHeight); //!< Gets the Video/Image properties.
	virtual Int32 SetVideoImgProperties(Uint32 dwWidth, Uint32 dwHeight); //!< Sets the Video/Image properties.

private:
	//! Critical Section
    boost::mutex _CS;

private:
	Uint32 _dwWidth;
	Uint32 _dwHeight;
};

 } //namespace FEF
