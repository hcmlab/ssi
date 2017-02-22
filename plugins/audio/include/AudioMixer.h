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

#ifndef _SSI_AUDIO_AUDIOMIXER_H
#define _SSI_AUDIO_AUDIOMIXER_H

#include "ssi.h"
#include "base/IFilter.h"
using namespace ssi;

namespace ssi{

	class AudioMixer : public IFilter {

	public:

		static const ssi_char_t *GetCreateName() { return "STKAudioMixer"; };
		static IObject *Create(const ssi_char_t *file) { return new AudioMixer(file); };
		~AudioMixer();

		IOptions *getOptions() { return 0; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Mixes incoming audio signals"; };

		void transform_enter(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		void transform(ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		void transform_flush(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) 
		{
			return sample_dimension_in;
		}

		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) 
		{
			return sample_bytes_in;
		}

		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) 
		{
			return sample_type_in;
		}

	protected:

		AudioMixer(const ssi_char_t *file = 0);
		static char ssi_log_name[];

	};
}
#endif
