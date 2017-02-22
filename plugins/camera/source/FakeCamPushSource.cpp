// FakePushCamSource.cpp
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
#include "FakeCamPushSource.h"



#ifdef _DEBUG
	#include "SSI_LeakWatcher.h"
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
#endif

	/**********************************************
 *
 *  CPushPinFakeCamSource Class
 *
 **********************************************/

CPushPinFakeCamSource::CPushPinFakeCamSource(HRESULT *phr, CSource *pFilter)
        : CSourceStream(NAME("Push Source Fake Cam"), phr, pFilter, L"Output"),
        m_FramesWritten(0),
        m_bZeroMemory(0),
        m_iFrameNumber(0),
        m_rtFrameLength(0),
        m_nCurrentBitDepth(0),
		m_iImageHeight(0),
		m_iImageWidth(0),
		m_nChannels (0),
		m_bIsFormatAlreadySet(0),
		m_lBufferFillStatus(0),
		m_bEndStream(0),
		m_pSSIImageData(0),
		m_lSSIBufferSize(0),
		m_nFlipFactor(1),
		m_nMirrorFactor (1),
		m_iImageStride (0),
		m_iNumberOfSkippedFrames(0)
{
	
	m_hWakeUpSampleArrived = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWakeUpSampleBufferFlushed = CreateEvent(NULL, TRUE, FALSE, NULL);
	// In the filter graph, we connect this filter to the AVI Mux, which creates 
    // the AVI file with the video frames we pass to it. In this case, 
    // the end result is a screen capture video (GDI images only, with no
    // support for overlay surfaces).

    
}

CPushPinFakeCamSource::~CPushPinFakeCamSource()
{
	CloseHandle(m_hWakeUpSampleArrived);
	CloseHandle(m_hWakeUpSampleBufferFlushed);

	m_lSSIBufferSize = 0L;
	delete[] m_pSSIImageData;
	return;
}

// Set: Cannot set any properties.
STDMETHODIMP CPushPinFakeCamSource::Set(REFGUID guidPropSet, DWORD dwID,
    void *pInstanceData, DWORD cbInstanceData, void *pPropData, 
    DWORD cbPropData)
{
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
STDMETHODIMP CPushPinFakeCamSource::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void *pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void *pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD *pcbReturned     // Return the size of the property.
)
{
    if (guidPropSet != AMPROPSETID_Pin) 
        return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)
        return E_POINTER;
    if (pcbReturned)
        *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)  // Caller just wants to know the size.
        return S_OK;
    if (cbPropData < sizeof(GUID)) // The buffer is too small.
        return E_UNEXPECTED;
    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
    return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
STDMETHODIMP CPushPinFakeCamSource::QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
    DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin)
        return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
        return E_PROP_ID_UNSUPPORTED;
    if (pTypeSupport)
        // We support getting this property, but not setting it.
        *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
    return S_OK;
}


HRESULT CPushPinFakeCamSource::GetMediaType(CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
	
	if(m_bIsFormatAlreadySet == false)
	{
		return E_UNEXPECTED;
	}

    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if(NULL == pvi)
        return(E_OUTOFMEMORY);

    // Initialize the VideoInfo structure before configuring its members
    ZeroMemory(pvi, sizeof(VIDEOINFO));

	switch(m_nCurrentBitDepth)
    {
        case 32:
        {    
            // Return our highest quality 32bit format
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount    = 32;
            break;
        }

        case 24:
        {   // Return our 24bit format
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount    = 24;
            break;
        }

        
		default:
		{
			return E_UNEXPECTED;
			break;
		}

    }

    // Adjust the parameters common to all formats
    pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth      = m_iImageWidth;
	pvi->bmiHeader.biHeight     = m_iImageHeight * m_nFlipFactor;
    pvi->bmiHeader.biPlanes     = 1;
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

	if(m_pSSIImageData == NULL)
	{
		m_pSSIImageData = new BYTE[pvi->bmiHeader.biSizeImage];
		m_lSSIBufferSize = pvi->bmiHeader.biSizeImage;
	}
	if(m_pSSIImageData == NULL)
	{
		m_lSSIBufferSize = 0L;
		return E_UNEXPECTED; 
	}
    return S_OK;

} // GetMediaType

