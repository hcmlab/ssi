// MyFusion.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/02/26
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

#include "MyFusion.h"
#include "ioput/file/File.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

MyFusion::MyFusion ()
	: _is_trained (false) {
}

MyFusion::~MyFusion () {
}

bool MyFusion::train (ssi_size_t n_models,
	IModel **models,
	ISamples &samples) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	ssi_size_t n_streams = samples.getStreamSize ();

	if (n_streams != n_models) {
		ssi_err ("#models (%u) differs from #streams (%u)", n_models, n_streams);
	}

	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		if (!models[n_model]->isTrained ()) {
			models[n_model]->train (samples, n_model);
		}
	}

	_is_trained = true;

	return true;
}

bool MyFusion::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t *streams[],
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	ssi_real_t *tmp_probs = new ssi_real_t[n_probs];
	models[0]->forward (*streams[0], n_probs, probs);
	
	for (ssi_size_t n_model = 1; n_model < n_models; n_model++) {
		models[n_model]->forward (*streams[n_model], n_probs, tmp_probs);
		for (ssi_size_t n_prob = 0; n_prob < n_probs; n_prob++) {
			if (probs[n_prob] < tmp_probs[n_prob]) {
				probs[n_prob] = tmp_probs[n_prob];
			}
		}
	}

	delete[] tmp_probs;

	return true;
}

void MyFusion::release () {
	_is_trained = false;
}

bool MyFusion::save (const ssi_char_t *filepath) {
	return true;
}

bool MyFusion::load (const ssi_char_t *filepath) {
	release ();
	return true;
}

}
