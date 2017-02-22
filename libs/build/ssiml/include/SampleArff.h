// SampleArff.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/12/09
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

#ifndef SSI_IOPUT_SAMPLEARFF_H
#define SSI_IOPUT_SAMPLEARFF_H

#include "base/ISamples.h"

namespace ssi {

class ArffData;
class ArffAttr;
class ArffParser;

class SampleArff : public ISamples {

public:

	SampleArff (const ssi_char_t *path, ssi_size_t class_index = 0);
	virtual ~SampleArff ();

	void reset () {
		_next_ind = 0;
	}
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next ();

	ssi_size_t getSize () {
		return _n_samples;
	}
	ssi_size_t getSize (ssi_size_t class_index);

	ssi_size_t getClassSize () {
		return _n_classes;
	}
	const ssi_char_t *getClassName (ssi_size_t index);

	ssi_size_t getUserSize () {
		return 1;
	};
	const ssi_char_t *getUserName (ssi_size_t index) {
		return "user";
	};

	ssi_size_t getStreamSize () {
		return 1;
	}
	ssi_stream_t getStream (ssi_size_t index) {
		return _stream;
	}

	bool hasMissingData () { 
		return false; 
	};
	bool supportsShallowCopy () { 
		return false; 
	};	

protected:

	ArffData *_data;
	ArffAttr *_class;
	ArffParser *_parser;

	ssi_size_t _n_features;
	ssi_size_t _n_classes;
	ssi_size_t _n_nominals;
	ssi_size_t *_ind_nominals;
	ssi_size_t _n_strings;
	ssi_size_t *_ind_strings;
	ssi_char_t **_class_names;
	ssi_size_t _class_ind;
	ssi_size_t _n_samples;
	ssi_size_t *_n_samples_per_class;
	ssi_size_t _n_garbage_class;
	ssi_stream_t _stream;
	ssi_size_t _next_ind;
	ssi_sample_t _sample;
};

}

#endif
