// Bundle.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/02/20
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#include "Bundle.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *Bundle::ssi_log_name = "bundle____";

Bundle::Bundle (const ssi_char_t *file) 
	: _thres(0),
	_hang_in (0),
	_hang_out(0),
	_replace_above(REPLACE::PREVIOUS),
	_replace_below(REPLACE::PREVIOUS),
	_set_above(0),
	_set_below(0),
	_above(0),
	_set_all(false),
	_first_call(false),
	_last(0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Bundle::~Bundle () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}	
}

void Bundle::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_thres = _options.thres;
	_hang_in = _options.hang_in;
	_hang_out = _options.hang_out;
	_replace_above = _options.replace_above;
	_replace_below = _options.replace_below;
	_set_above = _options.set_above;
	_set_below = _options.set_below;
	_set_all = _options.set_all;
	_above = new bool[stream_in.dim];		
	_first_call = true;
	_last = new ssi_real_t[stream_in.dim];
}

void Bundle::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *src = ssi_pcast(ssi_real_t, stream_in.ptr);
	ssi_real_t *dst = ssi_pcast(ssi_real_t, stream_out.ptr);

	if (_first_call) {

		if (info.delta_num < _hang_in) {
			ssi_err("#delta '%u' < #hang_in '%u'", info.delta_num, _hang_in);
		}
		if (info.delta_num < _hang_out) {
			ssi_err("#delta '%u' < #hang_out '%u'", info.delta_num, _hang_out);
		}

		for (ssi_size_t j = 0; j < stream_in.dim; j++) {
			_above[j] = src[j] > _thres;
			_last[j] = src[j];
		}		

		_first_call = false;
	}
	
	for (ssi_size_t i = 0; i < stream_in.num - info.delta_num; i++) {
		for (ssi_size_t j = 0; j < stream_in.dim; j++) {			
			_above[j] = check(src, dst++, _last[j], stream_in.dim, _above[j], _above[j] ? _hang_out : _hang_in);
		}
	}

	if (_set_all) {
		dst = ssi_pcast(ssi_real_t, stream_out.ptr);
		for (ssi_size_t i = 0; i < stream_in.num - info.delta_num; i++) {
			for (ssi_size_t j = 0; j < stream_in.dim; j++) {
				*dst = *dst <= _thres ? _set_below : _set_above;
				dst++;
			}
		}
	}

}

bool Bundle::check(ssi_real_t *in, ssi_real_t *out, ssi_real_t &last, ssi_size_t dim, bool above, ssi_size_t n) {

	ssi_size_t n_above = 0;
	for (ssi_size_t i = 0; i < n; i += dim) {
		if (in[i] > _thres) {
			n_above++;
		}
	}

	// previously below threshold and all values below threshold -> stay below
	if (!above && n_above == 0) {	
		last = *out = *in;
		return false;
	}

	// previously below threshold and all values above threshold -> go above
	if (!above && n_above == n) {
		last = *out = *in;
		return true;
	}

	// previously above threshold and all values above threshold -> stay above
	if (above && n_above == n) {
		last = *out = *in;
		return true;
	}

	// previously above threshold and all values below threshold -> go below
	if (above && n_above == 0) {
		last = *out = *in;
		return false;
	}

	// if above / below threshold, but current value below / above replace with last value
	if (above && *in <= _thres) {	
		replace(in, out, last, dim, above, n, _replace_below);
	} else if (!above && *in > _thres) {		
		replace(in, out, last, dim, above, n, _replace_above);
	} else {
		last = *out = *in;
	}

	return above;
}

void Bundle::replace(ssi_real_t *in, ssi_real_t *out, ssi_real_t last, ssi_size_t dim, bool above, ssi_size_t n, REPLACE::List method) {
	
	switch (_replace_below) {

		case REPLACE::EXTREMUM: {
			ssi_real_t value = in[0];
			for (ssi_size_t i = 0; i < n; i += dim) {
				if ((!above && in[i] <= value) || (above && in[i] > value)) {
					value = in[i];
				}
			}
			*out = value;
			break;
		}

		case REPLACE::MEAN: {
			ssi_real_t value = 0;
			ssi_size_t count = 0;
			for (ssi_size_t i = 0; i < n; i += dim) {
				if ((above && in[i] > value) || (!above && in[i] <= value)) {
					value += in[i];
					count++;
				}
			}
			*out = value / count;
			break;
		}

		case REPLACE::PREVIOUS: {
			*out = last;
			break;
		}

		case REPLACE::THRESHOLD: {
			*out = _thres;
			break;
		}

		case REPLACE::SET: {
			*out = above ? _set_above : _set_below;
			break;
		}

	}
}

void Bundle::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete[] _above; _above = 0;
	delete[] _last; _last = 0;
}

}
