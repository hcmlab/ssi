// EMGRectify.h
// author: Daniel Schork
// created: 2016
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

#ifndef SSI_EMGRECTIFY_H
#define SSI_EMGRECTIFY_H

#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OverlapBuffer;

class EMGRectify : public ITransformer{

public:

	class Options : public OptionList {

	public:

		Options() : print_info(false) {

			addOption("print_info", &print_info, 1, SSI_BOOL, "print info");


		};

		bool print_info;
		
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "EMGRectify"; };
	static IObject *Create (const ssi_char_t *file) { return new EMGRectify (file); };
	~EMGRectify ();

	EMGRectify::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Rectifies EMG signal"; };

	void transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	void peak (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area);
	void slope (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area, ssi_real_t slope);

	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
		return sizeof (ssi_real_t);
	}
	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err("type '%s' not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {

		if (sample_dimension_in > 2){
			ssi_err("dim is %i, expected 1 or 2\n", sample_dimension_in);
		}
		return 1;
	}

	ssi_size_t getSampleNumberOut(ssi_size_t sample_number_in){
		return sample_number_in;
	}

    void printFeatures();

protected:


	EMGRectify (const ssi_char_t *file = 0);
	EMGRectify::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
	
};

}

#endif
