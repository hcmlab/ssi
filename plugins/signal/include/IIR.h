// IIR.h
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

#ifndef SSI_SIGNAL_IIR_H
#define SSI_SIGNAL_IIR_H

#include "base/IFilter.h"
#include "signal/MatrixOps.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class IIR : public IFilter {

public:

	static const ssi_char_t *GetCreateName () { return "IIR"; };
	static IObject *Create (const ssi_char_t *file) { return new IIR (file); };
	~IIR ();

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Infinite impulse response (IIR) filter."; };

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

	virtual void setCoefs (const Matrix<ssi_real_t> *coefs);
	virtual const Matrix<ssi_real_t> *getCoefs () { return _coefs; };

protected:

	IIR (const ssi_char_t *file = 0);

	int _sections;
	Matrix<ssi_real_t> *_coefs;
	Matrix<ssi_real_t> *_history;

public:

	virtual Matrix< std::complex<ssi_real_t> > *Response(const Matrix<ssi_real_t> *coefs, int nfft);

};

}

#endif