HRESULT CPushPinFakeCamSource::SetMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Pass the call up to my base class
    HRESULT hr = CSourceStream::SetMediaType(pMediaType);

    if(SUCCEEDED(hr))
    {
        VIDEOINFO * pvi = (VIDEOINFO *) m_mt.Format();
        if (pvi == NULL)
            return E_UNEXPECTED;

        switch(pvi->bmiHeader.biBitCount)
        {
            case 24:    // RGB24
            case 32:    // RGB32
                // Save the current media type and bit depth
                m_MediaType = *pMediaType;

                if(m_nCurrentBitDepth != pvi->bmiHeader.biBitCount)
				{
					hr = E_INVALIDARG;
				}
				else
				{
					hr = S_OK;
				}
                break;

            default:
                // We should never agree any other media types
                ASSERT(FALSE);
                hr = E_INVALIDARG;
                break;
        }
    } 

    return hr;

} // SetMediaType

//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT CPushPinFakeCamSource::DecideBufferSize(IMemAllocator *pAlloc,
                                      ALLOCATOR_PROPERTIES *pProperties)
{
    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = S_OK;

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory. NOTE: the function
    // can succeed (return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted.
    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if(FAILED(hr))
    {
        return hr;
    }

    // Is this allocator unsuitable?
    if(Actual.cbBuffer < pProperties->cbBuffer)
    {
        return E_FAIL;
    }

    // Make sure that we have only 1 buffer (we erase the ball in the
    // old buffer to save having to zero a 200k+ buffer every time
    // we draw a frame)
    ASSERT(Actual.cBuffers == 1);
	SetEvent(m_hWakeUpSampleBufferFlushed);
    return S_OK;

} // DecideBufferSize

STDMETHODIMP CPushPinFakeCamSource::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

	
	if(riid == IID_IFakeCamPushControl)
	{
		return GetInterface((IFakeCamPushControl *) this, ppv);
	}
	//else if (riid == IID_IAMStreamConfig)
	//{
    //    return GetInterface((IAMStreamConfig *) this, ppv);
    //}
	else if(riid == IID_IKsPropertySet)
	{
		return GetInterface((IKsPropertySet *) this, ppv);
	}
	else
	{
        return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
    }
}

// FillBuffer is called once for every sample in the stream.
HRESULT CPushPinFakeCamSource::FillBuffer(IMediaSample *pSample)
{
	BYTE *pData;
    long cbData;

    CheckPointer(pSample, E_POINTER);

    CAutoLock cAutoLockShared(&m_cSharedState);

	while(m_lBufferFillStatus == 0L)
	{
		if(m_bEndStream == TRUE)
		{
			return S_FALSE;
		}

		WaitForSingleObject(m_hWakeUpSampleArrived, 200);
	}

	ResetEvent(m_hWakeUpSampleArrived);

    // Access the sample's data buffer
    pSample->GetPointer(&pData);
    cbData = pSample->GetSize();

    // Check that we're still using video
    ASSERT(m_mt.formattype == FORMAT_VideoInfo);

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

    int nSize = min(pVih->bmiHeader.biSizeImage, (DWORD) cbData);
	memcpy(pData, m_pSSIImageData, nSize);
    


	// Set the timestamps that will govern playback frame rate.
	// If this file is getting written out as an AVI,
	// then you'll also need to configure the AVI Mux filter to 
	// set the Average Time Per Frame for the AVI Header.
    // The current time is the sample's start.
	REFERENCE_TIME rtStart = 0;
	REFERENCE_TIME rtStop = 0;
	{
		CAutoLock cAutoLockShared(&m_cSharedState3);
		m_iFrameNumber += m_iNumberOfSkippedFrames;
		m_iNumberOfSkippedFrames = 0;
		rtStart = m_iFrameNumber * m_rtFrameLength;
		++m_iFrameNumber;
		rtStop  = m_iFrameNumber * m_rtFrameLength;
	}

    pSample->SetTime(&rtStart, &rtStop);

	// Set TRUE on every sample for uncompressed frames
    pSample->SetSyncPoint(TRUE);

	SetEvent(m_hWakeUpSampleBufferFlushed);
	InterlockedCompareExchange(&m_lBufferFillStatus, 0L, 1L);
	
    return S_OK;
}

STDMETHODIMP CPushPinFakeCamSource::GetConnectedMediaType(AM_MEDIA_TYPE **ppMediaType)
{
	if(ppMediaType == NULL)
	{
		return E_POINTER;
	}

	CAutoLock cAutoLock(m_pFilter->pStateLock());

	if(m_MediaType.IsValid())
	{
		return E_FAIL;
	}

	*ppMediaType = CreateMediaType( static_cast<AM_MEDIA_TYPE*>(&m_MediaType) );
	return S_OK;
}

