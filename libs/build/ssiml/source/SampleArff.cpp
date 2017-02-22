// SampleArff.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/12/09
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

#include "SampleArff.h"
#include "arff_parser.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

SampleArff::SampleArff (const ssi_char_t *path, ssi_size_t class_index)
	: _parser (0),
	_data (0),
	_n_features (0), 
	_n_samples (0),
	_n_nominals (0),
	_ind_nominals(0),
	_n_strings (0),
	_ind_strings (0),
	_n_classes (0),
	_class_names (0),
	_class_ind (class_index),
	_class (0),
	_n_samples_per_class (0),
	_n_garbage_class (0),
	_next_ind (0) {

	_parser = new ArffParser  (path);
	_data = _parser->parse ();

	if (!_data) {
		ssi_err ("could not parse '%s'", path);
	}

	_n_samples = _data->num_instances ();
	_n_features = _data->num_attributes ();
	_n_nominals = 0;
	_n_strings = 0;

	//count types
	for (int32 i = 0; i < _data->num_attributes (); i++) {
		if (_data->get_attr (i)->type () != NUMERIC){
			_n_features--;
			if (_data->get_attr (i)->type () == NOMINAL) {
				_n_nominals++;
			}
			if (_data->get_attr (i)->type () == STRING) {
				_n_strings++;
			}

		}
	}
	//if (!_class) {
	if (_n_nominals == 0 || _class_ind > _n_nominals - 1) {
		ssi_err ("could not find class nominal '%s'", path);
	}

	//indices
	_ind_nominals = new ssi_size_t[_n_nominals];
	_ind_strings  = new ssi_size_t[_n_strings];
	ssi_size_t n_nom = 0;
	ssi_size_t n_str = 0;
	for (int32 i = 0; i < _data->num_attributes (); i++) {
		if (_data->get_attr (i)->type () == NOMINAL) {
			_ind_nominals[n_nom] = ssi_cast (ssi_size_t, i);
			n_nom++;
		}
		if (_data->get_attr (i)->type () == STRING) {
			_ind_strings[n_str] = ssi_cast (ssi_size_t, i);
			n_str++;
		}
	}
	
	_class = _data->get_attr(_ind_nominals[_class_ind]);

	ArffNominal n = _data->get_nominal (_class->name ());

	_n_classes = ssi_cast (ssi_size_t, n.size ());
	_class_names = new ssi_char_t *[_n_classes];
	_n_samples_per_class = new ssi_size_t[_n_classes];
	for (size_t i = 0; i < n.size (); i++) {
		_class_names[i] = ssi_strcpy (n[i].c_str ());
		_n_samples_per_class[i] = 0;
	}

	for (int i = 0; i < _data->num_instances (); i++) {		
		ArffValue *v = _data->get_instance (i)->get (_ind_nominals[_class_ind]);
		if (v->missing ()) {
			_n_garbage_class++;
		} else {
			for (size_t i = 0; i < n.size (); i++) {
				if (std::string (*v) == n[i]) {
					_n_samples_per_class[i]++;
					break;
				}
			}
		}
	}

	ssi_stream_init (_stream, 0, _n_features, sizeof (ssi_real_t), SSI_REAL, 0);
	ssi_sample_create (_sample, 1, 0, 0, 0, 0);
	_sample.streams[0] = new ssi_stream_t;
	*_sample.streams[0] = _stream;
	ssi_stream_adjust (*_sample.streams[0], 1); 

	reset ();
}

SampleArff::~SampleArff () {

	delete _parser;
	_parser = 0;

	delete[] _n_samples_per_class;
	_n_samples_per_class = 0;
	_n_garbage_class = 0;

	ssi_sample_destroy (_sample);

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		delete[] _class_names[i];
	}
	delete[] _class_names;
	_class_names = 0;
	
	delete[] _ind_nominals;
	_ind_nominals = 0;

	delete[] _ind_strings;
	_ind_strings = 0;

}

ssi_sample_t *SampleArff::get (ssi_size_t index) {

	if (index >= _n_samples) {
		ssi_wrn ("index out of range");
		return 0;
	}
	
	ArffNominal n = _data->get_nominal (_class->name ());
	ssi_real_t *ptr = ssi_pcast (ssi_real_t, _sample.streams[0]->ptr);

	ArffInstance *in = _data->get_instance (index);
	
	for (int i = 0; i < in->size (); i++) {
		bool valid_i = true;
		for(ssi_size_t n_nom = 0; n_nom < _n_nominals; n_nom++){
			if(i == _ind_nominals[n_nom]){
				valid_i = false;
			}
		}
		for(ssi_size_t n_str = 0; n_str < _n_strings; n_str++){
			if(i == _ind_strings[n_str]){
				valid_i = false;
			}
		}
		if (i == _ind_nominals[_class_ind]) {
			if (in->get (i)->missing ()) {
				_sample.class_id = SSI_ISAMPLES_GARBAGE_CLASS_ID;
			} else {			
				for (size_t j = 0; j < n.size (); j++) {
					if (std::string (*in->get (i)) == n[j]) {
						_sample.class_id = ssi_cast (ssi_size_t, j);
						break;
					}							
				}
			}
		} else if (valid_i){
			*ptr++ = float (*in->get (i));
		} else {
			//no values
		}
	}
	
	return &_sample;
}


ssi_sample_t *SampleArff::next () {

	if (_next_ind == _n_samples) {
		return 0;
	}

	return get (_next_ind++);
}

const char *SampleArff::getClassName (ssi_size_t index) {

	if (index >= _n_classes) {
		ssi_wrn ("index out of range");
		return 0;
	}
	
	return _class_names[index];
}

ssi_size_t SampleArff::getSize (ssi_size_t class_index) {

	if (class_index >= _n_classes) {
		ssi_wrn ("index out of range");
		return 0;
	}
	return _n_samples_per_class[class_index];
}

}

