// IFFT.h
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

/**

Provides fast fourier transformation.

*/

#pragma once

#ifndef SSI_SIGNAL_IFFT_H
#define SSI_SIGNAL_IFFT_H

#include "kiss_fftr.h"
#include "signal/Matrix.h"

namespace ssi {

class IFFT {

public:

	IFFT (ssi_size_t rfft,
		ssi_size_t dim);
	~IFFT ();

	void transform (Matrix< std::complex<ssi_real_t> > *src, 
		Matrix<ssi_real_t> *dst);
	void transform (std::complex<ssi_real_t> *src, 
		ssi_real_t *dst);

	const ssi_size_t nfft;
	const ssi_size_t rfft;
	const ssi_size_t dim;

private:
	
	kiss_fftr_cfg _icfg;
	kiss_fft_cpx **_fdata;
	ssi_real_t **_data;

	void separate (ssi_size_t n, const ssi_real_t *data);
	void join (ssi_size_t n, std::complex<ssi_real_t> *data);

};

}

#endif
