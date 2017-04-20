//------------------------------------------------------------------------------
// <copyright file="KinectAidopStream.h" company="Microsoft">
//   Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
// <summary>
//   Includes common headers and defines following classes:
//     - CStaticMediaBuffer: Helper class that implements IMediaBuffer
//     - MSSpeechAudioStream: IStream implementation that wraps Kinect audio DMO.
// </summary>
//------------------------------------------------------------------------------
#pragma once

// For IMediaObject and related interfaces
#include <dmo.h>

// For WAVEFORMATEX
#include <mmreg.h>

// For MMCSS functionality such as AvSetMmThreadCharacteristics
#include <avrt.h>

// Format of Kinect audio stream
static const WORD       AudioFormat = WAVE_FORMAT_PCM;

// Number of channels in Kinect audio stream
static const WORD       AudioChannels = 1;

// Samples per second in Kinect audio stream
static const DWORD      AudioSamplesPerSecond = 16000;

// Average bytes per second in Kinect audio stream
static const DWORD      AudioAverageBytesPerSecond = 32000;

// Block alignment in Kinect audio stream
static const WORD       AudioBlockAlign = 2;

// Bits per audio sample in Kinect audio stream
static const WORD       AudioBitsPerSample = 16;

/// <summary>
/// IMediaBuffer implementation for a statically allocated buffer.
/// </summary>
class CStaticMediaBuffer : public IMediaBuffer
{
public:
    // Constructor
    CStaticMediaBuffer() : m_dataLength(0) {}

    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        if (riid == IID_IUnknown)
        {
            AddRef();
            *ppv = (IUnknown*)this;
            return NOERROR;
        }
        else if (riid == IID_IMediaBuffer)
        {
            AddRef();
            *ppv = (IMediaBuffer*)this;
            return NOERROR;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    // IMediaBuffer methods
    STDMETHODIMP SetLength(DWORD length) {m_dataLength = length; return NOERROR;}
    STDMETHODIMP GetMaxLength(DWORD *pMaxLength) {*pMaxLength = sizeof(m_pData); return NOERROR;}
    STDMETHODIMP GetBufferAndLength(BYTE **ppBuffer, DWORD *pLength)
    {
        if (ppBuffer)
        {
            *ppBuffer = m_pData;
        }
        if (pLength)
        {
            *pLength = m_dataLength;
        }
        return NOERROR;
    }
    void Init(ULONG ulData)
    {
        m_dataLength = ulData;
    }

protected:
    // Statically allocated buffer used to hold audio data returned by IMediaObject
    BYTE m_pData[AudioSamplesPerSecond * AudioBlockAlign];

    // Amount of data currently being held in m_pData
    ULONG m_dataLength;
};

/// <summary>
/// Asynchronous IStream implementation that captures audio data from Kinect audio sensor in a background thread
/// and lets clients read captured audio from any thread.
/// </summary>
class MSSpeechAudioStream : public IStream
{
public:
    /////////////////////////////////////////////
    // MSSpeechAudioStream methods

    /// <summary>
    /// MSSpeechAudioStream constructor.
    /// </summary>
    MSSpeechAudioStream (ULONG n_buffer);

    /// <summary>
    /// MSSpeechAudioStream destructor.
    /// </summary>
    ~MSSpeechAudioStream ();

    /////////////////////////////////////////////
    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
    STDMETHODIMP_(ULONG) Release()
    {
        UINT ref = InterlockedDecrement(&m_cRef);
        if (ref == 0)
        {
            delete this;
        }
        return ref;
    }
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        if (riid == IID_IUnknown)
        {
            AddRef();
            *ppv = (IUnknown*)this;
            return S_OK;
        }
        else if (riid == IID_IStream)
        {
            AddRef();
            *ppv = (IStream*)this;
            return S_OK;
        }
        else
        {
            return E_NOINTERFACE;
        }
    }

    /////////////////////////////////////////////
    // IStream methods
    STDMETHODIMP Read(void *,ULONG,ULONG *);
    STDMETHODIMP Write(const void *,ULONG,ULONG *);
    STDMETHODIMP Seek(LARGE_INTEGER,DWORD,ULARGE_INTEGER *);
    STDMETHODIMP SetSize(ULARGE_INTEGER);
    STDMETHODIMP CopyTo(IStream *,ULARGE_INTEGER,ULARGE_INTEGER *,ULARGE_INTEGER *);
    STDMETHODIMP Commit(DWORD);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD);
    STDMETHODIMP Stat(STATSTG *,DWORD);
    STDMETHODIMP Clone(IStream **);

	// buffer access
    void Read (BYTE **data, ULONG* n_data);
    void Write (BYTE *data, UINT n_data);	
	void Stop ();

private:

	// running flag
	volatile bool _is_capturing;

    // audio buffer
    BYTE *_buffer;
	ULONG _n_buffer;
	ULONG _read_pos;	
	ULONG _write_pos;
	ULONG _n_available;
	ULONG _n_read;
    
    // Number of references to this object
    UINT m_cRef;

    // Event used to signal that there's captured audio data ready to be read
    HANDLE m_hDataReady;

    // Critical section used to synchronize multithreaded access to captured audio data
    CRITICAL_SECTION m_Lock;


};