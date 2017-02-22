// OverlapBuffer.h
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


#pragma once

#include "SSI_Cons.h"

#ifndef SSI_BIOSIG_OVERLAPBUFFER_H
#define SSI_BIOSIG_OVERLAPBUFFER_H

namespace ssi {

class OverlapBuffer {

public:

	OverlapBuffer (ssi_size_t n_overlap, ssi_size_t n_dimensions);
	virtual ~OverlapBuffer ();

	void push (ssi_size_t n_samples, 
		ssi_real_t *samples);
	ssi_real_t& operator[](unsigned int i);
	const ssi_real_t& operator[](unsigned int i)const;
	ssi_size_t convertRelativeToAbsoluteSampleIndex (ssi_size_t i) {
		if (!_first_push_over) 
			return i;
		return _n_total - _n_samples + i - _n_overlap;
	}

	ssi_size_t size () { 
		if (!_first_push_over) 
			return _n_samples;
		return _n_samples + _n_overlap;
	}
	ssi_size_t total ()  { return _n_total; };

protected:

	ssi_size_t _n_dimensions;
	ssi_real_t *_overlap;
	ssi_real_t *_overlap_next;
	ssi_size_t _n_overlap;
	ssi_size_t _n_overlap_values;
	ssi_real_t *_samples;
	ssi_size_t _n_samples;
	ssi_size_t _n_sample_values;
	ssi_size_t _n_total;
	bool _first_push, _first_push_over;

};

}

#endif
