// RandomFusion.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/06/30
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

#include "RandomFusion.h"
#include "ssiml/include/ISMissingData.h"
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

RandomFusion::RandomFusion ()
	: _is_trained (false) {

}

RandomFusion::~RandomFusion () {

	release ();
}

bool RandomFusion::train (ssi_size_t n_models,
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

	if (samples.hasMissingData ()) {
		ISMissingData samples_h (&samples);
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			if (!models[n_model]->isTrained ()) {
				samples_h.setStream (n_model);
				models[n_model]->train (samples_h, n_model);
			}
		}
	} else {
		for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
			if (!models[n_model]->isTrained ()) {		
				models[n_model]->train (samples, n_model);
			}
		}
	}

	_is_trained = true;

	return true;
}

bool RandomFusion::forward (ssi_size_t n_models,
	IModel **models,
	ssi_size_t n_streams,
	ssi_stream_t **streams,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence) {

	bool found_data = false;

	ssi_real_t **tmp_probs = new ssi_real_t *[n_models];
	ssi_real_t tmp_confidence = 0.0f;
	
	ssi_stream_t *stream = 0;
	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		stream = streams[n_model];
		if (stream->num > 0) {
			tmp_probs[n_model] = new ssi_real_t[n_probs];
			models[n_model]->forward (*stream, n_probs, tmp_probs[n_model], tmp_confidence);
			confidence += tmp_confidence;
			found_data = true;
		} else {
			tmp_probs[n_model] = 0;
		}
	}
	confidence /= n_models;

	if (found_data) {

		Randomf random(0, 1);

		while (1) {
			ssi_real_t rand = random.next();
			ssi_size_t selmodel = ssi_cast (ssi_size_t, rand * (n_models - 1) + 0.5);		
			if (tmp_probs[selmodel]) {

				//ssi_print ("selected %u\n", selmodel);

				for (ssi_size_t n_class = 0; n_class < n_probs; n_class++) {			
					probs[n_class] = tmp_probs[selmodel][n_class];								
				}
				break;
			}
		}
	}

	for (ssi_size_t n_model = 0; n_model < n_models; n_model++) {
		delete[] tmp_probs[n_model];
	}
	delete[] tmp_probs;

	return found_data;
}

void RandomFusion::release () {
	_is_trained = false;
}

bool RandomFusion::save (const ssi_char_t *filepath) {
	return true;
}

bool RandomFusion::load (const ssi_char_t *filepath) {
	release ();
	return true;
}

}
