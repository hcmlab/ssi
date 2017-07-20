// SNRatio.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/29
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

#include "SNRatio.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

int SNRatio::decibels[90] = { 
	32767, /*  0 dB */
	29204, /*  1 dB */
	26028, /*  2 dB */
	23197, /*  3 dB */
	20675, /*  4 dB */
	18426, /*  5 dB */
	16422, /*  6 dB */ 
	14636, /*  7 dB */
	13045, /*  8 dB */
	11626, /*  9 dB */
	10362, /* 10 dB */
	9235, /* 11 dB */
	8231, /* 12 dB */
	7336, /* 13 dB */
	6538, /* 14 dB */
	5827, /* 15 dB */
	5193, /* 16 dB */
	4628, /* 17 dB */
	4125, /* 18 dB */
	3677, /* 19 dB */
	3277, /* 20 dB */
	2920, /* 21 dB */
	2603, /* 22 dB */
	2320, /* 23 dB */
	2067, /* 24 dB */
	1843, /* 25 dB */
	1642, /* 26 dB */ 
	1464, /* 27 dB */
	1304, /* 28 dB */
	1163, /* 29 dB */
	1036, /* 30 dB */
	923, /* 31 dB */
	823, /* 32 dB */
	734, /* 33 dB */
	654, /* 34 dB */
	583, /* 35 dB */
	519, /* 36 dB */
	463, /* 37 dB */
	412, /* 38 dB */
	368, /* 39 dB */
	328, /* 40 dB */
	292, /* 41 dB */
	260, /* 42 dB */
	233, /* 43 dB */
	207, /* 44 dB */
	184, /* 45 dB */
	164, /* 46 dB */ 
	146, /* 47 dB */
	130, /* 48 dB */
	116, /* 49 dB */
	104, /* 50 dB */
	92, /* 51 dB */
	82, /* 52 dB */
	73, /* 53 dB */
	65, /* 54 dB */
	58, /* 55 dB */
	52, /* 56 dB */
	46, /* 57 dB */
	41, /* 58 dB */
	37, /* 59 dB */
	33, /* 60 dB */
	29, /* 61 dB */
	26, /* 62 dB */
	23, /* 63 dB */
	21, /* 64 dB */
	18, /* 65 dB */
	16, /* 66 dB */ 
	15, /* 67 dB */
	13, /* 68 dB */
	12, /* 69 dB */
	10, /* 70 dB */
	 9, /* 71 dB */
	 8, /* 72 dB */
	 7, /* 73 dB */
	 7, /* 74 dB */
	 6, /* 75 dB */
	 5, /* 76 dB */
	 5, /* 77 dB */
	 4, /* 78 dB */
	 4, /* 79 dB */ 
	 3, /* 80 dB */
	 3, /* 81 dB */
	 3, /* 82 dB */
	 2, /* 83 dB */
	 2, /* 84 dB */
	 2, /* 85 dB */
	 2, /* 86 dB */ 
	 1, /* 87 dB */
	 1, /* 88 dB */
	 1  /* 89 dB */
};

SNRatio::SNRatio (const ssi_char_t *file)
: input_is_float (false),
	_file(0){

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

}

SNRatio::~SNRatio () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}	

}

void SNRatio::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
	
	input_is_float = stream_in.type == SSI_FLOAT;
	if (input_is_float) {
		ssi_stream_init (stream_cast, 0, stream_in.dim, stream_in.byte, SSI_SHORT, stream_in.sr);
	}

    _thres_s = ssi_cast (int16_t, _options.thresh + 0.5);
	_thres_f = _options.thresh;
}

