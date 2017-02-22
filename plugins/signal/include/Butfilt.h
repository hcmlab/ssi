// Butfilt.h
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

/**

Provides butter low/high/band-pass filter

*/

#pragma once

#ifndef SSI_SIGNAL_BUTFILT_H
#define SSI_SIGNAL_BUTFILT_H

#include "IIR.h"
#include "signal/Matrix.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Butfilt : public IFilter {

public:

	enum TYPE : unsigned char {
		LOW = 0,
		HIGH,
		BAND
	};

	class Options : public OptionList {

	public:

		Options ()
			: type (LOW), order (1), norm (true), low (0.0f), high (1.0f), zero (true) {

			addOption ("type", &type, 1, SSI_UCHAR, "filter type (0=low, 1=high, 2=band)");		
			addOption ("order", &order, 1, SSI_UINT, "filter order");	
			addOption ("norm", &norm, 1, SSI_BOOL, "frequency values are normalized in interval [0..1], where 1 is the nyquist frequency (=half the sample rate)");
			addOption ("low", &low, 1, SSI_REAL, "low cutoff frequency given either as normalized value in interval [0..1] or as an absolute value in Hz (see -norm)", false);
			addOption ("high", &high, 1, SSI_REAL, "high cutoff frequency given either as normalized value in interval [0..1] or as an absolute value in Hz (see -norm)", false);
			addOption ("zero", &zero, 1, SSI_BOOL, "subtract first sample from signal to avoid artefacts at the beginning of the signal");
		};

		TYPE type;
		ssi_size_t order;
		bool norm;
		ssi_real_t low;
		ssi_real_t high;
		bool zero;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Butfilt"; };
	static IObject *Create (const ssi_char_t *file) { return new Butfilt (file); };
	~Butfilt ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Applies butter low/high/bandpass filter to input stream."; };

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
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	virtual const Matrix<ssi_real_t> *getCoefs ();

protected:

	Butfilt (const ssi_char_t *file = 0);
	Butfilt::Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;
	
	bool _norm;
	TYPE _type;
	ssi_size_t _order;
	ssi_real_t _low;
	ssi_real_t _high;

	void init_coefs (ssi_time_t sr);
	Matrix<ssi_real_t> *_coefs;
	IIR *_iir;
	ssi_real_t *_first_sample;
	bool _first_call;
};

}

#endif
