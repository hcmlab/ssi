// OverlapBuffer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/09/28 
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

#include "OverlapBuffer.h"

namespace ssi {

OverlapBuffer::OverlapBuffer (ssi_size_t n_overlap, ssi_size_t n_dimensions) 
: _n_overlap (n_overlap), 
	_n_dimensions (n_dimensions),
	_first_push (true),
	_first_push_over (false),
	_n_total (0) {

	_n_overlap_values = _n_overlap * _n_dimensions;
	_overlap = new ssi_real_t[_n_overlap_values];
	_overlap_next = new ssi_real_t[_n_overlap_values];
}

OverlapBuffer::~OverlapBuffer () {

	delete[] _overlap; _overlap = 0;
	delete[] _overlap_next; _overlap_next = 0;
	_n_overlap = 0;
	_n_dimensions = 0;
	_n_total = 0;
	_first_push = true;
	_first_push_over = false;
}

void OverlapBuffer::push (ssi_size_t n_samples, 
	ssi_real_t *samples) {

	if (_first_push) {
		_first_push = false;
		_first_push_over = false;
	} else {
		_first_push_over = true;
		memcpy (_overlap, _overlap_next, _n_overlap_values * sizeof (ssi_real_t));
	}

	_n_samples = n_samples;
	_n_sample_values = _n_samples * _n_dimensions;
	_samples = samples;
	_n_total += n_samples;

	memcpy (_overlap_next, _samples + (_n_sample_values - _n_overlap_values), _n_overlap_values * sizeof (ssi_real_t)); 

}

ssi_real_t& OverlapBuffer::operator[](unsigned int i) {

	SSI_ASSERT (i < _n_sample_values + _n_overlap_values);

	if (!_first_push_over) {
		return _samples[i];
	} else {
		if (i < _n_overlap_values) {
			return _overlap[i];
		} else {
			return _samples[i - _n_overlap_values];
		}
	}
}

const ssi_real_t& OverlapBuffer::operator[](unsigned int i) const {

	SSI_ASSERT (i < _n_sample_values + _n_overlap_values);

	if (i < _n_overlap_values) {
		return _overlap[i];
	} else {
		return _samples[i - _n_overlap_values];
	}
}

}

