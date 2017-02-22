// ISMergeSample.cpp
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

#include "ISMergeSample.h"
#include "signal/SignalTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISMergeSample::ISMergeSample (ssi_size_t n_lists, ISamples **lists)
	: _n_lists (n_lists),
	_has_missing_data (false) {

	_lists = new ISamples *[_n_lists];	
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		_lists[i] = lists[i];		
	}

	_ref = _lists[0];
	_n_streams = _ref->getStreamSize ();

	// calculate #users in new list and store user names
	_n_users = 0;
	ssi_size_t n_user_max = 0;
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		n_user_max += _lists[i]->getUserSize ();
	}
	_user_names = new ssi_char_t *[n_user_max];
	for (ssi_size_t i = 0; i < n_user_max; i++) {
		_user_names[i] = 0;
	}	
	_user_map = new ssi_size_t *[_n_lists];
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		_user_map[i] = new ssi_size_t[_lists[i]->getUserSize ()];
		for (ssi_size_t j = 0; j < _lists[i]->getUserSize (); j++) {
			const ssi_char_t *user = _lists[i]->getUserName (j);
			_user_map[i][j] = update_user_list (user);
		}
	}

	// calculate #classes in new list and store class names
	_n_classes = 0;
	ssi_size_t n_class_max = 0;
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		n_class_max += _lists[i]->getClassSize ();
	}
	_class_names = new ssi_char_t *[n_class_max];
	for (ssi_size_t i = 0; i < n_class_max; i++) {
		_class_names[i] = 0;
	}	
	_class_map = new ssi_size_t *[_n_lists];
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		_class_map[i] = new ssi_size_t[_lists[i]->getClassSize ()];
		for (ssi_size_t j = 0; j < _lists[i]->getClassSize (); j++) {
			const ssi_char_t *classname = _lists[i]->getClassName (j);
			_class_map[i][j] = update_class_list (classname);
		}
	}

	// calculate #samples in new list
	_n_samples = 0;
	_n_samples_per_class = new ssi_size_t[_n_classes];
	_n_samples_per_list = new ssi_size_t[_n_lists];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_n_samples_per_class[i] = 0;
	}	
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		_n_samples_per_list[i] = _lists[i]->getSize ();
		_n_samples += _n_samples_per_list[i];
		for (ssi_size_t j = 0; j < _lists[i]->getClassSize (); j++) {
			_n_samples_per_class[_class_map[i][j]] += _lists[i]->getSize (j);
		}
	}

	// check consistency of remaining properties
	for (ssi_size_t i = 1; i < _n_lists; i++) {
		if (_lists[i]->getStreamSize () != _n_streams) {
			ssi_err ("#streams not consistent");
		}
		for (ssi_size_t j = 0; j < _n_streams; j++) {
			if (_lists[i]->getStream (j).dim != _ref->getStream (j).dim) {
				ssi_err ("#dimensions in stream %u not consistent", j);
			}
		}
		if (_lists[i]->hasMissingData ()) {
			_has_missing_data = true;
		}
	}

	_current_list = 0;
}

ssi_size_t ISMergeSample::update_user_list (const ssi_char_t *name) {

	// check if already in list...
	for (ssi_size_t i = 0; i < _n_users; i++) {
		if (strcmp (name, _user_names[i]) == 0) {
			return i;
		}
	}

	// otherwise add user name
	_user_names[_n_users++] = ssi_strcpy (name);
	return _n_users-1;
}

ssi_size_t ISMergeSample::update_class_list (const ssi_char_t *name) {

	// check if already in list...
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		if (strcmp (name, _class_names[i]) == 0) {
			return i;
		}
	}

	// otherwise add user name
	_class_names[_n_classes++] = ssi_strcpy (name);
	return _n_classes-1;
}

ISMergeSample::~ISMergeSample () {

	delete[] _lists;
	delete[] _n_samples_per_class;	
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		delete[] _user_map[i];
	}
	delete[] _user_map;
	for (ssi_size_t i = 0; i < _n_users; i++) {	
		delete[] _user_names[i];
	}
	delete[] _user_names;
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		delete[] _class_map[i];
	}
	delete[] _class_map;
	for (ssi_size_t i = 0; i < _n_classes ; i++) {	
		delete[] _class_names[i];
	}
	delete[] _class_names;
	delete[] _n_samples_per_list;
}	

ssi_sample_t *ISMergeSample::get (ssi_size_t index) {

	if (index > _n_samples) { return 0; }

	ssi_size_t list_index = 0;	
	while (index >= _n_samples_per_list[list_index]) {
		index -= _n_samples_per_list[list_index++];		
	}
	
	ssi_sample_t *tmp = _lists[list_index]->get (index);
	if (tmp == 0) {
		return 0;
	}

	_sample_out = *tmp;
	_sample_out.user_id = _user_map[list_index][tmp->user_id];
	_sample_out.class_id = _class_map[list_index][tmp->class_id];
	return &_sample_out;	
}

ssi_sample_t *ISMergeSample::next () {

	if (_current_list >= _n_lists) {
		return 0;
	}

	ssi_sample_t *tmp = _lists[_current_list]->next ();
	if (!tmp) {		
		++_current_list;
		return next ();
	}

	_sample_out = *tmp;
	_sample_out.user_id = _user_map[_current_list][tmp->user_id];
	_sample_out.class_id = _class_map[_current_list][tmp->class_id];
	return &_sample_out;
}

void ISMergeSample::printDebug (FILE *file) {

	ssi_fprint (file, "#lists = %u\n#samples = %u ( ", _n_lists, _n_samples);
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		ssi_fprint (file, "%u ", _n_samples_per_list[i]);
	}
	ssi_fprint (file, ")\nusers = ");
	for (ssi_size_t i = 0; i < _n_users; i++) {
		ssi_fprint (file, "%s ", _user_names[i]);
	}	
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		ssi_fprint (file, "\nlist%02u = ", i);
		for (ssi_size_t j = 0; j < _lists[i]->getUserSize (); j++) {
			ssi_fprint (file, "%s ", _user_names[_user_map[i][j]]);
		}
	}
	ssi_fprint (file, "\n");
}

}
