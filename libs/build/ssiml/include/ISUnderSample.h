// ISUnderSample.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/07
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

#ifndef SSI_MODEL_ISUNDERSAMPLE_H
#define SSI_MODEL_ISUNDERSAMPLE_H

#include "base/ISamples.h"

namespace ssi {

class Randomf;

class ISUnderSample : public ISamples {

public:

enum Strategy {
	RANDOM = 0,
	KMEANS = 1
};

public:

	ISUnderSample (ISamples *samples);
	~ISUnderSample ();	

	void setSeed(ssi_size_t seed);

	bool setUnder (Strategy strategy);
	bool setUnder (ssi_size_t class_id, ssi_size_t reduce_by_n_samples, Strategy strategy);
	bool setUnder (ssi_size_t n_classes, ssi_size_t *reduce_by_n_samples, Strategy strategy);

	void reset () { _next_under_count = 0; };
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 

	ssi_size_t getSize () { return _n_under; };
	ssi_size_t getSize (ssi_size_t class_id) { return _n_under_per_class[class_id]; };

	ssi_size_t getClassSize () { return _samples.getClassSize (); };
	const ssi_char_t *getClassName (ssi_size_t class_id) { return _samples.getClassName (class_id); };
	
	ssi_size_t getUserSize () { return _samples.getUserSize (); };
	const ssi_char_t *getUserName (ssi_size_t user_id) { return _samples.getUserName (user_id); };

	ssi_size_t getStreamSize () { return _samples.getStreamSize (); };
	ssi_stream_t getStream (ssi_size_t index) { return _samples.getStream (index); };

	bool hasMissingData () { return _samples.hasMissingData (); };
	bool supportsShallowCopy () { return true; };

protected:

	static ssi_char_t *ssi_log_name;

	void release ();

	ISamples &_samples;	
	ssi_size_t _n_samples;
	ssi_size_t _n_under;
	ssi_size_t _n_classes;
	ssi_size_t *_n_under_per_class;	
	ssi_size_t _next_under_count;
	ssi_sample_t *_under;

	bool doRandom ();
	bool doKMeans ();
	
	static void Shuffle(Randomf *random, ssi_size_t n, ssi_size_t *arr, ssi_size_t seed);
	ssi_size_t _seed;

};

}

#endif
