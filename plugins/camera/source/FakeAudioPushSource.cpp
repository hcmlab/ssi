// FakePushAudioSource.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/07/23
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
#include "FakeAudioPushSource.h"



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

CPushPinFakeAudioSource::CPushPinFakeAudioSource(HRESULT *phr, CSource *pFilter)
        : CSourceStream(NAME("Push Source Fake Audio"), phr, pFilter, L"Output"),
        m_FramesWritten(0),
        m_bZeroMemory(0),
        m_iSampleNumber(0),
        m_rtSampleLength(0),
		m_bIsFormatAlreadySet(0),
		m_lBufferFillStatus(0),
		m_bEndStream(0),
		m_lSSIBufferSize(0),
		m_iNumberOfSkippedSamples(0),
		m_maxSampleSize (0)
{
	
	m_hWakeUpSampleArrived = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hWakeUpSampleBufferFlushed = CreateEvent(NULL, TRUE, FALSE, NULL);
	// In the filter graph, we connect this filter to the AVI Mux, which creates 
    // the AVI file with the video frames we pass to it. In this case, 
    // the end result is a screen capture video (GDI images only, with no
    // support for overlay surfaces).

	m_pSSIAudioData.ptr = 0;
}

CPushPinFakeAudioSource::~CPushPinFakeAudioSource()
{
	CloseHandle(m_hWakeUpSampleArrived);
	CloseHandle(m_hWakeUpSampleBufferFlushed);

	m_lSSIBufferSize = 0L;
	ssi_stream_destroy (m_pSSIAudioData);
	return;
}

