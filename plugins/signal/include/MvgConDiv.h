// MvgConDiv.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/11
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#ifndef SSI_SIGNAL_CONDIV_H
#define SSI_SIGNAL_CONDIV_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class MvgConDiv : public IFilter {

public:

enum METHOD {
	MOVING = 0,
	SLIDING		
};

class Options : public OptionList {

	public:

		Options () 
			: wins (5.0), winl (10.0), method (MOVING) {

			addOption ("wins", &wins, 1, SSI_TIME, "size of short moving/sliding window in seconds");	
			addOption ("winl", &winl, 1, SSI_TIME, "size of long moving/sliding window in seconds");
			addOption ("method", &method, 1, SSI_INT, "method (0=moving,1=sliding)");				
		};

		ssi_time_t wins;
		ssi_time_t winl;
		METHOD method;
	};


public:

	static const ssi_char_t *GetCreateName () { return "MvgConDiv"; };
	static IObject *Create (const ssi_char_t *file) { return new MvgConDiv (file); };
	~MvgConDiv ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computs moving/sliding convergence/divergence for a chosen window."; };

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
		return sample_dimension_in;
	}

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sizeof (ssi_real_t);
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

protected:

	MvgConDiv (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;

	ITransformer *_mvgs;	
	ITransformer *_mvgl;	
	ssi_stream_t _mvgs_tmp;
	ssi_stream_t _mvgl_tmp;
};

}

#endif
