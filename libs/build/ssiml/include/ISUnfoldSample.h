// ISUnfoldSample.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2018/02/25
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

#ifndef SSI_MODEL_ISUNFOLDSAMPLE_H
#define SSI_MODEL_ISUNFOLDSAMPLE_H

#include "base/ISamples.h"

namespace ssi {

class ISUnfoldSample : public ISamples {

public:

	ISUnfoldSample (ISamples *samples);
	~ISUnfoldSample ();	

	void reset () { _next_counter = 0; };
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 

	ssi_size_t getSize () { return _n_samples; };
	ssi_size_t getSize (ssi_size_t class_id) { return _n_samples_per_class[class_id]; };

	ssi_size_t getClassSize () { return _samples->getClassSize (); };
	const ssi_char_t *getClassName (ssi_size_t class_id) { return _samples->getClassName (class_id); };
	
	ssi_size_t getUserSize () { return _samples->getUserSize (); };
	const ssi_char_t *getUserName (ssi_size_t user_id) { return _samples->getUserName (user_id); };

	ssi_size_t getStreamSize () { return 1; };
	ssi_stream_t getStream (ssi_size_t index) { return *_sample_out.streams[0]; };

	bool hasMissingData () { return _samples->hasMissingData (); };
	bool supportsShallowCopy () { return false; };

	bool set(ssi_size_t stream_index, ssi_size_t n_context = 0);

protected:

	void release ();

	ISamples *_samples;
	ssi_size_t _stream_index;
	ssi_size_t _n_context;
	ssi_sample_t _sample_out;
	ssi_size_t _next_counter;
	ssi_size_t _n_samples;
	ssi_size_t *_n_samples_per_class;

	struct entry {
		ssi_byte_t *ptr;
		ssi_size_t class_id;
		ssi_size_t user_id;
		ssi_real_t score;
		ssi_time_t time;
	};
	entry *_map;
	
};

}

#endif
