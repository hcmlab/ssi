// TorchKMeans.h
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

#ifndef SSI_TORCH_TORCHKMEANS_H
#define SSI_TORCH_TORCHKMEANS_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

namespace Torch {
	class KMeans;
}

namespace ssi {

class TorchKMeans : public IModel {

public:

	class Options : public OptionList {

	public:

		Options () {

			n_cluster      = 3;
			prior           = 0.001f;
			accuracy        = 0.00001f;
			max_iter_kmeans = 25;
			threshold       = 0.001f;

			addOption ("clusteres", &n_cluster, 1, SSI_INT, "number of clusters");
			addOption ("prior", &prior, 1, SSI_REAL, "prior weights (only initialization)");
			addOption ("accuracy", &accuracy, 1, SSI_REAL, "accuracy");
			addOption ("iter_kmeans", &max_iter_kmeans, 1, SSI_INT, "max iterations during k-mean (only initalization)");
			addOption ("threshold", &threshold, 1, SSI_REAL, "accuracy");
		};

		int n_cluster;
		ssi_real_t prior;	
		ssi_real_t accuracy;
		int  max_iter_kmeans;
		ssi_real_t threshold;
	};

public:

	static const ssi_char_t *GetCreateName () { return "TorchKMeans"; };
	static IObject *Create (const ssi_char_t *file) { return new TorchKMeans (file); };	
	virtual ~TorchKMeans ();
	TorchKMeans::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "K-Means"; };

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
	ssi_size_t getClassSize () { return _options.n_cluster; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _kmeans != 0; };
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

	float **getCenters () { return _centers; };

protected:

	TorchKMeans (const ssi_char_t *file = 0);
	TorchKMeans::Options _options;
	ssi_char_t *_file;
	static const ssi_char_t *_name;
	static const ssi_char_t *_info;

	Torch::KMeans *_kmeans;
	float **_centers;
	ssi_size_t _n_features;
};

}

#endif
