// EmoVoiceSVM.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/19
// Copyright (C) 2007-14 University of Augsburg, Lab for Human Centered Multimedia
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

#ifndef SSI_EV_EMOVOICESVM_H
#define SSI_EV_EMOVOICESVM_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

extern "C" {
#include "ev_class.h"
}

namespace ssi {

class EmoVoiceSVM : public IModel {

public:

	class Options : public OptionList {

	public:

		Options () {

			svm_param.svm_type = C_SVC;
			svm_param.kernel_type = LINEAR;
			svm_param.degree = 1;
			svm_param.gamma = 0.01;	// 1/k
			svm_param.coef0 = 0;
			svm_param.nu = 0.5;
			svm_param.cache_size = 1048576;
			svm_param.C = 1;
			svm_param.eps = 1e-1; //1e-12
			svm_param.p = 0.1;
			svm_param.shrinking = 1;
			svm_param.probability = 1;
			svm_param.nr_weight = 0;
			svm_param.weight_label = NULL;
			svm_param.weight = NULL;
//			svm_param.max_iter = 0;

			addOption ("svm", &svm_param.svm_type, 1, SSI_INT, "SVM type ( C-SVC=0, nu-SVC=1)"); // , one-class SVM=2, epsilon-SVR=3, nu-SVR=4)");
			addOption ("kernel", &svm_param.kernel_type, 1, SSI_INT, "Kernel type ( 0=linear: u'*v, 1=polynomial: (gamma*u'*v + coef0)^degree), 2=radial basis function: exp(-gamma*|u-v|^2), 3=sigmoid: tanh(gamma*u'*v + coef0)");
			addOption ("degree", &svm_param.degree, 1, SSI_INT, "degree");
			addOption ("gamma", &svm_param.gamma, 1, SSI_DOUBLE, "gamma");
			addOption ("coef0", &svm_param.coef0, 1, SSI_DOUBLE, "coef0");
			addOption ("nu", &svm_param.nu, 1, SSI_DOUBLE, "nu in nu-SVC");
			addOption ("C", &svm_param.C, 1, SSI_DOUBLE, "cost in C-SVC");
			addOption ("eps", &svm_param.eps, 1, SSI_DOUBLE, "set tolerance of termination criterion");			
			//addOption ("p", &svm_param.p, 1, SSI_DOUBLE, "epsilon in loss function of epsilon-SVR");
			addOption ("shrink", &svm_param.shrinking, 1, SSI_INT, "whether to use the shrinking heuristics (0=false,1=true)");	
	//		addOption ("iter", &svm_param.max_iter, 1, SSI_UINT, "max iteration during optimization step");
			
		};

		svm_parameter svm_param;
	};

public:

	static const ssi_char_t *GetCreateName () { return "ssi_model_EmoVoiceSVM"; };
	static IObject *Create (const ssi_char_t *file) { return new EmoVoiceSVM (file); };
	~EmoVoiceSVM ();
	EmoVoiceSVM::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Support Vector Machine Classifier"; };

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

protected:	

	EmoVoiceSVM (const ssi_char_t *file = 0);
	EmoVoiceSVM::Options _options;	
	ssi_char_t *_file;

	classifier_t *_model;
	cType _cl_type;
	ssi_char_t *_cl_param;

	ssi_size_t _n_classes;
	ssi_size_t _n_features;
};

}

#endif
