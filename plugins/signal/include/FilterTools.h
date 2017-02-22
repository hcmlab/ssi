// FilterTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/12/20
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

Provides static window and filterbank functions.

*/

#pragma once

#ifndef SSI_SIGNAL_FILTERTOOLS_H
#define SSI_SIGNAL_FILTERTOOLS_H

#include "signal/Matrix.h"

namespace ssi {

enum WINDOW_TYPE {

    //! rectangular window
    WINDOW_TYPE_RECTANGLE = 0,
	//! triangle window
    WINDOW_TYPE_TRIANGLE,
    //! gauss window
    WINDOW_TYPE_GAUSS,
	//! hamming window
	WINDOW_TYPE_HAMMING
};


class FilterTools {

public:

	// sinus generator
	static Matrix<ssi_real_t> *Singen (double sample_rate, double duration, const Matrix<ssi_real_t> *freqs_and_ampls, ssi_real_t noise_level = 0);

	// window functions
	static Matrix<ssi_real_t> *Window (int size, WINDOW_TYPE type, MATRIX_DIMENSION dimension);

	// filterbank for spectral analysis
	static Matrix<ssi_real_t> *Filterbank (int size, double sample_rate, const Matrix<ssi_real_t> *intervals, WINDOW_TYPE type);
	static Matrix<ssi_real_t> *Filterbank (int size, double sample_rate, int banks_num, double min_freq, double max_freq, WINDOW_TYPE type);
	
	// butterworth filters
	static Matrix<ssi_real_t> *LPButter (ssi_size_t order, double cutoff);
	static Matrix<ssi_real_t> *HPButter (ssi_size_t order, double cutoff);
	static Matrix<ssi_real_t> *BPButter (ssi_size_t order, double lcutoff, double hcutoff);

	// block and boost filter
	static Matrix<ssi_real_t> *DCBlocker (double R);
	static Matrix<ssi_real_t> *Boost (double sample_rate, double gain, double center_freq, double bandwidth);

	// converts between hz and mel
	static ssi_real_t MelScale (ssi_real_t in, 
		bool inverse = false);
	static void MelScale (Matrix<ssi_real_t> *in, 
		bool inverse = false);

	// creates mel filterbank
	static Matrix<ssi_real_t> *MelBank (int n, 
		double sample_rate, 
		WINDOW_TYPE win_type = WINDOW_TYPE_TRIANGLE);
	static Matrix<ssi_real_t> *MelBank (int n, 
		ssi_time_t sample_rate, 
		int filt_num, 
		ssi_real_t min_freq, 
		ssi_real_t max_freq, 
		WINDOW_TYPE win_type = WINDOW_TYPE_TRIANGLE);

	// creates dct-II matrix
	static Matrix<ssi_real_t> *DCTMatrix (int size, 
		int cnum);
	static Matrix<ssi_real_t> *DCTMatrix (int size, 
		int cfirst, int clast);
	
		// adds noise to stream
	static void Noise (ssi_stream_t &series,
		ssi_real_t *amplitude);
	static void Noise (ssi_stream_t &series,
		ssi_real_t noise_ampl,
		double noise_mean,
		double noise_std,
		ssi_time_t cutoff = 0,
		ssi_time_t width = 0);

private:

	static Matrix<std::complex<double>> *ButterPoles (int sections, double frequency);

};

}

#endif
