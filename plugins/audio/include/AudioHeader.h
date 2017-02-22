// AudioHeader.h
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

#ifndef SSI_SENSOR_AUDIOHEADER_H
#define SSI_SENSOR_AUDIOHEADER_H

#include "AudioCons.h"
#include <memory>

namespace ssi
{
	class AudioHeader: public WAVEHDR
	{
	public:
		AudioHeader () {
			std::uninitialized_fill_n (ssi_pcast (char, this), sizeof (WAVEHDR), 0);
		}
		unsigned BufSize () const { return dwBufferLength; }
		bool IsDone () const { return (dwFlags & WHDR_DONE) != 0; }
	};
}

#endif // _WAVEHEADER_H
