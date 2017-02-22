// MFCC.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/03
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

#include "MFCC.h"
#include "FilterTools.h"
#include "signal/MatrixOps.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

MFCC::MFCC (const ssi_char_t *file)
	: _filterbank (0),
	_spectrogram (0),
	_dctmat (0),
	_spect (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

MFCC::~MFCC () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

ssi_size_t MFCC::getSampleDimensionOut (ssi_size_t sample_dimension_in) {

	if (sample_dimension_in != 1) {
		ssi_err("dimension > 1 not supported");
	}

	return _options.n_last - _options.n_first;
}

ssi_size_t MFCC::getSampleBytesOut (ssi_size_t sample_bytes_in) {

	return sizeof(ssi_real_t);
}

ssi_type_t MFCC::getSampleTypeOut (ssi_type_t sample_type_in) {

	if (sample_type_in != SSI_REAL) {
		ssi_err("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
	}

	return SSI_REAL;
}

void MFCC::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_time_t sample_rate = stream_in.sr;

	_filterbank = FilterTools::MelBank ((_options.n_ffts >> 1) + 1, sample_rate, WINDOW_TYPE_TRIANGLE);
	_spectrogram = ssi_pcast (Spectrogram, Spectrogram::Create (0));
	_spectrogram->setFilterbank (_filterbank, WINDOW_TYPE_HAMMING, true);
	_dctmat = FilterTools::DCTMatrix (_filterbank->rows, _options.n_first, _options.n_last);
	_spect = new Matrix<ssi_real_t> (1, _spectrogram->getSampleDimensionOut (sample_dimension));
}

void MFCC::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	Matrix<ssi_real_t> matrix_in (sample_number, sample_dimension, srcptr);
	Matrix<ssi_real_t> matrix_out (1, _options.n_last - _options.n_first, dstptr);

	transform (&matrix_in, &matrix_out);

	matrix_in.data = 0;
	matrix_out.data = 0;
}

void MFCC::transform (Matrix<ssi_real_t> *matrix_in, 
	Matrix<ssi_real_t> *matrix_out) {

	/* Matlab code

	coefs = cept (log10 (_spect (data, fs, flen, melbands ((wlen * fs) / 2 + 1, fs), 'hamming')), cnum);

	*/

	_spectrogram->transform (matrix_in, _spect);
	MatrixOps<ssi_real_t>::MultM (_spect, _dctmat, matrix_out);
}

void MFCC::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num ,
	ssi_stream_t xtra_stream_in[]) {
	
	delete _filterbank;
	delete _spectrogram;
	delete _dctmat;
	delete _spect;
}

}
