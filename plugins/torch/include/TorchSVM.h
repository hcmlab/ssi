// TorchSVM.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/19
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

#ifndef SSI_TORCH_TORCHSVM_H
#define SSI_TORCH_TORCHSVM_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

namespace Torch {
	class SVM;
	class Kernel;
}

namespace ssi {

class TorchSVM : public IModel {

public:

	class Options : public OptionList {

	public:

		Options ()
			: degree (-1), a_cst (1.0f), b_cst (1.0f), stdv (10.0f), iter_shrink (100), accuracy (0.01f), c_cst (100.0f), cache_size (50.0f) {

			addOption ("degree", &degree, 1, SSI_INT, "if positive, a polynomial kernel [(a xy + b)^d] with the specified degree is used, otherwise a gaussian kernel is used");
			addOption ("a", &a_cst, 1, SSI_FLOAT, "constant a in the polynomial kernel");
			addOption ("b", &b_cst, 1, SSI_FLOAT, "constant b in the polynomial kernel");
			addOption ("std", &stdv, 1, SSI_FLOAT, "the std parameter in the gaussian kernel [exp(-|x-y|^2/std^2)]");
			addOption ("iter", &iter_shrink, 1, SSI_FLOAT, "minimal number of iterations before shrinking");
			addOption ("acc", &accuracy, 1, SSI_FLOAT, "end accuracy");
			addOption ("c", &c_cst, 1, SSI_FLOAT, "trade off cst between error/margin");
			addOption ("cache", &cache_size, 1, SSI_FLOAT, "cache size in Mo");
		};

		int degree;
		float a_cst, b_cst;
		float stdv;
		int iter_shrink;
		float accuracy;
		float c_cst;
		float cache_size;
	};

public:

	static const ssi_char_t *GetCreateName () { return "TorchSVM"; };
	static IObject *Create (const ssi_char_t *file) { return new TorchSVM (file); };
	TorchSVM::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Support Vector Machine"; };

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _svms != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence);	
	bool forward(ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence, ssi_video_params_t &params);
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

protected:

	TorchSVM (const ssi_char_t *file = 0);
	virtual ~TorchSVM ();
	TorchSVM::Options _options;
	ssi_char_t *_file;

	Torch::Kernel *_kernel;
	Torch::SVM **_svms;
	ssi_size_t _n_classes;
	ssi_size_t _n_features;
};

}

#endif
