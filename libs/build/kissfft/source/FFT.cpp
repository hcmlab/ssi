// FFT.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/02
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

#include "FFT.h"
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

FFT::FFT (ssi_size_t nfft_, ssi_size_t dim_) 
	: nfft (nfft_), 
	rfft ((nfft_ >> 1) + 1),
	dim (dim_) {

	_cfg = kiss_fftr_alloc (nfft,0,0,0);	
	_data = new ssi_real_t *[dim];
	_fdata = new kiss_fft_cpx *[dim];
	for (ssi_size_t i = 0; i < dim; i++) {
		_data[i] = new ssi_real_t[nfft];
		_fdata[i] = new kiss_fft_cpx[rfft];
	}
}

FFT::~FFT () {
	
	for (ssi_size_t i = 0; i < dim; i++) {
		delete[] _data[i];
		delete[] _fdata[i];
	}
	delete[] _data; _data = 0;
	delete[] _fdata; _fdata = 0;
	free (_cfg);
}

void FFT::transform (Matrix<ssi_real_t> *src, 
	Matrix< std::complex<ssi_real_t> > *dst) {

	if (src->cols != dim) {
		ssi_err ("input matrix is %ux%u, but should be %ux%u", src->rows, src->cols, src->rows, dim);
	}
	if (!(dst->rows == 1 && dst->cols == dim * rfft)) {
		ssi_err ("output matrix is %ux%u, but should be %ux%u", dst->rows, dst->cols, 1, dim * rfft);
	}

	transform (src->rows, src->data, dst->data);
}

void FFT::transform (Matrix<ssi_real_t> *src, 
	Matrix<ssi_real_t> *dst) {

	if (src->cols != dim) {
		ssi_err ("input matrix is %ux%u, but should be %ux%u", src->rows, src->cols, src->rows, dim);
	}
	if (!(dst->rows == 1 && dst->cols == dim * rfft)) {
		ssi_err ("output matrix is %ux%u, but should be %ux%u", dst->rows, dst->cols, 1, dim * rfft);
	}

	transform (src->rows, src->data, dst->data);
}

void FFT::transform (ssi_size_t num,
	ssi_real_t *src,
	std::complex<ssi_real_t> *dst) {

	separate (num, src);

	for (ssi_size_t i = 0; i < dim; i++) {
		kiss_fftr (_cfg, _data[i], _fdata[i]);
	}

	join (rfft, dst);
}

void FFT::transform (ssi_size_t num,
	ssi_real_t *src,
	ssi_real_t *dst) {

	separate (num, src);

	for (ssi_size_t i = 0; i < dim; i++) {
		kiss_fftr (_cfg, _data[i], _fdata[i]);
	}

	join (rfft, dst);
}

void FFT::separate (ssi_size_t n, const ssi_real_t *data) {

	if (n >= nfft) {
		for (ssi_size_t i = 0; i < nfft; i++) {
			for (ssi_size_t j = 0; j < dim; j++) {
				_data[j][i] = *data++;
			}
		}
	} else { // if n < nfft fill up with zeros
		for (ssi_size_t i = 0; i < n; i++) {
			for (ssi_size_t j = 0; j < dim; j++) {
				_data[j][i] = *data++;
			}
		}
		for (ssi_size_t i = n; i < nfft; i++) {
			for (ssi_size_t j = 0; j < dim; j++) {
				_data[j][i] = 0;
			}
		}
	}
}

void FFT::join (ssi_size_t n, std::complex<ssi_real_t> *data) {

	kiss_fft_cpx value;
	for (ssi_size_t i = 0; i < n; i++) {
		for (ssi_size_t j = 0; j < dim; j++) {
			value = _fdata[j][i];
			 *data++ = std::complex<ssi_real_t> (value.r, value.i);
		}
	}
}

void FFT::join (ssi_size_t n, ssi_real_t *data) {

	kiss_fft_cpx value;
	for (ssi_size_t i = 0; i < n; i++) {
		for (ssi_size_t j = 0; j < dim; j++) {
			value = _fdata[j][i];
			 *data++ = sqrt (value.r * value.r + value.i * value.i);
		}
	}
}

void FFT::print (FILE *file) {

	kiss_fft_cpx value;
	for (ssi_size_t i = 0; i < rfft; i++) {
		for (ssi_size_t j = 0; j < dim; j++) {
			value = _fdata[j][i];
			fprintf (file, "%f+%fi ", value.r, value.i);
		}
		fprintf (file, "\n");
	}
}

}
