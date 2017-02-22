// ISReClass.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/01/10
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

#ifndef SSI_MODEL_ISRECLASS_H
#define SSI_MODEL_ISRECLASS_H

#include "base/ISamples.h"

namespace ssi {

class ISReClass : public ISamples {

public:

	ISReClass (ISamples *samples);
	~ISReClass ();	

	bool setReClass (ssi_size_t n_classes,
		const ssi_char_t **class_names,
		ssi_size_t n_class_ids,
		ssi_size_t *class_ids);

	void reset () { _counter = 0; _samples.reset (); };
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 

	ssi_size_t getSize () { return _samples.getSize (); };
	ssi_size_t getSize (ssi_size_t class_id) { return _n_ids_per_class[class_id]; };

	ssi_size_t getClassSize () { return _n_classes; };
	const ssi_char_t *getClassName (ssi_size_t class_id) { return _ids != 0 ? _class_names[class_id] : ""; };
	
	ssi_size_t getUserSize () { return _samples.getUserSize (); };
	const ssi_char_t *getUserName (ssi_size_t user_id) { return _samples.getUserName (user_id); };

	ssi_size_t getStreamSize () { return _samples.getStreamSize (); };
	ssi_stream_t getStream (ssi_size_t index) { return _samples.getStream (index); };

	bool hasMissingData () { return _samples.hasMissingData (); };
	bool supportsShallowCopy () { return false; };

protected:

	void release ();

	ISamples &_samples;
	ssi_size_t _n_classes;
	ssi_char_t **_class_names;
	ssi_size_t _n_ids;
	ssi_size_t *_n_ids_per_class;
	ssi_size_t *_ids;	
	ssi_sample_t _sample;
	ssi_size_t _counter;

};

}

#endif