void SNRatio::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_number = stream_in.num;

	if (input_is_float) {
		float *fltptr = ssi_pcast (float, stream_in.ptr);
		float *outptr = ssi_pcast (float, stream_out.ptr);		
		ssi_stream_adjust (stream_cast, stream_in.num);
        int16_t *inptr = ssi_pcast (int16_t, stream_cast.ptr);
		for (ssi_size_t i = 0; i < stream_in.num * stream_in.dim; i++) {
            *inptr++ = ssi_cast (int16_t, *fltptr++ * 32768.0f);
		}
        inptr = ssi_pcast (int16_t, stream_cast.ptr);
		float value = ssi_cast (float, signal_to_noise_ratio_short (inptr, sample_number - 64));
		*outptr = value >= _thres_f ? value : 0;
	} else {
        int16_t *inptr = ssi_pcast (int16_t, stream_in.ptr);
        int16_t *outptr = ssi_pcast (int16_t, stream_out.ptr);
        int16_t value = signal_to_noise_ratio_short (inptr, sample_number - 64);
		*outptr = value >= _thres_s ? value : 0;
	}
}

void SNRatio::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (input_is_float) {
		ssi_stream_destroy (stream_cast);
	}
}

int SNRatio::signal_to_noise_ratio_short (int16_t *input, int N) {

	int cov = 0;
	int maximum;
	int maximum_offset = 0;

	// Find the best covariance between two points
	for (int i = 1; i < 64; i ++) {
		maximum = covariance_short (&input[0], &input[i], N );
		if ( maximum > cov) {
			cov = maximum;
			maximum_offset = i;
		} 
	}

	// Calculate variance of first series
	int variance1 = variance_short (&input[0], N);

	// Calculate variance of second series
	int variance2 = variance_short (&input[maximum_offset], N);

	// Take mean of two variance measurements
	int var = static_cast<int> ((static_cast<long> (variance1) + static_cast<long> (variance2))/2);

	// Calculate log10 of covariance
	int covariance_in_dB = calculate_decibels (cov);

	// Calculate log10 of variance
	int variance_in_dB = calculate_decibels (var-cov);

	int signal_to_noise_ratio_in_dB = -(covariance_in_dB - variance_in_dB)/2;

	return signal_to_noise_ratio_in_dB > 0 ? signal_to_noise_ratio_in_dB : 0;
}

int SNRatio::covariance_short (int16_t *x, int16_t *y, int N) {

	int i;
	int shift = 0;
	int sum_x = 0;
	int sum_y = 0;
	int sum_xy = 0;
	int mean_x_mean_y; 

	i = N/2;
	shift = 0;

	/* Calculate shift equivalent to dividing by N */            
	while ( i > 0 ) {
		i /= 2;
		shift++;
	}

	int val_x, val_y;
    int16_t *ptr_x = x;
    int16_t *ptr_y = y;
	for ( i = 0 ; i < N ; i++) {
		val_x = *ptr_x++;
		val_y = *ptr_y++;
		sum_x += val_x;           
		sum_y += val_y;
		sum_xy += (val_x * val_y) >> 15;   
	}

	sum_x >>= shift;   /* Mean of x */
	sum_y >>= shift;   /* Mean of y */
	sum_xy >>= shift;  /* Mean of xy */

	mean_x_mean_y = (sum_x * sum_y) >> 15; /* mean_x * mean_y */

	return sum_xy - mean_x_mean_y;

}

int SNRatio::variance_short (int16_t *x, int N) {

	int i;
	int shift = 0;
	int sum_x = 0;            
	int sum_xx = 0;         
	int mean_x_mean_x;   /* Mean of x*x */

	i = N/2;
	shift = 0;

	/* Calculate shift equivalent to dividing by N */            
	while ( i > 0 ) {
		i /= 2;
		shift++;
	}

	int val;
    int16_t *ptr = x;
	for ( i = 0; i < N ; i++) 	{
		val = *ptr++;
		sum_x += val;           
		sum_xx += (val * val) >> 15;   
	}

	sum_x >>= shift;   /* Mean of x */
	sum_xx >>= shift;  /* Mean of xx */

	mean_x_mean_x = (sum_x * sum_x) >> 15;

	return sum_xx - mean_x_mean_x;
}

int SNRatio::calculate_decibels (int input) {

	int i;

	if ( input < 0) {
		input = -input;
	}

	for ( i = 0 ; i < 90 ; i++) {
		if ( input > decibels[i]) {
		 break;
		}
	}

	return i;
}

}
