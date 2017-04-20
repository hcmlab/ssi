// Evaluation.h
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

/*
 
lets you evaluate a model
   
*/

#pragma once

#ifndef SSI_MODEL_EVALUATION_H
#define SSI_MODEL_EVALUATION_H

#include "SampleList.h"
#include "base/IModel.h"
#include "base/IFusion.h"
#include "base/ISelection.h"

namespace ssi {

class Trainer;
class Annotation;

class Evaluation {

public:

	struct PRINT {
		enum List {		
			CONSOLE,			
			CONSOLE_EX,
			CSV,
			CSV_EX
		};
	};

public:

	Evaluation ();
	~Evaluation ();

	// set feature selection method
	void setFselMethod (ISelection *fselmethod,
		ISelection *pre_fselmethod = 0,
		ssi_size_t n_pre_feature = 0) {
		_fselmethod = fselmethod;
		_pre_fselmethod = pre_fselmethod;
		_n_pre_feature = n_pre_feature;
	};
	// set pre-processing mode
	void setPreprocMode (bool toggle,
		ssi_size_t n_streams_refs,
		ssi_stream_t stream_refs[]) {
		_preproc_mode = toggle;
		_n_streams_refs = n_streams_refs;
		_stream_refs = stream_refs;
	}

	// compares two annotations
	bool eval(Annotation *prediction, Annotation *truth);

	// eval using test set (it is assumed that model has already been trained)
	void eval(Trainer *trainer, ISamples &samples, IModel::TASK::List task = IModel::TASK::CLASSIFICATION);
	void eval(IModel &model, ISamples &samples, ssi_size_t stream_index, IModel::TASK::List task = IModel::TASK::CLASSIFICATION);
	void eval(IFusion &fusion, ssi_size_t n_models, IModel **models, ISamples &samples, IModel::TASK::List task = IModel::TASK::CLASSIFICATION);

	// eval using the first split% for training and the rest for testing (split = ]0..1[)
	void evalSplit(Trainer *trainer, ISamples &samples, ssi_real_t split, IModel::TASK::List task = IModel::TASK::CLASSIFICATION);

	// eval using k-fold (model is re-trained.. you may lose your old model!)
	void evalKFold(Trainer *trainer, ISamples &samples, ssi_size_t k, IModel::TASK::List task = IModel::TASK::CLASSIFICATION);

	// eval using leave one out (model is re-trained.. you may lose your old model!)
	void evalLOO(Trainer *trainer, ISamples &samples, IModel::TASK::List task = IModel::TASK::CLASSIFICATION);

	// eval using leave one user out (model is re-trained.. you may lose your old model!)
	void evalLOUO(Trainer *trainer, ISamples &samples, IModel::TASK::List task = IModel::TASK::CLASSIFICATION);

	// print evaluation to file
	void print(FILE *file = stdout, PRINT::List format = PRINT::CONSOLE);
	//print intermediate evauation results to file (LOUO only)
	void print_intermediate_louo(FILE *file = stdout, PRINT::List format = PRINT::CONSOLE);
	void print_result_vec (FILE *file = stdout);

	// reset
	void release ();

	// #correct/#all per class
	ssi_real_t get_class_prob (ssi_size_t index);
	// mean of class probs
	ssi_real_t get_classwise_prob ();
	// relative mean of class probs
	ssi_real_t get_accuracy_prob ();

	// mean of class probs (intermediate results, only woks for LOUO)
	std::vector<ssi_real_t> get_intermediate_louo_classwise_prob();
	// relative mean of class probs (intermediate results, only woks for LOUO)
	std::vector<ssi_real_t> get_intermediate_louo_accuracy_prob();
	ssi_real_t get_intermediate_louo_class_prob(ssi_size_t index, ssi_size_t nuser);

	// get confussion matrix
	ssi_size_t*const* get_conf_mat ();
	// get intermediate confusion matrices (LOUO only)
	std::vector<ssi_size_t*const*> get_intermediate_louo_conf_mat();
	// get correlation coefficient
	ssi_real_t get_correlation();
	// get result vector
	const ssi_size_t *get_result_vec (ssi_size_t &size) { size = _n_total; return _result_vec; };
	// get result regression vector
	const ssi_real_t *get_result_vec_reg(ssi_size_t &size) { size = _n_classified; return _result_vec_reg; };
	// get result probs
	const ssi_real_t *get_result_probs(ssi_size_t &n_samples, ssi_size_t &n_classes) { n_samples = _n_total; n_classes = _n_classes; return _result_probs; };

	// class size and names
	ssi_size_t get_class_size () {
		return _n_classes;
	}
	const ssi_char_t *get_class_name (ssi_size_t index) {
		return index >= _n_classes ? 0 : _class_names[index];
	}

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

	ssi_size_t get_n_classified () {
		return _n_classified;
	}
	ssi_size_t get_n_unclassified () {
		return _n_unclassified;
	}

	static void SetHandlingOfUnclassifiedSamples (bool allow_unclassified, int default_class_id) {
		_allow_unclassified = allow_unclassified;
		_default_class_id = default_class_id;
	}

protected:

	ssi_size_t cutString(const ssi_char_t *str, ssi_size_t n, ssi_char_t *cut);
	void init(ISamples &samples, Trainer *trainer, IModel::TASK::List task);
	void eval_h(ISamples &samples);
	ssi_real_t corrcoef(ssi_size_t n, ssi_real_t *values);

	ssi_size_t **_conf_mat_ptr;
	ssi_size_t *_conf_mat_data;	
	ssi_size_t _n_unclassified;
	ssi_size_t _n_classified;
	ssi_size_t _n_total;	 // unclassified + classified
	ssi_size_t *_result_vec; // result vector, for each sample the real id and the classified id is stored, SSI_SAMPLE_GARBAGE_CLASS_ID if unclassified		
	ssi_size_t *_result_vec_ptr;
	ssi_real_t *_result_vec_reg; // result vector for regression tasks, for each sample the real score and the classified score is stored, nan if unclassified	
	ssi_real_t *_result_vec_reg_ptr;	
	ssi_real_t *_result_probs;
	ssi_real_t *_result_probs_ptr;
	ssi_size_t _n_classes;
	ssi_size_t _n_streams;
	ssi_size_t *_n_features;
	ssi_char_t** _class_names;	

	Trainer *_trainer;	
	IModel::TASK::List _task;
	ISelection *_fselmethod;
	ISelection *_pre_fselmethod;
	ssi_size_t _n_pre_feature;
	bool _preproc_mode;
	ssi_size_t _n_streams_refs;
	ssi_stream_t *_stream_refs;

	static bool _allow_unclassified;
	static int _default_class_id;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
};

}

#endif
