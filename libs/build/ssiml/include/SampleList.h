// SampleList.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/06/18
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_MODEL_SAMPLELIST_H
#define SSI_MODEL_SAMPLELIST_H

#include "base/ISamples.h"

namespace ssi {

#define SAMPLELIST_DELIM '_'

	class SampleList : public ISamples {

public:

	SampleList ();
	virtual ~SampleList ();

	void reset ();
	ssi_sample_t &operator[] (ssi_size_t index);
	ssi_sample_t *get (ssi_size_t index);
	ssi_sample_t *next ();

	bool addSample (ssi_sample_t *sample, bool deep_copy = false);
	ssi_size_t addClassName (const ssi_char_t *class_name);
	bool setClassName(ssi_size_t class_index, const ssi_char_t *class_name);
	ssi_size_t addUserName (const ssi_char_t *user_name);
	bool setUserName(ssi_size_t class_index, const ssi_char_t *class_name);
	void clear ();

	ssi_size_t getSize ();
	ssi_size_t getSize (ssi_size_t class_index);

	ssi_size_t getClassSize ();	
	const ssi_char_t *getClassName (ssi_size_t index);

	ssi_size_t getUserSize ();
	const ssi_char_t *getUserName (ssi_size_t index);

	ssi_size_t getStreamSize () {
		return _n_streams;
	}
	ssi_stream_t getStream (ssi_size_t index);

	bool isEmpty ();	

	void sort ();

	void setFeatureNames (ssi_size_t number, ssi_char_t **names);
	ssi_size_t getFeatureSize ();
	const ssi_char_t **getFeatureNames ();

	bool hasMissingData () { 
		return _has_missing_data; 
	};
	void setMissingData (bool toggle) { 
		_has_missing_data = toggle; 
	};
	bool supportsShallowCopy () { 
		return true; 
	};	

	void print (FILE *file = stdout);
	void printInfo (FILE *file = stdout);

protected:

	static bool comparator (const ssi_sample_t *s1, const ssi_sample_t *s2) {		
		return s1->time < s2->time;
	}

	std::vector<ssi_sample_t *> _samples;
	std::vector<ssi_sample_t *>::iterator _samples_it;

	std::vector<ssi_char_t *> _classes;
	std::vector<ssi_char_t *>::iterator _classes_it;

	std::vector<ssi_char_t *> _users;
	std::vector<ssi_char_t *>::iterator _users_it;

	ssi_char_t **_feature_names;
	ssi_size_t _n_features;
	ssi_size_t _n_streams;
	ssi_stream_t *_streams;

	bool _has_missing_data;
};

}

#endif
