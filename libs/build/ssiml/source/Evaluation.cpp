// Evaluation.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/01/12
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

#include "Evaluation.h"
#include "ModelTools.h"
#include "ioput/file/File.h"
#include "ISSelectUser.h"
#include "ISSelectSample.h"
#include "Trainer.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif
#if __gnu_linux__
using std::min;
using std::max;
#endif
namespace ssi {

ssi_char_t *Evaluation::ssi_log_name = "evaluation";
int Evaluation::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

int Evaluation::_default_class_id = 0;
bool Evaluation::_allow_unclassified = true;

bool _louo_intermediate = false;
int*** _intermediate_louo_conf_mats;
int _current_intermediate_user = 0;
std::vector<std::string> user_names;

Evaluation::Evaluation () 
	: _n_classes (0),
	_n_streams(0),
	_n_features(0),
	_n_unclassified (0),
	_n_classified (0),
	_n_total (0),
	_result_vec (0),
	_result_vec_ptr (0),
	_result_vec_reg(0),
	_result_vec_reg_ptr(0),
	_result_probs(0),
	_result_probs_ptr(0),
	_conf_mat_ptr (0),
	_conf_mat_data (0),
	_trainer (0),
	_class_names (0),
	_fselmethod (0),
	_pre_fselmethod (0),
	_n_pre_feature (0),	
	_preproc_mode (false),
	_n_streams_refs (0),
	_stream_refs (0) {
}


Evaluation::~Evaluation () {

	release ();
}

void Evaluation::init(ISamples &samples, Trainer *trainer, IModel::TASK::List task) {

	release();

	_n_classes = samples.getClassSize ();
	_n_streams = samples.getStreamSize();
	_n_features = new ssi_size_t[_n_streams];
	for (ssi_size_t i = 0; i < _n_streams; i++)
	{
		if (trainer->hasSelection())
		{
			trainer->getSelection(i, _n_features[i]);
		}
		else
		{
			_n_features[i] = samples.getStream(i).dim;
		}
	}

	// store class names
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy (samples.getClassName (i));
	}

	// allocate confussion matrix
	_conf_mat_ptr = new ssi_size_t *[_n_classes];
	_conf_mat_data = new ssi_size_t[_n_classes * _n_classes];
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		_conf_mat_ptr[i] = _conf_mat_data + i*_n_classes;
	}

	// set all elements in the confussion matrix to zero
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		for (ssi_size_t j = 0; j < _n_classes; ++j) {
			_conf_mat_ptr[i][j] = 0;
		}
	}

	// allocate result vectors
	_n_total = samples.getSize();
	if (task == IModel::TASK::CLASSIFICATION)
	{
		_result_vec = new ssi_size_t[2 * _n_total];
		_result_vec_ptr = _result_vec;
	}
	else
	{
		_result_vec_reg = new ssi_real_t[2 * _n_total];
		_result_vec_reg_ptr = _result_vec_reg;
	}
	_result_probs = new ssi_real_t[samples.getClassSize() * _n_total];
	_result_probs_ptr = _result_probs;

	_n_unclassified = 0;
	_n_classified = 0;	

	_trainer = trainer;
	_task = task;
	

	if (_louo_intermediate) {
		_current_intermediate_user = 0;
		_intermediate_louo_conf_mats = new int**[samples.getUserSize()];
		for (ssi_size_t i = 0; i < samples.getUserSize(); i++) {
			_intermediate_louo_conf_mats[i] = new int*[_n_classes];
			for (ssi_size_t j = 0; j < _n_classes; j++) {
				_intermediate_louo_conf_mats[i][j] = new int[_n_classes];
			}
		}

		for (ssi_size_t i = 0; i < samples.getUserSize(); i++) {
			user_names.push_back(samples.getUserName(i));
		}
	}
}

