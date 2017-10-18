// GSRFeatures.cpp
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

#include "GSRFeatures.h"
#include "OverlapBuffer.h"
#include "../../signal/include/MvgAvgVar.h"
#include "base/Factory.h"

#include <ssiocv.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define FLT_BIG 1000000

#define STAT_MIN 0
#define STAT_MAX 1
#define STAT_AVG 2
#define STAT_VAR 3
#define	STAT_STD 4
#define STAT_NUM 5

#define TYPE_PEAK 0
#define TYPE_SLOPE 1
#define TYPE_DROP 2
#define TYPE_COMBO 3
#define TYPE_NUM 4

#define ATT_DURATION 0
#define ATT_AMPLITUDE 1
#define ATT_AREA 2
#define ATT_NUM 3

namespace ssi {

ssi_char_t *GSRFeatures::ssi_log_name = "gsrfeatures__";
int af = 0;

GSRFeatures::GSRFeatures (const ssi_char_t *file)
	: _file (0),
	_mvgvar (0),
	_findpeaks (0),
	_findslopes (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

GSRFeatures::~GSRFeatures () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void GSRFeatures::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in.type]);
	}

	if (stream_in.dim != 1) {
		ssi_err ("dimension > 1 not supported");
	}

	if (_options.print_features){
		printFeatures();
		_options.print_features = false;
	}

	MvgAvgVar *mvgvar = ssi_pcast (MvgAvgVar, Factory::Create (MvgAvgVar::GetCreateName (), 0, false));
	mvgvar->getOptions ()->format = MvgAvgVar::ALL;
	mvgvar->getOptions ()->win = _options.winsize;
	mvgvar->getOptions ()->method = MvgAvgVar::SLIDING;
	_mvgvar = mvgvar;
	ssi_stream_init (_var_stream, 0, _mvgvar->getSampleDimensionOut (stream_in.dim), _mvgvar->getSampleBytesOut (stream_in.byte), _mvgvar->getSampleTypeOut (stream_in.type), stream_in.sr);
	_mvgvar->transform_enter (stream_in, _var_stream);

	GSRFindPeaks::Params peaks_params;
	peaks_params.sr = stream_in.sr;
	peaks_params.maxdur = _options.peakmaxd;
	peaks_params.mindur = _options.peakmind;
	peaks_params.nstd = _options.peaknstd;
	_findpeaks = new GSRFindPeaks (this, peaks_params);
	_findpeaks->setLogLevel (ssi_log_level);

	GSRFindSlopes::Params slope_params;
	slope_params.sr = stream_in.sr;
	slope_params.maxdur = _options.slopemaxd;
	slope_params.mindur = _options.slopemind;
	slope_params.nstd = _options.slopenstd;
	_findslopes = new GSRFindSlopes (this, slope_params);
	_findslopes->setLogLevel (ssi_log_level);	
}

