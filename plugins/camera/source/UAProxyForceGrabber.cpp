// UAProxyForceGrabber.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/27
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

#include <streams.h>     // DirectShow (includes windows.h)
#include <initguid.h>    // declares DEFINE_GUID to declare an EXTERN_C const.
#include <tchar.h>
#include <stdio.h>
#include "iUAProxyForceGrabber.h"
#include "UAProxyForceGrabber.h"
//#include "UAProxyForceGrabberProp.h"


#ifdef _DEBUG
	#include "SSI_LeakWatcher.h"
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

//int RowWidth(int w) {
//	if (w % 4)
//		w += 4 - w % 4;
//	return w;
//}

//
// CUAProxyForceGrabber
//
// Constructor;
//
CUAProxyForceGrabber::CUAProxyForceGrabber(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
	: CTransInPlaceFilter (tszName, punk, CLSID_UAProxyForceGrabber, phr), 
      m_transform(NULL),
	  m_CallBackActive(false),
	  m_pBufferForGrabSample(NULL),
	  m_sizeOfReservedBuffer(0),
	  m_somethingInBuffer(false),
	  m_frameBufferActive(false),
	  m_pMediaType(NULL),
	  m_lasttimestamp (-1),
	  _framecounter (0)
{
	
}

//
// ~CUAProxyForceGrabber
//
// Destructor;
//
CUAProxyForceGrabber::~CUAProxyForceGrabber()
{
	if(m_pBufferForGrabSample != NULL)
	{
		delete[] m_pBufferForGrabSample;
	}
	if(m_pMediaType != NULL)
	{
		DeleteMediaType(m_pMediaType);
	}
}

//
// Transform
//
// Transforms the media sample in-place
//
HRESULT CUAProxyForceGrabber::Transform(IMediaSample *pSample)
{

	AM_MEDIA_TYPE *pTmpType;
	
	HRESULT hr = pSample->GetMediaType(&pTmpType);

	if(pTmpType == NULL && m_pMediaType == NULL)
	{
		pTmpType = &(m_pInput->CurrentMediaType());
		m_pMediaType = CreateMediaType(pTmpType);
		pTmpType = NULL;
	}
	else
	{
		if(m_pMediaType != NULL)
		{
			DeleteMediaType(m_pMediaType);
			m_pMediaType = NULL;
			
		}
		m_pMediaType = pTmpType;
		pTmpType = NULL;
	}


	if((m_CallBackActive == false && m_frameBufferActive == false) || (!m_transform && m_frameBufferActive == false))
	{
        return NOERROR;
    }
	
	BYTE*    pData;
	ssi::CALLBACKBMPINFO image;
    
	CAutoLock cAutoLock(&m_UAProxyForceGrabberGrabFrameLock);

    pSample->GetPointer(&pData);
    

    AM_MEDIA_TYPE* pType = &m_pInput->CurrentMediaType();
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pType->pbFormat;
    
    if((pvi->bmiHeader.biBitCount != 24)/* && (pvi->bmiHeader.biBitCount != 32)*/)
    {
        return NOERROR;
    }

    // Get the image properties from the BITMAPINFOHEADER
    int cxImage    = pvi->bmiHeader.biWidth;
    int cyImage    = abs(pvi->bmiHeader.biHeight);
	int stride     = (cxImage * ((pvi->bmiHeader.biBitCount) >> 3) + 3) & ~3;



	if(m_frameBufferActive == true)
	{
		if(m_pBufferForGrabSample == NULL)
		{
			m_pBufferForGrabSample = new BYTE[cyImage * stride];
			m_sizeOfReservedBuffer = cyImage * stride;
		}

		if(m_sizeOfReservedBuffer < (DWORD)(cyImage * stride))
		{
			delete[] m_pBufferForGrabSample;
			m_pBufferForGrabSample = NULL;
			m_pBufferForGrabSample = new BYTE[cyImage * stride];
			m_sizeOfReservedBuffer = cyImage * stride;
		}

		memcpy(m_pBufferForGrabSample, pData, cyImage * stride);
		m_frameGrabberInfoStruct.heightInPixels = cyImage;
		m_frameGrabberInfoStruct.widthInPixels = cxImage;
		m_frameGrabberInfoStruct.widthStepInBytes = stride;
		m_frameGrabberInfoStruct.mediaSubType = pType->subtype;
		m_frameGrabberInfoStruct.pInfoHeader = NULL;
		m_frameGrabberInfoStruct.pDataOfBMP = NULL;
		m_somethingInBuffer = true;
	}
	else
	{
		m_somethingInBuffer = false;
	}

	if(m_transform && m_CallBackActive)
    {
		image.pInfoHeader = &(pvi->bmiHeader);
		image.pDataOfBMP = pData;
		image.heightInPixels = cyImage;
		image.widthInPixels = cxImage;
		image.widthStepInBytes = stride;
		image.mediaSubType = pType->subtype;
        (*m_transform)(&image);
    }

	return NOERROR; 
}

STDMETHODIMP CUAProxyForceGrabber::SetCallback (void (*transform)(void*))
{
    CAutoLock cAutoLock(&m_UAProxyForceGrabberLock);
    m_transform = transform;
	m_CallBackActive = true;
    return NOERROR;
} 

STDMETHODIMP CUAProxyForceGrabber::SetCallback (void (*transform)(void*), bool activateCallback)
{
    CAutoLock cAutoLock(&m_UAProxyForceGrabberLock);
    m_transform = transform;
	m_CallBackActive = activateCallback;
    return NOERROR;
} 

STDMETHODIMP CUAProxyForceGrabber::ToggleCallBackActivation(bool activateCallback)
{
	CAutoLock cAutoLock(&m_UAProxyForceGrabberLock);
	m_CallBackActive = activateCallback;
	return NOERROR;
}

STDMETHODIMP CUAProxyForceGrabber::ToggleCallFrameBufferingForGrabbing(bool activateFrameBufferForGrabbing)
{
	CAutoLock cAutoLock(&m_UAProxyForceGrabberLock);
	m_frameBufferActive = activateFrameBufferForGrabbing;
	return NOERROR;
}

STDMETHODIMP CUAProxyForceGrabber::GrabFrame(BYTE *pBufferForSample, int *pBytesInBuffer, void *pBitMapInfoHeaderStruc)
{

	if(m_frameBufferActive == false)
	{
		m_frameBufferActive = true;
		return E_ABORT;
	}

	if((m_pBufferForGrabSample == NULL) || (m_somethingInBuffer == false))
	{
		return E_FAIL;
	}	

	CAutoLock cAutoLock(&m_UAProxyForceGrabberGrabFrameLock);

	//*
	AM_MEDIA_TYPE* pType = &m_pInput->CurrentMediaType();
	REFERENCE_TIME t = m_pInput->SampleProps ()->tStart;
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pType->pbFormat;

	// Get the image properties from the BITMAPINFOHEADER
	int cxImage    = m_frameGrabberInfoStruct.widthInPixels;
	int cyImage    = m_frameGrabberInfoStruct.heightInPixels;
	int stride     = m_frameGrabberInfoStruct.widthStepInBytes;

	if((stride * cyImage) > *pBytesInBuffer)
	{
		*pBytesInBuffer = stride * cyImage;
		return E_ABORT;
	}
	
	if(pBufferForSample == NULL)
	{
		return E_POINTER;
	}

	*pBytesInBuffer = stride * cyImage;
	memcpy(pBufferForSample, m_pBufferForGrabSample, *pBytesInBuffer);
	
	if(pBitMapInfoHeaderStruc != NULL)
	{
		BITMAPINFOHEADER *temp = NULL;
		if(((ssi::CALLBACKBMPINFO*)pBitMapInfoHeaderStruc)->pInfoHeader != NULL)
		{
			temp = ((ssi::CALLBACKBMPINFO*)pBitMapInfoHeaderStruc)->pInfoHeader;
			memcpy(temp, &(pvi->bmiHeader), sizeof(BITMAPINFOHEADER));
		}
		memcpy(pBitMapInfoHeaderStruc, &(m_frameGrabberInfoStruct), sizeof(m_frameGrabberInfoStruct));
		((ssi::CALLBACKBMPINFO*)pBitMapInfoHeaderStruc)->pDataOfBMP = pBufferForSample;
		((ssi::CALLBACKBMPINFO*)pBitMapInfoHeaderStruc)->pInfoHeader = temp;
	}
	
	if ( (m_lasttimestamp == t) ) {
		if (_framecounter++ == 5){
			return E_PENDING;
		}
	}
	else{
		m_lasttimestamp = t;
		_framecounter = 0;
	}
	
	return NOERROR;
}

STDMETHODIMP CUAProxyForceGrabber::GetConnectedMediaType(AM_MEDIA_TYPE **ppMediaType)
{
	if(m_pMediaType == NULL)
	{
		return E_FAIL;
	}

	if(ppMediaType == NULL)
	{
		return E_POINTER;
	}

	*ppMediaType = CreateMediaType( m_pMediaType);
	return NOERROR;
}

//
// NonDelegatingQueryInterface
//
// Reveals ITransformTemplate and ISpecifyPropertyPages
//
STDMETHODIMP CUAProxyForceGrabber::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IUAProxyForceGrabber)
	{
        return GetInterface((IUAProxyForceGrabber *) this, ppv);
    }
	//else if (riid == IID_ISpecifyPropertyPages)
	//{
    //    return GetInterface((ISpecifyPropertyPages *) this, ppv);
    //}
	else
	{
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

//
// CreateInstance
//
// Provide the way for COM to create a CUAProxyForceGrabber object
CUnknown * WINAPI CUAProxyForceGrabber::CreateInstance(LPUNKNOWN punk, HRESULT *phr) {

    CUAProxyForceGrabber *pNewObject = new CUAProxyForceGrabber(NAME("UAProxyForceGrabber"), punk, phr );
    if (pNewObject == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return pNewObject;
}

//
// CheckInputType
//
// Check a transform can be done.
//
HRESULT CUAProxyForceGrabber::CheckInputType(const CMediaType *mtIn)
{
    if (CanPerformTransform(mtIn))
		return S_OK;
	else
	    return VFW_E_TYPE_NOT_ACCEPTED;
}

//
// CanPerformTransform
//
// We support RGB24 and RGB32 input
//
BOOL CUAProxyForceGrabber::CanPerformTransform(const CMediaType *pMediaType) const
{
    if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)) {
        if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24)) {
            VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format();
            return (pvi->bmiHeader.biBitCount == 24);
        }
        /*if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB32)) {
            VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format();
            return (pvi->bmiHeader.biBitCount == 32);
        }*/
    }
    return FALSE;
} 
