// FindNN.h
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

#pragma once

#ifndef SSI_MODEL_FINDNN_H
#define SSI_MODEL_FINDNN_H

#include "base/ISamples.h"

namespace ssi {

class FindNN {

public:

	static bool Find (ssi_sample_t *sample, ISamples &samples, ssi_size_t stream_index, ssi_size_t n_neighbors, ssi_size_t *indices_of_neighbors, ssi_real_t *distances_of_neighbors);
	static bool Find (ssi_size_t n_samples, ssi_size_t n_features, ssi_real_t *sample, ssi_real_t **samples, ssi_size_t n_neighbors, ssi_size_t *indices_of_neighbors, ssi_real_t *distances_of_neighbors);

protected:

	static ssi_real_t Distance (ssi_real_t* v1, ssi_real_t* v2, ssi_size_t n) {
		ssi_real_t dist = 0.0f;
		ssi_real_t diff;
		for(ssi_size_t j = 0; j < n; j++) {
			diff = *v1++ - *v2++;
			dist += diff*diff;
		}
		return dist;
	}

};

}

#endif
