// EVector.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
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

#ifndef SSI_EVECTOR_H
#define SSI_EVECTOR_H

#include "SSI_Cons.h"

namespace ssi {

class EVector {

public:

	enum DECAY_TYPE {
		DECAY_TYPE_LIN = 0,
		DECAY_TYPE_EXP,
		DECAY_TYPE_HYP
	};

public:

	EVector();
	~EVector ();
	
	EVector (ssi_size_t dim, ssi_real_t weight, ssi_real_t speed, EVector::DECAY_TYPE type, ssi_real_t gradient, ssi_size_t time, bool decay_weight);
	void Release ();

	bool update(ssi_size_t framework_time, ssi_real_t *baseline);
	
	bool set_values(ssi_size_t dim, ssi_event_map_t *tuples);
	bool set_values(ssi_size_t dim, ssi_real_t *value);
	bool set_values_decay(ssi_size_t dim, const ssi_real_t *value);
	bool set_lifetime(ssi_size_t framework_time);
	
	//ssi_real_t* get_value_decay();
	const ssi_real_t *get_value_decay ();
	const ssi_real_t* get_value();
	ssi_real_t get_weight();
	ssi_real_t get_speed();
	EVector::DECAY_TYPE get_type();
	ssi_size_t get_time();
	ssi_real_t get_gradient();
	ssi_real_t get_norm();
	ssi_real_t get_norm_decay();
	bool get_does_decay_weight();
		
	void add_baseline(ssi_real_t *baseline);
	void subtract_baseline(ssi_real_t *baseline);
	bool norm_values();
	bool decay();
	
	void print();
	
protected:

	ssi_size_t _dim;
	EVector::DECAY_TYPE _type;
	ssi_real_t _gradient;

	ssi_size_t _creation_time;
	ssi_size_t _lifetime;

	ssi_real_t _speed;
	
	ssi_real_t *_value;
	ssi_real_t *_value_decay;
	ssi_real_t _norm;
	ssi_real_t _norm_decay;
	ssi_real_t *_param;
	ssi_real_t _starting_weight;
	ssi_real_t _current_weight;

	bool _decay_weight;
		
};

}

#endif
