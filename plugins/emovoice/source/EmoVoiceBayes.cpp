// EmoVoiceBayes.cpp
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

#include "EmoVoiceBayes.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

extern "C" {
#include "ev_class.h"
}

namespace ssi {

EmoVoiceBayes::EmoVoiceBayes (const ssi_char_t *file) 
	: _model (0),
	_n_classes (0),
	_n_features (0),
	_cl_type (naive_bayes),
	_cl_param (0) {	
} 

EmoVoiceBayes::~EmoVoiceBayes () { 

	release ();
}

bool EmoVoiceBayes::train (ISamples &samples, ssi_size_t stream_index) {

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

	samples.reset ();
	ssi_sample_t *sample = 0;
	while (sample = samples.next ()) {
		_model = cl_update_classifier (_model, _cl_type, sample->class_id, reinterpret_cast<mx_real_t *> (sample->streams[stream_index]->ptr));
	}
	_model = cl_finish_classifier (_model, _cl_type);

	return true;
}

bool EmoVoiceBayes::forward (ssi_stream_t &stream,
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
		probs[j] = class_prob (_model, _cl_type, ssi_pcast (mx_real_t, stream.ptr), j);	
		sum+=probs[j];
	}
	//normalisation
	for (ssi_size_t j = 0; j < n_probs; j++) {
		probs[j]/=sum;
	}

	return true;
}

bool EmoVoiceBayes::save (const ssi_char_t *filepath) {

	if (!_model) {
		ssi_wrn ("not trained");
		return false;
	}

	for (ssi_size_t n_class = 0; n_class < _n_classes; n_class++) {
		ssi_char_t string[200];
		ssi_sprint (string, "class%02u", n_class);
		_model->naiveBayes->mapping[n_class] = ssi_strcpy (string);
	}
	cl_output_classifier (_model, _cl_type, ssi_ccast (ssi_char_t *, filepath));

	return true;
}

bool EmoVoiceBayes::load (const ssi_char_t *filepath) {

	release ();

	_model = cl_create_classifier (ssi_ccast (ssi_char_t *, filepath), _cl_type);
	_n_classes = _model->naiveBayes->n_classes;
	_n_features = _model->naiveBayes->feature_dim;

	return true;
}

void EmoVoiceBayes::release () {

	if (_model) {
		cl_destroy (_model, _cl_type);
		_model = 0;
	}	
	_n_features = 0;
	_n_classes = 0;
}
/*
Trainer *EmoVoiceBayes::LoadOldFormat (const ssi_char_t *filepath) {
	
	// open file
	// try first without extension
	FILE *file = fopen (filepath, "rb");
	Trainer *trainer = 0;


	// if it fails try with extension
	if (!file) {		
		ssi_char_t *filename_ext = ssi_strcat (filepath, SSI_FILE_TYPE_MODEL);
		file = fopen (filename_ext, "rb");
		delete[] filename_ext;
	}

	if (file) {

		size_t len = 0, num = 0;

		ssi_size_t n_models, n_streams;
		int fusion_method;

		// read model number and stream number
		num = fread (&n_models, sizeof (n_models), 1, file);
		SSI_ASSERT (num == 1);
		num = fread (&n_streams, sizeof (n_streams), 1, file);
		SSI_ASSERT (num == 1);
		num = fread (&fusion_method, sizeof (fusion_method), 1, file);
		SSI_ASSERT (num == 1);

		// read stream information
		ssi_size_t *sample_dimension_in = new ssi_size_t[n_streams];
		int *statistic_formats = new int[n_streams];
		for (ssi_size_t i = 0; i < n_streams; i++) {
			// read sample dimension
			num = fread (&sample_dimension_in[i], sizeof (ssi_size_t), 1, file);
			SSI_ASSERT (num == 1);
			// read statistic formats
			num = fread (&statistic_formats[i], sizeof (int), 1, file);
			SSI_ASSERT (num == 1);
		}

		// read class names
		unsigned int class_num;
		num = fread (&class_num, sizeof (unsigned int), 1, file);
		SSI_ASSERT (num == 1);
		ssi_char_t **class_names = new ssi_char_t*[class_num];
		for (unsigned int i = 0; i < class_num; i++) {			
			num = fread (&len, sizeof (size_t), 1, file);
			SSI_ASSERT (num == 1);
			class_names[i] = new ssi_char_t[len];
			num = fread (class_names[i], sizeof (ssi_char_t), len, file);
			SSI_ASSERT (num == len);
		}

		// read trainer name
		num = fread (&len, sizeof (size_t), 1, file);
		SSI_ASSERT (num == 1);
		ssi_char_t *name = new ssi_char_t[len];
		num = fread (name, sizeof (ssi_char_t), len, file);
		SSI_ASSERT (num == len);

		// read model content
		IModel *model = EmoVoiceBayes::LoadOldFormatH (file, class_num, class_names);
		if (!model) {
			ssi_err ("could not load modal");
		}

		delete[] name;
	
		trainer = new Trainer (model);
		
		// save class names
		trainer->_n_classes = class_num;
		trainer->_class_names = class_names;
		trainer->_n_streams = 1;
		trainer->_stream_refs = new ssi_stream_t;
		ssi_stream_init (trainer->_stream_refs[0], 0, sample_dimension_in[0], sizeof (ssi_real_t), SSI_REAL, 0, 0);
		trainer->_is_trained = true;

		// close file
		fclose (file);

		// clean up
		delete[] sample_dimension_in;
		delete[] statistic_formats;

	} else {
		ssi_err ("could not open file");
	}
	return trainer;
}*/

