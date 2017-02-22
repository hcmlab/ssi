// EmoVoiceSVM.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/19
// Copyright (C) 2007-9 University of Augsburg, Johannes Wagner
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

#include "EmoVoiceSVM.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

EmoVoiceSVM::EmoVoiceSVM (const ssi_char_t *file) 
	: _model (0),
	_n_classes (0),
	_n_features (0),
	_cl_type (svm),
	_cl_param (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
} 

EmoVoiceSVM::~EmoVoiceSVM () { 

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool EmoVoiceSVM::train (ISamples &samples, ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = samples.getClassSize ();
	_n_features = samples.getStream (stream_index).dim;

	_model = cl_new_classifier (_cl_type, _n_classes, _n_features, _cl_param);
	_model->svm->param = _options.svm_param;
	if (!_model->svm->param.gamma)
      _model->svm->param.gamma=1.0/_model->svm->feature_dim;

	samples.reset ();
	ssi_sample_t *sample = 0;
	while (sample = samples.next ()) {
		_model = cl_update_classifier (_model, _cl_type, sample->class_id, reinterpret_cast<mx_real_t *> (sample->streams[stream_index]->ptr));
	}
	_model = cl_finish_classifier (_model, _cl_type);

	return true;
}

bool EmoVoiceSVM::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_model) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_probs != _n_classes) {
		ssi_wrn ("#classes differs");
		return false;
	}

	if (stream.type != SSI_REAL) {
		ssi_wrn ("type differs");
		return false;
	}

	if (stream.dim != _n_features) {
		ssi_wrn ("feature dimension differs");
		return false;
	}

	ssi_real_t sum=0;
	for (ssi_size_t j = 0; j < n_probs; j++) {
		probs[_model->svm->model->label[j]] = class_prob (_model, _cl_type, ssi_pcast (mx_real_t, stream.ptr), j);	
		sum += probs[_model->svm->model->label[j]];
	}
	//normalisation
	for (ssi_size_t j = 0; j < n_probs; j++) {
		probs[j]/=sum;
	}

	return true;
}

bool EmoVoiceSVM::save (const ssi_char_t *filepath) {

	if (!_model) {
		ssi_wrn ("not trained");
		return false;
	}

	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		ssi_char_t string[200];
		ssi_sprint (string, "class%02u", n_class);
		_model->svm->mapping[n_class] = ssi_strcpy (string);
	}
	cl_output_classifier (_model, _cl_type, ssi_ccast (ssi_char_t *, filepath));

	return true;
}

bool EmoVoiceSVM::load (const ssi_char_t *filepath) {

	release ();

	_model = cl_create_classifier (ssi_ccast (ssi_char_t *, filepath), _cl_type);
	_model->svm->param.svm_type = _model->svm->model->param.svm_type;
	_model->svm->param.kernel_type = _model->svm->model->param.kernel_type;
	_n_classes = _model->svm->n_classes;
	_n_features = _model->svm->feature_dim;

	return true;
}

void EmoVoiceSVM::release () {

	if (_model) {
		cl_destroy (_model, _cl_type);
		_model = 0;
	}	
	_n_features = 0;
	_n_classes = 0;
}

}
