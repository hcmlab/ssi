// SVM.h
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

#pragma once

#ifndef SSI_MODEL_SVM_H
#define SSI_MODEL_SVM_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"
#include "libsvm-3.20.h"

#define SSI_SVM_SCALE_UPPER 1
#define SSI_SVM_SCALE_LOWER -1

namespace ssi {

class SVM : public IModel {

public:

	struct BALANCE {
		enum List {
			OFF,
			UNDER,
			OVER
		};
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: balance(BALANCE::OFF), seed (0) {

			params.svm_type = C_SVC;
			params.kernel_type = LINEAR;
			params.degree = 1;
			params.gamma = 0.01;	// 1/k
			params.coef0 = 0;
			params.nu = 0.5;
			params.cache_size = 1048576;
			params.C = 1;
			params.eps = 1e-1; //1e-12
			params.p = 0.1;
			params.shrinking = 1;
			params.probability = 1;
			params.nr_weight = 0;
			params.weight_label = 0;
			params.weight = 0;
			params.multicore = false;

			addOption ("svm", &params.svm_type, 1, SSI_INT, "SVM type ( C-SVC=0, nu-SVC=1)"); // , one-class SVM=2, epsilon-SVR=3, nu-SVR=4)");
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

			addOption("multicore", &params.multicore, 1, SSI_BOOL, "use all available CPU cores");
			
		};

		unsigned int seed;
		BALANCE::List balance;
		svm_parameter params;
	};

public:

	static const ssi_char_t *GetCreateName () { return "SVM"; };
	static IObject *Create (const ssi_char_t *file) { return new SVM (file); };
	virtual ~SVM ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Support vector machine classifier"; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _model != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

protected:	

	SVM (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

	ssi_size_t _n_classes;
	ssi_size_t _n_features;
	ssi_size_t _n_samples;
	ssi_char_t **_class_names;

	double *_min, *_max;
	svm_model *_model;
	svm_problem *_problem;

	void init_class_names (ISamples &samples);
	void free_class_names ();
	bool readLine (FILE *fp, ssi_size_t num, ssi_char_t *string);
		
	static void create_scaling (struct svm_problem problem, int n_features, double *_max, double *_min) {

		int i,j, idx;
		double temp;

		for (i=0;i<n_features;i++){
			_max[i]=-DBL_MAX;
			_min[i]=DBL_MAX;
		}

		for (i=0;i<problem.l;i++) {
		idx=0;
		for (j=0;j<n_features;j++) {
			if (problem.x[i][idx].index != j+1)
				temp=0;
			else {
				temp=problem.x[i][idx].value;
				idx++;
			}

			if (temp < _min[j])
				_min[j] = temp;
			if (temp > _max[j])
				_max[j] = temp;
		}
		}
	}

	static void scale_instance (struct svm_node *instance[], int n_features, double *max, double *min) {

		int j=0, idx=0, n_idx=0;
		double temp;
		svm_node *usInstance = new svm_node[n_features+1];

		while ((* instance)[j].index != -1) {
			usInstance[j].index=(* instance)[j].index;
			usInstance[j].value=(* instance)[j].value;
			j++;
		}
		usInstance[j].index=-1;

		for (j=0;j<n_features;j++) {
			if (usInstance[idx].index != j+1)
				temp=0;
			else 
				temp=usInstance[idx++].value;
			if (max[j]-min[j])
				temp=SSI_SVM_SCALE_LOWER+(SSI_SVM_SCALE_UPPER-SSI_SVM_SCALE_LOWER)*(temp-min[j])/(max[j]-min[j]);
			else
				temp=SSI_SVM_SCALE_LOWER+(SSI_SVM_SCALE_UPPER-SSI_SVM_SCALE_LOWER)*(temp-min[j])/FLT_MIN;
			if (temp) {
				(* instance)[n_idx].index=j+1;
				(* instance)[n_idx++].value=temp;
			}
		}
		(* instance)[n_idx].index=-1;
		
		delete[] usInstance;
	}
};

}

#endif
