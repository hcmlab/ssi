// AudioIn.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/06
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

#include "AudioIn.h"
#include "AudioHeader.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

AudioIn::AudioIn ()
	: _status (MMSYSERR_BADDEVICEID)
{
}

void AudioIn::Start ()
{
	_status = ::waveInStart(_handle);
	_isStarted = true;
}

void AudioIn::Stop ()
{
	_status = ::waveInStop(_handle);
	_isStarted = false;
}

AudioIn::~AudioIn ()
{
	if (Ok ())
	{
		::waveInReset(_handle);
		::waveInClose (_handle);
	}
}

bool AudioIn::Open (Event *event, WAVEFORMATEX *format, unsigned devId)
{
	_devId = devId;

	_status = ::waveInOpen (
        &_handle, 
        _devId, 
        format, 
		reinterpret_cast<DWORD> (event->handle),
        0, // callback instance data
        CALLBACK_EVENT);
	if (!Ok ())
		return false;

	return true;
}

void AudioIn::Prepare (AudioHeader * pHeader)
{
	_status = ::waveInPrepareHeader(_handle, pHeader, sizeof (WAVEHDR));
	CheckResult();
}

void AudioIn::SendBuffer (AudioHeader * pHeader)
{
	_status = ::waveInAddBuffer (_handle, pHeader, sizeof (WAVEHDR));
	CheckResult();
}

void AudioIn::Unprepare (AudioHeader * pHeader)
{
	_status = ::waveInUnprepareHeader (_handle, pHeader, sizeof (WAVEHDR));
	CheckResult();
}

void AudioIn::Reset ()
{
	if (Ok())
		::waveInReset (_handle);
}

void AudioIn::Close ()
{
	if (Ok ())
	{
		::waveInReset(_handle);
		if (::waveInClose (_handle) != 0) {
			ssi_wrn ("could not close audio device");				
		}
		_status = MMSYSERR_BADDEVICEID;
	}
}

// Returns position (sample count) from the beggining of recording
unsigned long AudioIn::GetSamplePosition ()
{
	MMTIME mtime;
	mtime.wType = TIME_SAMPLES;
	::waveInGetPosition (_handle, &mtime, sizeof (MMTIME));
	return mtime.u.sample;
}

void AudioIn::GetErrorText (char * buf, int len)
{
	::waveInGetErrorText (_status, reinterpret_cast<LPSTR> (buf), len);
}

bool AudioIn::CheckResult() {
	
	if(Ok())
		return true;

	char buf[164];
	GetErrorText (buf, sizeof (buf));
	ssi_wrn (buf);
	return false;
}

}
