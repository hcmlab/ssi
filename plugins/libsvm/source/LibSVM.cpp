// LibSVM.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 22/5/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "LibSVM.h"
#include "ISUnderSample.h"
#include "ISOverSample.h"
#include "svm.h"
#include "base/Random.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *LibSVM::ssi_log_name = "libsvm____";

LibSVM::LibSVM (const ssi_char_t *file) 
	: _n_features (0),
	_n_classes (0),
	_file (0),
	_model (0),
	_problem (0),
	_n_samples (0),
	ssi_log_level(SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

LibSVM::~LibSVM () { 

	release ();
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void LibSVM::release () {

	if (_problem) {
		for (ssi_size_t nsamp = 0; nsamp < _n_samples; nsamp++) {
			delete[] _problem->x[nsamp];
		}
		delete[] _problem->y;
		delete[] _problem->x;
		delete _problem; _problem = 0;
	}

	if (_model) {
		svm_free_and_destroy_model (&_model);
		_model = 0;
	}

	_n_classes = 0;
	_n_features = 0;
	_n_samples = 0;
}

IModel::TYPE::List LibSVM::getModelType()
{
	if (_options.params.svm_type == C_SVC
		|| _options.params.svm_type == NU_SVC)
	{
		return IModel::TYPE::CLASSIFICATION;
	}
	else
	{
		return IModel::TYPE::REGRESSION;
	}
}

bool LibSVM::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (isTrained()) {
		ssi_wrn("already trained");
		return false;
	}

	ssi_size_t seed = 0;
	if (_options.seed > 0) {
		seed = _options.seed;
	} else {
		seed = Random::Seed();
	}

	ISamples *s_balance = 0;
	switch (_options.balance) {
	case BALANCE::OFF: {
		s_balance = &samples;
		break;
	}
	case BALANCE::OVER: {		
		s_balance = new ISOverSample(&samples);
		ssi_pcast(ISOverSample, s_balance)->setSeed(seed);
		ssi_pcast(ISOverSample, s_balance)->setOver(ISOverSample::RANDOM);
		ssi_msg(SSI_LOG_LEVEL_BASIC, "balance training set '%u' -> '%u'", samples.getSize(), s_balance->getSize());
		break;
	}
	case BALANCE::UNDER: {		
		s_balance = new ISUnderSample(&samples);
		ssi_pcast(ISUnderSample, s_balance)->setSeed(seed);
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

	_n_classes = s_balance->getClassSize();
	_n_features = s_balance->getStream(stream_index).dim;
	ssi_size_t elements = _n_samples * (_n_features + 1);

	// parse parameters

	if (_options.params_str[0] != '\0')
	{
		if (!parseParams(&_options.params, _options.params_str))
		{
			ssi_wrn("could not parse parameters");
			return false;
		}
	}

	IModel::TYPE::List task = getModelType();

	// prepare problem

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
		_problem->y[n_sample] = ssi_cast (float, task == IModel::TYPE::REGRESSION ? sample->score : sample->class_id);
		node = _problem->x[n_sample];
		for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
			node->index = nfeat+1;
			node->value = *ptr++;
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

	FILE *fp = 0;
	if (_options.silent) {
		svm_set_print_string_function(silent);
	}

	svm_parameter params;
	memcpy(&params, &_options.params, sizeof(parameter));

	const char *error_msg = svm_check_parameter(_problem, &params);
	if (error_msg)
	{
		ssi_wrn("%s", error_msg);
		return false;
	}

	_model = svm_train (_problem, &params);

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


bool LibSVM::forward(ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence, ssi_video_params_t &params)
{
	return forward(stream, n_probs, probs, confidence);
}

bool LibSVM::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence) {

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

	/*if (stream.dim != _n_features) {
		ssi_wrn ("feature dimension differs");
		return false;
	}*/
	_n_features = stream.dim;

	svm_node *x = new svm_node[_n_features+1];
	float *ptr = ssi_pcast (float, stream.ptr);
    for (ssi_size_t i = 0; i < _n_features; i++) {
		x[i].index = i + 1;
		x[i].value = *ptr++;
    }
    x[_n_features].index=-1;

	if (_n_classes > 1) // MULTI-CLASS
	{
		double *prob_estimates = new double[_n_classes];
		svm_predict_probability(_model, x, prob_estimates);

		ssi_real_t sum = 0;
		for (ssi_size_t i = 0; i < _n_classes; i++) {
			probs[_model->label[i]] = ssi_cast(ssi_real_t, prob_estimates[i]);
			sum += probs[_model->label[i]];
		}

		for (ssi_size_t j = 0; j < _n_classes; j++) {
			probs[j] /= sum;
		}

		ssi_max(n_probs, 1, probs, &confidence);
		delete[] prob_estimates;
	}
	else // REGRESSION
	{
		confidence = probs[0] = ssi_real_t(svm_predict(_model, x));
	}

	delete[] x;
	
	return true;
}

bool LibSVM::load (const ssi_char_t *filepath) {

	release ();

	if (_model = svm_load_model(filepath)) {
		if (_model->param.svm_type == TYPE::EPSILON_SVR || // REGRESSION
			_model->param.svm_type == TYPE::NU_SVR)
		{
			_n_classes = 1;
		}
		else // MULTI-CLASS
		{
			_n_classes = _model->nr_class;
		}
	}

	return _model != 0;
}

bool LibSVM::save (const ssi_char_t *filepath) {

	if (!isTrained ()) {
		ssi_wrn ("not trained");
		return false;
	}

	return svm_save_model(filepath, _model) == 0;
}

bool LibSVM::parseParams(void *params, const ssi_char_t *string)
{
	int i;
	void(*print_func)(const char*) = _options.silent ? silent : NULL;	// default printing to stdout

	parameter &param = *(parameter*)params;
											// default values
	//param.svm_type = C_SVC;
	//param.kernel_type = RBF;
	//param.degree = 3;
	//param.gamma = 0;	// 1/num_features
	//param.coef0 = 0;
	//param.nu = 0.5;
	//param.cache_size = 100;
	//param.C = 1;
	//param.eps = 1e-3;
	//param.p = 0.1;
	//param.shrinking = 1;
	//param.probability = 0;
	//param.nr_weight = 0;
	//param.weight_label = NULL;
	//param.weight = NULL;

	int argc = (int)ssi_split_string_count(string, ' ');
	if (argc == 0)
	{
		return true;
	}

	ssi_char_t **argv = new ssi_char_t *[argc];
	ssi_split_string(argc, argv, string, ' ');

	// parse options
	for (i = 0; i<argc; i++)
	{
		if (argv[i][0] != '-') break;
		if (++i > argc)
			exit_with_help();
		switch (argv[i - 1][1])
		{
		case 's':
			param.svm_type = atoi(argv[i]);
			break;
		case 't':
			param.kernel_type = atoi(argv[i]);
			break;
		case 'd':
			param.degree = atoi(argv[i]);
			break;
		case 'g':
			param.gamma = atof(argv[i]);
			break;
		case 'r':
			param.coef0 = atof(argv[i]);
			break;
		case 'n':
			param.nu = atof(argv[i]);
			break;
		case 'm':
			param.cache_size = atof(argv[i]);
			break;
		case 'c':
			param.C = atof(argv[i]);
			break;
		case 'e':
			param.eps = atof(argv[i]);
			break;
		case 'p':
			param.p = atof(argv[i]);
			break;
		case 'h':
			param.shrinking = atoi(argv[i]);
			break;
		case 'b':
			param.probability = atoi(argv[i]);
			break;
		case 'q':
			ssi_wrn("option 'q' is ignored");
			//print_func = &print_null;
			i--;
			break;
		case 'v':
			ssi_wrn("option 'v' is ignored");
			//cross_validation = 1;
			//nr_fold = atoi(argv[i]);
			//if (nr_fold < 2)
			//{
			//	fprintf(stderr, "n-fold cross validation: n must >= 2\n");
			//	exit_with_help();
			//}
			break;
		case 'w':
			++param.nr_weight;
			param.weight_label = (int *)realloc(param.weight_label, sizeof(int)*param.nr_weight);
			param.weight = (double *)realloc(param.weight, sizeof(double)*param.nr_weight);
			param.weight_label[param.nr_weight - 1] = atoi(&argv[i - 1][2]);
			param.weight[param.nr_weight - 1] = atof(argv[i]);
			break;
		default:
			fprintf(stderr, "Unknown option: -%c\n", argv[i - 1][1]);
			exit_with_help();
		}
	}

	svm_set_print_string_function(print_func);

	// determine filenames

	if (i > argc)
		exit_with_help();

	return true;
}

void LibSVM::exit_with_help()
{
	ssi_print(
		"Usage: svm-train [options] training_set_file [model_file]\n"
		"options:\n"
		"-s svm_type : set type of SVM (default 0)\n"
		"	0 -- C-SVC		(multi-class classification)\n"
		"	1 -- nu-SVC		(multi-class classification)\n"
		"	2 -- one-class SVM\n"
		"	3 -- epsilon-SVR	(regression)\n"
		"	4 -- nu-SVR		(regression)\n"
		"-t kernel_type : set type of kernel function (default 2)\n"
		"	0 -- linear: u'*v\n"
		"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
		"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
		"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
		"	4 -- precomputed kernel (kernel values in training_set_file)\n"
		"-d degree : set degree in kernel function (default 3)\n"
		"-g gamma : set gamma in kernel function (default 1/num_features)\n"
		"-r coef0 : set coef0 in kernel function (default 0)\n"
		"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
		"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
		"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
		"-m cachesize : set cache memory size in MB (default 100)\n"
		"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
		"-h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
		"-b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)\n"
		"-wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)\n"
		"-v n: n-fold cross validation mode\n"
		"-q : quiet mode (no outputs)\n"
	);
}

}
