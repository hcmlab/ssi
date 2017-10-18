// EMGFeaturesSpectral.h
// author: Daniel Schork
// created: 2016
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

#ifndef SSI_EMGFEATURESSPECTRAL_H
#define SSI_EMGFEATURESSPECTRAL_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class EMGFeaturesSpectral : public IFeature{

public:

	enum AR_METHOD {
		MAXENTROPY = 0,
		LEASTSQUARES,
	};

	class Options : public OptionList {

	public:

		Options () 
			: print_info(false), print_features(false), n_ar_coeffs(4), ar_method(MAXENTROPY), fr_threshold(0.666666667){

			addOption("print_info", &print_info, 1, SSI_BOOL, "print info about feature calculation");
			addOption("print_features", &print_features, 1, SSI_BOOL, "print feature name and number");
			addOption("n_ar_coeffs", &n_ar_coeffs, 1, SSI_SIZE, "number of auto-regressive coefficients");
			addOption("ar_method", &ar_method, 1, SSI_SIZE, "auto-regressive coefficient calculation method (maxentropy = 0, leastsquares = 1)");
			addOption("fr_threshold", &fr_threshold, 1, SSI_REAL, "threshold for frequency ratio [0..1]");

						
		};

		bool print_info, print_features;
		ssi_size_t n_ar_coeffs, ar_method;
		ssi_real_t fr_threshold;
		
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "EMGFeaturesSpectral"; };
	static IObject *Create (const ssi_char_t *file) { return new EMGFeaturesSpectral (file); };
	~EMGFeaturesSpectral ();

	EMGFeaturesSpectral::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "calculates emg features in the spectral domain"; };

	void transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	void peak (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area);
	void slope (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area, ssi_real_t slope);

	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
		return sizeof (ssi_real_t);
	}
	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err("type '%s' not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {

		if (sample_dimension_in > 2){
			ssi_err("dim is %i, expected 1 or 2\n", sample_dimension_in);
		}

		return _options.n_ar_coeffs + 5;
	}

    void printFeatures();

protected:

	EMGFeaturesSpectral (const ssi_char_t *file = 0);
    Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	float calcFrequencyMedian(ssi_real_t* in, ssi_size_t num, bool modified);
	float calcFrequencyMean(ssi_real_t* in, ssi_size_t num, bool modified);
	float calcFrequencyRatio(ssi_real_t* in, ssi_size_t num, bool modified, ssi_real_t threshold);

	int calcARcoefficients(float *inputseries, int length, int degree, float *coefficients, int method);
	int arMaxEntropy(float *inputseries, int length, int degree, float **ar, float *per, float *pef, float *h, float *g);
	int arLeastSquare(float *inputseries, int length, int degree, float *coefficients);
	int arSolveLE(float **mat, float *vec, unsigned int n);

	
};

}

#endif
