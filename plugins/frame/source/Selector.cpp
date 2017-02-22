// Selector.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/08/21
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "Selector.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Selector::Selector (const ssi_char_t *file) 
	: _n_select(0),
	_select (0),
	_offset (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Selector::~Selector () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void Selector::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t maxbit = 0;

	if (!_select && !Parse(_options.indices, stream_in.dim)) {
		_n_select = 0;
	} else {

		maxbit = _select[0];

		//generate offset values
		_offset = new int[_n_select + 1];

		_offset[0] = _select[0] * sizeof(float);
		for (ssi_size_t i = 1; i < _n_select; i++) {
			_offset[i] = ((int) _select[i] - (int) _select[i-1]) * sizeof(float); 
			if (maxbit < _select[i]) {
				maxbit = _select[i];
			}
		}

		//save one extra offset that closes the gap between last selected dimension and highest dimension in stream
		_offset[_n_select] = (stream_in.dim - _select[_n_select - 1]) * sizeof(float);
	}

	if (maxbit >= stream_in.dim) {
		ssi_err ("selected index '%u' exceeds dimensions '%u'", maxbit, stream_in.dim);
	}
}

void Selector::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t num = stream_in.num;
	ssi_size_t dim = stream_in.dim;
	ssi_size_t byte = stream_in.byte;

	ssi_byte_t *ptr_in = stream_in.ptr;
	ssi_byte_t *ptr_out = stream_out.ptr;

	if (0 == _n_select) {
		memcpy(ptr_out, ptr_in, stream_out.tot);
	} else {
		for (ssi_size_t i = 0; i < stream_out.num; i++) {
			for (ssi_size_t j = 0; j < _n_select; j++) {
				ptr_in += _offset[j];
				memcpy(ptr_out, ptr_in, sizeof(float));
				ptr_out += stream_out.byte;
			}
			ptr_in += _offset[_n_select];
		}
	}
}

void Selector::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete[] _select; _select = 0;
	delete[] _offset; _offset = 0;
	_select = 0;
	
}

bool Selector::Parse (const ssi_char_t *indices, ssi_size_t n_values) {
	
	if (!indices || indices[0] == '\0') {
		return false;
	}

	ssi_size_t n_select_org;
	int *select_org = ssi_parse_indices(indices, n_select_org, _options.sort, ",");

	if (n_select_org == 0) {
		return false;
	}

	ssi_size_t n_iter = (_options.multiples > 0 && n_values > _options.multiples) ? n_values / _options.multiples : 1;
	_n_select = n_iter * n_select_org;
	_select = new ssi_size_t[_n_select];
	ssi_size_t k = 0;
	for (ssi_size_t j = 0; j < n_iter; j++) {
		for (ssi_size_t i = 0; i < n_select_org; i++) {
			_select[k++] = min(n_values - 1, select_org[i] + j * _options.multiples);
		}
	}

	delete[] select_org;

	return true;
}

}