STDMETHODIMP CPushPinFakeCamSource::PumpSampleIntoFilter(BYTE *pSampleBuffer, LONG nBufferSize)
{
	CheckPointer(pSampleBuffer, E_POINTER);

	CAutoLock cAutoLockShared(&m_cSharedState2);

	if(m_pSSIImageData == NULL || m_bIsFormatAlreadySet == FALSE)
	{
		return E_FAIL;
	}

	while(m_lBufferFillStatus == 1L)
	{
		WaitForSingleObject(m_hWakeUpSampleBufferFlushed, INFINITE);
	}
	ResetEvent(m_hWakeUpSampleBufferFlushed);

	if (m_nMirrorFactor == -1)
	{
		BYTE *dstptr;
		const BYTE *srcptr;
		for(int j = 0; j < m_iImageHeight; ++j)
		{
			//dstptr = m_pSSIImageData + ((m_nFlipFactor==1)?((m_iImageHeight) - 1 - j):(j)) * m_iImageStride;
			dstptr = m_pSSIImageData + j * m_iImageStride;
			srcptr = pSampleBuffer + j * m_iImageStride + ((m_iImageWidth - 1) * m_nChannels);
			for (int i = 0; i < m_iImageWidth; i++)
			{
				memcpy(dstptr, srcptr, m_nChannels);
				dstptr +=m_nChannels;
				srcptr -=m_nChannels;
			}
		}
	} else {
		memcpy(m_pSSIImageData, pSampleBuffer, min(m_lSSIBufferSize, nBufferSize));
	}

	SetEvent(m_hWakeUpSampleArrived);
	InterlockedCompareExchange(&m_lBufferFillStatus, 1L, 0L);

	return S_OK;
}

STDMETHODIMP CPushPinFakeCamSource::SetVideoFormatForSource(double fps, int width, int height, int channels, BOOL bFlipSample, BOOL bMirrorSample )
{
	if(m_bIsFormatAlreadySet)
	{
		return E_FAIL;
	}

	m_nChannels = channels;
	switch(channels)
	{
	case 3:
	case 4:
		m_nCurrentBitDepth = 8 * channels;
		break;
	default:
		return E_INVALIDARG;
		break;
	}

	m_rtFrameLength = (LONGLONG)((double)UNITS / fps);
	m_iImageWidth = width;
	m_iImageHeight = height;
	m_iImageStride = ((((m_iImageWidth * channels * (8 & ~0x80000000) + 7) >> 3) + 3) & (~3));

	if(bFlipSample)
		m_nFlipFactor = -1;

	if(bMirrorSample)
		m_nMirrorFactor = -1;

	m_bIsFormatAlreadySet = TRUE;
	return S_OK;
}

STDMETHODIMP CPushPinFakeCamSource::SignalEndOfStream()
{
	CAutoLock cAutoLockShared(&m_cSharedState2);
	m_bEndStream = TRUE;

	return S_OK;
}

STDMETHODIMP CPushPinFakeCamSource::AddSkippedFrames(int numberOfSkippedFramesToAdd)
{
	CAutoLock cAutoLockShared(&m_cSharedState3);

	m_iNumberOfSkippedFrames += numberOfSkippedFramesToAdd;

	return S_OK;
}

/**********************************************
 *
 *  CFakeCamPushSource Class
 *
 **********************************************/

CFakeCamPushSource::CFakeCamPushSource(IUnknown *pUnk, HRESULT *phr)
           : CSource(NAME("FakeCamPushSource"), pUnk, CLSID_FakeCamPushSource)
{
    // The pin magically adds itself to our pin array.
    m_pPin = new CPushPinFakeCamSource(phr, this);

	if (phr)
	{
		if (m_pPin == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}  
}


CFakeCamPushSource::~CFakeCamPushSource()
{
    delete m_pPin;
}

STDMETHODIMP CFakeCamPushSource::GetState(DWORD dwMSecs, __out FILTER_STATE *pState)
{
	CheckPointer(pState, E_POINTER);
    *pState = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;

}

CUnknown * WINAPI CFakeCamPushSource::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
    CFakeCamPushSource *pNewFilter = new CFakeCamPushSource(pUnk, phr);

	if (phr)
	{
		if (pNewFilter == NULL) 
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
    return pNewFilter;

}
