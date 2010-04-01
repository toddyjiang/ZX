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
   @file	 FxMediaTypes.h
   @brief	 Contains the  Main and Sub Media Types

   This file contains the definitions and declarations of the Media used by the 
   Fx plugin pins.
*//*_______________________________________________________________________*/
#pragma once

namespace FEF {

/*----------------------------------------------------------------------*//*!
	Main media types. Main types are used to define media of fx pins
	and data flow. Each main type has several sub types.
	Please contact SMProcess for additional media types.
*//*-----------------------------------------------------------------------*/
typedef enum _FX_MAIN_MEDIA_TYPE {
	MAIN_TYPE_UNDEFINED = 0,
    AUDIO_TYPE,		//!< Audio waveform type
	VIDEO_TYPE,		//!< Video / Image type
	TEXT_TYPE,		//!< Text type
	DATA_TYPE,		//!< raw type (Vector/Matrix)
	USER_TYPE		//!< User type
} FX_MAIN_MEDIA_TYPE;

/*----------------------------------------------------------------------*//*!
	Sub media types. Types PCM, VECTOR, MATRIX and the types between BGR
	and Y800 has an Fx interface.
	Please contact SMProcess for additional interfaces or additional
	media sub-types.

*//*-----------------------------------------------------------------------*/
typedef enum _FX_SUB_MEDIA_TYPE {

	SUB_TYPE_UNDEFINED = 0,

	//! Audio sub types

	PCM,			//!< Generic Audio pulse code modulation (see IFxPcmFormat)

    PCMU=40,		//!< G.711 u-Law. The PCMU audio codec is described in RFC 3551

	PCMA,			//!< G.711 a-Law. The PCMA audio codec is described in RFC 3551

    G721=60,		/*!  ADPCM - Subsumed by G.726. The packetization of the G721 audio
						codec is described in RFC 3551 */

	G722,			/*!  G.722. The packetization of the G722 audio
						codec is described in RFC 3551 */

	G723,			/*!  G.723.1 at 6.3kbps or 5.3 kbps. The packetization of the G723 audio
						codec is described in RFC 3551 */

    G726 = G721,

	G728,			/*!  G.728 16kbps CELP. The packetization of the G728 audio
						codec is described in RFC 3551 */

	G729,			/*!  G.729 8kbps. The packetization of the G729 audio
						codec is described in RFC 3551 */

    GSM,			/*!  GSM 06.10. The packetization of the GSM audio
						codec is described in RFC 3551 */

	LPC,			/*!  LPC-10 Linear Predictive CELP, The packetization of the LPC audio
						codec is described in RFC 3551 */

	QCELP,			/*!  The Electronic Industries Association (EIA) & Telecommunications
						Industry Association (TIA) standard IS-733.
						The packetization of the QCELP audio codec is described in RFC 2658*/
	    
    DVI4_8K = 100,  /*!  DVI4 at 8kHz sample rate. The packetization of the DVI_8K audio
						codec is described in RFC 3551 */

	DVI4_11K,		/*!  DVI4 at 11kHz sample rate. The packetization of the DVI4_11K audio
						codec is described in RFC 3551 */

    DVI4_16K,		/*!  DVI4 at 16kHz sample rate. The packetization of the DVI4_16K audio
						codec is described in RFC 3551 */

	DVI4_22K,		/*!  DVI4 at 22kHz sample rate. The packetization of the DVI4_22K audio
						codec is described in RFC 3551 */
       
	MPEGA,			/*!  MPEGA denotes the ISO standard packet of MPEG-1 or MPEG-2 audio.
						The encoding is defined in ISO standards ISO/IEC 11172-3 and 13818-3 */

	MPA=120,        /*!  MPA denotes MPEG-1 or MPEG-2 audio encapsulated as elementary
						streams.  The encoding is defined in ISO standards ISO/IEC 11172-3
						and 13818-3.  The encapsulation is specified in RFC 2250 */ 

	DOLBY_AC3,		//!< Dolby AC3 audio */

	AAC,			/*! MPEG-2 AAC audio standard. The encoding is defined in ISO standards
						ISO/IEC 13818-7 */

	WMA,			//!< Windows Media Audio standard packet */

	//! Video sub types

	MPV = 150,		/*!  MPV designates the use of MPEG-1 and MPEG-2 video encoding elementary
						streams as specified in ISO Standards ISO/IEC 11172 and 13818-2,
						respectively.  The RTP payload format is as specified in RFC 2250 */

	CELB,			/*!  The CELL-B encoding is a proprietary encoding proposed by Sun
					   Microsystems.  The byte stream format is described in RFC 2029 */ 

	JPEG,			/*!  The encoding is specified in ISO Standards 10918-1 and 10918-2.  The
					   RTP payload format is as specified in RFC 2435 */

    BGR = 199,		//!< 24 bits BGR Format. 8 bits Blue / 8 bits Green / 8bits Red */

    RGB,			//!< 24 bits RGB Format. 8 bits Red / 8 bits Green / 8bits Blue */

	R_COLOR,  		//!< 8 bits Red only */

	G_COLOR,  		//!< 8 bits Green only */

	B_COLOR,        //!< 8 bits Blue only */
					
	UYVY,   		//!< 16 bits YUV Format (packed 4:2:2). Each macropixel is 4 bytes and contains 2 pixels (U0 Y0 V0 Y1) */

	YUY2,   		//!< 16 bits YUV Format (packed 4:2:2). Each macropixel is 4 bytes and contains 2 pixels (Y0 U0 Y1 V0) */

	IYUV,   		//!< 12 bits YUV Format. 8-bpp Y plane, followed by 8-bpp 2? U and V planes */

	YV12,   		/*!  12 bits YUV Format. YV12 is identical to IYUV but the order of the U and V planes is
						switched, so the V plane comes before the U plane */
	
	NV12,   		//!< 12 bits YUV Format. 8-bpp Y plane, followed by 8-bpp 2? interlaced U and V planes */

	NV21,   		//!< 12 bits YUV Format. 8-bpp Y plane, followed by 8-bpp 2? interlaced V and U planes */

	Y800,   		//!< 8 bits YUV Format */
	
	H261,			/*!  The encoding is specified in ITU-T Recommendation H.261, "Video codec
						for audiovisual services at p x 64 kbit/s".  The packetization and
						RTP-specific properties are described in RFC 2032 */

	H263,			/*!  The encoding is specified in the 1996 version of ITU-T Recommendation
						H.263, "Video coding for low bit rate communication".  The
						packetization and RTP-specific properties are described in RFC 2190 */

	H263_1998,		/*!  The encoding is specified in the 1998 version of ITU-T Recommendation
						H.263, "Video coding for low bit rate communication".  The
						packetization and RTP-specific properties are described in RFC 2429 */

	MP2T,			/*!  MP2T designates the use of MPEG-2 transport streams, for either audio
						or video.  The RTP payload format is described in RFC 2250 */
   
	WMV,			//!< Windows Media Video standard */

	DIVX,			//!< DIVX codec packet */
    XVID,			//!< XVID codec packet */

	//! Texte sub types

	UTF_8 = 300,	//!< Unicode UTF-8 encoding represents Unicode characters as sequences of 8-bit integers */
	UTF_16,			//!< Unicode UTF-16 encoding represents Unicode characters as sequences of 16-bit integers */
	ASCII,			//!< ASCII encoding represents the Latin alphabet as single 7-bit ASCII characters */

	//! Data sub types

	VECTOR = 400,	//!< Generic Vector format (see IFxVectorFormat) */
	MATRIX,			//!< Generic Matrix format (see IFxMatrixFormat) */

	//! User sub types
	USER_SUB_TYPES = 1000 //!< Use this sub type to define a private type */
       
} FX_SUB_MEDIA_TYPE;

} //namespace FEF


