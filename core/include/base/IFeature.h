// IFeature.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/26
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

#ifndef SSI_IFEATURE_H
#define SSI_IFEATURE_H

#include "base/ITransformer.h"

/**
 * \brief Interface of a transformer that serves as a feature.
 *
 * A feature works just like a transformer (see ITransformer), but
 * always reduces the sample number of the input stream to 1.
 * 
 * @author Johannes Wagner
 * @date  Feb 2009
 */

namespace ssi {

class IFeature : public ITransformer {

public:

	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in) {
		return 1;
	}

	ssi_object_t getType () { return SSI_FEATURE; };

};

}

#endif
