// SampleList.cpp
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

#include "SampleList.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

SampleList::SampleList ()
	: _n_features (0), 
	_feature_names(0),
	_n_streams (0),
	_streams (0),
	_has_missing_data (false) {

	reset ();
}

SampleList::~SampleList () {

	clear ();
	if (_feature_names) {
		for (unsigned int i = 0; i < _n_features; i++) {
			delete[] _feature_names[i];
		}
		delete[] _feature_names;
	}
}

bool SampleList::addSample (ssi_sample_t *sample, bool deep_copy) {

	if (sample->user_id >= _users.size() && sample->class_id != SSI_SAMPLE_GARBAGE_USER_ID) {
		ssi_wrn ("sample exceeds user id");
		return false;
	}

	if (sample->class_id >= _classes.size (	) && sample->class_id != SSI_SAMPLE_GARBAGE_CLASS_ID) {
		ssi_wrn ("sample exceeds class id");
		return false;
	}

	if (!_samples.empty ()) {
		if (sample->num != _n_streams) {
			ssi_wrn ("#stream not compatible");
			return false;
		}
		for (ssi_size_t i = 0; i < _n_streams; i++) {			
			if (sample->streams[i]->dim != _streams[i].dim || sample->streams[i]->byte != _streams[i].byte || sample->streams[i]->type != _streams[i].type) {
				ssi_wrn ("stream#%02u not compatible", i);
				return false;
			}		
		}
	} else {
		_n_streams = sample->num;
		_streams = new ssi_stream_t[_n_streams];
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			ssi_stream_init (_streams[i], 0, sample->streams[i]->dim, sample->streams[i]->byte, sample->streams[i]->type, sample->streams[i]->sr, 0);
		}
	}

	if (!_has_missing_data) {
		for (ssi_size_t i = 0; i < sample->num; i++) {
			if (sample->streams[i]->num == 0) {
				_has_missing_data = true;
			}
		}
	}

	if (deep_copy) {
		ssi_sample_t *clone = new ssi_sample_t;
		ssi_sample_clone (*sample, *clone);
		_samples.push_back (clone);
	} else {
		_samples.push_back (sample);
	}

	return true;
}


ssi_size_t SampleList::addClassName (const char *label_name) {

	// check if label already exists
	ssi_size_t index = 0;
	for (_classes_it = _classes.begin (); _classes_it != _classes.end (); _classes_it++) {
		if (strcmp (*_classes_it, label_name) == 0) {
			return index;
		} else {
			index++;
		}
	}

	// otherwise add label
	char *label = new char[strlen (label_name) + 1];
	ssi_strcpy (label, label_name);
	_classes.push_back (label);	
	return index;
}

ssi_size_t SampleList::addUserName (const char *id_name) {

	// check if id already exists
	ssi_size_t index = 0;
	for (_users_it = _users.begin (); _users_it != _users.end (); _users_it++) {
		if (strcmp (*_users_it, id_name) == 0) {
			return index;
		} else {
			index++;
		}
	}

	// otherwise add id
	char *id = new char[strlen (id_name) + 1];
	ssi_strcpy (id, id_name);
	_users.push_back (id);	
	return index;
}


void SampleList::reset () {

	_samples_it = _samples.begin ();
}

ssi_sample_t &SampleList::operator[] (ssi_size_t index) {

	if (index >= getSize ()) {
		ssi_err ("index out of range");
	}

	return *_samples[index];
}

ssi_sample_t *SampleList::get (ssi_size_t index) {

	if (index >= getSize ()) {
		ssi_wrn ("index out of range");
		return 0;
	}

	return _samples.at (index);
}


ssi_sample_t *SampleList::next () {

	if (_samples_it == _samples.end ()) {
		return 0;
	}

	return *_samples_it++;
}

void SampleList::clear () {

	for (_users_it = _users.begin (); _users_it != _users.end (); _users_it++)
		delete[] *_users_it;
	_users.clear ();

	for (_classes_it = _classes.begin (); _classes_it != _classes.end (); _classes_it++)
		delete[] *_classes_it;
	_classes.clear ();

	for (_samples_it = _samples.begin (); _samples_it != _samples.end (); _samples_it++) {
		ssi_sample_destroy (**_samples_it);
		delete *_samples_it;
	}
	_samples.clear ();
	_has_missing_data = false;
	delete[] _streams; _streams = 0;

	reset ();
}

