// Cast.h
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

#ifndef SSI_FRAME_CAST_H
#define SSI_FRAME_CAST_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Cast : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options () : cast (SSI_FLOAT) {			
			
			addOption ("type", &cast, 1, SSI_SIZE, "cast to type (CHAR = 1, UCHAR = 2, SHORT = 3, USHORT = 4, INT = 5, UINT = 6, LONG = 7, ULONG = 8, FLOAT = 9, DOUBLE = 10, LDOUBLE = 11, STRUCT = 12, IMAGE = 13, BOOL = 14");
		};

		ssi_type_t cast;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Cast"; };
	static IObject *Create (const ssi_char_t *file) { return new Cast (file); };
	virtual ~Cast ();
	
	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Cast filter, will be applied to all sample values in the stream."; };
	
	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) { 
		return sample_dimension_in; 
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return ssi_type2bytes (_options.cast); 
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) { 
		return _options.cast;
	}

	void transform (ITransformer::info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

protected:

	Cast (const ssi_char_t *file = 0);
	Cast::Options _options;
	ssi_char_t *_file;
};

}

#endif
