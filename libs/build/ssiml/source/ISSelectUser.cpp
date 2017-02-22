// ISSelectUser.cpp
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2010/05/06
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

#include "ISSelectUser.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ISSelectUser::ISSelectUser (ISamples *samples)
	: _samples (*samples),
	_uset (0),
	_n_users (0),
	_n_samples (0),
	_n_classes (0),
	_user_inds (0),
	_user_ids (0),
	_n_samples_per_class (0){
}

ISSelectUser::~ISSelectUser () {release();}

void ISSelectUser::release(){
	
	delete[] _uset;
	_uset = 0;	

	delete[] _n_samples_per_class;
	_n_samples_per_class = 0;

	delete[] _user_inds;
	_user_inds = 0;

	delete[] _user_ids;
	_user_ids = 0;

	_n_classes = 0;
	_n_samples = 0;
	_n_users = 0;
}

bool ISSelectUser::setSelection (ssi_size_t n_users, 
	const ssi_size_t* uset,
	bool invert) {

	release();

	_n_classes = _samples.getClassSize();
	_n_users = invert ? _samples.getUserSize () - n_users : n_users;
	_uset = new ssi_size_t [_n_users];

	if (invert) {
		for (ssi_size_t n = 0, m = 0; n < _samples.getUserSize (); n++){
			bool found = true;
			for (ssi_size_t nselected = 0; nselected < n_users; nselected++){
				if (uset[nselected] == n) {
					found = false;
					break;
				}
			}
			if (found) {
				_uset[m++] = n;			
			}
		}
	} else {
		for (ssi_size_t nselected = 0; nselected < _n_users; nselected++){
			_uset[nselected] = uset[nselected];
		}
	}
	
	_n_samples_per_class = new ssi_size_t[_n_classes];
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		_n_samples_per_class[nclass] = 0;
	}
		
	ssi_sample_t *tmp = 0;
	_samples.reset();
	while( tmp = _samples.next() ){
		for(ssi_size_t nselected = 0; nselected < _n_users; nselected++){
			if (tmp->user_id == _uset[nselected]){
				_n_samples_per_class[tmp->class_id]++;
				_n_samples++;
			}
		}
	}
	_samples.reset();

	ssi_size_t counter_org = 0;
	ssi_size_t counter_sel = 0;
	_user_inds = new ssi_size_t[_n_samples];
	_user_ids = new ssi_size_t[_n_samples];
	while( tmp = _samples.next() ){
		for(ssi_size_t nselected = 0; nselected < _n_users; nselected++){
			if (tmp->user_id == _uset[nselected]){
				_user_inds[counter_sel] = counter_org;
				_user_ids[counter_sel] = nselected;
				counter_sel++;
			}
		}
		counter_org++;
	}
	_samples.reset();
	
	return true;
}

ssi_sample_t *ISSelectUser::get (ssi_size_t index) {

	ssi_sample_t *tmp = 0;
	tmp = _samples.get (_user_inds[index]);

	if (tmp == 0) {
		return 0;
	}
	
	_sample = *tmp;
	_sample.user_id = _user_ids[index];

	return &_sample;	
}

ssi_sample_t *ISSelectUser::next () {

	ssi_sample_t *tmp = 0;
	while( tmp = _samples.next() ){
		for(ssi_size_t nselected = 0; nselected < _n_users; nselected++){
			if (tmp->user_id == _uset[nselected]){
				_sample = *tmp;
				_sample.user_id = nselected;
				return &_sample;
			}
		}
	}
	return 0;
}

}

