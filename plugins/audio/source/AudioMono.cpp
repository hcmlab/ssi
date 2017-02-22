// AudioMono.h
// author: Johannes Wager <wagner@hcm-lab.de>
// created: 2016/05/31
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

#include "AudioMono.h"
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

AudioMono::AudioMono(const ssi_char_t *file)
	: _file(0) {

	if (file) {

		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);

	}
}

AudioMono::~AudioMono() {

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}
}

void AudioMono::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void AudioMono::transform(ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.dim == 1)
	{
		memcpy(stream_out.ptr, stream_in.ptr, stream_out.tot);
	}
	else if (stream_in.type == SSI_SHORT)
	{
		int16_t *srcptr = ssi_pcast(int16_t, stream_in.ptr);
		int16_t *dstptr = ssi_pcast(int16_t, stream_out.ptr);
		
		ssi_size_t dim = stream_in.dim;
		ssi_size_t num = stream_in.num;

		if (_options.normalize)
		{
			for (ssi_size_t i = 0; i < num * dim; i++)
			{
				*srcptr++ /= dim;
			}
		}

		srcptr = ssi_pcast(int16_t, stream_in.ptr);

		for (ssi_size_t i = 0; i < num; i++)
		{
			for (ssi_size_t j = 0; j < dim; j++)
			{
				*dstptr += *srcptr++;
			}
			dstptr++;
		}
	} 
	else 
	{
		ssi_real_t *srcptr = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *dstptr = ssi_pcast(ssi_real_t, stream_out.ptr);

		ssi_size_t dim = stream_in.dim;
		ssi_size_t num = stream_in.num;

		if (_options.normalize)
		{
			for (ssi_size_t i = 0; i < num * dim; i++)
			{
				*srcptr++ /= dim;
			}
		}

		srcptr = ssi_pcast(ssi_real_t, stream_in.ptr);

		for (ssi_size_t i = 0; i < num; i++)
		{
			for (ssi_size_t j = 0; j < dim; j++)
			{
				*dstptr += *srcptr++;
			}
			dstptr++;
		}
	}
}

void AudioMono::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}


}