bool Evaluation::eval(Annotation *prediction, Annotation *truth)
{
	release();

	if (prediction->getScheme()->type != SSI_SCHEME_TYPE::DISCRETE && 
		prediction->getScheme()->type != SSI_SCHEME_TYPE::CONTINUOUS)
	{
		ssi_wrn("scheme type not supported '%s'", SSI_SCHEME_NAMES[prediction->getScheme()->type]);
		return false;
	}

	if (prediction->getScheme()->type != truth->getScheme()->type)
	{
		ssi_wrn("scheme type differ '%s != %s'", SSI_SCHEME_NAMES[prediction->getScheme()->type], SSI_SCHEME_NAMES[truth->getScheme()->type]);
		return false;
	}	

	if (prediction->size() != truth->size())
	{
		ssi_wrn("annotations differ in size '%u != %u'", (ssi_size_t) prediction->size(), (ssi_size_t) truth->size());
	}

	_task = prediction->getScheme()->type == SSI_SCHEME_TYPE::CONTINUOUS ? IModel::TASK::REGRESSION : IModel::TASK::CLASSIFICATION;

	if (_task == IModel::TASK::CLASSIFICATION)
	{
		const ssi_scheme_t *scheme = prediction->getScheme();

		_n_classes = scheme->discrete.n;

		// store class names
		_class_names = new ssi_char_t *[_n_classes];
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			_class_names[i] = ssi_strcpy(scheme->discrete.names[i]);
		}

		// allocate confussion matrix
		_conf_mat_ptr = new ssi_size_t *[_n_classes];
		_conf_mat_data = new ssi_size_t[_n_classes * _n_classes];
		for (ssi_size_t i = 0; i < _n_classes; ++i) {
			_conf_mat_ptr[i] = _conf_mat_data + i*_n_classes;
		}

		// set all elements in the confussion matrix to zero
		for (ssi_size_t i = 0; i < _n_classes; ++i) {
			for (ssi_size_t j = 0; j < _n_classes; ++j) {
				_conf_mat_ptr[i][j] = 0;
			}
		}

		// allocate result vectors
		_n_total = (ssi_size_t) min(prediction->size(), truth->size());

		if (_n_total == 0)
		{
			ssi_wrn("found empty annotation");
			return false;
		}

		_result_vec = new ssi_size_t[2 * _n_total];
		_result_vec_ptr = _result_vec;		
		_result_probs = new ssi_real_t[_n_classes * _n_total];
		_result_probs_ptr = _result_probs;
		_n_unclassified = 0;
		_n_classified = 0;
		_trainer = 0;

		// walk through sample list and test trainer against each sample		
		const ssi_sample_t *sample = 0;
		ssi_size_t index, real_index;						
		Annotation::iterator pit = prediction->begin();
		Annotation::iterator tit = truth->begin();
		for (; pit != prediction->end() && tit != truth->end(); pit++, tit++)
		{
			if(!prediction->getClassIndex(pit->discrete.id, index)
				|| !truth->getClassIndex(tit->discrete.id, real_index))
			{
				*_result_vec_ptr++ = SSI_SAMPLE_GARBAGE_CLASS_ID;
				*_result_vec_ptr++ = SSI_SAMPLE_GARBAGE_CLASS_ID;				
				for (ssi_size_t i = 0; i < _n_classes; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
				_n_unclassified++;				
			}
			else
			{
				*_result_vec_ptr++ = real_index;
				*_result_vec_ptr++ = index;
				for (ssi_size_t i = 0; i < _n_classes; i++) {
					*_result_probs_ptr++ = i == index ? 1.0f : 0.0f;
				}					
				_conf_mat_ptr[real_index][index]++;
				_n_classified++;				
			}
		}
	}
	else
	{
		ssi_err("NOT IMPLEMENTED");
	}

	return true;
}

void Evaluation::eval(Trainer *trainer, ISamples &samples, IModel::TASK::List task) {

	init(samples, trainer, task);	
	eval_h (samples);
}

