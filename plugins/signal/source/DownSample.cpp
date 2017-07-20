// DownSample.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/03/12
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

#include "DownSample.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

DownSample::DownSample (const ssi_char_t *file)
	: _tmp(0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

DownSample::~DownSample () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void DownSample::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_options.keep == 0)
	{
		_options.keep = 1;
	}

	_tmp = new ssi_real_t[stream_in.dim];
	for (ssi_size_t j = 0; j < stream_in.dim; j++)
	{
		_tmp[j] = 0;
	}
	_count = 0;
}

void DownSample::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	// down sample
	for (ssi_size_t i = 0; i < sample_number; i++)
	{
		_count++;		
		if (_count == _options.keep || _options.mean)
		{
			for (ssi_size_t j = 0; j < sample_dimension; j++)
			{
				_tmp[j] += *srcptr++;
			}
		}
		else
		{
			srcptr += sample_dimension;
		}

		if (_count == _options.keep)
		{
			for (ssi_size_t j = 0; j < sample_dimension; j++)
			{
				*dstptr++ = _tmp[j];
				_tmp[j] = 0;
			}
			_count = 0;
		}
	}
}

void DownSample::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete[] _tmp; _tmp = 0;
}

}
