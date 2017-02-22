// IDspFilter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/06/26
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

#include "DspFilters\Dsp.h"
#include "IDspFilter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *IDspFilter::ssi_log_name = "dspfilter_";

IDspFilter::IDspFilter ()
	: _filters (0),
	_n_channels (0),
	_n_per_channel (0),
	_channels (0),
	_offsets (0),
	_offset (false) {
}

IDspFilter::~IDspFilter () {
}

void IDspFilter::response (ssi_time_t sr,
	ssi_size_t n,
	ssi_real_t *impulse,
	bool clear) {

	DspFiltersTools::FilterProperty prop;
	prop.rate = sr;
	prop.n_channels = 1;
	prop.offset = false;
	Dsp::Filter *filter = createFilter (prop);

	if (clear) {
		Dsp::zero (n, impulse);
		impulse[0] = 1;
		impulse[1] = -1;
	}
	filter->process (n, &impulse);
	
	delete filter;
}

void IDspFilter::gain (ssi_time_t sr,
	ssi_size_t n, 
	ssi_real_t *gain) {

	DspFiltersTools::FilterProperty prop;
	prop.rate = sr;
	prop.n_channels = 1;
	prop.offset = false;
	Dsp::Filter *filter = createFilter (prop);

	for (ssi_size_t xi = 0; xi < n; ++xi )
    {
		float x = xi / float(n);
		float f = x;
		Dsp::complex_t c = filter->response (f/2.f);
		float y = float(std::abs(c));
		if (y < 1e-5f) {
			y = 1e-5f;
		}
		y = 20 * log10 (y);

		gain[xi] = y;
    }

	delete filter;
}

void IDspFilter::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_n_channels = stream_in.dim;
	_channels = new ssi_real_t *[_n_channels];
	_filters = new Dsp::Filter *[_n_channels];	
	_prop.rate = stream_in.sr;
	_prop.n_channels = stream_in.dim;
	for (ssi_size_t i = 0; i < _n_channels; i++) {
		_channels[i] = 0;		
		_filters[i] = createFilter (_prop);
		if (!_filters[i]) {
			ssi_err ("could not create dsp filter '%s'", DspFiltersTools::GetInfo (_prop));
		}
		_filters[i]->reset ();		
	}	
	_offset = _prop.offset;

	SSI_DBG (SSI_LOG_LEVEL_DEFAULT, "created '%s'", DspFiltersTools::GetInfo (_prop));
}		

void IDspFilter::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	
	ssi_size_t n = stream_in.num;
	ssi_real_t *src = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast (ssi_real_t, stream_out.ptr);

	if (_offset && !_offsets) {
		_offsets = new ssi_real_t[_n_channels];
		for (ssi_size_t i = 0; i < _n_channels; i++) {
			_offsets[i] = src[i];
		}
	}

	separate (n, src);	
	for (ssi_size_t i = 0; i < _n_channels; i++) {
		_filters[i]->process (n, _channels + i);
	}
	join (n, dst);
}

void IDspFilter::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
		
	for (ssi_size_t i = 0; i < _n_channels; i++) {
		delete[] _channels[i];
		delete _filters[i];
	}
	delete[] _channels; _channels = 0;
	delete[] _filters; _filters = 0;
	delete[] _offsets; _offsets = 0;
	_n_channels = 0;
	_n_per_channel = 0;
	
}

void IDspFilter::separate (ssi_size_t n, const ssi_real_t *data) {

	if (_n_per_channel < n) {
		for (ssi_size_t i = 0; i < _n_channels; i++) {
			delete[] _channels[i];
			_channels[i] = new ssi_real_t[n];
		}
		_n_per_channel = n;
	}

	if (_offset) {
		for (ssi_size_t i = 0; i < n; i++) {
			for (ssi_size_t j = 0; j < _n_channels; j++) {
				_channels[j][i] = *data++ - _offsets[j];
			}
		}
	} else {
		for (ssi_size_t i = 0; i < n; i++) {
			for (ssi_size_t j = 0; j < _n_channels; j++) {
				_channels[j][i] = *data++;
			}
		}
	}
}

void IDspFilter::join (ssi_size_t n, ssi_real_t *data) {

	if (_offset) {
		for (ssi_size_t i = 0; i < n; i++) {
			for (ssi_size_t j = 0; j < _n_channels; j++) {
				 *data++ = _offsets[j] + _channels[j][i];
			}
		}
	} else {
		for (ssi_size_t i = 0; i < n; i++) {
			for (ssi_size_t j = 0; j < _n_channels; j++) {
				 *data++ = _channels[j][i];
			}
		}
	}
}

}
