// FindNN.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/02
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

// based on an implementation by Samy Bengio (bengio@idiap.ch)
// see Torch 3.1 (http://www.torch.ch/)

#include "FindNN.h"
#include "ModelTools.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

	bool FindNN::Find (ssi_sample_t *sample, ISamples &samples, ssi_size_t stream_index, ssi_size_t n_neighbors, ssi_size_t *indices, ssi_real_t *distances) {

		if (samples.getSize () == 0) {
			ssi_wrn ("empty sample list");
			return false;
		}

		if (sample->streams[stream_index]->type != SSI_FLOAT || samples.getStream (stream_index).type != SSI_FLOAT) {
			ssi_wrn ("invalid sample type");
			return false;
		}

		ssi_size_t n_samples;
		ssi_size_t n_features;
		ssi_real_t **matrix;
		ssi_size_t *classes;

		ModelTools::CreateSampleMatrix (samples, stream_index, n_samples, n_features, &classes, &matrix);
		ssi_real_t *vector = ssi_pcast (ssi_real_t, sample->streams[stream_index]->ptr);
		Find (n_samples, n_features, vector, matrix, n_neighbors, indices, distances);
	
		ModelTools::ReleaseSampleMatrix (n_samples, classes, matrix);

		return true;
	}

	bool FindNN::Find (ssi_size_t n_samples, ssi_size_t n_features, ssi_real_t *sample, ssi_real_t **samples, ssi_size_t K, ssi_size_t *indices, ssi_real_t *distances) {

		// verify that n_examples > K		
		if (n_samples < K) {
			ssi_wrn ("#neighbors (%u) exceeds #samples (%u)", K, n_samples);
			return false;
		}

		// initialization of distances to big values;
		for (ssi_size_t i = 0; i < K; i++) {
			distances[i] = FLT_MAX;
			indices[i] = -1;
		}

		// compute the K nearest neighbords

		// for each vector in data		
		ssi_real_t *current = 0; 
		for (ssi_size_t i = 0; i < n_samples; i++) {
		
			current = samples[i];

			// calculate euclidean distance between sample and current vector
			ssi_real_t dist = Distance (current, sample, n_features);

			// eventually add current vector to K nearest neighbors
			if (dist < distances[K-1]) {

				// find insertion point
				ssi_real_t* bptr = distances;
				ssi_real_t* eptr = distances + K - 1;
				ssi_real_t* mptr = bptr + (eptr - bptr) / 2;
				do {
					if (dist < *mptr)
						eptr = mptr;
					else
						bptr = mptr + 1;
					mptr = bptr + (eptr - bptr) / 2;
				} while (mptr < eptr);
				// insert the point by shifting all subsequent distances
				eptr = distances + K - 1;
				bptr = eptr - 1;

				ssi_size_t* eptr_idx = indices + K - 1;
				ssi_size_t* bptr_idx = eptr_idx - 1;

				while (eptr > mptr) {
					*eptr-- = *bptr--;      /*   distances   */
					*eptr_idx-- = *bptr_idx--;      /*   indices   */
				}
				*mptr = dist;
				indices[mptr - distances] = i;
			}
		}

		return true;
	}
}
