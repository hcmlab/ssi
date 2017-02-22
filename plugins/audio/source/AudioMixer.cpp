// AudioMixer.h
// author: Stephan Brehm <stephanbrehm@hotmail.de>
// created: 2014/06/17
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

#include "AudioMixer.h"

namespace ssi {

	char AudioMixer::ssi_log_name[] = "audiomix__";

	AudioMixer::AudioMixer(const ssi_char_t *file)
	{
	}

	AudioMixer::~AudioMixer() 
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "deconstruct");
	}

	void AudioMixer::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) 
	{
	}

	void AudioMixer::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) 
	{
		ssi_real_t *base_in = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
		ssi_size_t n_bytes = stream_in.byte * stream_in.dim;

		for(ssi_size_t num = 0; num <stream_in.num; num++){
			ssi_real_t sum = base_in[num];
			for (ssi_size_t stream = 0; stream < xtra_stream_in_num; stream++){
				ssi_real_t *tmp_ptr = ssi_pcast(ssi_real_t, xtra_stream_in[stream].ptr);
				sum += tmp_ptr[num];
			}
			sum = tanh(sum);
			memcpy(&ptr_out[num], &sum, sizeof(ssi_real_t));
		}
	}

	void AudioMixer::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) 
	{
	}
}
