// Multiply.h
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

#ifndef SSI_SIGNAL_MULTIPLY_H
#define SSI_SIGNAL_MULTIPLY_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "thread/Lock.h"

namespace ssi {

class Multiply : public IFilter {

public:

	struct JOIN {
		enum List {
			OFF = 0,
			MULT = 1,
			SUM = 2,
			SUMSQUARE = 3
		};
	};

public:

	class Options : public OptionList {

	public:

		Options () : single (true), join (JOIN::OFF), factor (1.0f) {
			
			factors[0] = '\0';	

			addOption ("single", &single, 1, SSI_BOOL, "a single multiplication factor is given (factor), otherwise one per dimension (factors)");
			addOption ("factor", &factor, 1, SSI_REAL, "multiplication factor", false);		
			addOption ("factors", factors, SSI_MAX_CHAR, SSI_CHAR, "multiplication factors separated by comma", false);					
			addOption ("join", &join, 1, SSI_INT, "join dimensions (0=off, 1=multiply, 2=sum up, 3=sum up squares", false);
		};

		void setFactors (ssi_size_t n_inds, ssi_real_t *values) {
			factors[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%f", values[0]);
				strcat (factors, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%f", values[i]);
					strcat (factors, s);
				}
			}
		}

		bool single;
		ssi_real_t factor;
		ssi_char_t factors[SSI_MAX_CHAR];
		JOIN::List join;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Multiply"; };
	static IObject *Create (const ssi_char_t *file) { return new Multiply (file); };
	~Multiply ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Multiplies input stream with a constant factor."; };

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
		return _options.join == JOIN::OFF ? sample_dimension_in : 1;
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

	bool notify(COMMAND::List command, const ssi_char_t *message = 0);

protected:

	Multiply (const ssi_char_t *file = 0);
	Multiply::Options _options;
	ssi_char_t *_file;
	JOIN::List _join;

	void readOptions();

	Mutex _factor_mutex;
	ssi_real_t _factor;
	ssi_size_t _factor_id;
	ssi_size_t _n_factors;
	ssi_real_t *_factors;
};

}

#endif
