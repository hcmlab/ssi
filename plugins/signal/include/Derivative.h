// Derivative.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/04
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

#ifndef SSI_SIGNAL_DERIVATIVE_H
#define SSI_SIGNAL_DERIVATIVE_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class IDerivative : public IFilter {

public:

	enum FORMAT : ssi_bitmask_t {
		NONE = 0,
		D0TH = 0x1,
		D1ST = 0x2,
		D2ND = 0x4,
		D3RD = 0x8,
		D4TH = 0x10,
		ALL = D0TH | D1ST | D2ND | D3RD | D4TH
	};

protected:

	static const ssi_size_t FORMAT_SIZE;
	static const ssi_char_t *FORMAT_NAMES[];
	static ssi_bitmask_t Names2Format (const ssi_char_t *names);
	static ssi_bitmask_t Name2Format (const ssi_char_t *name);
	static int FindHighestOrderBit (ssi_bitmask_t format);
	static ssi_size_t CountSetBits (ssi_bitmask_t format);
};

class Derivative : public IDerivative {

public:

	class Options : public OptionList {

	public:

		Options () {

			names[0] = '\0';
			addOption ("names", names, SSI_MAX_CHAR, SSI_CHAR, "names of derivatives separated by blank (0th,1st,2nd,3rd,4th) or leave empty to select all)");		
		};

		virtual void set (ssi_bitmask_t format);

		ssi_char_t names[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "Derivative"; };
	static IObject *Create (const ssi_char_t *file) { return new Derivative (file); };
	~Derivative ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes derivative of input stream by subtracting consecutive samples."; };

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

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		return CountSetBits (Names2Format (_options.names)) * sample_dimension_in;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	ssi_size_t getSize () { return CountSetBits (Names2Format (_options.names)); };
	const ssi_char_t *getName (ssi_size_t index);

protected:

	Derivative (const ssi_char_t *file = 0);
	Derivative::Options _options;
	ssi_char_t *_file;

	ssi_bitmask_t _format;
	ssi_size_t _depth;
	ssi_real_t *_history;
	char *_store_value;
	bool _first_call;
};

}

#endif
