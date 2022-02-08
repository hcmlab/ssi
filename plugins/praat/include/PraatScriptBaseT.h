// PraatScriptBaseT.h
// author: Andreas Seiderer
// created: 2013/09/16
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

#ifndef SSI_TRANSFORMER_PRAATSCRIPT_H
#define SSI_TRANSFORMER_PRAATSCRIPT_H

#include "base/IFilter.h"
#include "PraatScriptOptions.h"
#include "PraatScriptIParser.h"

namespace ssi {

class PraatScriptBaseT : public IFilter {

public:

	~PraatScriptBaseT ();
	PraatScriptOptions *getOptions () { return &_options; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		return _options.dimensions;
	}
	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in) {
		return _options.samples;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sizeof (ssi_real_t);
	};
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL && sample_type_in != SSI_SHORT) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_REAL;
	}

	// abstract methods
	virtual PraatScriptIParser *getParser () = 0;

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	PraatScriptBaseT (const ssi_char_t *file = 0);
	PraatScriptOptions _options;
	ssi_char_t *_file;
	static char *ssi_log_name;
	int ssi_log_level;

	PraatScriptIParser *_parser;
	bool _ready;

};

}

#endif
