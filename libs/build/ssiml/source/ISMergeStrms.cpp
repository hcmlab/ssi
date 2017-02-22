// ISMergeStrms.cpp
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

#include "ISMergeStrms.h"
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

ISMergeStrms::ISMergeStrms (ssi_size_t n_lists, ISamples **lists, bool check_classes, bool check_users)
	: _n_lists (n_lists),
	_has_missing_data (false),
	_check_classes (check_classes),
	_check_users (check_users) {

	_lists = new ISamples *[_n_lists];
	_n_streams = 0;
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		_lists[i] = lists[i];
		_n_streams += _lists[i]->getStreamSize ();
	}

	_ref = _lists[0];
	_n_classes = _ref->getClassSize ();
	_n_users = _ref->getUserSize ();
	ssi_size_t n_samples = _ref->getSize ();

	for (ssi_size_t i = 1; i < _n_lists; i++) {
		if (_lists[i]->getSize () != n_samples) {
			ssi_err ("#samples not consistent");
		}
		if (check_classes) {
			if (_lists[i]->getClassSize () != _n_classes) {
				ssi_err ("#classes not consistent");
			}
			for (ssi_size_t j = 0; j < _n_classes; j++) {
				if (strcmp (_lists[i]->getClassName (j), _ref->getClassName (j)) != 0) {
					ssi_err ("class names not consistent");
				}
			}
		}
		if (check_users) {
			if (_lists[i]->getUserSize () != _n_users) {
				ssi_err ("#users not consistent");
			}		
			for (ssi_size_t j = 0; j < _n_users; j++) {
				if (strcmp (_lists[i]->getUserName (j), _ref->getUserName (j)) != 0) {
					ssi_err ("user names not consistent");
				}
			}
		}
		if (_lists[i]->hasMissingData ()) {
			_has_missing_data = true;
		}
	}

	_samples_in = new ssi_sample_t *[_n_lists];
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		_samples_in[i] = 0;
	}

	_streams = new ssi_stream_t *[_n_streams];
	_stream_size = new ssi_size_t[_n_lists];
	_streams_ref = new ssi_stream_t[_n_streams];
	ssi_size_t nstrm = 0;
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		_stream_size[i] = _lists[i]->getStreamSize ();
		for (ssi_size_t j = 0; j < _stream_size[i]; j++) {
			_streams[nstrm] = 0;
			_streams_ref[nstrm] = _lists[i]->getStream (j);
			++nstrm;
		}
	}

	_sample_out.num = _n_streams;
	_sample_out.streams = _streams;
}

ISMergeStrms::~ISMergeStrms () {

	delete[] _lists;
	delete[] _streams;
	delete[] _samples_in;
	delete[] _stream_size;
	delete[] _streams_ref;
}	

ssi_sample_t *ISMergeStrms::get (ssi_size_t index) {

	ssi_sample_t *tmp = _ref->get (index);

	if (tmp == 0) {
		return 0;
	}

	_samples_in[0] = tmp;
	for (ssi_size_t i = 1; i < _n_lists; i++) {
		_samples_in[i] = _lists[i]->get (index);
	}
	merge ();
	return &_sample_out;	
}

ssi_sample_t *ISMergeStrms::next () {

	ssi_sample_t *tmp = _ref->next ();
	if (!tmp) {
		return 0;
	}

	_samples_in[0] = tmp;
	for (ssi_size_t i = 1; i < _n_lists; i++) {
		_samples_in[i] = _lists[i]->next ();
	}
	merge ();
	return &_sample_out;
}

SSI_INLINE void ISMergeStrms::merge () {

	_sample_out.class_id = _samples_in[0]->class_id;
	_sample_out.user_id = _samples_in[0]->user_id;
	_sample_out.score = _samples_in[0]->score;
	_sample_out.time = _samples_in[0]->time;
	
	ssi_size_t nstrm = 0;
	for (ssi_size_t i = 0; i < _n_lists; i++) {
		if (_check_classes && _samples_in[i]->class_id != _sample_out.class_id) {
			ssi_err ("class id of merged sample not consistent");
		}
		if (_check_users && _samples_in[i]->user_id != _sample_out.user_id) {
			ssi_err ("user id of merged sample not consistent");
		}
		for (ssi_size_t j = 0; j < _stream_size[i]; j++) {			
			_streams[nstrm++] = _samples_in[i]->streams[j];
		}
	}
}


}
