// LibSVM.h
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

#pragma once

#ifndef SSI_LIBSVM_LIBSVM_H
#define SSI_LIBSVM_LIBSVM_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

#define SSI_SVM_SCALE_UPPER 1
#define SSI_SVM_SCALE_LOWER -1

struct svm_model;
struct svm_problem;

namespace ssi {

class LibSVM : public IModel {

public:

	struct BALANCE {
		enum List {
			OFF,
			UNDER,
			OVER,
		};
	};

	struct TYPE {
		enum List {
			C_SVC = 0,       // multi-class
			NU_SVC = 1,      // multi-class
			EPSILON_SVR = 3, // regression
			NU_SVR = 4,      // regression
		};
	};

	struct KERNEL {
		enum List {
			LINEAR,
			POLYNOMIAL,
			RADIAL,
			SIGMOID,
		};
	};

	struct parameter
	{
		int svm_type;
		int kernel_type;
		int degree;	/* for poly */
		double gamma;	/* for poly/rbf/sigmoid */
		double coef0;	/* for poly/sigmoid */

		/* these are for training only */
		double cache_size; /* in MB */
		double eps;	/* stopping criteria */
		double C;	/* for C_SVC, EPSILON_SVR and NU_SVR */
		int nr_weight;		/* for C_SVC */
		int *weight_label;	/* for C_SVC */
		double* weight;		/* for C_SVC */
		double nu;	/* for NU_SVC, ONE_CLASS, and NU_SVR */
		double p;	/* for EPSILON_SVR */
		int shrinking;	/* use the shrinking heuristics */
		int probability; /* do probability estimates */
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: balance(BALANCE::OFF), seed(0), silent(false) {

			params.svm_type = TYPE::C_SVC;
			params.kernel_type = KERNEL::RADIAL;
			params.degree = 3;
			params.gamma = 0;
			params.coef0 = 0;
			params.nu = 0.5;
			params.cache_size = 100;
			params.C = 1;
			params.eps = 1e-3;
			params.p = 0.1;
			params.shrinking = 1;
			params.probability = 1; // do not change, we need class probabilities!
			params.nr_weight = 0;
			params.weight_label = NULL;
			params.weight = NULL;

			params_str[0] = '\0';
			addOption("params", params_str, SSI_MAX_CHAR, SSI_CHAR, "libsvm parameters (see documentation, overwrites options)");

			addOption ("svm", &params.svm_type, 1, SSI_INT, "SVM type (multi-class: 0=C-SVC 1=nu-SVC, regression: 3=epsilon-SVR 4=nu-SVR)"); // , one-class SVM=2)");
			addOption ("kernel", &params.kernel_type, 1, SSI_INT, "Kernel type ( 0=linear: u'*v, 1=polynomial: (gamma*u'*v + coef0)^degree), 2=radial basis function: exp(-gamma*|u-v|^2), 3=sigmoid: tanh(gamma*u'*v + coef0)");
			addOption ("degree", &params.degree, 1, SSI_INT, "degree");
			addOption ("gamma", &params.gamma, 1, SSI_DOUBLE, "gamma");
			addOption ("coef0", &params.coef0, 1, SSI_DOUBLE, "coef0");
			addOption ("nu", &params.nu, 1, SSI_DOUBLE, "nu in nu-SVC");
			addOption ("C", &params.C, 1, SSI_DOUBLE, "cost in C-SVC");
			addOption ("eps", &params.eps, 1, SSI_DOUBLE, "set tolerance of termination criterion");			
			addOption ("p", &params.p, 1, SSI_DOUBLE, "epsilon in loss function of epsilon-SVR");
			addOption ("shrink", &params.shrinking, 1, SSI_INT, "whether to use the shrinking heuristics (0=false,1=true)");	
						
			addOption("srand", &seed, 1, SSI_UINT, "if >0 use fixed seed to initialize random number generator, otherwise timestamp will be used");
			addOption("balance", &balance, 1, SSI_INT, "balance #samples per class (0=off, 1=remove surplus, 2=create missing)");	
			addOption("silent", &silent, 1, SSI_BOOL, "suppress libsvm messages");
		};

		void setParams(const ssi_char_t *params_str) {
			ssi_strcpy(this->params_str, params_str);
		}

		ssi_char_t params_str[SSI_MAX_CHAR];
		unsigned int seed;
		BALANCE::List balance;
		parameter params;
		bool silent;
	};

public:

	static const ssi_char_t *GetCreateName () { return "LibSVM"; };
	static IObject *Create(const ssi_char_t *file) { return new LibSVM(file); };
	virtual ~LibSVM();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Support vector machine classifier"; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _model != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

	void setLogLevel(int level)
	{
		ssi_log_level = level;
	}

protected:	

	LibSVM (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ssi_size_t _n_classes;
	ssi_size_t _n_features;
	ssi_size_t _n_samples;

	svm_model *_model;
	svm_problem *_problem;

	static void silent(const char *s) {}

	void exit_with_help();
	bool parseParams(void *params, const ssi_char_t *string);
};

}

#endif
