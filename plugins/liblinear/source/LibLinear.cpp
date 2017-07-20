// LibLinear.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 27/9/2016
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
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

#include "LibLinear.h"
#include "ISUnderSample.h"
#include "ISOverSample.h"
#include "linear.h"
#include "base/Random.h"

namespace ssi {

	char LibLinear::ssi_log_name[] = "liblinear_";

	LibLinear::LibLinear(const ssi_char_t *file)
		: _file(0),
		ssi_log_level(SSI_LOG_LEVEL_DEFAULT) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		_model = 0;
	}

	LibLinear::~LibLinear() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}	
	}

	void LibLinear::release() {

		if (_model) {
			model *model_ = (model *)_model;
			free_and_destroy_model(&model_);
			_model = 0;
		}		
		
		_n_classes = 0;
		_n_features = 0;
		_n_samples = 0;
	}

	void LibLinear::exit_with_help()
	{
		ssi_print(			
			"options:\n"
			"-s type : set type of solver (default 1)\n"
			"  for multi-class classification\n"
			"	 0 -- L2-regularized logistic regression (primal)\n"
			"	 1 -- L2-regularized L2-loss support vector classification (dual)\n"
			"	 2 -- L2-regularized L2-loss support vector classification (primal)\n"
			"	 3 -- L2-regularized L1-loss support vector classification (dual)\n"
			"	 4 -- support vector classification by Crammer and Singer\n"
			"	 5 -- L1-regularized L2-loss support vector classification\n"
			"	 6 -- L1-regularized logistic regression\n"
			"	 7 -- L2-regularized logistic regression (dual)\n"
			"  for regression\n"
			"	11 -- L2-regularized L2-loss support vector regression (primal)\n"
			"	12 -- L2-regularized L2-loss support vector regression (dual)\n"
			"	13 -- L2-regularized L1-loss support vector regression (dual)\n"
			"-c cost : set the parameter C (default 1)\n"
			"-p epsilon : set the epsilon in loss function of SVR (default 0.1)\n"
			"-e epsilon : set tolerance of termination criterion\n"
			"	-s 0 and 2\n"
			"		|f'(w)|_2 <= eps*min(pos,neg)/l*|f'(w0)|_2,\n"
			"		where f is the primal function and pos/neg are # of\n"
			"		positive/negative data (default 0.01)\n"
			"	-s 11\n"
			"		|f'(w)|_2 <= eps*|f'(w0)|_2 (default 0.001)\n"
			"	-s 1, 3, 4, and 7\n"
			"		Dual maximal violation <= eps; similar to libsvm (default 0.1)\n"
			"	-s 5 and 6\n"
			"		|f'(w)|_1 <= eps*min(pos,neg)/l*|f'(w0)|_1,\n"
			"		where f is the primal function (default 0.01)\n"
			"	-s 12 and 13\n"
			"		|f'(alpha)|_1 <= eps |f'(alpha0)|,\n"
			"		where f is the dual function (default 0.1)\n"
			"-B bias : if bias >= 0, instance x becomes [x; bias]; if < 0, no bias term added (default -1)\n"
			"-wi weight: weights adjust the parameter C of different classes (see README for details)\n"
			"-v n: n-fold cross validation mode\n"
			"-C : find parameter C (only for -s 0 and 2)\n"
			"-q : quiet mode (no outputs)\n"
		);					
	}

	bool LibLinear::parseParams(void *params, const ssi_char_t *string)
	{
		int i;
		void(*print_func)(const char*) = _options.silent ? silent : NULL;	// default printing to stdout
		
		parameter &param = *(parameter*)params;
												// default values
		param.solver_type = L2R_L2LOSS_SVC_DUAL;
		param.C = 1;
		param.eps = HUGE_VAL; // see setting below
		param.p = 0.1;
		param.nr_weight = 0;
		param.weight_label = NULL;
		param.weight = NULL;
		param.init_sol = NULL;		
		flag_C_specified = 0;
		flag_solver_specified = 0;
		flag_find_C = 0;
		bias = -1;
		nr_fold = 5;

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
				param.solver_type = atoi(argv[i]);
				flag_solver_specified = 1;
				break;

			case 'c':
				param.C = atof(argv[i]);
				flag_C_specified = 1;
				break;

			case 'p':
				param.p = atof(argv[i]);
				break;

			case 'e':
				param.eps = atof(argv[i]);
				break;

			case 'B':
				bias = atof(argv[i]);
				break;

			case 'w':
				++param.nr_weight;
				param.weight_label = (int *)realloc(param.weight_label, sizeof(int)*param.nr_weight);
				param.weight = (double *)realloc(param.weight, sizeof(double)*param.nr_weight);
				param.weight_label[param.nr_weight - 1] = atoi(&argv[i - 1][2]);
				param.weight[param.nr_weight - 1] = atof(argv[i]);
				break;

			case 'v':
				ssi_wrn("option 'v' is ignored");
			//	flag_cross_validation = 1;
			//	nr_fold = atoi(argv[i]);
			//	if (nr_fold < 2)
			//	{
			//		fprintf(stderr, "n-fold cross validation: n must >= 2\n");
			//		exit_with_help();
			//	}
				break;

			case 'q':
				ssi_wrn("option 'q' is ignored");
			//	print_func = &print_null;
				i--;
				break;

			case 'C':
				flag_find_C = 1;
				i--;
				break;

			default:
				ssi_wrn("unknown option: -%c", argv[i - 1][1]);
				exit_with_help();
				break;
			}
		}

		set_print_string_function(print_func);	

		// default solver for parameter selection is L2R_L2LOSS_SVC
		if (flag_find_C)
		{		
			if (!flag_solver_specified)
			{
				ssi_wrn("Solver not specified. Using -s 2");
				param.solver_type = L2R_L2LOSS_SVC;
			}
			else if (param.solver_type != L2R_LR && param.solver_type != L2R_L2LOSS_SVC)
			{
				ssi_wrn("Warm-start parameter search only available for -s 0 and -s 2");
				exit_with_help();
			}
		}

		if (param.eps == HUGE_VAL)
		{
			switch (param.solver_type)
			{
			case L2R_LR:
			case L2R_L2LOSS_SVC:
				param.eps = 0.01;
				break;
			case L2R_L2LOSS_SVR:
				param.eps = 0.001;
				break;
			case L2R_L2LOSS_SVC_DUAL:
			case L2R_L1LOSS_SVC_DUAL:
			case MCSVM_CS:
			case L2R_LR_DUAL:
				param.eps = 0.1;
				break;
			case L1R_L2LOSS_SVC:
			case L1R_LR:
				param.eps = 0.01;
				break;
			case L2R_L1LOSS_SVR_DUAL:
			case L2R_L2LOSS_SVR_DUAL:
				param.eps = 0.1;
				break;
			}
		}

		return true;
	}

	bool LibLinear::train(ISamples &samples,
		ssi_size_t stream_index) {

		if (isTrained()) {
			ssi_wrn("already trained");
			return false;
		}

		ssi_size_t seed = 0;
		if (_options.seed > 0) {
			seed = _options.seed;
		}
		else {
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
			ssi_wrn("empty sample list");
			return false;
		}

		_n_classes = s_balance->getClassSize();
		_n_features = s_balance->getStream(stream_index).dim;
		ssi_size_t elements = _n_samples * (_n_features + 1);

		// parse paramters

		parameter param;
		if (!parseParams(&param, _options.params))
		{
			ssi_wrn("could not parse parameters");
			return false;
		}		

		// prepare problem

		problem prob;
		prob.l = ssi_cast(int, _n_samples);
		prob.y = new double[prob.l];
		prob.x = new feature_node *[prob.l];
		prob.bias = bias;
		
		int max_index = _n_features + 1;

		s_balance->reset();
		ssi_sample_t *sample;
		int n_sample = 0;
		float *ptr = 0;
		feature_node *node = 0;
		while (sample = s_balance->next()) {
			ptr = ssi_pcast(float, sample->streams[stream_index]->ptr);
			prob.x[n_sample] = new feature_node[_n_features + 1 + 1]; // one extra for bias term
			prob.y[n_sample] = ssi_cast(float, _n_classes == 1 ? sample->score : sample->class_id);
			node = prob.x[n_sample];
			for (ssi_size_t nfeat = 0; nfeat < _n_features; nfeat++) {
				node->index = nfeat + 1;
				node->value = *ptr++;
				++node;
			}
			if (prob.bias >= 0)
			{				
				node->index = _n_features + 1;
				node->value = bias;
				++node;
			}
			node->index = -1;
			++n_sample;
		}
		
		prob.n = prob.bias >= 0 ? max_index + 1 : max_index;		

		// train

		const char *error_msg = check_parameter(&prob, &param);
		if (error_msg)
		{
			ssi_wrn("%s", error_msg);
			return false;
		}

		if (flag_find_C)
		{
			double start_C, best_C, best_rate;
			double max_C = 1024;
			if (flag_C_specified)
				start_C = param.C;
			else
				start_C = -1.0;
			ssi_msg(SSI_LOG_LEVEL_BASIC, "doing parameter search with %d-fold cross validation", nr_fold);
			find_parameter_C(&prob, &param, nr_fold, start_C, max_C, &best_C, &best_rate);
			ssi_msg(SSI_LOG_LEVEL_BASIC, "best C = %g  CV accuracy = %g%%\n", best_C, 100.0*best_rate);
			param.C = best_C;
		}

		model *result = train_model(&prob, &param);
		_model = result;

		// clean up

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

		for (int nsamp = 0; nsamp < prob.l; nsamp++) {
			delete[] prob.x[nsamp];
		}
		delete[] prob.y;
		delete[] prob.x;

		destroy_param(&param);

		return true;
	}

	bool LibLinear::forward(ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs) {

		if (!isTrained()) {
			ssi_wrn("not trained");
			return false;
		}

		if (n_probs != _n_classes) {
			ssi_wrn("#classes differs");
			return false;
		}

		if (stream.type != SSI_REAL) {
			ssi_wrn("type differs");
			return false;
		}

		_n_features = stream.dim;

		feature_node *x = new feature_node[_n_features + 1 + 1]; // one extra for bias term
		float *ptr = ssi_pcast(float, stream.ptr);
		ssi_size_t index = 0;
		for (; index < _n_features; index++) {
			x[index].index = index + 1;
			x[index].value = *ptr++;
		}
		if (bias >= 0)
		{
			x[index].index = _n_features + 1;
			x[index].value = bias;
			index++;
		}
		x[index].index = -1;

		if (n_probs >= 2) // MULTICLASS
		{									
			double *prob_estimates = new double[n_probs];
			if (check_probability_model((model*)_model))
			{
				predict_probability((model*)_model, x, prob_estimates);
				for (ssi_size_t j = 0; j < _n_classes; j++) {
					probs[j] = (ssi_real_t)prob_estimates[j];
				}
			}
			else
			{
				if (n_probs == 2)
				{
					double label = predict((model*)_model, x);
					probs[0] = label == 0 ? 1.0f : 0.0f;
					probs[1] = label == 1 ? 1.0f : 0.0f;
				}
				else
				{
					predict_values((model*)_model, x, prob_estimates);
					for (ssi_size_t j = 0; j < _n_classes; j++) {
						probs[j] = (ssi_real_t)prob_estimates[j];
					}
					ssi_real_t minval, maxval;
					ssi_minmax(n_probs, 1, probs, &minval, &maxval);
					ssi_real_t sum = 0;
					for (ssi_size_t j = 0; j < _n_classes; j++) {
						probs[j] -= minval;
						sum += probs[j];
					}
					for (ssi_size_t j = 0; j < _n_classes; j++) {
						probs[j] /= sum;
					}
				}
			}
			
			delete[] prob_estimates;
		}
		else // REGRESSION 
		{			
			ssi_real_t score = (ssi_real_t) predict((model*)_model, x);
			probs[0] = score;
		}

		delete[] x;		

		return true;
	}

	bool LibLinear::load(const ssi_char_t *filepath) {

		release();

		model *result = load_model(filepath);
		if (result) {
			_n_classes = check_regression_model(result) ? 1 : result->nr_class;
			bias = result->bias;
		}
		_model = result;

		return _model != 0;
	}

	bool LibLinear::save(const ssi_char_t *filepath) {

		if (!isTrained()) {
			ssi_wrn("not trained");
			return false;
		}
		
		model *m = (model *)_model;
		return save_model(filepath, m) == 0;
	}

}
