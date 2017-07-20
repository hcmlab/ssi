// SVM.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/05/05
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

#include "SVM.h"
#include "ssiml/include/ISUnderSample.h"
#include "ssiml/include/ISOverSample.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *SVM::ssi_log_name = "svm_______";

SVM::SVM (const ssi_char_t *file) 
	: _n_features (0),
	_n_classes (0),
	_file (0),
	_model (0),
	_max (0),
	_min (0),
	_class_names (0),
	_problem (0),
	_n_samples (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

SVM::~SVM () { 

	release ();
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void SVM::release () {

	if (_problem) {
		for (ssi_size_t nsamp = 0; nsamp < _n_samples; nsamp++) {
			delete[] _problem->x[nsamp];
		}
		delete[] _problem->y;
		delete[] _problem->x;
		delete _problem; _problem = 0;
	}

	free_class_names ();
	if (_model) {
		svm_free_and_destroy_model (&_model);
		_model = 0;
	}
	delete[] _max; _max = 0;
	delete[] _min; _min = 0;

	//svm_destroy_param (&_options.params);

	_n_classes = 0;
	_n_features = 0;
	_n_samples = 0;
}

void SVM::init_class_names (ISamples &samples) {

	free_class_names ();

	_n_classes = samples.getClassSize ();
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy (samples.getClassName (i));
	}
}

void SVM::free_class_names () {

	if (_class_names) {
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			delete[] _class_names[i];
		}
		delete[] _class_names;
		_class_names = 0;
	}	
}

bool SVM::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (_options.seed > 0) {
		srand(_options.seed);
	} else {
		srand(ssi_time_ms());
	}

	ISamples *s_balance = 0;
	switch (_options.balance) {
	case BALANCE::OFF: {
		s_balance = &samples;
		break;
	}
	case BALANCE::OVER: {		
		s_balance = new ISOverSample(&samples);
		ssi_pcast(ISOverSample, s_balance)->setOver(ISOverSample::RANDOM);
		ssi_msg(SSI_LOG_LEVEL_BASIC, "balance training set '%u' -> '%u'", samples.getSize(), s_balance->getSize());
		break;
	}
	case BALANCE::UNDER: {		
		s_balance = new ISUnderSample(&samples);
		ssi_pcast(ISUnderSample, s_balance)->setUnder(ISUnderSample::RANDOM);
		ssi_msg(SSI_LOG_LEVEL_BASIC, "balance training set '%u' -> '%u'", samples.getSize(), s_balance->getSize());
		break;
	}
	}

	_n_samples = s_balance->getSize();

	if (_n_samples == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = s_balance->getClassSize();
	_n_features = s_balance->getStream(stream_index).dim;
	ssi_size_t elements = _n_samples * (_n_features + 1);

	init_class_names(*s_balance);

	_problem = new svm_problem;
	_problem->l = ssi_cast (int, _n_samples);
	_problem->y = new double[_problem->l];
	_problem->x = new svm_node *[_problem->l];	

	s_balance->reset();
	ssi_sample_t *sample;
	int n_sample = 0;
	float *ptr = 0;
	svm_node *node = 0;
	while (sample = s_balance->next()) {
		ptr = ssi_pcast (float, sample->streams[stream_index]->ptr);		
		_problem->x[n_sample] = new svm_node[_n_features + 1];
		_problem->y[n_sample] = ssi_cast (float, sample->class_id);
		node = _problem->x[n_sample];
		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
			node->index = nfeat+1;
            node->value = *ptr;
            ptr++;
			++node;
		}
		node->index = -1;		
		++n_sample;
	}

	if(_options.params.gamma == 0 && _n_features > 0) {
		_options.params.gamma = 1.0 / _n_features;
	}
	
	if (_options.params.kernel_type == PRECOMPUTED) {
		int max_index = ssi_cast (int, _n_features);
		for (int i = 0; i < _problem->l; i++) {
			if (_problem->x[i][0].index != 0) {
				ssi_err ("wrong input format: first column must be 0:sample_serial_number");				
			}
			if ((int)_problem->x[i][0].value <= 0 || (int)_problem->x[i][0].value > max_index) {
				ssi_err ("wrong input format: sample_serial_number out of range");
			}
		}
	}
			
	_max = new double[_n_features];
	_min = new double[_n_features];
	create_scaling (*_problem, _n_features, _max, _min);
	for (int i = 0; i < _problem->l; i++) {
		scale_instance (&(_problem->x[i]), _n_features, _max, _min);
	}

	_model = svm_train (_problem, &_options.params);

	switch (_options.balance) {
	case BALANCE::OVER: {
		delete s_balance;
		break;
	}
	case BALANCE::UNDER: {
		delete s_balance;
		break;
	}
	}

	return true;
}

