// Statistics.cpp
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

#include "Statistics.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi{


	Statistics::~Statistics()
	{
		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file; _file = 0;
		}
	}

	Statistics::Statistics(const ssi_char_t *file /*= 0*/) : _file(0), _res(0), _tmp_arr(0), _dim(0), _old_num_real(0), _running_stats(0)
	{
		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}
	}

	void Statistics::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num /*= 0*/, ssi_stream_t xtra_stream_in[] /*= 0*/)
	{
		if (xtra_stream_in_num > 0){
			ssi_err("x-tra streams not supported.");
		}

		if (_options.getSelection().empty()){
			ssi_err("no statistical function enabled.");
		}
		_dim = stream_in.dim;

		_res = new ssi_real_t[_dim * _options.getSelection().size()];
		_running_stats = new running_stat_t[_dim];
		provide_temp_array(stream_in.num_real);
	}

	void Statistics::transform(ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num /*= 0*/, ssi_stream_t xtra_stream_in[] /*= 0*/)
	{
		provide_temp_array(stream_in.num_real);

		calculate(stream_in);
		ssi_real_t * out_ptr = ssi_pcast(ssi_real_t, stream_out.ptr);

 		for (ssi_size_t i = 0; i < _dim * _options.getSelection().size(); i++)
		{
			out_ptr[i] = _res[i];
		}
	}

	void Statistics::transform_flush(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num /*= 0*/, ssi_stream_t xtra_stream_in[] /*= 0*/)
	{
		delete[] _res; _res = 0;
		delete[] _running_stats; _running_stats = 0;

		if (!_tmp_arr)
		{
			return;
		}

		for (ssi_size_t i = 0; i < stream_in.dim; i++)
		{
			if (_tmp_arr[i]){
				delete[] _tmp_arr[i]; _tmp_arr[i] = 0;
			}
		}
		delete[] _tmp_arr; _tmp_arr = 0;
	}

	void Statistics::calculate(ssi_stream_t &stream_in)
	{
		ssi_real_t * in_ptr = ssi_pcast(ssi_real_t, stream_in.ptr);

		for (ssi_size_t d_i = 0; d_i < _dim; d_i++)
		{
			for (ssi_size_t num_i = 0; num_i < stream_in.num; num_i++){
				_tmp_arr[d_i][num_i] = in_ptr[d_i*stream_in.num + num_i];
			}
		}

		calculate_running_stats(stream_in.num);

		std::vector<stat_fn> stat_fns = _options.getSelection();

		for (ssi_size_t d_i = 0; d_i < _dim; d_i++)
		{
			for (ssi_size_t f_i = 0; f_i < stat_fns.size(); f_i++){

				ssi_size_t r_idx = d_i * ssi_size_t (stat_fns.size()) + f_i;
				switch (stat_fns[f_i])
				{
				case STAT_KURTOSIS:
					_res[r_idx] = calculateKurtosis(d_i);
					break;
				case STAT_SKEWNESS:
					_res[r_idx] = calculateSkewness(d_i);
					break;
				case STAT_MEAN:
					_res[r_idx] = calculateMean(d_i);
					break;
				case STAT_STDDEV:
					_res[r_idx] = calculateStandardDeviation(d_i);
					break;
				case STAT_VARIANCE:
					_res[r_idx] = calculateVariance(d_i);
					break;
				case STAT_NUMBER_VALS:
					_res[r_idx] = calulateNumberVals(d_i);
					break;
				default:
					ssi_err("stat_fn not implemented");
				}
			}
		}
	}

	// algorithm see http://www.johndcook.com/blog/skewness_kurtosis/
	void Statistics::calculate_running_stats(ssi_size_t sample_num)
	{
		running_stat_t * rs;
		ssi_stat_ultra_precision_t delta, delta_n, delta_n2;
		ssi_size_t n1;
		ssi_stat_ultra_precision_t term1;

		ssi_real_t * curr_arr = 0;
		for (ssi_size_t dim_i = 0; dim_i < _dim; dim_i++)
		{
			curr_arr = _tmp_arr[dim_i];
			rs = &(_running_stats[dim_i]);

			rs->n = n1 = 0;
			rs->M2 = rs->M3 = rs->M4 = rs->mean = delta = delta_n = delta_n2 = term1 = 0.0;
			for (ssi_size_t i = 0; i < sample_num; i++){
				n1 = rs->n;
				rs->n++;
				delta = curr_arr[i] - rs->mean;
				delta_n = delta / rs->n;
				delta_n2 = delta_n * delta_n;
				term1 = delta * delta_n * n1;
				rs->mean += delta_n;
				rs->M4 += term1 * delta_n2 * (rs->n*rs->n - 3 * rs->n + 3) + 6 * delta_n2 * rs->M2 - 4 * delta_n * rs->M3;
				rs->M3 += term1 * delta_n * (rs->n - 2) - 3 * delta_n * rs->M2;
				rs->M2 += term1;
			}
		}
	}

	ssi_real_t Statistics::calculateKurtosis(ssi_size_t dim_idx)
	{
		running_stat_t * rs = &(_running_stats[dim_idx]);

		if (rs->M2 == 0.0){
			return 0.0f;
		}

		return ssi_cast(ssi_real_t, ssi_cast(ssi_stat_ultra_precision_t, rs->n)*rs->M4 / (rs->M2*rs->M2) - 3.0);
	}

	ssi_real_t Statistics::calculateSkewness(ssi_size_t dim_idx)
	{
		running_stat_t * rs = &(_running_stats[dim_idx]);
		if (rs->M2 == 0.0){
			return 0.0f;
		}
		return ssi_cast(ssi_real_t, sqrt(ssi_cast(ssi_stat_ultra_precision_t, rs->n)) * rs->M3 / pow(rs->M2, ssi_cast(ssi_stat_ultra_precision_t, 1.5)));
	}

	ssi_real_t Statistics::calculateStandardDeviation(ssi_size_t dim_idx)
	{
		return sqrt(calculateVariance(dim_idx));
	}

	ssi_real_t Statistics::calculateVariance(ssi_size_t dim_idx)
	{
		running_stat_t * rs = &(_running_stats[dim_idx]);
		if (rs->n == 1){
			return 0.0f;
		}
		return ssi_cast(ssi_real_t, rs->M2 / (rs->n - 1.0));
	}

	ssi_real_t Statistics::calculateMean(ssi_size_t dim_idx)
	{
		return ssi_cast(ssi_real_t, _running_stats[dim_idx].mean);
	}

	ssi_real_t Statistics::calulateNumberVals(ssi_size_t dim_idx)
	{
		return ssi_cast(ssi_real_t, _running_stats[dim_idx].n);
	}

	void Statistics::provide_temp_array(ssi_size_t num_real)
	{
		if (num_real == _old_num_real){
			return;
		}

		if (!_tmp_arr){
			_tmp_arr = new ssi_real_t *[_dim];
			for (ssi_size_t i = 0; i < _dim; i++){
				_tmp_arr[i] = 0;
			}
		}

		for (ssi_size_t i = 0; i < _dim; i++)
		{
			if (_tmp_arr[i]){
				delete[] _tmp_arr[i]; _tmp_arr[i] = 0;
			}
			_tmp_arr[i] = new ssi_real_t[num_real];
		}

		_old_num_real = num_real;
	}

	std::vector<stat_fn> Statistics::Options::getSelection()
	{
		_selection.clear();
		
		if (kurtosis)
		{
			_selection.push_back(STAT_KURTOSIS);
		}

		if (skewness)
		{
			_selection.push_back(STAT_SKEWNESS);
		}

		if (mean){
			_selection.push_back(STAT_MEAN);
		}

		if (stddev){
			_selection.push_back(STAT_STDDEV);
		}

		if (var){
			_selection.push_back(STAT_VARIANCE);
		}

		if (number_vals){
			_selection.push_back(STAT_NUMBER_VALS);
		}
		return _selection;
	}


}
