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
   @file	 FxErr.h
   @brief This file contains the definitions of the FxEngine Framework
   Errors and return values.
*//*_________________________________________________________________________*/
#pragma once

namespace FEF {

#define FX_NOERROR      0								//!< Error Base

#define FX_OK					(FX_NOERROR)			//!< no error

//! Errors
#define FX_ERROR				(FX_NOERROR		- 1)	//!< Unspecified error
#define FX_INVALPARAM			(FX_NOERROR		- 2)	//!< Invalid parameter
#define FX_INVALHANDLE			(FX_NOERROR		- 3)	//!< Handle is invalid
#define FX_NOMEM				(FX_NOERROR		- 4)	//!< Memory allocation error
#define FX_MEDIANOTSUPPORTED	(FX_NOERROR		- 5)	//!< Media isn't supported
#define FX_SUBMEDIANOTSUPPORTED	(FX_NOERROR		- 6)	//!< SubMedia isn't supported
#define FX_FMTNOTSUPPORTED		(FX_NOERROR		- 7)	//!< Format isn't supported
#define FX_ERRORSTATE			(FX_NOERROR		- 8)	//!< Fx state error
#define FX_NOINTERFACE			(FX_NOERROR		- 9)	//!< FX interface not found
#define FX_INVALPINTYPE			(FX_NOERROR		- 10)	//!< Invalid pin type
#define FX_TIMEOUT				(FX_NOERROR		- 11)	//!< TimeOut occurs

//! Return values
#define FX_REPEATFXMEDIA		(FX_NOERROR		+ 1)	//!< The FxMedia must be repeated

} //namespace FEF