bool SVM::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!isTrained ()) {
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

	svm_node *x = new svm_node[_n_features+1];
	float *ptr = ssi_pcast (float, stream.ptr);
    for (ssi_size_t i = 0; i < _n_features; i++) {
		x[i].index = i + 1;
        x[i].value = *ptr;
        ptr++;
    }
    x[_n_features].index=-1;

	scale_instance (&x, _n_features, _max, _min);

	double *prob_estimates = new double[_n_classes];
	svm_predict_probability (_model, x, prob_estimates);

	ssi_real_t sum = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		probs[_model->label[i]] = ssi_cast (ssi_real_t, prob_estimates[i]);
		sum += probs[_model->label[i]];
	}

	for (ssi_size_t j = 0; j < _n_classes; j++) {
		probs[j]/=sum;
	}

	delete[] x;
	delete[] prob_estimates;

	return true;
}

bool SVM::load (const ssi_char_t *filepath) {

	release ();
  
	char line[SSI_MAX_CHAR], *token;
  
    FILE *fp = ssi_fopen (filepath,"r");
	if (!fp) {
		ssi_wrn ("can't open file %s!", filepath);      
		return false;
	}

    do {
		readLine (fp, SSI_MAX_CHAR, line);
	} while (line[0] == '#');

    token = (char *) strtok(line,"\t");
	if (token) {
		_n_classes = atoi(token);
	} else {
		ssi_wrn ("can't read number of classes from svm classifier file '%s'!", filepath);
		return false;
	}
    token = (char *) strtok(NULL,"\t");
	if (token) {
		_n_features = atoi(token); 
	} else {
		ssi_wrn ("can't read feature dimension from svm classifier file '%s'!", filepath);
		return false;
	}

	do {
		readLine (fp, SSI_MAX_CHAR, line);
	} while (line[0] == '#');
	
    token = strtok (line, " ");
    int i = 0;
	_class_names = new ssi_char_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_class_names[i] = ssi_strcpy (token);
		token = strtok (NULL," ");
	}

	_max = new double[_n_features];
	_min = new double[_n_features];
    fscanf (fp, "# Scaling: max\tmin\n");
	for (ssi_size_t i = 0; i < _n_features; i++) {
		if (fscanf (fp, "%lg %lg", &_max[i], &_min[i]) != 2) {
			ssi_wrn ("can't read scaling information for SVM classifier from file '%s'!", filepath);
			return false;
		}
	}

    fscanf(fp,"\n\n");


	if (_model = svm_load_model_h(fp)) {
		_n_classes = _model->nr_class;
		fclose(fp);
		return true;
	}
	else {
		fclose(fp);
		return false;
	}
}

bool SVM::save (const ssi_char_t *filepath) {

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}

	FILE *fp = ssi_fopen (filepath, "w");

	if (!fp) {
		ssi_wrn ("Cannot open file %s!",filepath);
		return false;
	};

    fprintf(fp, "# Classifier type:\tsvm\n# number of classes\tfeature space dimension\n%d\t%d\n# class names\n", _n_classes, _n_features);
    
	for (ssi_size_t i = 0; i < _n_classes-1; i++) {
		fprintf (fp, "%s ", _class_names[i]);
	}
    fprintf (fp, "%s\n", _class_names[_n_classes-1]);

    fprintf (fp, "# Scaling: max\tmin\n");
	for (ssi_size_t i=0; i < _n_features; i++) {
		fprintf (fp, "%g\t%g\n", _max[i], _min[i]);
	}
    fprintf(fp,"\n");

	svm_save_model_h (fp, _model);

	fclose(fp);

	return true;
}

bool SVM::readLine (FILE *fp, ssi_size_t num, ssi_char_t *string) {
	
	char *string_ptr = string;
	char c;
	c = getc (fp);
	while (c != '\n' && c != EOF) {
		if (c != '\r') {
			if (--num == 0) {
				*string_ptr = '\0';
				ssi_wrn ("input string to short");
				return false;
			}
            *string_ptr = c;
            string_ptr++;
		}
		c = getc (fp);
	}
	*string_ptr = '\0';

	if (ferror (fp)) {
		ssi_wrn ("readLine() failed");
		return false;
	}

	return true;
}

}