void GSRFeatures::transform( ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {
	
	peaks.clear();
	slopes.clear();
	drops.clear();
	psd_combo.clear();

	ssi_size_t n = stream_in.num;
	ssi_time_t sr = stream_in.sr;

	ITransformer::info tinfo;
	tinfo.delta_num = n;
	tinfo.time = info.time;

	ssi_stream_adjust (_var_stream, n);
	_mvgvar->transform (tinfo, stream_in, _var_stream);
	ssi_real_t *var = ssi_pcast (ssi_real_t, _var_stream.ptr);

	ssi_real_t *gsr = ssi_pcast (ssi_real_t, stream_in.ptr);
	_findpeaks->process (n, sr, gsr, var);

	gsr = ssi_pcast (ssi_real_t, stream_in.ptr);
	_findslopes->process (n, sr, gsr, var);

	//combination of all 3 types (peak/slope/drop)
	psd_combo.insert(psd_combo.end(), peaks.begin(), peaks.end());
	psd_combo.insert(psd_combo.end(), slopes.begin(), slopes.end());
	psd_combo.insert(psd_combo.end(), drops.begin(), drops.end());


	float f_peak = FLT_MIN;
	float f_si_corr = 0;
	float f_p2pmv = 0; //peak to peak mean value

	if (_options.new_features) {
		

		ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);

		ssi_real_t *ptr_ref = ssi_pcast(ssi_real_t, _reference_stream->ptr); //TODO check if set


		for (int nsamp = 0; nsamp < stream_in.num; nsamp++) {
			if (ptr_in[nsamp] > f_peak) {
				f_peak = ptr_in[nsamp];
			}
		}
		if (f_peak < (FLT_MIN / 2)) f_peak = 0;

		float* xc_in = new float[stream_in.num]();
		float* xc_ref = new float[stream_in.num]();

		float mean_in = 0;
		float mean_ref = 0;

		for (int i = 0; i < stream_in.num; i++) {
			mean_in += ptr_in[i];
			mean_ref += ptr_ref[i];
		}

		mean_in /= stream_in.num;
		mean_ref /= stream_in.num; 

		for (int i = 0; i < stream_in.num; i++) {
			xc_in[i] = ptr_in[i] - mean_in;
			xc_ref[i] = ptr_ref[i] - mean_ref;
		}

		cv::Mat xc_mat(stream_in.num, 2, CV_32F);

		for (int i = 0; i < (stream_in.num); i++) {
			xc_mat.at<float>(cv::Point2i(0, i)) = xc_ref[i];
			xc_mat.at<float>(cv::Point2i(1, i)) = xc_in[i];
		}

		cv::Mat c_mat;
		cv::Mat d_mat(2, 1, CV_32F);

		c_mat = (xc_mat.t() * xc_mat);
		c_mat /= (float)(stream_in.num - 1);

		d_mat.at<float>(cv::Point2i(0, 0)) = sqrtf(c_mat.at<float>(cv::Point2i(0, 0)));
		d_mat.at<float>(cv::Point2i(0, 1)) = sqrtf(c_mat.at<float>(cv::Point2i(1, 1)));

		cv::Mat dxdt = d_mat * d_mat.t();

		for (int i = 0; i < c_mat.cols; i++) {
			for (int j = 0; j < c_mat.rows; j++) {
				c_mat.at<float>(cv::Point2i(i, j)) /= dxdt.at<float>(cv::Point2i(i, j));
			}
		}

		f_si_corr = c_mat.at<float>(cv::Point2i(0, 1));

		delete[] xc_in;
		delete[] xc_ref;

		//peak to peak menan value

		std::vector<int> minima;
		std::vector<int> maxima;
		int state = GSR_NONE;

		ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
		float ptr_prev = *ptr_in;
		for (int nsamp = 0; nsamp < stream_in.num; nsamp++) {

			if (*ptr_in < ptr_prev) {
				if (state == GSR_RISING) { //peak
					maxima.push_back(*ptr_in);
				}
				state = GSR_FALLING;
			}
			else if (*ptr_in > ptr_prev) {
				if (state == GSR_FALLING) { //trough
					minima.push_back(*ptr_in);
				}
				state = GSR_RISING;
			}
						
			ptr_prev = *ptr_in;
			ptr_in++;

		}

		float avg_maxima = 0.0;
		float avg_minima = 0.0;

		for (int i = 0; i < maxima.size(); i++) avg_maxima += maxima[i];
		for (int i = 0; i < minima.size(); i++) avg_minima += minima[i];

		if (maxima.size() > 0) avg_maxima /= ((float)maxima.size());
		if (minima.size() > 0) avg_minima /= ((float) minima.size());
		
        f_p2pmv = std::fabs(avg_maxima - avg_minima);

	}
	
	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);

	//number
	*ptr_out = ((float)peaks.size());
	ptr_out++;
	*ptr_out = ((float)slopes.size());
	ptr_out++;
	*ptr_out = ((float)drops.size());
	ptr_out++;
	*ptr_out = ((float)psd_combo.size());
	ptr_out++;

	//duration min
	*ptr_out = getMin(peaks, ATT_DURATION);
	ptr_out++;
	*ptr_out = getMin(slopes, ATT_DURATION);
	ptr_out++;
	*ptr_out = getMin(drops, ATT_DURATION);
	ptr_out++;
	*ptr_out = getMin(psd_combo, ATT_DURATION);
	ptr_out++;

	//amplitude min
	*ptr_out = getMin(peaks, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getMin(slopes, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getMin(drops, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getMin(psd_combo, ATT_AMPLITUDE);
	ptr_out++;

	//area min
	*ptr_out = getMin(peaks, ATT_AREA);
	ptr_out++;
	*ptr_out = getMin(slopes, ATT_AREA);
	ptr_out++;
	*ptr_out = getMin(drops, ATT_AREA);
	ptr_out++;
	*ptr_out = getMin(psd_combo, ATT_AREA);
	ptr_out++;

	//duration max
	*ptr_out = getMax(peaks, ATT_DURATION);
	ptr_out++;
	*ptr_out = getMax(slopes, ATT_DURATION);
	ptr_out++;
	*ptr_out = getMax(drops, ATT_DURATION);
	ptr_out++;
	*ptr_out = getMax(psd_combo, ATT_DURATION);
	ptr_out++;

	//amplitude max
	*ptr_out = getMax(peaks, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getMax(slopes, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getMax(drops, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getMax(psd_combo, ATT_AMPLITUDE);
	ptr_out++;

	//area max
	*ptr_out = getMax(peaks, ATT_AREA);
	ptr_out++;
	*ptr_out = getMax(slopes, ATT_AREA);
	ptr_out++;
	*ptr_out = getMax(drops, ATT_AREA);
	ptr_out++;
	*ptr_out = getMax(psd_combo, ATT_AREA);
	ptr_out++;

	//duration avg
	*ptr_out = getAvg(peaks, ATT_DURATION);
	ptr_out++;
	*ptr_out = getAvg(slopes, ATT_DURATION);
	ptr_out++;
	*ptr_out = getAvg(drops, ATT_DURATION);
	ptr_out++;
	*ptr_out = getAvg(psd_combo, ATT_DURATION);
	ptr_out++;

	//amplitude avg
	*ptr_out = getAvg(peaks, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getAvg(slopes, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getAvg(drops, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getAvg(psd_combo, ATT_AMPLITUDE);
	ptr_out++;

	//area avg
	*ptr_out = getAvg(peaks, ATT_AREA);
	ptr_out++;
	*ptr_out = getAvg(slopes, ATT_AREA);
	ptr_out++;
	*ptr_out = getAvg(drops, ATT_AREA);
	ptr_out++;
	*ptr_out = getAvg(psd_combo, ATT_AREA);
	ptr_out++;

	//duration var
	*ptr_out = getVar(peaks, ATT_DURATION);
	ptr_out++;
	*ptr_out = getVar(slopes, ATT_DURATION);
	ptr_out++;
	*ptr_out = getVar(drops, ATT_DURATION);
	ptr_out++;
	*ptr_out = getVar(psd_combo, ATT_DURATION);
	ptr_out++;

	//amplitude var
	*ptr_out = getVar(peaks, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getVar(slopes, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getVar(drops, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getVar(psd_combo, ATT_AMPLITUDE);
	ptr_out++;

	//area var
	*ptr_out = getVar(peaks, ATT_AREA);
	ptr_out++;
	*ptr_out = getVar(slopes, ATT_AREA);
	ptr_out++;
	*ptr_out = getVar(drops, ATT_AREA);
	ptr_out++;
	*ptr_out = getVar(psd_combo, ATT_AREA);
	ptr_out++;

	//duration stdev
	*ptr_out = getStDev(peaks, ATT_DURATION);
	ptr_out++;
	*ptr_out = getStDev(slopes, ATT_DURATION);
	ptr_out++;
	*ptr_out = getStDev(drops, ATT_DURATION);
	ptr_out++;
	*ptr_out = getStDev(psd_combo, ATT_DURATION);
	ptr_out++;

	//amplitude stdev
	*ptr_out = getStDev(peaks, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getStDev(slopes, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getStDev(drops, ATT_AMPLITUDE);
	ptr_out++;
	*ptr_out = getStDev(psd_combo, ATT_AMPLITUDE);
	ptr_out++;

	//area stdev
	*ptr_out = getStDev(peaks, ATT_AREA);
	ptr_out++;
	*ptr_out = getStDev(slopes, ATT_AREA);
	ptr_out++;
	*ptr_out = getStDev(drops, ATT_AREA);
	ptr_out++;
	*ptr_out = getStDev(psd_combo, ATT_AREA);
	ptr_out++;

	if (_options.new_features) {
		*ptr_out = f_peak;
		ptr_out++;
		*ptr_out = f_si_corr;
		ptr_out++;
		*ptr_out = f_p2pmv; //peak to peak mean value
	}



}

float GSRFeatures::getMin(std::vector<peakslopedrop> in, ssi_size_t attribute){
	float r = FLT_BIG;

	for (int i = 0; i < in.size(); i++){
		float tmp = getAttribute(in[i], attribute);
		if (tmp < r){
			r = tmp;
		}
	}

	if (r >((float)FLT_BIG - 1.0f) || r < ((float)-FLT_BIG + 1.0f)) r = 0;
	return r;
}

float GSRFeatures::getMax(std::vector<peakslopedrop> in, ssi_size_t attribute){
	float r = -FLT_BIG;

	for (int i = 0; i < in.size(); i++){
		float tmp = getAttribute(in[i], attribute);
		if (tmp > r){
			r = tmp;
		}
	}

	if (r >((float)FLT_BIG - 1.0f) || r < ((float)-FLT_BIG + 1.0f)) r = 0;
	return r;
}

float GSRFeatures::getAvg(std::vector<peakslopedrop> in, ssi_size_t attribute){
	float r = 0;

	if (in.size() != 0){
		for (int i = 0; i < in.size(); i++){
			float tmp = getAttribute(in[i], attribute);
			r += tmp;
		}
		r /= (float) in.size();
	}
	return r;
}

float GSRFeatures::getVar(std::vector<peakslopedrop> in, ssi_size_t attribute){
	float r = 0;
	float avg = 0;

	if (in.size() != 0){
		for (int i = 0; i < in.size(); i++){
			float tmp = getAttribute(in[i], attribute);
			avg += tmp;
		}
		avg /= ((float) in.size());

		for (int i = 0; i < in.size(); i++){
			float tmp2 = getAttribute(in[i], attribute);
			r += ((avg - tmp2) * (avg - tmp2));
		}

		if (avg != 0){
			r /= avg;
		}
		else{
			r = 0;
		}
		
	}
	
	return r;
}

float GSRFeatures::getStDev(std::vector<peakslopedrop> in, ssi_size_t attribute){
	float r = 0;
	float var = 0;
	float avg = 0;

	if (in.size() != 0){
		for (int i = 0; i < in.size(); i++){
			float tmp = getAttribute(in[i], attribute);
			avg += tmp;
		}
		avg /= (float) in.size();

		for (int i = 0; i < in.size(); i++){
			float tmp2 = getAttribute(in[i], attribute);
			var += ((avg - tmp2) * (avg - tmp2));
		}

		if (avg != 0){
			var /= avg;
            r = std::sqrt(var);
		}
		else{
			r = 0;
		}
	}

	return r;
}

float GSRFeatures::getAttribute(peakslopedrop e, ssi_size_t att){

	if (att == ATT_DURATION) return (e.to - e.from);
	if (att == ATT_AMPLITUDE) return e.amplitude;
	if (att == ATT_AREA) return e.area;

	ssi_wrn("[GSRFeatures] Attribute %i does not exist!\n", att);
	getchar();
}

void GSRFeatures::peak (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area) {

		if(_options.print_info) ssi_print("peak [%.2f %.2f]s   amp: %.2f area: %.2f\n", from, to, amplitude, area);
		
		struct peakslopedrop tmp;
		tmp.from = from;
		tmp.to = to;
		tmp.amplitude = amplitude;
		tmp.area = area;
		peaks.push_back(tmp);

}

void GSRFeatures::slope (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area, ssi_real_t gradient) {

	struct peakslopedrop tmp;
	tmp.from = from;
	tmp.to = to;
	tmp.amplitude = amplitude;
	tmp.area = area;

	if (gradient > 0) {
		if (_options.print_info) ssi_print("slope [%.2f - %.2f]s   amp: %.2f area: %.2f\n", from, to, amplitude, area);
		slopes.push_back(tmp);
	} else {
		if (_options.print_info) ssi_print("drop [%.2f - %.2f]s   amp: %.2f area: %.2f\n", from, to, amplitude, area);
		drops.push_back(tmp);
	}
}

void GSRFeatures::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete _findpeaks; _findpeaks = 0;
	delete _findslopes; _findslopes = 0;

	_mvgvar->transform_flush (stream_in, _var_stream);
	ssi_stream_destroy (_var_stream);
	delete _mvgvar; _mvgvar = 0;
}

void GSRFeatures::printFeatures(){

	af = 0;
	ssi_print("\n[GSRFeatures] Activated Features:\n");

	if (_options.f_number){
		if (_options.f_type_peaks) ssi_print(" %i: n peaks\n", af++);
		if (_options.f_type_slopes) ssi_print(" %i: n slopes\n", af++);
		if (_options.f_type_drops) ssi_print(" %i: n drops\n", af++);
		if (_options.f_type_combo) ssi_print(" %i: n combos\n", af++);
	}

	for (int i = 0; i < STAT_NUM; i++){
		printFeatureStat(i);
	}

	ssi_print("\n");
}

void GSRFeatures::printFeatureStat(int st){
	std::string __stat;
	
	if (st == STAT_MIN && _options.f_stat_min) __stat = "min";
	if (st == STAT_MAX && _options.f_stat_max) __stat = "max";
	if (st == STAT_AVG && _options.f_stat_avg) __stat = "avg";
	if (st == STAT_VAR && _options.f_stat_var) __stat = "var";
	if (st == STAT_STD && _options.f_stat_std) __stat = "std";
	
	for (int i = 0; i < ATT_NUM; i++){
		printFeatureAttribute(i, __stat);
	}
}

void GSRFeatures::printFeatureAttribute(int at, std::string str_stat){
	std::string _att;

	if (at == ATT_DURATION && _options.f_att_duration) _att =   "duration ";
	if (at == ATT_AMPLITUDE && _options.f_att_amplitude) _att = "amplitude";
	if (at == ATT_AREA && _options.f_att_area) _att =           "area     ";

	for (int i = 0; i < TYPE_NUM; i++){
		printFeatureType(i, str_stat, _att);
	}
}

void GSRFeatures::printFeatureType(int ft, std::string str_stat, std::string str_att){
	std::string _type;
	
	if (ft == TYPE_PEAK && _options.f_type_peaks) _type =   "peak ";
	if (ft == TYPE_SLOPE && _options.f_type_slopes) _type = "slope";
	if (ft == TYPE_DROP && _options.f_type_drops) _type =   "drop ";
	if (ft == TYPE_COMBO && _options.f_type_combo) _type =  "combo";

	if (af < 10){
		ssi_print(" %i: %s %s %s\n", af++, str_stat.c_str(), _type.c_str(), str_att.c_str())
	}
	else{
		ssi_print("%i: %s %s %s\n", af++, str_stat.c_str(), _type.c_str(), str_att.c_str())
	}
}




}