void Evaluation::eval_h(ISamples &samples) {

	if (_task == IModel::TASK::CLASSIFICATION)
	{
		// walk through sample list and test trainer against each sample
		samples.reset();
		const ssi_sample_t *sample = 0;
		ssi_size_t index, real_index;
		ssi_size_t n_probs = samples.getClassSize();
		ssi_real_t *probs = new ssi_real_t[n_probs];
		ssi_real_t max_probs;
		while (sample = samples.next()) {
			real_index = sample->class_id;
			*_result_vec_ptr = real_index;
			_result_vec_ptr++;
			if (_trainer->forward_probs(sample->num, sample->streams, n_probs, probs)) {
				index = 0;
				max_probs = probs[0];
				*_result_probs_ptr = probs[0];
				_result_probs_ptr++;
				for (ssi_size_t i = 1; i < n_probs; i++) {
					*_result_probs_ptr = probs[i];
					_result_probs_ptr++;
					if (max_probs < probs[i]) {
						max_probs = probs[i];
						index = i;
					}
				}
				*_result_vec_ptr = index;
				_result_vec_ptr++;
				_conf_mat_ptr[real_index][index]++;
				_n_classified++;
			}
			else if (!_allow_unclassified) {
				index = _default_class_id;
				*_result_vec_ptr = index;
				_result_vec_ptr++;
				_conf_mat_ptr[real_index][index]++;
				_n_classified++;
				for (ssi_size_t i = 0; i < n_probs; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
			else {
				*_result_vec_ptr = SSI_SAMPLE_GARBAGE_CLASS_ID;
				_result_vec_ptr++;
				_n_unclassified++;
				for (ssi_size_t i = 0; i < n_probs; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
		}


		if (_louo_intermediate) {

			for (ssi_size_t i = 0; i < _n_classes; i++) {
				for (ssi_size_t j = 0; j < _n_classes; j++) {
					_intermediate_louo_conf_mats[_current_intermediate_user][i][j] = _conf_mat_ptr[i][j];

					for (int k = 0; k < _current_intermediate_user; k++) {
						_intermediate_louo_conf_mats[_current_intermediate_user][i][j] -= _intermediate_louo_conf_mats[k][i][j];
					}
				}
			}

			_current_intermediate_user++;
		}

		delete[] probs;
		
	}
	else
	{
		// walk through sample list and test trainer against each sample
		samples.reset();
		const ssi_sample_t *sample = 0;
		ssi_size_t n_probs = samples.getClassSize();
		ssi_real_t *probs = new ssi_real_t[n_probs];
		while (sample = samples.next()) {
			if (_trainer->forward_probs(sample->num, sample->streams, n_probs, probs)) {								
				*_result_vec_reg_ptr = sample->score;
				_result_vec_reg_ptr++;				
				for (ssi_size_t i = 0; i < n_probs; i++) {
					*_result_probs_ptr = probs[i];
					_result_probs_ptr++;
				}				
				*_result_vec_reg_ptr = probs[0];
				_result_vec_reg_ptr++;				
				_n_classified++;
			}			
			else {
				_n_unclassified++;
				for (ssi_size_t i = 0; i < n_probs; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
		}
	}
}

void Evaluation::eval(IModel &model, ISamples &samples, ssi_size_t stream_index, IModel::TASK::List task) {

	init(samples, 0, task);

	ssi_size_t n_classes = samples.getClassSize ();
	ssi_real_t *probs = new ssi_real_t[n_classes];

	samples.reset ();
	const ssi_sample_t *sample = 0;	
	while (sample = samples.next ()) {

		if (task == IModel::TASK::CLASSIFICATION)
		{
			ssi_size_t real_index = sample->class_id;
			*_result_vec_ptr = real_index;
			_result_vec_ptr++;

			if (model.forward(*sample->streams[stream_index], n_classes, probs)) {

				ssi_size_t max_ind = 0;
				ssi_real_t max_val = probs[0];
				*_result_probs_ptr = probs[0];
				_result_probs_ptr++;
				for (ssi_size_t i = 1; i < n_classes; i++) {
					*_result_probs_ptr = probs[i];
					_result_probs_ptr++;
					if (probs[i] > max_val) {
						max_val = probs[i];
						max_ind = i;
					}
				}

				*_result_vec_ptr = max_ind;
				_result_vec_ptr++;
				_conf_mat_ptr[real_index][max_ind]++;
				_n_classified++;

			}
			else if (!_allow_unclassified) {
				ssi_size_t max_ind = _default_class_id;
				*_result_vec_ptr = max_ind;
				_result_vec_ptr++;
				_conf_mat_ptr[real_index][max_ind]++;
				_n_classified++;
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
			else {
				*_result_vec_ptr = SSI_SAMPLE_GARBAGE_CLASS_ID;
				_result_vec_ptr++;
				_n_unclassified++;
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
		}
		else
		{
			if (model.forward(*sample->streams[stream_index], n_classes, probs)) {				
				*_result_vec_reg_ptr = sample->score;
				_result_vec_reg_ptr++;				
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = probs[i];
					_result_probs_ptr++;				
				}
				*_result_vec_reg_ptr = probs[0];
				_result_vec_reg_ptr++;
				_n_classified++;
			}
			else {			
				_n_unclassified++;
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
		}

	}

	delete[] probs;
}

void Evaluation::eval(IFusion &fusion, ssi_size_t n_models, IModel **models, ISamples &samples, IModel::TASK::List task) {
	
	init(samples, 0, task);

	ssi_size_t n_classes = samples.getClassSize ();	
	ssi_real_t *probs = new ssi_real_t[n_classes];

	samples.reset ();
	const ssi_sample_t *sample = 0;	
	while (sample = samples.next ()) {

		if (task == IModel::TASK::CLASSIFICATION)
		{
			ssi_size_t real_index = sample->class_id;
			*_result_vec_ptr = real_index;
			_result_vec_ptr++;
			if (fusion.forward(n_models, models, sample->num, sample->streams, n_classes, probs)) {

				ssi_size_t max_ind = 0;
				ssi_real_t max_val = probs[0];
				*_result_probs_ptr = probs[0];
				_result_probs_ptr++;
				for (ssi_size_t i = 1; i < n_classes; i++) {
					*_result_probs_ptr = probs[i];
					_result_probs_ptr++;
					if (probs[i] > max_val) {
						max_val = probs[i];
						max_ind = i;
					}
				}

				*_result_vec_ptr = max_ind;
				_result_vec_ptr++;
				_conf_mat_ptr[real_index][max_ind]++;
				_n_classified++;

			}
			else if (!_allow_unclassified) {
				ssi_size_t max_ind = _default_class_id;
				*_result_vec_ptr = max_ind;
				_result_vec_ptr++;
				_conf_mat_ptr[real_index][max_ind]++;
				_n_classified++;
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
			else {
				*_result_vec_ptr = SSI_SAMPLE_GARBAGE_CLASS_ID;
				_result_vec_ptr++;
				_n_unclassified++;
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
		}
		else
		{
			if (fusion.forward(n_models, models, sample->num, sample->streams, n_classes, probs)) {
				*_result_vec_reg_ptr = sample->score;
				_result_vec_reg_ptr++;
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = probs[i];
					_result_probs_ptr++;
				}
				*_result_vec_reg_ptr = probs[0];
				_result_vec_reg_ptr++;
				_n_classified++;
			}
			else {
				_n_unclassified++;
				for (ssi_size_t i = 0; i < n_classes; i++) {
					*_result_probs_ptr = 0;
					_result_probs_ptr++;
				}
			}
		}
	}

	delete[] probs;
}

void Evaluation::evalSplit(Trainer *trainer, ISamples &samples, ssi_real_t split, IModel::TASK::List task) {

	if (split <= 0 || split >= 1) {
		ssi_err ("split must be a value between 0 and 1");
	}
	
	init(samples, trainer, task);
	
	ssi_size_t *indices = new ssi_size_t[samples.getSize ()];
	ssi_size_t *indices_count_lab = new ssi_size_t[samples.getClassSize ()];
	ssi_size_t indices_count_all;

	indices_count_all = 0;
	for (ssi_size_t j = 0; j < samples.getClassSize (); j++) {
		indices_count_lab[j] = 0;
	}

	ssi_size_t label;
	ssi_size_t label_size;
	for (ssi_size_t j = 0; j < samples.getSize (); j++) {
		label = samples.get (j)->class_id;
		label_size = samples.getSize (label);
        indices_count_lab[label]++;
        if (indices_count_lab[label] <= ssi_cast (ssi_size_t, label_size * split + 0.5f)) {
            indices[indices_count_all] = j;
            indices_count_all++;
		}
	}

	SampleList strain;
	SampleList stest;

	// split off samples
	ModelTools::SelectSampleList (samples, strain, stest, indices_count_all, indices);

	// train with remaining samples
	_trainer->release ();	
	if (_preproc_mode) {
		_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
	} else if (_fselmethod) {
		_trainer->setSelection (strain, _fselmethod, _pre_fselmethod, _n_pre_feature);
	}
	_trainer->train (strain);		

	// test with remaining samples
	eval_h (stest);

	delete[] indices;
	delete[] indices_count_lab;

}

void Evaluation::evalKFold(Trainer *trainer, ISamples &samples, ssi_size_t k, IModel::TASK::List task) {
	
	init(samples, trainer, task);
	
	ssi_size_t *indices = new ssi_size_t[samples.getSize ()];
	ssi_size_t *indices_count_lab = new ssi_size_t[samples.getClassSize ()];
	ssi_size_t indices_count_all;

	for (ssi_size_t i = 0; i < k; ++i) {

		indices_count_all = 0;
		for (ssi_size_t j = 0; j < samples.getClassSize (); j++) {
			indices_count_lab[j] = 0;
		}

		ssi_size_t label;
		for (ssi_size_t j = 0; j < samples.getSize (); j++) {
			label = samples.get (j)->class_id;
            indices_count_lab[label]++;
            if (indices_count_lab[label] % k == i) {
                indices[indices_count_all] = j;
                indices_count_all++;
			}
		}

		SampleList strain;
		SampleList stest;	
		// split off i'th fold
		ModelTools::SelectSampleList (samples, stest, strain, indices_count_all, indices);

		// train with i'th fold
		_trainer->release ();
		if (_fselmethod) {
			_trainer->setSelection (strain, _fselmethod, _pre_fselmethod, _n_pre_feature);
		}
		if (_preproc_mode) {
			_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
		}
		_trainer->train (strain);

		// test with remaining samples
		eval_h (stest);
	}

	delete[] indices;
	delete[] indices_count_lab;
}

void Evaluation::evalLOO(Trainer *trainer, ISamples &samples, IModel::TASK::List task) {

	init(samples, trainer, task);
	ssi_size_t n_samples = samples.getSize ();

	ssi_size_t itest  = 0;
	ssi_size_t *itrain = new ssi_size_t[n_samples - 1];
	for (ssi_size_t nsample = 0; nsample < n_samples - 1; ++nsample) {
		itrain[nsample] = nsample+1;
	}
	
	ISSelectSample strain (&samples);
	ISSelectSample stest (&samples);

	strain.setSelection  (n_samples-1, itrain);
	stest.setSelection (1, &itest);

	_trainer->release ();
	if (_fselmethod) {
		_trainer->setSelection (strain, _fselmethod, _pre_fselmethod, _n_pre_feature);
	}
	if (_preproc_mode) {
		_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
	}
	_trainer->train (strain);
	eval_h(stest);

	for (ssi_size_t nsample = 1; nsample < n_samples; ++nsample) {
		
		itrain[nsample-1] = nsample-1;
		itest = nsample;

		strain.setSelection  (n_samples-1, itrain);
		stest.setSelection (1, &itest);

		_trainer->release ();	
		if (_fselmethod) {
			_trainer->setSelection (strain, _fselmethod, _pre_fselmethod, _n_pre_feature);
		}
		if (_preproc_mode) {
			_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
		}
		_trainer->train (strain);

		eval_h(stest);
	}

	delete [] itrain;

}

void Evaluation::evalLOUO(Trainer *trainer, ISamples &samples, IModel::TASK::List task){
	
	_louo_intermediate = true;

	init(samples, trainer, task);

	ssi_size_t n_users = samples.getUserSize ();

	ssi_size_t itest  = 0;
	ssi_size_t *itrain = new ssi_size_t[n_users - 1];
	for (ssi_size_t nuser = 0; nuser < n_users - 1; ++nuser) {
		itrain[nuser] = nuser+1;
	}
	
	ISSelectUser strain (&samples);
	ISSelectUser stest (&samples);

	strain.setSelection  (n_users-1, itrain);
	stest.setSelection (1, &itest);

	_trainer->release ();
	if (_fselmethod) {
		_trainer->setSelection (strain, _fselmethod, _pre_fselmethod, _n_pre_feature);
	}
	if (_preproc_mode) {
		_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
	}
	_trainer->train (strain);
	eval_h (stest);		

	for (ssi_size_t nuser = 1; nuser < n_users; ++nuser) {
		
		itrain[nuser-1] = nuser-1;
		itest = nuser;

		strain.setSelection  (n_users-1, itrain);
		stest.setSelection (1, &itest);

		_trainer->release ();
		if (_fselmethod) {
			_trainer->setSelection (strain, _fselmethod, _pre_fselmethod, _n_pre_feature);
		}
		if (_preproc_mode) {
			_trainer->setPreprocMode (_preproc_mode, _n_streams_refs, _stream_refs);
		}
		_trainer->train (strain);

		eval_h(stest);
	}

	delete [] itrain;
}

ssi_size_t Evaluation::cutString(const ssi_char_t *str, ssi_size_t n_cut_max, ssi_char_t *cut) {

	if (n_cut_max == 0 || !cut) {
		return 0;
	}

	if (!str || str[0] == '\0') {
		cut[0] = '\0';
		return 0;
	}

	ssi_size_t n_str = ssi_strlen(str);
	if (n_cut_max > n_str) {
		ssi_strcpy(cut, str);
		return n_str;
	}

	ssi_size_t n_front = (n_cut_max-1) / 2;
	ssi_size_t n_back = (n_cut_max - 1) - n_front;
	for (ssi_size_t i = 0; i < n_front; i++) {
		cut[i] = str[i];
	}
	for (ssi_size_t i = 0; i < n_back; i++) {
		cut[n_cut_max-2-i] = str[n_str-1-i];
	}
	cut[n_cut_max-1] = '\0';

	return n_cut_max-1;
}

void Evaluation::print (FILE *file, PRINT::List format) {

	if (!_conf_mat_ptr) {
		ssi_wrn ("nothing to print");
		return;
	}		

	ssi_size_t max_label_len = 0;
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		ssi_size_t len = ssi_cast (ssi_size_t, strlen (_class_names[i]));
		if (len > max_label_len) {
			max_label_len = len;
		}
	}

	if (format == PRINT::CSV || format == PRINT::CSV_EX) {

		File *tmp = File::Create(File::ASCII, File::WRITE, 0, file);
		tmp->setType(SSI_UINT);
		tmp->setFormat(";", "");

		ssi_fprint(file, "#classes;%u\n", _n_classes);
		ssi_fprint(file, "#features");
		for (ssi_size_t i = 0; i < _n_streams; i++)
		{
			ssi_fprint(file, ";#%u", _n_features[i]);
		}
		ssi_fprint(file, "\n");
		ssi_fprint(file, "#total;%u\n", _n_classified + _n_unclassified);
		ssi_fprint(file, "#classified;%u\n", _n_classified);
		ssi_fprint(file, "#unclassified;%u\n", _n_unclassified);

		if (_task == IModel::TASK::CLASSIFICATION)
		{

			for (ssi_size_t i = 0; i < _n_classes; ++i) {
				ssi_fprint(file, ";%s", _class_names[i]);
			}

			ssi_fprint(file, "\n");
			for (ssi_size_t i = 0; i < _n_classes; ++i) {
				ssi_fprint(file, "%s;", _class_names[i]);
				tmp->write(_conf_mat_ptr[i], 0, _n_classes);
				ssi_fprint(file, "%f\n", 100 * get_class_prob(i));
			}
			for (ssi_size_t i = 0; i < _n_classes; ++i) {
				ssi_fprint(file, "; ");
			}
			ssi_fprint(file, ";%f;%f\n", 100 * get_classwise_prob(), 100 * get_accuracy_prob());

			if (format == PRINT::CSV_EX) {
				ssi_fprint(file, "\ntruth;prediction");
				for (ssi_size_t i = 0; i < _n_classes; i++) {
					ssi_fprint(file, ";%s", _class_names[i]);
				}
				ssi_fprint(file, "\n");
				ssi_size_t *vec_ptr = _result_vec;
				ssi_real_t *probs_ptr = _result_probs;
				for (ssi_size_t i = 0; i < _n_total; i++) {
					ssi_fprint(file, "%u;%u", vec_ptr[0], vec_ptr[1]);
					vec_ptr += 2;
					for (ssi_size_t j = 0; j < _n_classes; j++) {
						ssi_fprint(file, ";%f", *probs_ptr++);
					}
					ssi_fprint(file, "\n");
				}
			}
		}
		else
		{
			ssi_fprint(file, "\n");
			ssi_fprint(file, "correlation:   %f\n", get_correlation());

			if (format == PRINT::CSV_EX) {
				ssi_fprint(file, "\ntruth;prediction");
				for (ssi_size_t i = 0; i < _n_classes; i++) {
					ssi_fprint(file, ";%s", _class_names[i]);
				}
				ssi_fprint(file, "\n");
				ssi_real_t *vec_ptr = _result_vec_reg;				
				for (ssi_size_t i = 0; i < _n_total; i++) {
					ssi_fprint(file, "%f;%f", vec_ptr[0], vec_ptr[1]);
					vec_ptr += 2;
					ssi_fprint(file, "\n");
				}
			}
		}

	} else {

		File *tmp = File::Create(File::ASCII, File::WRITE, 0, file);
		tmp->setType(SSI_UINT);
		tmp->setFormat(" ", "6");

		ssi_fprint_off(file, "#classes:      %u\n", _n_classes);
		ssi_fprint_off(file, "#features:    ");
		for (ssi_size_t i = 0; i < _n_streams; i++)
		{
			ssi_fprint(file, " %u", _n_features[i]);
		}
		ssi_fprint(file, "\n");
		ssi_fprint_off(file, "#total:        %u\n", _n_classified + _n_unclassified);
		ssi_fprint_off(file, "#classified:   %u\n", _n_classified);
		ssi_fprint_off(file, "#unclassified: %u\n", _n_unclassified);

		if (_task == IModel::TASK::CLASSIFICATION)
		{
			ssi_fprint_off(file, "");
			if (format == PRINT::CONSOLE_EX) {
				for (ssi_size_t j = 0; j < max_label_len + 3; j++) {
					ssi_fprint(file, " ");
				}
				ssi_char_t cut[7];
				for (ssi_size_t i = 0; i < _n_classes; ++i) {
					cutString(_class_names[i], 7, cut);
					ssi_fprint(file, " %6s", cut);
				}
			}
			ssi_fprint(file, "\n");
			for (ssi_size_t i = 0; i < _n_classes; ++i) {
				ssi_fprint_off(file, "%*s: ", max_label_len, _class_names[i]);
				tmp->write(_conf_mat_ptr[i], 0, _n_classes);
				ssi_fprint(file, "   -> %8.2f%%\n", 100 * get_class_prob(i));
			}
			ssi_fprint_off(file, "   %*s  => %8.2f%% | %.2f%%\n", max_label_len + _n_classes * 7, "", 100 * get_classwise_prob(), 100 * get_accuracy_prob());
			delete tmp;
		}		
		else
		{
			ssi_fprint(file, "\n");
			ssi_fprint_off(file, "correlation:   %f\n", get_correlation());
		}
	}

	fflush (file);
}

void Evaluation::print_intermediate_louo(FILE *file, PRINT::List format) {

	if (!_conf_mat_ptr) {
		ssi_wrn("nothing to print");
		return;
	}

	ssi_size_t max_label_len = 0;
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		ssi_size_t len = ssi_cast(ssi_size_t, strlen(_class_names[i]));
		if (len > max_label_len) {
			max_label_len = len;
		}
	}

	if (format == PRINT::CSV || format == PRINT::CSV_EX) {

		for (int h = 0; h < _current_intermediate_user; h++) {

			ssi_fprint(file, "#User;%u;%s\n", h, user_names[h].c_str());

			File *tmp = File::Create(File::ASCII, File::WRITE, 0, file);
			tmp->setType(SSI_UINT);
			tmp->setFormat(";", "");

			int n_classified_intermediate = 0;
			for (ssi_size_t i = 0; i < _n_classes; ++i) {
				for (ssi_size_t j = 0; j < _n_classes; ++j) {
					n_classified_intermediate += _intermediate_louo_conf_mats[h][i][j];
				}
			}

			//TODO intermediate n_unclassified

			ssi_fprint(file, "#classes;%u\n", _n_classes);
			ssi_fprint(file, "#total;%u\n", n_classified_intermediate + _n_unclassified);
			ssi_fprint(file, "#classified;%u\n", n_classified_intermediate);
			ssi_fprint(file, "#unclassified;%u\n", _n_unclassified);

			if (_task == IModel::TASK::CLASSIFICATION)
			{

				for (ssi_size_t i = 0; i < _n_classes; ++i) {
					ssi_fprint(file, ";%s", _class_names[i]);
				}

				ssi_fprint(file, "\n");
				for (ssi_size_t i = 0; i < _n_classes; ++i) {
					ssi_fprint(file, "%s;", _class_names[i]);
					tmp->write(_intermediate_louo_conf_mats[h][i], 0, _n_classes);
					ssi_fprint(file, "%f\n", 100 * get_intermediate_louo_class_prob(i, h));
				}
				for (ssi_size_t i = 0; i < _n_classes; ++i) {
					ssi_fprint(file, "; ");
				}

				std::vector<float> il_cw = get_intermediate_louo_classwise_prob();
				std::vector<float> il_acc = get_intermediate_louo_accuracy_prob();

				ssi_fprint(file, ";%f;%f\n", 100 * il_cw[h], 100 *il_acc[h]);

				if (format == PRINT::CSV_EX) {
					ssi_fprint(file, "\ntruth;prediction");
					for (ssi_size_t i = 0; i < _n_classes; i++) {
						ssi_fprint(file, ";%s", _class_names[i]);
					}
					ssi_fprint(file, "\n");
					ssi_size_t *vec_ptr = _result_vec;
					ssi_real_t *probs_ptr = _result_probs;
					for (ssi_size_t i = 0; i < _n_total; i++) {
						ssi_fprint(file, "%u;%u", vec_ptr[0], vec_ptr[1]);
						vec_ptr += 2;
						for (ssi_size_t j = 0; j < _n_classes; j++) {
							ssi_fprint(file, ";%f", *probs_ptr++);
						}
						ssi_fprint(file, "\n");
					}
				}
			}
			else
			{
				ssi_fprint(file, "\n");
				ssi_fprint(file, "correlation:   %f\n", get_correlation());

				if (format == PRINT::CSV_EX) {
					ssi_fprint(file, "\ntruth;prediction");
					for (ssi_size_t i = 0; i < _n_classes; i++) {
						ssi_fprint(file, ";%s", _class_names[i]);
					}
					ssi_fprint(file, "\n");
					ssi_real_t *vec_ptr = _result_vec_reg;
					for (ssi_size_t i = 0; i < _n_total; i++) {
						ssi_fprint(file, "%f;%f", vec_ptr[0], vec_ptr[1]);
						vec_ptr += 2;
						ssi_fprint(file, "\n");
					}
				}
			}

		}

	}
	else {
		for (int h = 0; h < _current_intermediate_user; h++) {

			File *tmp = File::Create(File::ASCII, File::WRITE, 0, file);
			tmp->setType(SSI_UINT);
			tmp->setFormat(" ", "6");

			int n_classified_intermediate = 0;
			for (ssi_size_t i = 0; i < _n_classes; ++i) {
				for (ssi_size_t j = 0; j < _n_classes; ++j) {
					n_classified_intermediate += _intermediate_louo_conf_mats[h][i][j];
				}
			}

			//TODO intermediate n_unclassified
			ssi_fprint(file, "\n");
			ssi_fprint_off(file, "#User:         %u (%s)\n", h, user_names[h].c_str());
			ssi_fprint_off(file, "#classes:      %u\n", _n_classes);
			ssi_fprint_off(file, "#total:        %u\n", n_classified_intermediate + _n_unclassified);
			ssi_fprint_off(file, "#classified:   %u\n", n_classified_intermediate);
			ssi_fprint_off(file, "#unclassified: %u\n", _n_unclassified);

			if (_task == IModel::TASK::CLASSIFICATION)
			{
				ssi_fprint_off(file, "");
				if (format == PRINT::CONSOLE_EX) {
					for (ssi_size_t j = 0; j < max_label_len + 3; j++) {
						ssi_fprint(file, " ");
					}
					ssi_char_t cut[7];
					for (ssi_size_t i = 0; i < _n_classes; ++i) {
						cutString(_class_names[i], 7, cut);
						ssi_fprint(file, " %6s", cut);
					}
				}
				ssi_fprint(file, "\n");
				for (ssi_size_t i = 0; i < _n_classes; ++i) {
					ssi_fprint_off(file, "%*s: ", max_label_len, _class_names[i]);
					tmp->write(_intermediate_louo_conf_mats[h][i], 0, _n_classes);
					ssi_fprint(file, "   -> %8.2f%%\n", 100 * get_intermediate_louo_class_prob(i, h));
				}

				std::vector<float> il_cw = get_intermediate_louo_classwise_prob();
				std::vector<float> il_acc = get_intermediate_louo_accuracy_prob();

				ssi_fprint_off(file, "   %*s  => %8.2f%% | %.2f%%\n", max_label_len + _n_classes * 7, "", 100 * il_cw[h], 100 * il_acc[h]);
				delete tmp;
			}
			else
			{
				ssi_fprint(file, "\n");
				ssi_fprint_off(file, "correlation:   %f\n", get_correlation());
			}

		}
	}



	fflush(file);
}

void Evaluation::print_result_vec (FILE *file) {

	if (_task == IModel::TASK::CLASSIFICATION)
	{
		if (!_result_vec) {
			ssi_wrn("nothing to print");
			return;
		}

		ssi_size_t *ptr = _result_vec;
		for (ssi_size_t i = 0; i < _n_total; i++) {
			ssi_fprint(file, "%u %u\n", *ptr, *(ptr + 1));
			ptr += 2;
		}
	}
	else
	{
		if (!_result_vec_reg) {
			ssi_wrn("nothing to print");
			return;
		}

		ssi_real_t *ptr = _result_vec_reg;
		for (ssi_size_t i = 0; i < _n_total; i++) {
			ssi_fprint(file, "%f %f\n", *ptr, *(ptr + 1));
			ptr += 2;
		}
	}
}

ssi_real_t Evaluation::get_class_prob (ssi_size_t index) {

	ssi_size_t sum = 0;
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		sum += _conf_mat_ptr[index][i];
	}
	ssi_real_t prob = sum > 0 ? ssi_cast (ssi_real_t, _conf_mat_ptr[index][index]) / ssi_cast (ssi_real_t, sum) : 0; 

	return prob;
}

ssi_real_t Evaluation::get_intermediate_louo_class_prob(ssi_size_t index, ssi_size_t nuser) {

	ssi_size_t sum = 0;
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		sum += _intermediate_louo_conf_mats[nuser][index][i];
	}
	ssi_real_t prob = sum > 0 ? ssi_cast(ssi_real_t, _intermediate_louo_conf_mats[nuser][index][index]) / ssi_cast(ssi_real_t, sum) : 0;

	return prob;
}

ssi_real_t Evaluation::get_classwise_prob () {

	ssi_real_t prob = 0;
	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		prob += get_class_prob (i);
	}

	return prob / _n_classes;
}

ssi_real_t Evaluation::get_accuracy_prob () {

	ssi_size_t sum_correct = 0;
	ssi_size_t n_clasified_intermediate = 0;

	for (ssi_size_t i = 0; i < _n_classes; ++i) {
		sum_correct += _conf_mat_ptr[i][i];
	}

	ssi_real_t prob = _n_classified > 0 ? ssi_cast (ssi_real_t, sum_correct) / ssi_cast (ssi_real_t, _n_classified) : 0; 

	return prob;
}

std::vector<ssi_real_t> Evaluation::get_intermediate_louo_classwise_prob() {

	std::vector<ssi_real_t> probs;
	for (int h = 0; h < _current_intermediate_user; ++h) {
		ssi_real_t prob = 0.0;
		for (ssi_size_t i = 0; i < _n_classes; ++i) {
			prob += get_intermediate_louo_class_prob(i, h);
		}

		prob /= ((ssi_real_t)_n_classes);
		probs.push_back(prob);
	}

	return probs;
}

std::vector<ssi_real_t> Evaluation::get_intermediate_louo_accuracy_prob() {

	std::vector<ssi_real_t> probs;
	ssi_real_t prob = 0.0;

	for (int h = 0; h < _current_intermediate_user; ++h) {
		ssi_size_t sum_correct = 0;
		ssi_size_t n_classified_interediate = 0;
		for (ssi_size_t i = 0; i < _n_classes; ++i) {
			sum_correct += _intermediate_louo_conf_mats[h][i][i];
		}

		for (ssi_size_t i = 0; i < _n_classes; ++i) {
			for (ssi_size_t j = 0; j < _n_classes; ++j) {
				n_classified_interediate += _intermediate_louo_conf_mats[h][i][j];
			}
		}

		prob = _n_classified > 0 ? ssi_cast(ssi_real_t, sum_correct) / ssi_cast(ssi_real_t, n_classified_interediate) : 0;
		probs.push_back(prob);

	}

	return probs;
}



ssi_size_t*const* Evaluation::get_conf_mat () {

	ssi_size_t **conf_ptr = _conf_mat_ptr;

	return conf_ptr;
}

std::vector<ssi_size_t*const*> Evaluation::get_intermediate_louo_conf_mat() {

	std::vector<ssi_size_t*const*> r;

	for (int h = 0; h < _current_intermediate_user; ++h) {
		ssi_size_t **conf_ptr = _conf_mat_ptr;
		r.push_back(conf_ptr);
	}

	return r;
}

void Evaluation::release () {

	delete[] _conf_mat_ptr;
	_conf_mat_ptr = 0;
	delete[] _conf_mat_data;
	_conf_mat_data = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		delete[] _class_names[i];
		_class_names[i] = 0;
	}
	delete[] _class_names;
	_class_names = 0;
	_n_classes = 0;
	_n_streams = 0;
	delete[] _n_features; 
	_n_features = 0;
	_n_total = 0;
	delete[] _result_vec;
	_result_vec = 0;
	_result_vec_ptr = 0;
	delete[] _result_vec_reg;
	_result_vec_reg = 0;
	_result_vec_reg_ptr = 0;
	delete[] _result_probs;
	_result_probs = 0;
	_result_probs_ptr = 0;
	_trainer = 0;

	if (_louo_intermediate) {
		for (size_t i = 0; i < user_names.size(); i++) {
			for (ssi_size_t j = 0; j < _n_classes; j++) {
				delete _intermediate_louo_conf_mats[i][j];
			}
		}
	}
}


ssi_real_t Evaluation::corrcoef(ssi_size_t n, ssi_real_t *values)
{
	ssi_real_t r, nr = 0, dr_1 = 0, dr_2 = 0, dr_3 = 0, dr = 0;
	ssi_real_t *xx = new ssi_real_t[n];
	ssi_real_t *xy = new ssi_real_t[n];
	ssi_real_t *yy = new ssi_real_t[n];
	ssi_real_t sum_y = 0, sum_yy = 0, sum_xy = 0, sum_x = 0, sum_xx = 0;
	ssi_size_t i;
	for (i = 0; i<n; i++)
	{		
		xx[i] = values[i * 2] * values[i * 2];
		yy[i] = values[i * 2 + 1] * values[i * 2 + 1];
	}
	for (i = 0; i<n; i++)
	{
		sum_x += values[i * 2];
		sum_y += values[i * 2 + 1];
		sum_xx += xx[i];
		sum_yy += yy[i];
		sum_xy += values[i * 2] * values[i * 2 + 1];
	}
	nr = (n*sum_xy) - (sum_x*sum_y);
	ssi_real_t sum_x2 = sum_x*sum_x;
	ssi_real_t sum_y2 = sum_y*sum_y;
	dr_1 = (n*sum_xx) - sum_x2;
	dr_2 = (n*sum_yy) - sum_y2;
	dr_3 = dr_1*dr_2;
	dr = sqrt(dr_3);
	r = (nr / dr);

	return r;
}

// get correlation coefficient
ssi_real_t Evaluation::get_correlation()
{
	if (_n_classified == 0 || _result_vec_reg == 0)
	{
		return 0;
	}

	return corrcoef(_n_classified, _result_vec_reg);
}


}
