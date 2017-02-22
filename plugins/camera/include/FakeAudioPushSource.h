// FakePushAudioSource.h
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

#pragma once

#ifndef SSI_SENSOR_FAKEPUSHAUDIOSOURCE_H_
#define SSI_SENSOR_FAKEPUSHAUDIOSOURCE_H_

#include "iFakeAudioPushControl.h"
#include "SSI_Cons.h"

// {1E68BA3D-72B2-4429-B62D-969AA76CC817}
DEFINE_GUID(CLSID_FakeAudioPushSource, 
0x1e68ba3d, 0x72b2, 0x4429, 0xb6, 0x2d, 0x96, 0x9a, 0xa7, 0x6c, 0xc8, 0x17);


class CPushPinFakeAudioSource : public CSourceStream,
							  //public IAMStreamConfig
							  public IFakeAudioPushControl,
							  public IKsPropertySet
{
protected:

    unsigned int m_FramesWritten;				// To track where we are in the file
    BOOL m_bZeroMemory;                 // Do we need to clear the buffer?
    //CRefTime m_rtSampleTime;	        // The time stamp for each sample

    unsigned int m_iSampleNumber;
    REFERENCE_TIME m_rtSampleLength;

	WAVEFORMATEX m_audio_format;        // The current audio format
	int m_maxSampleSize;
	int m_iRepeatTime;                  // Time in msec between frames
	int m_iNumberOfSkippedSamples;

    CMediaType m_MediaType;
    CCritSec m_cSharedState;            // Protects our internal state
	CCritSec m_cSharedState2;            // Protects our internal state
	CCritSec m_cSharedState3;            // Protects our internal state

	BOOL m_bEndStream;
	BOOL m_bIsFormatAlreadySet;
	LONG m_lBufferFillStatus;
	LONG m_lSSIBufferSize;

	HANDLE m_hWakeUpSampleArrived;
	HANDLE m_hWakeUpSampleBufferFlushed;

	ssi_stream_t m_pSSIAudioData;


public:

	DECLARE_IUNKNOWN;

    CPushPinFakeAudioSource(HRESULT *phr, CSource *pFilter);
    ~CPushPinFakeAudioSource();

    // Override the version that offers exactly one media type
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);
    
    // Set the agreed media type and set up the necessary parameters
    HRESULT SetMediaType(const CMediaType *pMediaType);

    // Support multiple display formats
    //HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(CMediaType *pmt);

    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

	// Reveals ITransformTemplate and ISpecifyPropertyPages
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
	STDMETHODIMP Get(
		REFGUID guidPropSet,   // Which property set.
		DWORD dwPropID,        // Which property in that set.
		void *pInstanceData,   // Instance data (ignore).
		DWORD cbInstanceData,  // Size of the instance data (ignore).
		void *pPropData,       // Buffer to receive the property data.
		DWORD cbPropData,      // Size of the buffer.
		DWORD *pcbReturned     // Return the size of the property.
		);
	STDMETHODIMP QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

	//IAMStreamConfig - Functions
	/*STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);
	STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
	STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
	STDMETHODIMP GetStreamCaps(int iIndex, AM_MEDIA_TYPE **ppmt, BYTE *pSCC);*/

	//STDMETHODIMP GetConnectedMediaType(AM_MEDIA_TYPE **ppMediaType);
	STDMETHODIMP PumpSampleIntoFilter(BYTE *pSampleBuffer, LONG nSamples);
	STDMETHODIMP SetAudioFormatForSource(WAVEFORMATEX &theWaveFormatExToSetAsOutput, int maxSampleSize);
	STDMETHODIMP SignalEndOfStream();
	STDMETHODIMP AddSkippedFrames(int numberOfSkippedSamplesToAdd);
};

class CFakeAudioPushSource : public CSource
{
protected:

	STDMETHODIMP GetState(DWORD dwMSecs, __out FILTER_STATE *pState);

private:
    
    CPushPinFakeAudioSource *m_pPin;

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);
	// Constructor is private because you have to use CreateInstance
    CFakeAudioPushSource(IUnknown *pUnk, HRESULT *phr);
    ~CFakeAudioPushSource();


};

#endif