IModel *EmoVoiceBayes::LoadOldFormatH (FILE *file, 
	ssi_size_t n_classes, 
	ssi_char_t **class_names) {
    
	EmoVoiceBayes *model = 0;

	cType c_type;
	fread (&c_type, sizeof (c_type), 1, file);	

	switch (c_type) {

		case naive_bayes:
		{			  
			model = new EmoVoiceBayes ("naive_bayes");	

			int n_classes = 0;
			fread (&n_classes, sizeof (n_classes), 1, file);				
			int feature_dim = 0;
			fread (&feature_dim, sizeof (feature_dim), 1, file);				
			
			model->_model = cl_new_classifier (c_type, n_classes, feature_dim, 0);
		
			for (int i = 0; i < model->_model->naiveBayes->n_classes; i++) {
				fread (&model->_model->naiveBayes->class_probs[i], sizeof (model->_model->naiveBayes->class_probs[i]), 1, file);				
				for (int j = 0; j < model->_model->naiveBayes->feature_dim; j++) {					
					fread (&model->_model->naiveBayes->means[i][j], sizeof (model->_model->naiveBayes->means[i][j]), 1, file);
					fread (&model->_model->naiveBayes->std_dev[i][j], sizeof (model->_model->naiveBayes->std_dev[i][j]), 1, file);
				}
			}
			for (int i = 0; i < n_classes; i++) {
				model->_model->naiveBayes->mapping[i] = ssi_strcpy (class_names[i]);
			}
			model->_model->naiveBayes->finished = 1;
			break;
		}

		default:
			ssi_err ("not implemented");
	}

	return model;
} 

/*
void EmoVoiceBayes::save (FILE *file) {

	SSI_ASSERT (_model);

	switch (_cl_type) {
		case naive_bayes:
		{			  

			fwrite (&_cl_type, sizeof (_cl_type), 1, file);
			fwrite (&_model->naiveBayes->n_classes, sizeof (_model->naiveBayes->n_classes), 1, file);				
			fwrite (&_model->naiveBayes->feature_dim, sizeof (_model->naiveBayes->feature_dim), 1, file);	

			for (int i = 0; i < _model->naiveBayes->n_classes; i++) {
				fwrite (&_model->naiveBayes->class_probs[i], sizeof (_model->naiveBayes->class_probs[i]), 1, file);				
				for (int j = 0; j < _model->naiveBayes->feature_dim; j++) {
					fwrite (&_model->naiveBayes->means[i][j], sizeof (_model->naiveBayes->means[i][j]), 1, file);
					fwrite (_model->naiveBayes->std_dev[i] + j, sizeof (_model->naiveBayes->std_dev[i][j]), 1, file);
				}
			}
			break;
		}
		
		default:
			ssi_err ("not implemented");
	}
} */

}
