// ISTransform.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/01
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

#ifndef SSI_MODEL_ISTRANSFORM_H
#define SSI_MODEL_ISTRANSFORM_H

#include "base/ISamples.h"
#include "base/ITransformer.h"

namespace ssi {

class ISTransform : public ISamples {

public:

	ISTransform (ISamples *samples);
	~ISTransform ();	

	bool setTransformer (ssi_size_t index, 
		ITransformer &transformer);
	bool setTransformer (ssi_size_t index, 
		ITransformer &transformer, 
		ssi_time_t frame_size, 
		ssi_time_t delta_size = 0);
	bool setTransformer (ssi_size_t index, 
		ITransformer &transformer, 
		ssi_size_t frame_size, 
		ssi_size_t delta_size = 0);
	void callEnter ();
	void callFlush ();

	void reset () { _samples.reset (); };
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 

	ssi_size_t getSize () { return _samples.getSize (); };
	ssi_size_t getSize (ssi_size_t class_id) { return _samples.getSize (class_id); };

	ssi_size_t getClassSize () { return _samples.getClassSize (); };
	const ssi_char_t *getClassName (ssi_size_t class_id) { return _samples.getClassName (class_id); };
	
	ssi_size_t getUserSize () { return _samples.getUserSize (); };
	const ssi_char_t *getUserName (ssi_size_t user_id) { return _samples.getUserName (user_id); };

	ssi_size_t getStreamSize () { return _samples.getStreamSize (); };
	ssi_stream_t getStream (ssi_size_t index) { 
		if (_transformers[index]) {
			ssi_stream_t s;
			ssi_stream_init (s, 0, _transformers[index]->getSampleDimensionOut (_samples.getStream (index).dim), _transformers[index]->getSampleBytesOut (_samples.getStream (index).byte), _transformers[index]->getSampleTypeOut (_samples.getStream (index).type), 0, 0);
			return s;
		} else {
			return _samples.getStream (index); 
		}
	};

	bool hasMissingData () { return _samples.hasMissingData (); };
	bool supportsShallowCopy () { return false; };

protected:

	void transform (bool enter = false, bool flush = false);

	ISamples &_samples;
	ssi_sample_t _sample_in, _sample_out;
	ITransformer **_transformers;
	ssi_size_t _n_transformer;
	ssi_stream_t *_streams;
	ssi_time_t *_frame_sizes;
	ssi_time_t *_delta_sizes;
	ssi_size_t *_delta_sizes_in_samples;
	ssi_size_t *_frame_sizes_in_samples;

};

}

#endif
