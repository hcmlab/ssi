// AudioLoopBackHelper.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 15/6/2018
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

// Original Source: https://raw.githubusercontent.com/Tsarpf/windows-default-playback-device-to-pcm-file/master/AudioListener.cpp

//-----------------------------------------------------------
// Record an audio stream from the default audio capture
// device. The RecordAudioStream function allocates a shared
// buffer big enough to hold one second of PCM audio data.
// The function uses this buffer to stream data from the
// capture device. The main loop runs every 1/2 second.
//-----------------------------------------------------------

#include <initguid.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include "AudioLoopBackHelper.h"
#include <mmreg.h>
#include "AudioLoopBackInterface.h"
#include <stdio.h>

namespace ssi
{

	AudioLoopBackHelper::AudioLoopBackHelper(AudioLoopBackInterface* sink)
		: m_pSink(sink)
	{		
		CoInitialize(nullptr);
	}

	AudioLoopBackHelper::~AudioLoopBackHelper()
	{
		CoTaskMemFree(m_pwfx);
		SAFE_RELEASE(m_pEnumerator)
		SAFE_RELEASE(m_pDevice)
		SAFE_RELEASE(m_pAudioClient)
		SAFE_RELEASE(m_pCaptureClient)
	}

	bool AudioLoopBackHelper::Init(WAVEFORMATEX& format, bool scale)
	{
		HRESULT hr;
		REFERENCE_TIME hnsRequestedDuration = m_refTimesPerSec;

		hr = CoCreateInstance(
			m_CLSID_MMDeviceEnumerator, NULL,
			CLSCTX_ALL, m_IID_IMMDeviceEnumerator,
			(void**)&m_pEnumerator);

		if (hr) { return false; }

		hr = m_pEnumerator->GetDefaultAudioEndpoint(
			//eCapture, eConsole, &pDevice); //set this to capture from default recording device instead of render device
			eRender, eConsole, &m_pDevice);
		if (hr) { return false; }

		hr = m_pDevice->Activate(
			m_IID_IAudioClient, CLSCTX_ALL,
			NULL, (void**)&m_pAudioClient);
		if (hr) { return false; }

		hr = m_pAudioClient->GetMixFormat(&m_pwfx);
		if (hr) { return false; }

		//// find default format			
		//PROPVARIANT prop;
		//::IPropertyStore *store;
		//m_pDevice->OpenPropertyStore(STGM_READ, &store);
		//hr = store->GetValue(PKEY_AudioEngine_DeviceFormat, &prop);
		//PWAVEFORMATEX f = (PWAVEFORMATEX) prop.blob.pBlobData;	

		int n_bytes_per_sample = scale ? sizeof(float) : sizeof(short);
		m_pwfx->wBitsPerSample = 8 * n_bytes_per_sample;
		m_pwfx->wFormatTag = scale ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
		m_pwfx->nBlockAlign = m_pwfx->nChannels * n_bytes_per_sample;
		m_pwfx->nAvgBytesPerSec = m_pwfx->nSamplesPerSec * m_pwfx->nBlockAlign;
		m_pwfx->cbSize = 0;
		format = *m_pwfx;

		WAVEFORMATEX *pClosestMatch = 0;
		hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, m_pwfx, &pClosestMatch);
		if (hr) { return false; }		

		hr = m_pAudioClient->Initialize(
			//AUDCLNT_SHAREMODE_EXCLUSIVE,
			AUDCLNT_SHAREMODE_SHARED,
			//0, //set this instead of loopback to capture from default recording device
			AUDCLNT_STREAMFLAGS_LOOPBACK,
			hnsRequestedDuration,
			0,
			m_pwfx,
			NULL);
		if (hr) { return false; }

		// Get the size of the allocated buffer.
		hr = m_pAudioClient->GetBufferSize(&m_bufferFrameCount);
		if (hr) { return false; }

		hr = m_pAudioClient->GetService(
			m_IID_IAudioCaptureClient,
			(void**)&m_pCaptureClient);
		if (hr) { return false; }

		// Calculate the actual duration of the allocated buffer.
		m_hnsActualDuration = (double) m_refTimesPerSec * m_bufferFrameCount / m_pwfx->nSamplesPerSec;

		return !hr;
	}

	bool AudioLoopBackHelper::Start()
	{
		HRESULT hr;

		hr = m_pAudioClient->Start();  // Start recording.

		return !hr;
	}

	bool AudioLoopBackHelper::Read()
	{
		HRESULT hr;
		BYTE *pData;
		DWORD flags;
		UINT32 packetLength = 0;
		UINT32 numFramesAvailable;

		// Sleep for half the buffer duration.
		Sleep(m_hnsActualDuration / m_refTimesPerMS / 2);

		hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
		if (hr) { return false; }

		while (packetLength != 0)
		{
			// Get the available data in the shared buffer.
			hr = m_pCaptureClient->GetBuffer(
				&pData,
				&numFramesAvailable,
				&flags, NULL, NULL);
			if (hr) { return false; }

			if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
			{
				pData = NULL;  // Tell CopyData to write silence.
			}

			// Copy the available capture data to the audio sink.
			hr = m_pSink->CopyData(pData, numFramesAvailable);
			if (hr) { return false; }

			hr = m_pCaptureClient->ReleaseBuffer(numFramesAvailable);
			if (hr) { return false; }

			hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
			if (hr) { return false; }
		}
		
		return !hr;
	}

	bool AudioLoopBackHelper::Stop()
	{
		HRESULT hr;

		hr = m_pAudioClient->Stop();  // Stop recording.		

		return !hr;
	}
}