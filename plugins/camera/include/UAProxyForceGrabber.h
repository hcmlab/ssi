// UAProxyForceGrabber.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2008/04/01
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_SENSOR_UAPROXYFORCEGRABBER_H
#define SSI_SENSOR_UAPROXYFORCEGRABBER_H

#include "CameraCons.h"

class CUAProxyForceGrabber
	: public CTransInPlaceFilter,
	  public IUAProxyForceGrabber
{
public:

    static CUnknown *WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

    // Constructor
    CUAProxyForceGrabber(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

    // Destructor
    ~CUAProxyForceGrabber();

    // Overrriden from CTransformFilter base class
    HRESULT Transform(IMediaSample *pSample);
    HRESULT CheckInputType(const CMediaType* mtIn);

	//HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
    //HRESULT DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties);
    //HRESULT CheckTransform(const CMediaType *mtIn,const CMediaType *mtOut);
    
    // Reveals ITransformTemplate and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	STDMETHODIMP SetCallback (void (*transform)(void*));
	STDMETHODIMP SetCallback (void (*transform)(void*), bool activateCallback);
	STDMETHODIMP ToggleCallBackActivation(bool activateCallback);
	STDMETHODIMP GrabFrame   (BYTE *pBufferForSample, int *pBytesInBuffer, void *pBitMapInfoHeaderStruc);
	STDMETHODIMP ToggleCallFrameBufferingForGrabbing(bool activateFrameBufferForGrabbing);
	STDMETHODIMP GetConnectedMediaType(AM_MEDIA_TYPE **ppMediaType);


private:
	BOOL CanPerformTransform(const CMediaType *pMediaType) const;

	CCritSec			m_UAProxyForceGrabberLock;
	CCritSec			m_UAProxyForceGrabberGrabFrameLock;
	void				(*m_transform)(void*);
	bool				m_CallBackActive;
	BYTE				*m_pBufferForGrabSample;
	ssi::CALLBACKBMPINFO		m_frameGrabberInfoStruct;
	DWORD				m_sizeOfReservedBuffer;
	bool				m_somethingInBuffer;
	bool				m_frameBufferActive;
	AM_MEDIA_TYPE		*m_pMediaType;
	REFERENCE_TIME      m_lasttimestamp;
	int					_framecounter;
};

#endif
