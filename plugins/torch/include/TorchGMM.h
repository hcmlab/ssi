// TorchGMM.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/27
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

#ifndef SSI_TORCH_TORCHGMM_H
#define SSI_TORCH_TORCHGMM_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

namespace Torch {
	class DiagonalGMM;
}

namespace ssi {

class TorchGMM : public IModel {

public:

	class Options : public OptionList {

	public:

		Options () {

			n_gaussians     = 3;
			prior           = 0.001f;
			accuracy        = 0.00001f;
			max_iter_kmeans = 25;
			threshold       = 0.001f;
			max_iter_gmm    = 25;
			retrain			= 2;

			addOption ("gaussians", &n_gaussians, 1, SSI_INT, "number of gaussians");
			addOption ("prior", &prior, 1, SSI_REAL, "prior weights (only initialization)");
			addOption ("accuracy", &accuracy, 1, SSI_REAL, "accuracy");
			addOption ("iter_kmeans", &max_iter_kmeans, 1, SSI_INT, "max iterations during k-mean (only initalization)");
			addOption ("threshold", &threshold, 1, SSI_REAL, "accuracy");
			addOption ("iter_gmm", &max_iter_gmm, 1, SSI_INT, "max iterations during training (only retraining)");
			addOption ("retrain", &retrain, 1, SSI_INT, "number of retrains");
		};

		int n_gaussians;
		ssi_real_t prior;	
		ssi_real_t accuracy;
		int  max_iter_kmeans;
		ssi_real_t threshold;
		int  max_iter_gmm;
		int  retrain;
	};

public:

	static const ssi_char_t *GetCreateName () { return "TorchGMM"; };
	static IObject *Create (const ssi_char_t *file) { return new TorchGMM (file); };	
	TorchGMM::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Gaussian Mixture Model"; };

	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _gmms != 0; };
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

	void TorchGMM::print (FILE *file = stdout);

protected:

	TorchGMM (const ssi_char_t *file = 0);
	virtual ~TorchGMM ();
	TorchGMM::Options _options;
	ssi_char_t *_file;

	Torch::DiagonalGMM **_gmms;
	ssi_size_t _n_classes;
	ssi_size_t _n_features;
};

}

#endif
