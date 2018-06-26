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

// Original Source: https://raw.githubusercontent.com/Tsarpf/windows-default-playback-device-to-pcm-file/master/AudioListener.h

#pragma once

#ifndef SSI_AUDIO_AUDIOLOOPBACKHELPER_H
#define SSI_AUDIO_AUDIOLOOPBACKHELPER_H

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>
#include "AudioLoopBackInterface.h"

namespace ssi
{

	class AudioLoopBackHelper
	{
	private:
#define SAFE_RELEASE(punk)  \
				  if ((punk) != NULL)  \
					{ (punk)->Release(); (punk) = NULL; }

		WAVEFORMATEX* m_pwfx = NULL;

		AudioLoopBackInterface* m_pSink = NULL;
		IAudioClient* m_pAudioClient = NULL;
		IAudioCaptureClient* m_pCaptureClient = NULL;
		IMMDeviceEnumerator* m_pEnumerator = NULL;
		IMMDevice* m_pDevice = NULL;

		UINT32 m_bufferFrameCount;
		REFERENCE_TIME m_hnsActualDuration;

		const int m_refTimesPerMS = 10000;
		const int m_refTimesPerSec = 10000000;

		const CLSID m_CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
		const IID m_IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
		const IID m_IID_IAudioClient = __uuidof(IAudioClient);
		const IID m_IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

	public:

		AudioLoopBackHelper(AudioLoopBackInterface* sink);
		~AudioLoopBackHelper();

		bool Init(WAVEFORMATEX& format, bool scale);
		bool Start();
		bool Read();
		bool Stop();
	};

}

#endif