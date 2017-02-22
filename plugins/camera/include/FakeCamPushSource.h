// FakePushCamSource.h
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

#pragma once

#ifndef SSI_SENSOR_FAKEPUSHCAMSOURCE_H
#define SSI_SENSOR_FAKEPUSHCAMSOURCE_H

#include "iFakeCamPushControl.h"

// {CFFA5BB5-9B13-4fcf-8FDC-2B8C22A0E8F6}
DEFINE_GUID(CLSID_FakeCamPushSource, 
0xcffa5bb5, 0x9b13, 0x4fcf, 0x8f, 0xdc, 0x2b, 0x8c, 0x22, 0xa0, 0xe8, 0xf6);

class CPushPinFakeCamSource : public CSourceStream,
							  //public IAMStreamConfig
							  public IFakeCamPushControl,
							  public IKsPropertySet
{
protected:

    unsigned int m_FramesWritten;				// To track where we are in the file
    BOOL m_bZeroMemory;                 // Do we need to clear the buffer?
    //CRefTime m_rtSampleTime;	        // The time stamp for each sample

    unsigned int m_iFrameNumber;
    REFERENCE_TIME m_rtFrameLength;


    int m_iImageHeight;                 // The current image height
    int m_iImageWidth;                  // And current image width
    int m_iRepeatTime;                  // Time in msec between frames
    int m_nCurrentBitDepth;             // Screen bit depth
	int m_iImageStride;
	int m_nFlipFactor;
	int m_nMirrorFactor;
	int m_iNumberOfSkippedFrames;
	int m_nChannels;

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

	BYTE *m_pSSIImageData;

public:

	DECLARE_IUNKNOWN;

    CPushPinFakeCamSource(HRESULT *phr, CSource *pFilter);
    ~CPushPinFakeCamSource();

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

	
	STDMETHODIMP PumpSampleIntoFilter(BYTE *pSampleBuffer, LONG nBufferSize);
	STDMETHODIMP SetVideoFormatForSource(double fps, int width, int height, int channels, BOOL bFlipSample, BOOL bMirrorSample);
	STDMETHODIMP GetConnectedMediaType(AM_MEDIA_TYPE **ppMediaType);
	STDMETHODIMP SignalEndOfStream();
	STDMETHODIMP AddSkippedFrames(int numberOfSkippedFramesToAdd);
};

class CFakeCamPushSource : public CSource
{
protected:
	STDMETHODIMP GetState(DWORD dwMSecs, __out FILTER_STATE *pState);

private:
    
    CPushPinFakeCamSource *m_pPin;

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);
	// Constructor is private because you have to use CreateInstance
    CFakeCamPushSource(IUnknown *pUnk, HRESULT *phr);
    ~CFakeCamPushSource();


};

#endif
