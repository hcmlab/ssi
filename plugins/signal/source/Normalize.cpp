// Normalize.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/09/18
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

#include "Normalize.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Normalize::Normalize (const ssi_char_t *file)
	: _min_abs (0),
	_max_abs (0),
	_min_rel (0),
	_max_rel (0),
	_min_val (0),
	_max_val (0),
	_min_pos (0),
	_max_pos (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Normalize::~Normalize () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void Normalize::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;

	_min_abs = _options.minval;
	_max_abs = _options.maxval;

	_min_rel = new ssi_real_t[sample_dimension];
	_max_rel = new ssi_real_t[sample_dimension];
	_min_val = new ssi_real_t[sample_dimension];
	_max_val = new ssi_real_t[sample_dimension];
	_min_pos = new ssi_size_t[sample_dimension];
	_max_pos = new ssi_size_t[sample_dimension];

	_first_call = true;
}

void Normalize::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	// do normalization
	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	// find relative min, max
	if (_first_call) {
		ssi_minmax (sample_number, sample_dimension, srcptr, _min_rel, _min_pos, _max_rel, _max_pos);
		_first_call = false;
	} else {
		ssi_minmax (sample_number, sample_dimension, srcptr, _min_val, _min_pos, _max_val, _max_pos);
		for (ssi_size_t i = 0; i < sample_dimension; i++) {
			if (_min_val[i] < _min_rel[i]) {
				_min_rel[i] = _min_val[i];
			}
			if (_max_val[i] > _max_rel[i]) {
				_max_rel[i] = _max_val[i];
			}
		}
	}

	// normalize
	for (ssi_size_t i = 0; i < sample_number; i++) {
		for (ssi_size_t j = 0; j < sample_dimension; j++) {
			ssi_real_t value = *srcptr++;
			if (_max_rel[j] - _min_rel[j] == 0) {
				*dstptr++ = _min_abs;
			} else {
				*dstptr++ = (_max_abs - _min_abs) * ((value - _min_rel[j]) / (_max_rel[j] - _min_rel[j])) + _min_abs;
			}
		}
	}

}

void Normalize::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

	delete[] _min_rel; _min_rel = 0;
	delete[] _max_rel; _max_rel = 0;
	delete[] _min_val; _min_val = 0;
	delete[] _max_val; _max_val = 0;
	delete[] _min_pos; _min_pos = 0;
	delete[] _max_pos; _max_pos = 0;
}


}
