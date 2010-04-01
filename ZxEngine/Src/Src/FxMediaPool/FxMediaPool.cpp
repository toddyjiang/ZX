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
#include "FxMediaPool.h"
#include "FxUtils/FxUtils.h"

namespace FEF {

inline boost::xtime delay(int secs=0, int msecs=0, int nsecs=0)
{
    const int MILLISECONDS_PER_SECOND = 1000;
    const int NANOSECONDS_PER_SECOND = 1000000000;
    const int NANOSECONDS_PER_MILLISECOND = 1000000;

    boost::xtime xt;
    if (boost::TIME_UTC != boost::xtime_get (&xt, boost::TIME_UTC))
		return xt;

    nsecs += xt.nsec;
    msecs += nsecs / NANOSECONDS_PER_MILLISECOND;
    secs += msecs / MILLISECONDS_PER_SECOND;
    nsecs += (msecs % MILLISECONDS_PER_SECOND) * NANOSECONDS_PER_MILLISECOND;
    xt.nsec = nsecs % NANOSECONDS_PER_SECOND;
    xt.sec += secs + (nsecs / NANOSECONDS_PER_SECOND);

    return xt;
}

CFxMediaPool::CFxMediaPool(Uint32 dwSize, Uint32 dwNumber)
{
	_dwSize = dwSize;
	_dwNumber = dwNumber;

	/*! init Media Pool */
	CFxMedia* pFxMedia = NULL;

	/*! Create the pool of Media object */
	for(Uint32 Idx=0; Idx<dwNumber; Idx++)
	{
		pFxMedia = new CFxMedia(dwSize, this);
		if(pFxMedia != NULL)
		{
			/*! Setup free list  */
            _FreeFxMediaL.push_back(static_cast<IFxMedia*>(pFxMedia));
		}
		else
			return;
	}

}

CFxMediaPool::~CFxMediaPool()
{
	ReleaseAllMedia();

	AutoLock lock(_CS);

    CFxMedia* pFxMedia = NULL;
    IFxMediaL::iterator Iter;
    for ( Iter = _FreeFxMediaL.begin( ); Iter != _FreeFxMediaL.end( ); Iter++ )
	{
        pFxMedia = static_cast<CFxMedia*>(*Iter);
        SAFE_DELETE_OBJECT(pFxMedia);
    }
}

Int32 CFxMediaPool::UpdateMedia(Uint32 dwSize, Uint32 dwNumber)
{
    /*! Update all free Media */
    IFxMediaL::iterator Iter;

	AutoLock lock(_CS);
	Uint32 dwCurrentSize;
	/*! For each FxMedia in used list */
	for ( Iter = _FreeFxMediaL.begin( ); Iter != _FreeFxMediaL.end( ); Iter++ )
	{
        (*Iter)->GetSize(&dwCurrentSize);
        if(dwCurrentSize != dwSize)
        {
            (*Iter)->SetSize(dwSize);
		    (*Iter)->SetUserParams((FX_PTR)NULL, (FX_PTR)NULL);
		    (*Iter)->SetDataLength(0);
		    (*Iter)->SetMediaMarker(UNDEFINED_MARKER);
		    (*Iter)->SetTimeStamp(0);
        }
	}

    CFxMedia* pFxMedia = NULL;
    if(_dwNumber < dwNumber)
    {
        for( Iter = _FreeFxMediaL.begin( ); Iter != _FreeFxMediaL.end( ), _dwNumber != dwNumber; Iter++ ,_dwNumber++)
	    {
            pFxMedia = new CFxMedia(dwSize, this);
		    if(pFxMedia != NULL)
		    {
			    /*! Setup free list  */
                _FreeFxMediaL.push_back(static_cast<IFxMedia*>(pFxMedia));
		    }
        }
    }
    else if(_dwNumber > dwNumber)
    {
        for ( Iter = _FreeFxMediaL.begin( ); Iter != _FreeFxMediaL.end( ), _dwNumber != dwNumber; _dwNumber--)
	    {
            pFxMedia = static_cast<CFxMedia*>(*Iter);
            SAFE_DELETE_OBJECT(pFxMedia);
            Iter = _FreeFxMediaL.erase(Iter);
        }
    }

    _dwSize = dwSize;
	_dwNumber = dwNumber;
    return FX_OK;
}

Int32 CFxMediaPool::GetFreeMediaNumber(Uint32* pdwFreeMediaNumber)
{
	AutoLock lock(_CS);

	if(pdwFreeMediaNumber)
	{
		*pdwFreeMediaNumber = (Uint32)_FreeFxMediaL.size();
		return FX_OK;
	}
	return FX_ERROR;
}

Int32 CFxMediaPool::GetDeliveryMedia(IFxMedia** ppIFxMedia, Uint32 dwTimeOut)
{
	Bool IsEmpty;
	Bool IsTimeNotReached = TRUE;
	*ppIFxMedia = NULL;

	IFxMedia* pFxMedia;

	AutoLock lock(_CS);
	
	IsEmpty = _FreeFxMediaL.empty();

	if (IsEmpty == TRUE)
	{
        if(dwTimeOut == INFINITE_TIME)
			MediaPool_not_empty.wait(lock);
        else
		{
			/* return immediately */
			if(dwTimeOut == 0)
				return FX_TIMEOUT;
			boost::xtime xt = delay(0,dwTimeOut,0);
			IsTimeNotReached = MediaPool_not_empty.timed_wait(lock, xt);
		}

		if(IsTimeNotReached == FALSE)
			return FX_TIMEOUT;

        if(_FreeFxMediaL.empty() == TRUE) //!< Case when no available media and want to stop pin
            return FX_TIMEOUT;
	}
	pFxMedia = _FreeFxMediaL.front();
	*ppIFxMedia = pFxMedia;
    _FreeFxMediaL.pop_front();

	_UsedFxMediaL.push_back(pFxMedia);
	return FX_OK;
}

Int32 CFxMediaPool::ReleaseMedia(IFxMedia* pIFxMedia)
{
	IFxMediaL::iterator Iter;

	if(pIFxMedia == NULL)
		return FX_ERROR;

	AutoLock lock(_CS);

	/*! Find FxMedia in used list */
	Iter = std::find(_UsedFxMediaL.begin(), _UsedFxMediaL.end(), pIFxMedia);
	if( Iter != _UsedFxMediaL.end() )
	{
		_UsedFxMediaL.erase(Iter);
		_FreeFxMediaL.push_back(pIFxMedia);
		MediaPool_not_empty.notify_one();
	}

	return FX_OK;
}

Int32 CFxMediaPool::ReleaseAllMedia()
{
	IFxMediaL::iterator Iter;

	AutoLock lock(_CS);

	/*! For each FxMedia in used list */
	for ( Iter = _UsedFxMediaL.begin( ); Iter != _UsedFxMediaL.end( ); Iter++ )
	{
		(*Iter)->SetUserParams((FX_PTR)NULL, (FX_PTR)NULL);
		(*Iter)->SetDataLength(0);
		(*Iter)->SetMediaMarker(UNDEFINED_MARKER);
		(*Iter)->SetTimeStamp(0);
		(*Iter)->SetFxMediaName("");
		(static_cast<CFxMedia*>(*Iter))->SetInitMedia(FALSE);
        _FreeFxMediaL.push_back((*Iter));
	}
	_UsedFxMediaL.clear();

	MediaPool_not_empty.notify_one();

	return FX_OK;
}

Int32 CFxMediaPool::StopWaiting()
{
    AutoLock lock(_CS);
	MediaPool_not_empty.notify_one();
	return FX_OK;
}

 } //namespace FEF