// Set: Cannot set any properties.
STDMETHODIMP CPushPinFakeAudioSource::Set(REFGUID guidPropSet, DWORD dwID,
    void *pInstanceData, DWORD cbInstanceData, void *pPropData, 
    DWORD cbPropData)
{
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
STDMETHODIMP CPushPinFakeAudioSource::Get(
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
STDMETHODIMP CPushPinFakeAudioSource::QuerySupported(REFGUID guidPropSet, DWORD dwPropID,
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


HRESULT CPushPinFakeAudioSource::GetMediaType(CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
	
	if(m_bIsFormatAlreadySet == false)
	{
		return E_UNEXPECTED;
	}

    WAVEFORMATEX *pAud = (WAVEFORMATEX *) pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
    if(NULL == pAud)
        return(E_OUTOFMEMORY);

    // Initialize pAud structure before configuring its members
    ZeroMemory(pAud, sizeof(WAVEFORMATEX));
	memcpy (pAud, &m_audio_format, sizeof (WAVEFORMATEX));

	pmt->SetType(&MEDIATYPE_Audio);
    pmt->SetFormatType(&FORMAT_WaveFormatEx);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    pmt->SetSubtype(&MEDIASUBTYPE_PCM);
	//TODO?
    pmt->SetSampleSize(0);

	// prepare audio buffer
	ssi_stream_init (m_pSSIAudioData, 0, pAud->nChannels, pAud->wBitsPerSample / 8, SSI_SHORT, pAud->nSamplesPerSec);

    return S_OK;

} // GetMediaType

HRESULT CPushPinFakeAudioSource::SetMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Pass the call up to my base class
    HRESULT hr = CSourceStream::SetMediaType(pMediaType);
    return hr;

} // SetMediaType

//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT CPushPinFakeAudioSource::DecideBufferSize(IMemAllocator *pAlloc,
                                      ALLOCATOR_PROPERTIES *pProperties)
{
    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = S_OK;

    WAVEFORMATEX *pAud = (WAVEFORMATEX *) m_mt.Format();
	if (IsEqualGUID(m_mt.formattype, FORMAT_WaveFormatEx) == FALSE)
	{
		return E_FAIL;
	}
    pProperties->cBuffers = 1;
	pProperties->cbBuffer = m_maxSampleSize * pAud->nBlockAlign;

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

STDMETHODIMP CPushPinFakeAudioSource::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

	
	if(riid == IID_IFakeAudioPushControl)
	{
		return GetInterface((IFakeAudioPushControl *) this, ppv);
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
HRESULT CPushPinFakeAudioSource::FillBuffer(IMediaSample *pSample)
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

    // Check that we're still using audio
	//ASSERT(IsEqualGUID(m_mt.formattype, FORMAT_WaveFormatEx) == TRUE);

    //WAVEFORMATEX *pAud = (WAVEFORMATEX*)m_mt.pbFormat;		
	int bufferSize = min (ssi_cast (int, m_pSSIAudioData.num), m_maxSampleSize) * m_audio_format.nBlockAlign;
	ASSERT((int)m_pSSIAudioData.num <= m_maxSampleSize);

	memcpy (pData, m_pSSIAudioData.ptr, bufferSize);
	pSample->SetActualDataLength (bufferSize);
    
	// Set the timestamps that will govern playback frame rate.
	// If this file is getting written out as an AVI,
	// then you'll also need to configure the AVI Mux filter to 
	// set the Average Time Per Frame for the AVI Header.
    // The current time is the sample's start.
	REFERENCE_TIME rtStart = 0;
	REFERENCE_TIME rtStop = 0;
	{
		CAutoLock cAutoLockShared(&m_cSharedState3);
		m_iSampleNumber += m_iNumberOfSkippedSamples;
		m_iNumberOfSkippedSamples = 0;
		rtStart = m_iSampleNumber * m_rtSampleLength;
		m_iSampleNumber += m_pSSIAudioData.num;
		rtStop  = m_iSampleNumber * m_rtSampleLength;
	}

    pSample->SetTime(&rtStart, &rtStop);

	// Set TRUE on every sample for uncompressed frames
    pSample->SetSyncPoint(TRUE);

	SetEvent(m_hWakeUpSampleBufferFlushed);
	InterlockedCompareExchange(&m_lBufferFillStatus, 0L, 1L);
	
    return S_OK;
}

//STDMETHODIMP CPushPinFakeAudioSource::GetConnectedMediaType(AM_MEDIA_TYPE **ppMediaType)
//{
//	if(ppMediaType == NULL)
//	{
//		return E_POINTER;
//	}
//
//	CAutoLock cAutoLock(m_pFilter->pStateLock());
//
//	if(m_MediaType.IsValid())
//	{
//		return E_FAIL;
//	}
//
//	*ppMediaType = CreateMediaType( static_cast<AM_MEDIA_TYPE*>(&m_MediaType) );
//	return S_OK;
//}

STDMETHODIMP CPushPinFakeAudioSource::PumpSampleIntoFilter(BYTE *pSampleBuffer, LONG nSamples)
{
	CheckPointer(pSampleBuffer, E_POINTER);

	CAutoLock cAutoLockShared(&m_cSharedState2);


	ssi_stream_adjust (m_pSSIAudioData, nSamples);

	if(m_pSSIAudioData.ptr == NULL || m_bIsFormatAlreadySet == FALSE)
	{
		return E_FAIL;
	}

	while(m_lBufferFillStatus == 1L)
	{
		WaitForSingleObject(m_hWakeUpSampleBufferFlushed, INFINITE);
	}
	ResetEvent(m_hWakeUpSampleBufferFlushed);
	

	

	memcpy(m_pSSIAudioData.ptr, pSampleBuffer, m_pSSIAudioData.tot);


	SetEvent(m_hWakeUpSampleArrived);
	InterlockedCompareExchange(&m_lBufferFillStatus, 1L, 0L);

	return S_OK;
}

STDMETHODIMP CPushPinFakeAudioSource::SetAudioFormatForSource(WAVEFORMATEX &theWaveFormatExToSetAsOutput, int maxSampleSize)
{
	if(m_bIsFormatAlreadySet)
	{
		return E_FAIL;
	}

	m_maxSampleSize = maxSampleSize;
	memcpy (&m_audio_format, &theWaveFormatExToSetAsOutput, sizeof (WAVEFORMATEX));

	m_rtSampleLength = (LONGLONG)((double)UNITS / (double)theWaveFormatExToSetAsOutput.nSamplesPerSec);

	m_bIsFormatAlreadySet = TRUE;
	return S_OK;
}

STDMETHODIMP CPushPinFakeAudioSource::SignalEndOfStream()
{
	CAutoLock cAutoLockShared(&m_cSharedState2);
	m_bEndStream = TRUE;

	return S_OK;
}

STDMETHODIMP CPushPinFakeAudioSource::AddSkippedFrames(int numberOfSkippedSamplesToAdd)
{
	CAutoLock cAutoLockShared(&m_cSharedState3);

	m_iNumberOfSkippedSamples += numberOfSkippedSamplesToAdd;

	return S_OK;
}

/**********************************************
 *
 *  CFakeCamPushSource Class
 *
 **********************************************/

CFakeAudioPushSource::CFakeAudioPushSource(IUnknown *pUnk, HRESULT *phr)
           : CSource(NAME("FakeAudioPushSource"), pUnk, CLSID_FakeAudioPushSource)
{
    // The pin magically adds itself to our pin array.
    m_pPin = new CPushPinFakeAudioSource(phr, this);

	if (phr)
	{
		if (m_pPin == NULL)
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}  
}


CFakeAudioPushSource::~CFakeAudioPushSource()
{
    delete m_pPin;
}

STDMETHODIMP CFakeAudioPushSource::GetState(DWORD dwMSecs, __out FILTER_STATE *pState)
{
	CheckPointer(pState, E_POINTER);
    *pState = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;

}

CUnknown * WINAPI CFakeAudioPushSource::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
    CFakeAudioPushSource *pNewFilter = new CFakeAudioPushSource(pUnk, phr);

	if (phr)
	{
		if (pNewFilter == NULL) 
			*phr = E_OUTOFMEMORY;
		else
			*phr = S_OK;
	}
    return pNewFilter;

}
