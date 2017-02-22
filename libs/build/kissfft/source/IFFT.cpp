// IFFT.cpp
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

#include "IFFT.h"
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

IFFT::IFFT (ssi_size_t rfft_, ssi_size_t dim_) 
: rfft (rfft_), nfft ((rfft_ - 1) << 1), dim (dim_) {

	_icfg = kiss_fftr_alloc (nfft,1,0,0);
	_data = new ssi_real_t *[dim];
	_fdata = new kiss_fft_cpx *[dim];
	for (ssi_size_t i = 0; i < dim; i++) {
		_data[i] = new ssi_real_t[nfft];
		_fdata[i] = new kiss_fft_cpx[rfft];
	}
}

IFFT::~IFFT () {
	
	for (ssi_size_t i = 0; i < dim; i++) {
		delete[] _data[i];
		delete[] _fdata[i];
	}	
	delete[] _data;
	delete[] _fdata;
	free (_icfg);
}

void IFFT::transform (Matrix< std::complex<ssi_real_t> > *src, 
	Matrix<ssi_real_t> *dst) {
	
	// SSI_ASSERT that:	
	// - src has 1 row and dim * rfft columns
	// - dst has nfft rows and dim colums
	if (!(src->rows == 1 && src->cols == dim * rfft)) {
		ssi_err ("input matrix is %ux%u, but should be %ux%u", src->rows, src->cols, 1, dim * rfft);
	}
	if (!(dst->rows == nfft && dst->cols >= dim)) {
		ssi_err ("input matrix is %ux%u, but should be %ux%u", dst->rows, dst->cols, nfft, dim);
	}

	transform (src->data, dst->data);
}

void IFFT::transform (std::complex<ssi_real_t> *src, 
	ssi_real_t *dst) {

	for (ssi_size_t j = 0; j < dim; j++) {
		kiss_fft_cpx *fptr = _fdata[j];
		std::complex<ssi_real_t> *srcptr = src + j;
		for (ssi_size_t i = 0; i < rfft; i++) {
			(*fptr).r = (*srcptr).real ();
			(*fptr).i = (*srcptr).imag ();
			srcptr += dim;
			fptr++;
		}
		kiss_fftri (_icfg, _fdata[j], _data[j]);
	}
			
	for (ssi_size_t i = 0; i < nfft; i++) {
		for (ssi_size_t j = 0; j < dim; j++) {
			*dst++ = _data[j][i] / nfft;
		}
	}


}

}
