// AudioIn.h
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
//
// following a tutorial by Reliable Software (http://www.relisoft.com/Freeware/recorder.html)

#pragma once

#ifndef SSI_SENSOR_AUDIOIN_H
#define SSI_SENSOR_AUDIOIN_H

#include "AudioCons.h"
#include "AudioHeader.h"
#include "thread/Event.h"

namespace ssi {

	class AudioFormat;

	class AudioIn {

	public:

		AudioIn ();
		~AudioIn ();
		bool Open (Event *event, WAVEFORMATEX *format, unsigned devId);
		void Close ();
		void Start ();
		void Stop ();
		void Prepare (AudioHeader * header);
		void Unprepare (AudioHeader * header);
		void SendBuffer (AudioHeader * header);

		void Reset ();
		bool Ok () { return _status == MMSYSERR_NOERROR; }
		bool IsInUse () { return _status == MMSYSERR_ALLOCATED; }
		bool CheckResult ();

		unsigned long GetSamplePosition ();
		unsigned GetError () { return _status; }
		void     GetErrorText (char* buf, int len);

	private:

		unsigned		_devId;
		HWAVEIN			_handle;
		MMRESULT		_status;
		bool			_isStarted;
	};
}
#endif // _WAVEIN_H
