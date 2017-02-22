// AudioOut.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/08/19
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
//
// following a tutorial by David Overton (http://www.planet-source-code.com/vb/scripts/ShowCode.asp?txtCodeId=4422&lngWId=3)

#pragma once

#ifndef SSI_SENSOR_AUDIOOUT_H
#define SSI_SENSOR_AUDIOOUT_H

#include "AudioCons.h"
#include "thread/Event.h"

namespace ssi {

class AudioOut {

public:

	struct WaveBuffer {
		WAVEHDR header;
		ssi_size_t usedBytes;
		ssi_size_t id;
		bool locked;
		LARGE_INTEGER time; //debug
	};

	AudioOut();

	HWAVEOUT init (int device_id, WAVEFORMATEX wfx, ssi_size_t bufferSizeInBytes, ssi_size_t numBuffers);
	void clean (HWAVEOUT hWaveOut);

	static void CALLBACK waveOutProc (HWAVEOUT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR); 
	void allocateBuffers (int size, int count); 
	void freeBuffers (); 
	void writeAudio (HWAVEOUT hWaveOut, LPSTR data, int size);

	WaveBuffer* getFreeBuffer();
	void freeBuffer(ssi_size_t bufferID);

	bool checkResult (const ssi_char_t *func, MMRESULT err);

protected:

	ssi_size_t _bufferSize;
	ssi_size_t _numBuffers;

	WaveBuffer*		_waveBuffers; 

	int				_freeBufferCounter; 
	ssi_size_t		_currentBuffer;

	CRITICAL_SECTION	_waveCriticalSection; 
	Event				_freeBuffer;
};

}

#endif
