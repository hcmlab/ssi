// RSPFeatures.h
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

#ifndef SSI_RSPFEATURES_H
#define SSI_RSPFEATURES_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {


	enum RSP_STATE{
		RSP_NONE = 0,
		RSP_RISING,
		RSP_FALLING
	};

	enum PTZ_TYPE{
		PEAK = 0,
		TROUGH,
		ZC_FALLING,
		ZC_RISING
	};

class RSPFeatures : public IFeature{

public:

	class Options : public OptionList {

	public:

		Options () 
			: print_info(false), print_features(false), sr(256){

			addOption("print_info", &print_info, 1, SSI_BOOL, "print info about feature calculation");
			addOption("print_features", &print_features, 1, SSI_BOOL, "print feature name and number");
			addOption("sr", &sr, 1, SSI_REAL, "sample rate pf the sensor");
		};

		bool print_info, print_features;
		ssi_real_t sr;
		
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "RSPFeatures"; };
	static IObject *Create (const ssi_char_t *file) { return new RSPFeatures (file); };
	~RSPFeatures ();

	RSPFeatures::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "calculates respiratory features"; };

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

		if (sample_dimension_in != 1){
			ssi_err("dim is %i, expected 1\n", sample_dimension_in);
		}

		
		return 18;
	}

	void RSPFeatures::printFeatures();

protected:

	struct peaktrough {
		ssi_size_t pos;
		ssi_real_t value;
		ssi_size_t type;
	};

	struct less_than_value
	{
		inline bool operator() (const peaktrough& struct1, const peaktrough& struct2)
		{
			return (struct1.value < struct2.value);
		}
	};

	RSPFeatures (const ssi_char_t *file = 0);
	RSPFeatures::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ssi_real_t ptr_prev;
	int state;
	void RSPFeatures::printFeatureStat(int st);

	void RSPFeatures::getBreathByBreathCorreleation(ssi_real_t* in, std::vector<peaktrough> troughs, ssi_real_t* bbc, ssi_real_t* bbc_sd);
	float RSPFeatures::getMaxCrossCorrelation(float* x, float* y, int n);

	void RSPFeatures::getSampleEntropy(ssi_real_t* in, std::vector<peaktrough> _ptz, ssi_real_t* se_peaks, ssi_real_t* se_troughs);
	void RSPFeatures::getVolumeBasedFeatures(ssi_real_t* in, std::vector<peaktrough> _pt, ssi_real_t* f_vol_br_median, ssi_real_t* f_vol_in_median, ssi_real_t* f_vol_ex_median, ssi_real_t* f_fr_br_median, ssi_real_t* f_fr_in_median, ssi_real_t* f_fr_ex_median);

	ssi_real_t getVolume(ssi_real_t* in, ssi_size_t from, ssi_size_t to);
	ssi_real_t getFlowRate(ssi_real_t* in, ssi_size_t from, ssi_size_t to);

	ssi_real_t RSPFeatures::sampen(double *y, int M, double r, int n);

};

}

#endif
