// ConvPower.h
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2008/01/18
// Copyright (C) 2007-11 University of Augsburg, Johannes Wagner
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

#ifndef SSI_SIGNAL_CONVPOWER_H
#define SSI_SIGNAL_CONVPOWER_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class ConvPower : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: global (false) {

			addOption ("global", &global, 1, SSI_BOOL, "calculate energy accross all dimensions");		
		};

		bool global;
	};

public:

	static const ssi_char_t *GetCreateName () { return "ConvPower"; };
	static IObject *Create (const ssi_char_t *file) { return new ConvPower (file); };
	~ConvPower ();

	ConvPower::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes energy from the convolution of the input stream."; };

	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		return _options.global ? 1 : sample_dimension_in;
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

protected:

	ConvPower (const ssi_char_t *file = 0);
	ConvPower::Options _options;
	ssi_char_t *_file;

	static void conv (int lenx, int leny, int dim, ssi_real_t X[], ssi_real_t Y[], ssi_real_t Z[])
	{
		ssi_real_t *zptr,*xp,*yp;
		int lenz;
		int n,n_lo,n_hi;

		lenz = lenx + leny - 1;
		zptr = Z;

		ssi_real_t *s = new ssi_real_t[dim];
		for (int i = 0; i < lenz; i++) {
			for (int j = 0; j < dim; j++) {
				s[j] = 0;
			}
			n_lo = 0 > (i-leny+1) ? 0 : i-leny+1;
			n_hi = lenx-1 < i ? lenx-1 : i;
			xp = X + n_lo * dim;
			yp = Y + (i-n_lo) * dim;
			for (n = n_lo; n <= n_hi; n++) {
				for (int j = 0; j < dim; j++) {
					s[j] += xp[j] * yp[j];					
				}
				yp -= dim;
				xp += dim;
			}
			for (int j = 0; j < dim; j++) {
				*zptr++ = s[j];
			}
		}
		delete[] s;
	}
};

}

#endif
