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
#include "FxVectorFormat.h" 
#include "FxUtils/FxUtils.h"

namespace FEF {

CFxVectorFormat::CFxVectorFormat(void)
{
	_UnitType = NOT_DEFINED_TYPE;
	_dwM = 0;
}

CFxVectorFormat::~CFxVectorFormat()
{

}

Int32 CFxVectorFormat::GetUnitType(FX_UNIT_TYPE* pUnitType) //!< Retrieves the Unit of Matrix component.
{
	if(pUnitType == NULL)
		return FX_ERROR;

	AutoLock lock(_CS);

	*pUnitType = _UnitType;

	return FX_OK;
}

Int32 CFxVectorFormat::SetUnitType(FX_UNIT_TYPE UnitType) //!< sets the Unit of Matrix component.
{
	AutoLock lock(_CS);

	_UnitType = UnitType;

	return FX_OK;
}

Int32 CFxVectorFormat::GetVectorProperties(Uint32* pdwM)
{
	AutoLock lock(_CS);

	*pdwM = _dwM;

	return FX_OK;
}

Int32 CFxVectorFormat::SetVectorProperties(Uint32 dwM)
{
	AutoLock lock(_CS);

	_dwM = dwM;

	return FX_OK;
}

 } //namespace FEF
