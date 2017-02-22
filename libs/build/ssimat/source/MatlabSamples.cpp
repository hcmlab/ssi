// MatlabSamples.cpp
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

#include "MatlabSamples.h"

namespace ssi {

MatlabSamples::MatlabSamples (const ssi_char_t *path) 
	: _matfile (path, MatlabFile::READ),
	_samples_ptr (0),
	_counter (0) {

	_n_samples = ssi_pcast (ssi_size_t, _matfile["n_samples"].getPtr ())[0];
	_n_classes = ssi_pcast (ssi_size_t, _matfile["n_classes"].getPtr ())[0];	
	_n_users = ssi_pcast (ssi_size_t, _matfile["n_users"].getPtr ())[0];
	_n_streams = _matfile["samples"].cols;
	_n_features = new ssi_size_t[_n_streams];
	_samples = new MatlabVar *[_n_streams];
	_streams = new ssi_stream_t[_n_streams];
	_samples_ptr = new ssi_real_t *[_n_streams];
	ssi_sample_create (_sample, _n_streams, 0, 0, 0, 1);
	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		_samples[nstream] = &_matfile["samples"].getCell (nstream);
		ssi_real_t *test = ssi_pcast (ssi_real_t, _samples[nstream]->getPtr ());
		_n_features[nstream] = _samples[nstream]->rows;
		ssi_stream_init (_streams[nstream], 0, _n_features[nstream], sizeof (ssi_real_t), SSI_REAL, 0, 0);
		_streams[nstream].num_real = _streams[nstream].num = 1;
		_streams[nstream].tot_real = _streams[nstream].tot = _streams[nstream].num * _streams[nstream].dim *  _streams[nstream].byte;
		_samples_ptr[nstream] = 0;
		_sample.streams[nstream] = &_streams[nstream];
	}	
	_class_ids = ssi_pcast (ssi_size_t, _matfile["classes"].getPtr ());
	_user_ids = ssi_pcast (ssi_size_t, _matfile["users"].getPtr ());
	
	reset ();		
}

MatlabSamples::~MatlabSamples () {

	delete[] _n_features;
	delete[] _samples;
	delete[] _samples_ptr;
	delete[] _streams;
	delete[] _sample.streams;
	
};	

void MatlabSamples::reset () {

	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		_samples_ptr[nstream] = ssi_pcast (ssi_real_t, _samples[nstream]->getPtr ());
	}
	_counter = 0;
}

ssi_sample_t *MatlabSamples::get (ssi_size_t index) {

	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		_streams[nstream].ptr = ssi_pcast (ssi_byte_t, ssi_pcast (ssi_real_t, _samples[nstream]->getPtr ()) + index * _streams[nstream].dim);
	}
	_sample.class_id = _class_ids[index];
	_sample.user_id = _user_ids[index];

	return &_sample;
}

ssi_sample_t *MatlabSamples::next () {

	if (_counter++ >= getSize ()) {
		return 0;
	}
	
	_sample.class_id = _class_ids[_counter-1];
	_sample.user_id = _user_ids[_counter-1];
	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		_streams[nstream].ptr = ssi_pcast (ssi_byte_t, _samples_ptr[nstream]);	
		_samples_ptr[nstream] += _streams[nstream].dim;	
	}

	return &_sample;
}

ssi_sample_t *MatlabSamples::next (ssi_size_t class_index) {

	do {
		if (_counter++ >= getSize ()) {
			return 0;
		}
		_sample.class_id = _class_ids[_counter-1];
		_sample.user_id = _user_ids[_counter-1];
		for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
			_samples_ptr[nstream] += _streams[nstream].dim;
		}
	} while (_sample.class_id != class_index);
	
	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		_streams[nstream].ptr = ssi_pcast (ssi_byte_t, _samples_ptr[nstream] - _streams[nstream].dim);	
	}

	return &_sample;
}

ssi_size_t MatlabSamples::getSize (ssi_size_t class_index) {

	return ssi_pcast (ssi_size_t, _matfile["n_samples_per_class"].getPtr ())[class_index];
}

const ssi_char_t *MatlabSamples::getClassName (ssi_size_t index) {

	return _matfile["cnames"].getCell (index).getString ();
}

const ssi_char_t *MatlabSamples::getUserName (ssi_size_t index) {

	return _matfile["unames"].getCell (index).getString ();
}

void MatlabSamples::printInfo (FILE *file) {

	ssi_fprint (file, "relation\t%s\n", _matfile["relation"].getString ());
	ssi_fprint (file, "samples\t\t%ux%u\n", _n_samples, _n_streams);	
	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		ssi_fprint (file, "stream#%02u\t%u\n", nstream, _n_features[nstream]);	
	}
	ssi_fprint (file, "per class\t");
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		ssi_fprint (file, "%u ", getSize (nclass));
	}
	ssi_fprint (file, "\nnames\t\t");
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		ssi_fprint (file, "%s ", getClassName (nclass));
	}
	ssi_fprint (file, "\nusers\t\t");
	for (ssi_size_t nuser = 0; nuser < _n_users; nuser++) {
		ssi_fprint (file, "%s ", getUserName (nuser));
	}
}

void MatlabSamples::printSample (ssi_size_t index, FILE *file) {

	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		_streams[nstream].ptr = ssi_pcast (ssi_byte_t, ssi_pcast (ssi_real_t, _samples[nstream]->getPtr ()) + index * _streams[nstream].dim);
	}
	_sample.class_id = _class_ids[index];
	_sample.user_id = _user_ids[index];

	for (ssi_size_t nstream = 0; nstream < _n_streams; nstream++) {
		ssi_fprint (file, "stream#%02u\n", nstream);
		for (ssi_size_t nfeat = 0; nfeat < _n_features[nstream]; nfeat++) {
			ssi_fprint (file, "%.2f ", *ssi_pcast (ssi_real_t, _streams[nstream].ptr));
			_streams[nstream].ptr += _streams[nstream].byte;
		}
		ssi_fprint (file, "\n");
	}
	ssi_fprint (file, " -> %s", getClassName (index));
}

}
