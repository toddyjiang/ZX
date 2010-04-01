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
#include "Fx.h" 

namespace FEF {

Int32 IFxBase::PauseFx()
{return NOT_IMPLEMENTED;}

Int32 IFxBase::RunFx()
{return NOT_IMPLEMENTED;}

Int32 IFxBase::DisplayFxPropertyPage(Pvoid pvWndParent)
{return NOT_IMPLEMENTED;}

Int32 IFxBase::UpdateFxParam(const std::string strParamName, FX_PARAMETER FxParameter)
{return NOT_IMPLEMENTED;}

Int32 IFxBase::GetFxFrame(const Char** ppbFxFrame)
{
    *ppbFxFrame = NULL;
    return NOT_IMPLEMENTED;
}

Int32 IFxBase::GetFxUserInterface(Pvoid* ppvUserInterface)
{return NOT_IMPLEMENTED;}

Int32 IFxBase::GetFxSubFxEngine(FX_HANDLE* phFxEngine)
{
	*phFxEngine = NULL;
	return FX_OK;
}

Void CFxStateCallback::FxStateCallback(FX_STATE FxState, FX_HANDLE hObserverId, FX_PTR dwParam)
{return;}

 } //namespace FEF
