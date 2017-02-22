// ISMergeStrms.h
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

#ifndef SSI_MODEL_ISMERGESTRMS_H
#define SSI_MODEL_ISMERGESTRMS_H

#include "base/ISamples.h"
#include "base/ITransformer.h"

namespace ssi {

class ISMergeStrms : public ISamples {

public:

	ISMergeStrms (ssi_size_t n_lists, ISamples **lists, bool check_classes = true, bool check_users = true);
	~ISMergeStrms ();	

	void reset () { for (ssi_size_t i = 0; i < _n_lists; i++) {_lists[i]->reset (); } };
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next (); 

	ssi_size_t getSize () { return _ref->getSize (); };
	ssi_size_t getSize (ssi_size_t class_id) { return _ref->getSize (class_id); };

	ssi_size_t getClassSize () { return _ref->getClassSize (); };
	const ssi_char_t *getClassName (ssi_size_t class_id) { return _ref->getClassName (class_id); };
	
	ssi_size_t getUserSize () { return _ref->getUserSize (); };
	const ssi_char_t *getUserName (ssi_size_t user_id) { return _ref->getUserName (user_id); };

	ssi_size_t getStreamSize () { return _n_streams; };
	ssi_stream_t getStream (ssi_size_t index) { return _streams_ref[index]; };

	bool hasMissingData () { return _has_missing_data; };
	bool supportsShallowCopy () { return false; };

protected:

	void merge ();	

	ssi_size_t _n_lists;
	ISamples **_lists;
	ssi_size_t _n_classes, _n_streams, _n_users;
	ISamples *_ref;
	
	ssi_sample_t **_samples_in, _sample_out;
	ssi_stream_t **_streams;
	ssi_size_t *_stream_size;
	ssi_stream_t *_streams_ref;

	bool _has_missing_data;
	bool _check_classes;
	bool _check_users;

};

}

#endif
