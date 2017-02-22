// MatlabSamples.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/29
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

#ifndef SSI_MATLABSAMPLES_H
#define SSI_MATLABSAMPLES_H

#include "SSI_Cons.h"
#include "base/ISamples.h"
#include "MatlabFile.h"

namespace ssi {

class MatlabSamples : public ISamples {

public:

	MatlabSamples (const ssi_char_t *path);
	~MatlabSamples ();	

	void reset ();
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 
	ssi_sample_t *next (ssi_size_t class_index);

	ssi_size_t getSize () { return _n_samples; };
	ssi_size_t getSize (ssi_size_t class_index);

	ssi_size_t getClassSize () { return _n_classes; };
	const ssi_char_t *getClassName (ssi_size_t index);
	
	ssi_size_t getUserSize () { return _n_users; };
	const ssi_char_t *getUserName (ssi_size_t index);
	
	ssi_size_t getStreamSize () { return _n_streams; };	
	ssi_stream_t getStream (ssi_size_t stream_index) { return _streams[stream_index]; };

	bool supportsShallowCopy () { return false; };
	bool hasMissingData () { return false; };

	void printInfo (FILE *file = stdout);
	void printSample (ssi_size_t index, FILE *file = stdout);

protected:

	MatlabFile _matfile;
	
	MatlabVar **_samples;
	ssi_size_t _n_streams;
	ssi_stream_t *_streams;

	ssi_real_t **_samples_ptr;
	ssi_size_t _counter;
	ssi_sample_t _sample;
	
	ssi_size_t _n_samples;
	ssi_size_t *_n_features;
	ssi_size_t _n_classes;
	ssi_size_t *_class_ids;
	ssi_size_t _n_users;
	ssi_size_t *_user_ids;
};

}

#endif
