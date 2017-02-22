// Statistics.h
// author: Florian Obermayer <florian.obermayer@student.uni-augsburg.de>
//         Fabian Hertwig <fabian.hertwig@student.uni-augsburg.de>
// created: 2015/03/06
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
// version 3 of the License, or any later version.
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

#ifndef SSI_SIGNAL_STATISTICS_H
#define SSI_SIGNAL_STATISTICS_H

#define ssi_stat_ultra_precision_t ssi_time_t 

#include "ioput/option/OptionList.h"
#include "SSI_Cons.h"
#include "base/IFeature.h"

namespace ssi {

	enum stat_fn
	{
		STAT_KURTOSIS = 0,
		STAT_SKEWNESS = 1,
		STAT_MEAN = 2,
		STAT_STDDEV = 3,
		STAT_VARIANCE = 4,
		STAT_NUMBER_VALS = 5,
	};

	struct running_stat_t
	{
		ssi_stat_ultra_precision_t mean;
		ssi_stat_ultra_precision_t M2;
		ssi_stat_ultra_precision_t M3;
		ssi_stat_ultra_precision_t M4;
		ssi_size_t n;
	};


class Statistics : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options () : kurtosis(false), skewness(false), mean(false), stddev(false), var(false), number_vals(false) {

			addOption("A-kurtosis", &kurtosis, 1, SSI_BOOL, "outputs kurtosis as a dimension");
			addOption("B-skewness", &skewness, 1, SSI_BOOL, "outputs skewness as a dimension");
			addOption("C-mean", &mean, 1, SSI_BOOL, "outputs mean as a dimension");
			addOption("D-stddev", &stddev, 1, SSI_BOOL, "outputs stddev as a dimension");
			addOption("E-var", &var, 1, SSI_BOOL, "outputs var as a dimension");
			addOption("F-number-of-values", &number_vals, 1, SSI_BOOL, "outputs the number of values as a dimension");
		};

		bool kurtosis;
		bool skewness;
		bool mean;
		bool stddev;
		bool var;
		bool number_vals;
		std::vector<stat_fn> getSelection();

	private:
		std::vector<stat_fn> _selection;
	};

public:
	static const ssi_char_t *GetCreateName () { return "Statistics"; };
	static IObject *Create(const ssi_char_t *file) { return new Statistics(file); };
	~Statistics();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Raises statistical evaluations on the input stream."; };

protected:

	Statistics(const ssi_char_t *file = 0);

	virtual void transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num = 0, ssi_stream_t xtra_stream_in[] = 0) override;

	virtual void transform(ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num = 0, ssi_stream_t xtra_stream_in[] = 0) override;

	virtual void transform_flush(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num = 0, ssi_stream_t xtra_stream_in[] = 0) override;

	virtual ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) override{
		return sample_dimension_in * ssi_size_t (_options.getSelection().size());
	}

	virtual ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) override{
		return sample_bytes_in;
	}

	virtual ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) override{
		return SSI_REAL;
	}

	Statistics::Options _options;
	ssi_char_t *_file;

private:
	
	void calculate(ssi_stream_t &stream_in);

	void calculate_running_stats(ssi_size_t sample_num);
	ssi_real_t calculateKurtosis(ssi_size_t dim_idx);
	ssi_real_t calculateSkewness(ssi_size_t dim_idx);
	ssi_real_t calculateStandardDeviation(ssi_size_t dim_idx);
	ssi_real_t calculateVariance(ssi_size_t dim_idx);
	ssi_real_t calculateMean(ssi_size_t dim_idx);
	ssi_real_t calulateNumberVals(ssi_size_t dim_idx);
	void provide_temp_array(ssi_size_t num_real);
	ssi_size_t _dim;
	ssi_real_t *_res;
	ssi_real_t ** _tmp_arr;
	running_stat_t * _running_stats;

	ssi_size_t _old_num_real;
};

}

#endif // ! SSI_SIGNAL_STATISTICS_H