void SampleList::print (FILE *file) {

	if (_samples.empty ()) {
		return;
	}

	for (_samples_it = _samples.begin (); _samples_it != _samples.end (); _samples_it++) {
		ssi_fprint (file, "%s %s %.3lf [ ", _users.at ((*_samples_it)->user_id), _classes.at ((*_samples_it)->class_id), (*_samples_it)->time);
		for (ssi_size_t i = 0; i < (*_samples_it)->num; i++) {
			ssi_fprint (file, "%ux%u ", (*_samples_it)->streams[i]->num, (*_samples_it)->streams[i]->dim);
		}
		ssi_fprint (file, "]\n");
	}	

	reset ();
}

void SampleList::printInfo (FILE *file) {

	ssi_fprint (file, "_samples\t\t%ux%u\n", getSize (), getStreamSize ());	
	ssi_fprint (file, "per class\t");
	for (ssi_size_t nclass = 0; nclass < getClassSize (); nclass++) {
		ssi_fprint (file, "%u ", getSize (nclass));
	}
	ssi_fprint (file, "\nnames\t\t");
	for (ssi_size_t nclass = 0; nclass < getClassSize (); nclass++) {
		ssi_fprint (file, "%s ", getClassName (nclass));
	}
	ssi_fprint (file, "\nstreams\t\t");
	for (ssi_size_t nstream = 0; nstream < getStreamSize (); nstream++) {
		ssi_fprint (file, "%ux%u %s", getSize (), _streams[nstream].dim, SSI_TYPE_NAMES[_streams[nstream].type]);
	}
	ssi_fprint (file, "\n");
	
}

void SampleList::setFeatureNames (unsigned int number, char **names) {

	if (_samples.empty () || number == getFeatureSize ()) {
		ssi_err ("number of features (%d) not compatible to sample list (%d)", number, getFeatureSize ());
	}

	if (_feature_names) {
		for (unsigned int i = 0; i < _n_features; i++) {
			delete[] _feature_names[i];
		}
		delete[] _feature_names;
	}

	_n_features = number;
	_feature_names = new char *[_n_features];
	for (unsigned int i = 0; i < _n_features; i++) {
		_feature_names[i] = new char[strlen (names[i]) + 1];
		ssi_strcpy (_feature_names[i], names[i]);
	}
}


ssi_size_t SampleList::getSize () {

	return ssi_cast (ssi_size_t, _samples.size ());
}

ssi_size_t SampleList::getSize (ssi_size_t label_index) {

	if (label_index >= _classes.size ()) {
		ssi_wrn ("index out of range");
		return 0;
	}

	ssi_size_t size = 0;

	for (_samples_it = _samples.begin (); _samples_it != _samples.end (); _samples_it++) {
		if ((*_samples_it)->class_id == label_index) {
			++size;
		}
	}
	reset ();

	return size;
}

ssi_size_t SampleList::getClassSize () {
	
	return ssi_cast (ssi_size_t, _classes.size ());
}

const char *SampleList::getClassName (ssi_size_t index) {

	if (index == SSI_SAMPLE_GARBAGE_CLASS_ID)
	{
		return SSI_SAMPLE_GARBAGE_CLASS_NAME;
	}

	if (index >= _classes.size ()) {
		ssi_wrn ("index out of range");
		return 0;
	}

	return _classes.at (index);
}

ssi_size_t SampleList::getUserSize () {

	return ssi_cast (ssi_size_t, _users.size ());
}

const char *SampleList::getUserName (ssi_size_t index) {

	SSI_ASSERT (index < _users.size ());
	return _users.at (index);
}

const char **SampleList::getFeatureNames () {
	return ssi_ccast (const ssi_char_t **, _feature_names);
};

ssi_size_t SampleList::getFeatureSize () {

	if (_samples.empty ()) {
		return 0;
	}

	ssi_size_t tot_feat_num = 0;
	for (ssi_size_t i = 0; i < _samples[0]->num; i++) {
		tot_feat_num += _samples[0]->streams[i]->dim;
	}

	return tot_feat_num;
}

bool SampleList::isEmpty () {
	return _samples.empty ();
}

ssi_stream_t SampleList::getStream (ssi_size_t index) {
	if (index >= _n_streams) {
		ssi_err ("index '%u' exceeds #streams '%u'", index, _n_streams);
	}
	return _streams[index];
}

void SampleList::sort () {
	std::sort (_samples.begin (), _samples.end (), SampleList::comparator);
}

}

