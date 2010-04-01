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
#include "IFxPcmFormat.h"
#include <map>

#include <boost/thread/thread.hpp>

using namespace std;

namespace FEF {

typedef std::map<FX_UNIT_TYPE, Uint16> BitPerSampleMap;

class CFxPcmFormat : public IFxPcmFormat {

public:
	CFxPcmFormat(void);
	virtual ~CFxPcmFormat();

private:
	//! Critical Section
    boost::mutex _CS;

private:
	FX_PCM_FORMAT _PcmFormat;

private:
	BitPerSampleMap _BitPerSampleMap;

public:
	  virtual Int32 GetPcmFormat(PFX_PCM_FORMAT pPcmFormat);
	  virtual Int32 SetPcmFormat(PFX_PCM_FORMAT pPcmFormat);
	  virtual Int32 GetBitsPerSample(Uint16* pwBitsPerSample);

};

} //namespace FEF

