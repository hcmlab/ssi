// Clone.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/04/09
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

/**

Provides second-order infinite impulse response (IIR) filtering.

*/

#pragma once

#ifndef SSI_FRAME_CLONE_H
#define SSI_FRAME_CLONE_H

#include "base/IFilter.h"

namespace ssi {

class Clone : public IFilter {

public:

	static const ssi_char_t *GetCreateName () { return "Clone"; };
	static IObject *Create (const ssi_char_t *file) { return new Clone (file); };
	~Clone ();

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Clones input stream."; };

	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		return sample_dimension_in;
	}

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		return sample_type_in;
	}

protected:

	Clone (const ssi_char_t *file);
		
};

}

#endif
