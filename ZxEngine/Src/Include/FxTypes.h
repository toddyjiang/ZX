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
   @file	 Fxtypes.h
   @brief This file contains the main types of the FxEngine Framework.

   This file defines all types used by the FxEngine Framework, FxEngine API
   and Fx plugins.
*//*_______________________________________________________________________*/
#ifndef __FXTYPES_H__
#define __FXTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif
#if defined(WIN32)
	//! Microsoft definitions
	#include <windows.h>
	#include "winbase.h"
#endif
	
namespace FEF {
/*----------------------------------------------------------------------*//*!
							Win32 VC++
*//*-----------------------------------------------------------------------*/
#if (defined(WIN32) && !defined(__PLATFORM__))
//! Versions check:
//! Framework don't support Visual C++ prior to version 8
#if _MSC_VER < 1400
#error "*********** Compiler not supported ***********"
#endif
//! Framework don't support Visual C++ after to version 9
#if (_MSC_VER > 1600)
#error "*********** Compiler not supported ***********"
#endif

#if (_MSC_VER >= 1600)
	#define __COMPILER__             "COMPILER_UNKNOWN"
#elif   (_MSC_VER >= 1500)
    #define __COMPILER__             "COMPILER_VC2008"
#elif   (_MSC_VER >= 1400)
    #define __COMPILER__             "COMPILER_VC2005"
#else
	#define __COMPILER__             "COMPILER_UNKNOWN"
#endif

   #define __PLATFORM__				 "Win32"

//! 64-bit portability issues
#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

//! Generic pointer size (32/64)
#if defined(_WIN64)
    typedef _W64 unsigned __int64 FX_PTR;
#else //_WIN64
    typedef _W64 unsigned long FX_PTR;
#endif //#endif

	//! Generic types

	//! Type character
	typedef char                     Char; //!<  str

	//! Type void
	typedef void                     Void; //!<  v

	//! Type void*
	typedef void*                    Pvoid; //!<  pv

	//! Type Bool and bool
#ifdef __cplusplus
/*! Obsolete */
//#ifndef BOOL
//	typedef int						BOOL;
//#endif
	typedef bool					Bool;
#else
	typedef unsigned char           Bool;
#endif
/*! Obsolete */
//	typedef unsigned char			BYTE;

//! Bool values
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

	//! Generic bit types

	//! Type Signed Integer 8 bits
// 	typedef signed char              Int8; //!<  sb
#ifndef Int8
#define Int8 signed char
#endif

	//! Type Unsigned Integer 8 bits
// 	typedef unsigned char            Uint8; //!<  b
#ifndef Uint8
#define Uint8 unsigned char
#endif

	//! Type Signed Integer 16 bits
// 	typedef signed short             Int16; //!<  sw
#ifndef Int16
#define Int16 signed short
#endif

	//! Type Unsigned Integer 16 bits
// 	typedef unsigned short           Uint16; //!<  w
#ifndef Uint16
#define Uint16 unsigned short
#endif

	//! Type Signed Integer 32 bits
// 	typedef signed long int          Int32; //!<  sdw
#ifndef Int32
#define Int32 signed long int
#endif

	//! Type Unsigned Integer 32 bits
// 	typedef unsigned long int        Uint32; //!<  dw
#ifndef Uint32
#define Uint32 unsigned long int
#endif

	//! Type Signed Integer 64 bits
// 	typedef signed __int64           Int64; //!<  sq
#ifndef Int64
#define Int64 signed __int64
#endif

	//! Type Unsigned Integer 64 bits
// 	typedef unsigned __int64         Uint64; //!<  q
#ifndef Uint64
#define Uint64 unsigned __int64 
#endif

	//! Type Signed Float 32 bits
// 	typedef float                    Float; //!<  f
#ifndef Float
#define Float float
#endif

	//! Type Signed Float 64 bits
// 	typedef double                   Double; //!<  d
#ifndef Double
#define Double double
#endif

/*----------------------------------------------------------------------*//*!
							Linux32 GCC
*//*-----------------------------------------------------------------------*/
#elif(defined(__i386__) && defined(__GNUC__) && !defined(__PLATFORM__))

//! Versions check:
#if ( __GNUC__ != 4 )
//! Framework supports GCC 4.x version only
#error "*********** Compiler not supported ***********"
#endif

#define __COMPILER__            "COMPILER_GNUC" __VERSION__

#define __PLATFORM__			"Linux32"

//! Generic pointer size (32/64)
typedef unsigned long FX_PTR;

	//! Generic types

	//! Type character
	typedef char                     Char; //!<  str

	//! Type void
	typedef void                     Void; //!<  v

	//! Type void*
	typedef void*                    Pvoid; //!<  pv

	//! Type Bool and bool
#ifdef __cplusplus
/*! Obsolete */
//#ifndef BOOL
//	typedef int						BOOL;
//#endif
	typedef bool					Bool;
#else
	typedef unsigned char           Bool;
#endif

//! Bool values
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

	//! Generic bit types

	//! Type Signed Integer 8 bits
	typedef signed char              Int8; //!<  sb

	//! Type Unsigned Integer 8 bits
	typedef unsigned char            Uint8; //!<  b

	//! Type Signed Integer 16 bits
	typedef signed short             Int16; //!<  sw

	//! Type Unsigned Integer 16 bits
	typedef unsigned short           Uint16; //!<  w

	//! Type Signed Integer 32 bits
	typedef signed long int          Int32; //!<  sdw

	//! Type Unsigned Integer 32 bits
	typedef unsigned long int        Uint32; //!<  dw

	//! Type Signed Integer 64 bits
	typedef signed long long           Int64; //!<  sq

	//! Type Unsigned Integer 64 bits
	typedef unsigned long long         Uint64; //!<  q

	//! Type Signed Float 32 bits
	typedef float                    Float; //!<  f

	//! Type Signed Float 64 bits
	typedef double                   Double; //!<  d
#else
#error ERROR: Only Win32 or Linux32 targets supported!
#endif   // WIN32
 } //namespace FEF

	#ifdef __cplusplus
}
#endif
#endif // __FXTYPES_H__

