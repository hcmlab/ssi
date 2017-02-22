// IModel.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2010/03/01
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_ICONTINUOUSMODEL_H
#define SSI_ICONTINUOUSMODEL_H

#include "base/IModel.h"

namespace ssi {

class IContinuousModel : public IModel {

public:
	///returns the timestamp of the last output
	virtual ssi_time_t getLastOutputTime () { return 0; };

	/**
	 * returns model output with timestamp.
	 * @return true if there is any new output, false otherwise
	 * @param n_probs number of expected probabilities
	 * @param probs pointer to a float array where the probabilities will be written
	 * @param time pointer to a float where the timestamp of the output will be written
	 * @param dur pointer to a float where the duration of the classification result will be written
	 */
	virtual bool getOutput (ssi_size_t n_probs,
		ssi_real_t *probs,
		ssi_time_t *time,
		ssi_time_t *dur) = 0;	

	ssi_object_t getType () { return SSI_MODEL_CONTINUOUS; };

};

}

#endif
