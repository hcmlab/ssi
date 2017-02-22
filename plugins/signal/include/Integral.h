// Integral.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/08/21
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

/**

Integrates signal using cumulative trapezoidal numerical integration

Set bits define until which depth the signal is integrated and which
states are finally kept.

e.g. if you want to calculate position from acceleration
     and also keep the original signal use format = 5 (000101)

	 you would get the following result:

	 [a1 p1
	  a2 p2
	   ...
	  aN pN]

*/

#pragma once

#ifndef SSI_SIGNAL_INTEGRAL_H
#define SSI_SIGNAL_INTEGRAL_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class IIntegral : public IFilter {

public:

	enum FORMAT : ssi_bitmask_t {
		NONE = 0,
		I0TH = 0x1,
		I1ST = 0x2,
		I2ND = 0x4,
		I3RD = 0x8,
		I4TH = 0x10,
		ALL = I0TH | I1ST | I2ND | I3RD | I4TH
	};

protected:

	static const ssi_size_t FORMAT_SIZE;
	static const ssi_char_t *FORMAT_NAMES[];
	static ssi_bitmask_t Names2Format (const ssi_char_t *names);
	static ssi_bitmask_t Name2Format (const ssi_char_t *name);
	static int FindHighestOrderBit (ssi_bitmask_t format);
	static ssi_size_t CountSetBits (ssi_bitmask_t format);
};

class Integral : public IIntegral {

public:

	class Options : public OptionList {

	public:

		Options () 
			: reset (false) {

			names[0] = '\0';
			addOption ("names", names, SSI_MAX_CHAR, SSI_CHAR, "names of integral separated by blank (0th,1st,2nd,3rd,4th) or leave empty to select all)");	
			addOption ("reset", &reset, 1, SSI_BOOL, "always reset");
		};

		virtual void set (ssi_bitmask_t format);

		ssi_char_t names[SSI_MAX_CHAR];
		bool reset;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Integral"; };
	static IObject *Create (const ssi_char_t *file) { return new Integral (file); };
	~Integral ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes integral of input stream by summing consecutive samples."; };

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

	Integral (const ssi_char_t *file = 0);
	Integral::Options _options;
	ssi_char_t *_file;

	ssi_bitmask_t _format;
	ssi_size_t _depth;
	ssi_real_t *_history;
	char *_store_value;
	bool _first_call;
	ssi_real_t _fact;
	bool _always_reset;
};

}

#endif
