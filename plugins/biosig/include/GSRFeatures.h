// GSRFeatures.h
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

#ifndef SSI_GSRFEATURES_H
#define SSI_GSRFEATURES_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include "GSRFindPeaks.h"
#include "GSRFindSlopes.h"

namespace ssi {

class OverlapBuffer;

class GSRFeatures : public IFeature, public GSRFindPeaks::ICallback, public GSRFindSlopes::ICallback{

public:

	class Options : public OptionList {

	public:

		Options () 
			: print_info(false), print_features(false), winsize (15.0), peaknstd (0.25f), peakmind (1.0f), peakmaxd (4.0f), slopenstd (0.5f), slopemind (0.5f), slopemaxd (4.0f),
			f_number(true), f_type_peaks(true), f_type_slopes(true), f_type_drops(true), f_type_combo(true),
			f_att_duration(true), f_att_amplitude(true), f_att_area(true),
			f_stat_min(true), f_stat_max(true), f_stat_avg(true), f_stat_var(true), f_stat_std(true), new_features(false){

			addOption("print_info", &print_info, 1, SSI_BOOL, "print info about feature calculation");
			addOption("print_features", &print_features, 1, SSI_BOOL, "print feature name and number");

			addOption("winsize", &winsize, 1, SSI_REAL, "size in seconds of detrend window");
			addOption("peakmind", &peakmind, 1, SSI_REAL, "peak minimum duration in seconds");		
			addOption("peakmaxd", &peakmaxd, 1, SSI_REAL, "peak maximum duration in seconds");		
			addOption("peaknstd", &peaknstd, 1, SSI_REAL, "peak threshold = n * standard deviation");		
			addOption("slopemind", &slopemind, 1, SSI_REAL, "slope minimum duration in seconds");		
			addOption("slopemaxd", &slopemaxd, 1, SSI_REAL, "slope maximum duration in seconds");		
			addOption("slopenstd", &slopenstd, 1, SSI_TIME, "slope threshold = n * standard deviation");


			//features

			addOption("f_number", &f_number, 1, SSI_BOOL, "compute number of peaks/slopes/drops/combo");
			
			addOption("f_type_peaks", &f_type_peaks, 1, SSI_BOOL, "compute peaks");
			addOption("f_type_slopes", &f_type_slopes, 1, SSI_BOOL, "compute slopes");
			addOption("f_type_drops", &f_type_drops, 1, SSI_BOOL, "compute drops");
			addOption("f_type_combo", &f_type_combo, 1, SSI_BOOL, "compute combination of peaks, slopes and drops");

			addOption("f_att_duration", &f_att_duration, 1, SSI_BOOL, "compute duration attribute of peaks/slopes/drops/combo");
			addOption("f_att_amplitude", &f_att_amplitude, 1, SSI_BOOL, "compute amplitude attribute of peaks/slopes/drops/combo");
			addOption("f_att_area", &f_att_area, 1, SSI_BOOL, "compute area attribute of peaks/slopes/drops/combo");
			
			addOption("f_stat_min", &f_stat_min, 1, SSI_BOOL, "compute minimum of selected attributes");
			addOption("f_stat_max", &f_stat_max, 1, SSI_BOOL, "compute maximum of selected attributes");
			addOption("f_stat_avg", &f_stat_avg, 1, SSI_BOOL, "compute average of selected attributes");
			addOption("f_stat_var", &f_stat_var, 1, SSI_BOOL, "compute variance of selected attributes");
			addOption("f_stat_std", &f_stat_std, 1, SSI_BOOL, "compute standard deviation of selected attributes");

			addOption("new_features", &new_features, 1, SSI_BOOL, "include the top 13 features from s. gruss");

		};
		bool f_number, f_type_peaks, f_type_slopes, f_type_drops, f_type_combo, f_att_duration, f_att_amplitude, f_att_area, f_stat_min, f_stat_max, f_stat_avg, f_stat_var, f_stat_std, new_features;

		bool print_info, print_features;
		ssi_time_t winsize;
		ssi_real_t peaknstd;
		ssi_real_t peakmind;
		ssi_real_t peakmaxd;
		ssi_real_t slopenstd;
		ssi_real_t slopemind;
		ssi_real_t slopemaxd;

		
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "GSRFeatures"; };
	static IObject *Create (const ssi_char_t *file) { return new GSRFeatures (file); };
	~GSRFeatures ();

	GSRFeatures::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "calculates gsr features"; };

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

		if (sample_dimension_in != 1){
			ssi_err("dim is %i, expected 1\n", sample_dimension_in);
		}

		ssi_size_t type = 0;
		if (_options.f_type_peaks) type++;
		if (_options.f_type_slopes) type++;
		if (_options.f_type_drops) type++;
		if (_options.f_type_combo) type++;

		ssi_size_t att = 0;
		if (_options.f_att_duration) att++;
		if (_options.f_att_amplitude) att++;
		if (_options.f_att_area) att++;

		ssi_size_t stat = 0;
		if (_options.f_stat_min) stat++;
		if (_options.f_stat_max) stat++;
		if (_options.f_stat_avg) stat++;
		if (_options.f_stat_var) stat++;
		if (_options.f_stat_std) stat++;

		ssi_size_t dim = type * att * stat;
		if (_options.f_number) dim += type;

		if (_options.new_features) dim += 3;

		return dim;
	}

    virtual void printFeatures();

	virtual void setReferenceStream(ssi_stream_t* ref) {
		_reference_stream = ref;
	}



protected:

	enum GSR_STATE {
		GSR_NONE = 0,
		GSR_RISING,
		GSR_FALLING
	};

	struct peakslopedrop {
		ssi_time_t from;
		ssi_time_t to;
		ssi_real_t amplitude;
		ssi_real_t area;
	};

	GSRFeatures (const ssi_char_t *file = 0);
	GSRFeatures::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ITransformer *_mvgvar;
	ssi_stream_t _var_stream;
	GSRFindPeaks *_findpeaks;	
	GSRFindSlopes *_findslopes;	

	std::vector<peakslopedrop> peaks;
	std::vector<peakslopedrop> slopes;
	std::vector<peakslopedrop> drops;
	std::vector<peakslopedrop> psd_combo;

    float getMin(std::vector<peakslopedrop> in, ssi_size_t attribute);
    float getMax(std::vector<peakslopedrop> in, ssi_size_t attribute);
    float getAvg(std::vector<peakslopedrop> in, ssi_size_t attribute);
    float getVar(std::vector<peakslopedrop> in, ssi_size_t attribute);
    float getStDev(std::vector<peakslopedrop> in, ssi_size_t attribute);
    float getAttribute(peakslopedrop e, ssi_size_t att);

    void printFeatureStat(int st);
    void printFeatureAttribute(int at, std::string str_stat);
    void printFeatureType(int ft, std::string str_stat, std::string str_att);

	ssi_stream_t* _reference_stream;

};

}

#endif
