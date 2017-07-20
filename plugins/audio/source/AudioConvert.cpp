// AudioConvert.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2014/03/18
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

#include "AudioConvert.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

AudioConvert::AudioConvert (const ssi_char_t *file)
	: _file (0),
	_short_to_float (false) {

	if (file) {

		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	
	}
}

AudioConvert::~AudioConvert () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void AudioConvert::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_short_to_float = stream_in.type == SSI_SHORT;
}

void AudioConvert::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_short_to_float) {

        int16_t *srcptr = ssi_pcast (int16_t, stream_in.ptr);
		ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
		ssi_size_t N = stream_in.num * stream_in.dim;

		for (ssi_size_t i = 0; i < N; i++) {
			*dstptr++ = ssi_cast (ssi_real_t, *srcptr++) / 32768.0f;
		}		

	} else {

		ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
        int16_t *dstptr = ssi_pcast (int16_t, stream_out.ptr);
		ssi_size_t N = stream_in.num * stream_in.dim;
		
		for (ssi_size_t i = 0; i < N; i++) {
            *dstptr++ = ssi_cast (int16_t, *srcptr++ * 32768.0f);
		}		
	}	
}

void AudioConvert::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}


}

