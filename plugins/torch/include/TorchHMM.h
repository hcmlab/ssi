// TorchHMM.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/11
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

#ifndef SSI_TORCH_TORCHHMM_H
#define SSI_TORCH_TORCHHMM_H

#include "base/IModel.h"
#include "ioput/option/OptionList.h"

namespace Torch {
	class HMM;
	class EMTrainer;
	class DataSet;
	class XFile;
}

namespace ssi {

class TorchHMM : public IModel {

public:

	enum HMM_CONNECTIVITY {
		LINEAR = 0,
		LEFTRIGHT,
		ERGODIC
	};

public:

	class Options : public OptionList {

	public:

		Options () {

			single			= false;
			connectivity    = LINEAR;
			n_states        = 3;
			n_gaussians     = 3;
			prior           = 0.001f;
			accuracy        = 0.00001f;
			max_iter_kmeans = 25;
			threshold       = 0.001f;
			max_iter_hmm    = 25;
			retrain			= 2;

			addOption ("single", &single, 1, SSI_BOOL, "use single ergodic hmm with #states = #classes (overrides -states and -connectivity option)");
			addOption ("connectivity", &connectivity, 1, SSI_INT, "connectivity (linear=0, left-right=1, ergodic=2)");
			addOption ("states", &n_states, 1, SSI_INT, "number of states");
			addOption ("gaussians", &n_gaussians, 1, SSI_INT, "number of gaussians");
			addOption ("prior", &prior, 1, SSI_REAL, "prior weights (only initialization)");
			addOption ("accuracy", &accuracy, 1, SSI_REAL, "accuracy");
			addOption ("iter_kmeans", &max_iter_kmeans, 1, SSI_INT, "max iterations during k-mean (only initalization)");
			addOption ("threshold", &threshold, 1, SSI_REAL, "accuracy");
			addOption ("iter_gmm", &max_iter_hmm, 1, SSI_INT, "max iterations during training (only retraining)");
			addOption ("retrain", &retrain, 1, SSI_INT, "number of retrains");			
		};

		HMM_CONNECTIVITY connectivity;
		int n_states;
		int n_gaussians;
		ssi_real_t prior;	
		ssi_real_t accuracy;
		int  max_iter_kmeans;
		ssi_real_t threshold;
		int  max_iter_hmm;
		int  retrain;
		bool single;
	};

public:

	static const ssi_char_t *GetCreateName () { return "TorchHMM"; };
	static IObject *Create (const ssi_char_t *file) { return new TorchHMM (file); };
	TorchHMM::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Hidden Markov Model"; };

	ssi_size_t getClassSize () { return _n_classes; };
	ssi_size_t getStreamDim () { return _n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

	IModel::TYPE::List getModelType() { return IModel::TYPE::CLASSIFICATION; }
	bool train (ISamples &samples,
		ssi_size_t stream_index);	
	bool isTrained () { return _hmms != 0; };
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_real_t &confidence);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	virtual const ssi_size_t *getStateSequence (ssi_size_t &n) {
		n = _n_state_sequence;
		return _state_sequence;
	}

protected:

	TorchHMM (const ssi_char_t *file = 0);
	virtual ~TorchHMM ();
	TorchHMM::Options _options;
	ssi_char_t *_file;

	void setTransitions (ssi_real_t** transitions, int n_states, HMM_CONNECTIVITY connectivity);
	Torch::HMM *initHMM (int n_states, int n_gaussians, int n_inputs, HMM_CONNECTIVITY connectivity, ssi_real_t prior, Torch::EMTrainer *kmeans_trainer);
	void retrain (Torch::HMM *hmm, Torch::DataSet &data, Torch::XFile &logfile);
	void printTransMat ();

	ssi_size_t _n_hmms;
	Torch::HMM **_hmms;
	ssi_size_t _n_classes;
	ssi_size_t _n_features;

	ssi_size_t _n_state_sequence;
	ssi_size_t *_state_sequence;
};

}

#endif
