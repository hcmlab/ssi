// EMGFeaturesTime.cpp
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

#include "EMGFeaturesTime.h"
#include "base/Factory.h"
#include "../../signal/include/Spectrogram.h"
#include "frame/include/Selector.h"

#include <stdio.h>
#include "ssiocv.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define FLT_BIG 1000000

#define NOISE_NONE 0
#define NOISE_DIM 1
#define NOISE_XTRA 2

ssi_size_t noise_source = NOISE_NONE;

namespace ssi {

ssi_char_t *EMGFeaturesTime::ssi_log_name = "EMGFeaturesTime__";

ssi_real_t tf_mav_prev = 0;

EMGFeaturesTime::EMGFeaturesTime (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	tf_mav_prev = 0;
}

EMGFeaturesTime::~EMGFeaturesTime () {


	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void EMGFeaturesTime::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in.type]);
	}
		
	if (_options.print_features){
		printFeatures();
		_options.print_features = false;
	}

	noise_source = NOISE_NONE;
	if (stream_in.dim == 2){
		noise_source = NOISE_DIM;
	}
	else if (xtra_stream_in_num == 1) {
		if (xtra_stream_in != nullptr) {
			if (xtra_stream_in[0].dim == 1) {
				noise_source = NOISE_XTRA;
			}
		}

	}


	//if (noise_source == NOISE_NONE) ssi_wrn("[Time Features] No noise level received! (dim: %i, x num: %i)\n", stream_in.dim, xtra_stream_in_num);

	if (_options.hemg_segments > 0){

		Selector *sel = ssi_pcast(Selector, Factory::Create(Selector::GetCreateName(), 0, false));
		sel->getOptions()->set(0);
		_sel = sel;
		ssi_stream_init(_sel_stream, 0, _sel->getSampleDimensionOut(stream_in.dim), _sel->getSampleBytesOut(stream_in.byte), _sel->getSampleTypeOut(stream_in.type), stream_in.sr);
		_sel->transform_enter(stream_in, _sel_stream);

		Spectrogram *spec = ssi_pcast(Spectrogram, Factory::Create(Spectrogram::GetCreateName(), 0, false));
		spec->getOptions()->nbanks = _options.hemg_segments;
		_spec = spec;
		ssi_stream_init(_spec_stream, 0, _spec->getSampleDimensionOut(_sel_stream.dim), _spec->getSampleBytesOut(_sel_stream.byte), _spec->getSampleTypeOut(_sel_stream.type), stream_in.sr);
		_spec->transform_enter(stream_in, _spec_stream);
	}
}


//based on https://www.researchgate.net/publication/268363764_Stages_for_Developing_Control_Systems_using_EMG_and_EEG_Signals_A_survey
void EMGFeaturesTime::transform(ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {

	ssi_real_t weight1 = 0; //for mmav1
	ssi_real_t weight2 = 0; //for mmav2
	ssi_real_t mean = 0; //for var
	ssi_real_t ptr_in_prev = 0; //for WL
	ssi_real_t ptr_in_prev_prev = 0; //for WL
	ssi_real_t noise_level = FLT_EPSILON; //for ZC, SSC

	ssi_real_t *xtra_ptr_in = nullptr;
	if (noise_source == NOISE_XTRA){
		if (xtra_stream_in_num == 1){
			if (xtra_stream_in[0].dim == 1){
				xtra_ptr_in = ssi_pcast(ssi_real_t, xtra_stream_in[0].ptr);
			}
		}
	}

	ssi_real_t tf_iemg = 0;
	ssi_real_t tf_mav = 0;
	ssi_real_t tf_mmav1 = 0;
	ssi_real_t tf_mmav2 = 0;
	ssi_real_t tf_mavs = 0;
	ssi_real_t tf_rms = 0;
	ssi_real_t tf_var = 0;
	ssi_real_t tf_wl = 0;
	ssi_real_t tf_zc = 0;
	ssi_real_t tf_ssc = 0;
	ssi_real_t tf_wamp = 0;
	ssi_real_t tf_ssi = 0;

	ssi_real_t sin = (float)stream_in.num;
	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	for (int nsamp = 0; nsamp < stream_in.num; nsamp++){
				
		tf_iemg += fabsf(*ptr_in);

		if (nsamp < (sin * 0.25)){
			weight1 = 0.5;
			weight2 = (float)(4 * nsamp) / sin;
		}
		else if (nsamp <= sin * 0.75){
			weight1 = 1.0;
			weight2 = 1.0;
		}
		else {
			weight1 = 0.5;
			weight2 = 4.0f * (float)(nsamp - stream_in.num) / sin;
		}

		tf_mmav1 += weight1 * fabsf(*ptr_in);
		tf_mmav2 += weight2 * fabsf(*ptr_in);
		tf_rms += powf(*ptr_in, 2);
		mean += *ptr_in;

		tf_wl += fabsf(*ptr_in - ptr_in_prev);

		if (noise_source == NOISE_XTRA){
			noise_level = *xtra_ptr_in;
			xtra_ptr_in++;
		}
		else if (noise_source == NOISE_DIM){
			noise_level = *(ptr_in + 1);
		}

		if ((ptr_in_prev > 0 && *ptr_in < 0) || (ptr_in_prev < 0 && *ptr_in > 0) ){
			if (fabsf(ptr_in_prev - *ptr_in) >= noise_level){
				tf_zc += 1;
			}
		}

		if ((ptr_in_prev > ptr_in_prev_prev && ptr_in_prev > *ptr_in) || (ptr_in_prev < ptr_in_prev_prev && ptr_in_prev < *ptr_in)){
			if ((fabsf(ptr_in_prev - *ptr_in) >= noise_level) || (fabsf(ptr_in_prev - ptr_in_prev_prev) >= noise_level)){
				tf_ssc += 1;
			}
		}

		if (fabsf(ptr_in_prev - *ptr_in) > noise_level){
			tf_wamp += 1;
		}

		tf_ssi += powf(*ptr_in, 2);

		ptr_in_prev_prev = ptr_in_prev;
		ptr_in_prev = *ptr_in;
		ptr_in += stream_in.dim;
	}

	tf_mav = tf_iemg / (float)stream_in.num;
	tf_mmav1 /= sin;
	tf_mmav2 /= sin;

	tf_mavs = tf_mav_prev - tf_mav;
	tf_mav_prev = tf_mav;

	tf_rms /= sin;
	tf_rms = sqrtf(tf_rms);

	mean /= sin;

	ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	for (int nsamp = 0; nsamp < stream_in.num; nsamp++){
		tf_var += powf(*ptr_in - mean, 2);
		ptr_in += stream_in.dim;
	}

	tf_var /= sin;

	ssi_real_t *ptr_sel;
	ssi_real_t *ptr_spec;
	if (_options.hemg_segments > 0){

		//histogram
		ITransformer::info tinfo;
		tinfo.delta_num = stream_in.num;
		tinfo.time = info.time;

		ssi_stream_adjust(_sel_stream, stream_in.num);
		_sel->transform(tinfo, stream_in, _sel_stream);

		ssi_stream_adjust(_spec_stream, _sel_stream.num);
		_spec->transform(tinfo, _sel_stream, _spec_stream);
		ptr_spec = ssi_pcast(ssi_real_t, _spec_stream.ptr);

		if (_options.hemg_segments != _spec_stream.dim){
			ssi_wrn("[EMG time features] expected %i hemg segments, spectogram dim is %i\n", _options.hemg_segments, _spec_stream.dim);
			getchar();
		}
	}


	ssi_real_t tf_peak = FLT_MIN;
	ssi_real_t tf_me = 0; //standard deviation of the mean values
	ssi_real_t tf_ldf1 = 0;
	ssi_real_t tf_ldf2 = 0;
	ssi_real_t tf_mutinfo = 0;
	ssi_real_t tf_si_corr = 0;

	if (_options.new_features) {

		//ssi_real_t* ptr_in;
		if (stream_in.dim > 1) {
			ssi_real_t* ptr_in_original = ssi_pcast(ssi_real_t, stream_in.ptr);
			ptr_in = new ssi_real_t[stream_in.num];
			for (int i = 0; i < stream_in.num; i++) ptr_in[i] = ptr_in_original[i * stream_in.dim];
		}
		else {
			ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
		}


		for (int nsamp = 0; nsamp < stream_in.num; nsamp++) {
			if (ptr_in[nsamp] > tf_peak) {
				tf_peak = ptr_in[nsamp];
			}
		}
		if (tf_peak < (FLT_MIN/2)) tf_peak = 0;

		int n_me = (float)stream_in.num / (float)stream_in.sr; //1 second chunks

		if (n_me <= 0) n_me = 1;

		ssi_real_t* means = new ssi_real_t[n_me]();
		int n_mean_samples = stream_in.sr;

		for (int nmeans = 0; nmeans < n_me; nmeans++) {
			for (int nsamp = (nmeans * n_mean_samples); nsamp < ((nmeans+1) * n_mean_samples); nsamp++) {
				means[nmeans] += ptr_in[nsamp];
			}
			means[nmeans] /= n_mean_samples;
		}

		tf_me = getStDev(means, n_me);

		delete[] means;

		//features_linerarity
		float* xshift = new float[stream_in.num];
		float* xfit = new float[stream_in.num];

		//TODO ---
		/*int n_xshift = 10;
		xshift = new float[n_xshift];
		for (int i = 0; i < n_xshift; i++) xshift[i] = (float)(i + 2);
		xshift[n_xshift - 1] = 1.0;

		int n_y = 10;
		stream_in.num = n_y;
		ptr_in = new float[n_y];
		for (int i = 0; i < n_y; i++) ptr_in[i] = (float)(i + 1);*/
		// ---

		for (int i = 1; i <= 2; i++) {
			for (int j = 0; j < stream_in.num; j++) {
				if ((j+i) < stream_in.num) {
					xshift[j] = ptr_in[j + i];
				}
				else {
					xshift[j] = ptr_in[(j+i) - stream_in.num];
				}
			}

			int n_coefs = 2;
			float* coefs = new float[n_coefs];
			polyfit(xshift, stream_in.num, ptr_in, stream_in.num, coefs);
			polyval(coefs, n_coefs, xshift, stream_in.num, xfit);

			float ssi = 0.0;
			for (int j = 0; j < stream_in.num; j++) {
				ssi += ((ptr_in[j] - xfit[j]) * (ptr_in[j] - xfit[j]));
			}

			float in_mean = 0.0;
			for (int j = 0; j < stream_in.num; j++) {
				in_mean += ptr_in[j];
			}
			in_mean /= (float)stream_in.num;

			float ss0_l = 0.0;
			for (int j = 0; j < stream_in.num; j++) {
				ss0_l += ((ptr_in[j] - in_mean) * (ptr_in[j] - in_mean));
			}

			float ri = (ss0_l - ssi) / ss0_l;

			int ind_min = 0;
			int ind_max = 0;
			float val_min = FLT_MAX;
			float val_max = FLT_MIN;

			for (int j = 0; j < stream_in.num; j++) {
				if (ptr_in[j] < val_min) { val_min = ptr_in[j]; ind_min = j; }
				if (ptr_in[j] > val_max) { val_max = ptr_in[j]; ind_max = j; }
			}

			float sign = 0;
			if ((xfit[ind_max] - xfit[ind_min]) < 0) sign = -1.0;
			else if ((xfit[ind_max] - xfit[ind_min]) > 0) sign = 1.0;

			float ldf = sign * sqrtf(ri);

			if (i == 1) tf_ldf1 = ldf;
			else if (i == 2) tf_ldf2 = ldf;

			delete[] coefs;
		}

		delete[] xshift;
		delete[] xfit;


		// similarity: correlation coeffiicent

		ssi_real_t* ptr_ref;
		if (_reference_stream->dim > 1) {
			ssi_real_t* ptr_ref_original = ssi_pcast(ssi_real_t, _reference_stream->ptr);
			ptr_ref = new ssi_real_t[_reference_stream->num];
			for (int i = 0; i < _reference_stream->num; i++) ptr_ref[i] = ptr_ref_original[i * _reference_stream->dim];
		}
		else {
			ptr_ref = ssi_pcast(ssi_real_t, _reference_stream->ptr);
		}

		//TODO ---
		/*int n_in = 10;
		ptr_in = new float[n_in];
		for (int i = 0; i < n_in; i++) ptr_in[i] = (float)((i % 5) + 4);

		int n_ref = 10;
		ptr_ref = new float[n_ref];
		for (int i = 0; i < n_ref; i++) ptr_ref[i] = (float)((i % 5) + 5);

		stream_in.num = n_in;*/
		// ---

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

		tf_si_corr = c_mat.at<float>(cv::Point2i(0, 1));

		delete[] xc_in;
		delete[] xc_ref;

		//similarity: mutual information


		float* unqx = new float[stream_in.num]();
		float* unqy = new float[stream_in.num]();
		
		std::vector<i2> map;
		std::vector<int> freqxy;
		std::vector<float> join;

		int n_unqx = unique(ptr_ref, unqx, stream_in.num);
		int n_unqy = unique(ptr_in, unqy, stream_in.num);

		float* probx = new float[n_unqx]();
		float* proby = new float[n_unqy]();

		for (int i = 0; i < stream_in.num; i++) {
			int indx = findfirst(unqx, n_unqx, ptr_ref[i]);
			int indy = findfirst(unqy, n_unqy, ptr_in[i]);

			std::vector<int> tx = find(map, false, indx);
			std::vector<int> ty = find(map, true, indy);

			std::vector<int> inte = intersect(tx, ty);

			if (tx.size() > 0 && ty.size() > 0 && inte.size() > 0) {
				int ind = inte[inte.size() - 1];
				freqxy[ind] = (freqxy[ind] + 1);
			}
			else {
				freqxy.push_back(1);
				i2 tmp;
				tmp.x = indx;
				tmp.y = indy;
				map.push_back(tmp);
			}

			probx[indx] = probx[indx] + 1.0;
			proby[indy] = proby[indy] + 1.0;
		}

		for (int i = 0; i < n_unqx; i++) probx[i] = probx[i] / (float)stream_in.num;
		for (int i = 0; i < n_unqy; i++) proby[i] = proby[i] / (float)stream_in.num;
		for (int i = 0; i < freqxy.size(); i++) join.push_back(freqxy[i] / (float)stream_in.num);

		for (int i = 0; i < join.size(); i++) {
			int ind1 = map[i].x;
			int ind2 = map[i].y;

			if (!feq(join[i], 0.0) && !feq((probx[ind1] * proby[ind2]), 0.0)) {
				tf_mutinfo += join[i] * std::log2f(join[i] / (probx[ind1] * proby[ind2]));
			}
		}

		delete[] unqx;
		delete[] unqy;
		delete[] probx;
		delete[] proby;

		if (stream_in.dim > 1) delete[] ptr_in;
		if (_reference_stream->dim > 1) delete[] ptr_ref;


		ssi_print("\n");

	}

	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	*ptr_out = tf_iemg; //IEMG: Integrated EMG (sum of absolutes)
	ptr_out++;
	*ptr_out = tf_mav; //MAV: Mean Absolute Value
	ptr_out++;
	*ptr_out = tf_mmav1; //MMAV1: Modified Mean Absolute Value 1
	ptr_out++;
	*ptr_out = tf_mmav2; //MMAV2: Modified Mean Absolute Value 2
	ptr_out++;
	*ptr_out = tf_mavs; //MAVS: Modified Mean Absolute Value Slope
	ptr_out++;
	*ptr_out = tf_rms; //RMS: Root Mean Square
	ptr_out++;
	*ptr_out = tf_var; //VAR: Variance
	ptr_out++;
	*ptr_out = tf_wl; //WL: Waveform Length
	ptr_out++;
	*ptr_out = tf_zc; //ZC: Zero Crossings
	ptr_out++;
	*ptr_out = tf_ssc; //SSC: Slope Sign Changes
	ptr_out++;
	*ptr_out = tf_wamp; //WAMP: Willison Amplitude
	ptr_out++;
	*ptr_out = tf_ssi; //SSI: Simple Square Integral (energy)
	ptr_out++;

	for (int i = 0; i < _options.hemg_segments; i++){
		*ptr_out = *ptr_spec; //HEMG: Histogram of EMG
		ptr_spec++;
		ptr_out++;
	}

	if (_options.new_features) {

		*ptr_out = tf_peak;
		ptr_out++;
		*ptr_out = tf_me;
		ptr_out++;
		*ptr_out = tf_ldf1;
		ptr_out++;
		*ptr_out = tf_ldf2;
		ptr_out++;
		*ptr_out = tf_mutinfo;
		ptr_out++;
		*ptr_out = tf_si_corr;
	}

}

std::vector<int> EMGFeaturesTime::intersect(std::vector<int> a, std::vector<int> b) {

	std::vector<int> r;

	for (int i = 0; i < a.size(); i++) {
		if (!(std::find(r.begin(), r.end(), a[i]) != r.end())) { // r does not contain a[i]
			if (std::find(b.begin(), b.end(), a[i]) != b.end()) { // b contains a[i]
				r.push_back(a[i]);
			}
		}
	}

	return r;
}


std::vector<int> EMGFeaturesTime::find(std::vector<EMGFeaturesTime::i2> map, bool use_y, int ind) {

	std::vector<int> r;

	for (int i = 0; i < map.size(); i++) {
		int m = (use_y ? map[i].y : map[i].x);
		if (feq(m, ind)) r.push_back(i);
	}

	return r;
}


int EMGFeaturesTime::findfirst(float* in, int n_in, float val) {
	for (int i = 0; i < n_in; i++) {
		if (feq(in[i], val)) {
			return i;
		}
	}

	return -1;
}

bool EMGFeaturesTime::feq(float a, float b) { //float equals
    if (std::abs(a - b) < FLT_EPSILON) return true;
	return false;
}

int EMGFeaturesTime::unique(float* ptr, float* unq_ptr, int n) {

	int n_unq = 0;

	for (int i = 0; i < n; i++) {

		bool found = false;
		for (int j = 0; j < n_unq; j++) {
			if (feq(ptr[i], unq_ptr[j])) {
				found = true;
				j = n;
			}
		}

		if (!found) {
			unq_ptr[n_unq] = ptr[i];
			n_unq++;
		}
	}

	return n_unq;
}

void EMGFeaturesTime::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete _spec;
}

void EMGFeaturesTime::printFeatures(){

	ssi_print("\n[EMG Features Time] Activated Features:\n");
	ssi_print(" 0: iemg\n");
	ssi_print(" 1: mav\n");
	ssi_print(" 2: mmav1\n");
	ssi_print(" 3: mmav2\n");
	ssi_print(" 4: mavs\n");
	ssi_print(" 5: rms\n");
	ssi_print(" 6: var\n");
	ssi_print(" 7: wl\n");
	ssi_print(" 8: zc\n");
	ssi_print(" 9: ssc\n");
	ssi_print("10: wamp\n");
	ssi_print("11: ssi\n");

	for (int i = 0; i < _options.hemg_segments; i++){
		ssi_print("%i: hemg%i\n", i + 12, i + 1);
	}

	ssi_print("\n");

}

float EMGFeaturesTime::getStDev(float* in, int n_in) {
	float r = 0;
	float var = 0;
	float avg = 0;

	if (n_in != 0) {
		for (int i = 0; i < n_in; i++) {
			avg += in[i];
		}
		avg /= (float)n_in;

		for (int i = 0; i < n_in; i++) {
			var += ((avg - in[i]) * (avg - in[i]));
		}

		if (avg != 0) {
			var /= avg;
            r = std::sqrt(var);
		}
		else {
			r = 0;
		}
	}

	return r;
}

void EMGFeaturesTime::polyval(float* coefs, int n_coefs, float* xshift, int n_xshift, float* xfit) {
	
	int n_xfit = n_xshift;
	for (int i = 0; i < n_xfit; i++) xfit[i] = coefs[0];

	for (int c= 1; c < n_coefs; c++) {
		for (int i = 0; i < n_xfit; i++) {
			xfit[i] = xshift[i] * xfit[i] + coefs[c];
		}
	}

}

void EMGFeaturesTime::polyfit(float* x, int n_x, float* y, int n_y, float* coefs) {

	cv::Mat Q, R, p, Qty;
	cv::Mat V(n_x, 2, CV_64F);
	cv::Mat yMat(n_y, 1, CV_64F);

	for (int i = 0; i < V.rows; i++) {
		V.at<double>(cv::Point2i(0, i)) = x[i];
		V.at<double>(cv::Point2i(1, i)) = 1.0;
	}

	for (int i = 0; i < yMat.rows; i++) {
		yMat.at<double>(cv::Point2i(0, i)) = y[i];
	}

	QRDecomp(V, Q, R);
	Q = Q * -1.0; //Make results consistent with Matlab
	R = R * -1.0;

	Qty = (Q.t() * yMat);
	cv::solve(R, Qty, p, cv::DECOMP_NORMAL);

	coefs[0] = (float)p.at<double>(cv::Point2i(0, 0));
	coefs[1] = (float)p.at<double>(cv::Point2i(0, 1));

}

void EMGFeaturesTime::printMat(cv::Mat m, char* name) {

	ssi_print("\n-------- ");
	ssi_print("%s", name);
	ssi_print(": --------\n");

	for (int i = 0; i < m.rows; i++) {
		for (int j = 0; j < m.cols; j++) {
			ssi_print("%.4f\t", m.at<double>(cv::Point2i(j, i)));
		}
		ssi_print("\n");
	}
}

void EMGFeaturesTime::printMat_flt(cv::Mat m, char* name) {

	ssi_print("\n-------- ");
	ssi_print("%s", name);
	ssi_print(": --------\n");

	for (int i = 0; i < m.rows; i++) {
		for (int j = 0; j < m.cols; j++) {
			ssi_print("%.4f\t", m.at<float>(cv::Point2i(j, i)));
		}
		ssi_print("\n");
	}
}

void EMGFeaturesTime::QRDecomp(cv::Mat& m, cv::Mat& Q, cv::Mat& R)
{
	using namespace cv;
	using namespace std;

	vector<Mat> q(m.rows);
	Mat z = m.clone();


	for (int k = 0; k < m.cols && k < m.rows - 1; k++) {
		vector<double> e(m.rows, 0);
		vector<double> x(m.rows, 0);
		double a = 0;

		Mat z1 = Mat::zeros(z.rows, z.cols, CV_64F);

		for (int i = 0; i < k; i++) {
			z1.at<double>(i, i) = 1;
		}
		for (int y = k; y < z1.rows; y++) {
			for (int x = k; x < z1.cols; x++) {
				z1.at<double>(y, x) = z.at<double>(y, x);
			}
		}
		z = z1.clone();

		for (int i = 0; i < z.rows; i++) {
			a += pow(z.at<double>(i, k), 2);
			x[i] = z.at<double>(i, k);
		}

		a = sqrt(a);
		if (m.at<double>(k, k) > 0) {
			a = -a;
		}
		for (int i = 0; i < m.rows; i++) {
			if (i == k) {
				e[i] = 1;
			}
			else {
				e[i] = 0;
			}
		}

		for (int i = 0; i < m.rows; i++) {
			e[i] = x[i] + a * e[i];
		}

		double norm = 0;
		for (int i = 0; i < e.size(); i++) {
			norm += pow(e[i], 2);
		}
		norm = sqrt(norm);
		for (int i = 0; i < e.size(); i++) {
			if (norm != 0) {
				e[i] /= norm;
			}
		}
		Mat E(e.size(), 1, CV_64F);
		for (int i = 0; i < e.size(); i++) {
			E.at<double>(i, 0) = e[i];
		}

		q[k] = Mat::eye(m.rows, m.rows, CV_64F) - 2 * E * E.t();
		z1 = q[k] * z;
		z = z1.clone();
	}

	Q = q[0].clone();
	R = q[0] * m;

	for (int i = 1; i < m.cols && i < m.rows - 1; i++) {
		Q = q[i](Rect(0, 0, m.rows, m.cols)).clone() * Q;
		//Q = q[i] * Q;
	}

	R = Q * m;
	Q = Q.t();

	if (m.rows > m.cols) {
		R = R(Rect(0, 0, m.cols, m.cols)).clone();
		Q = Q(Rect(0, 0, m.cols, m.rows)).clone();
	}

}


void EMGFeaturesTime::QRDecomp_flt(cv::Mat& m, cv::Mat& Q, cv::Mat& R)
{
	using namespace cv;
	using namespace std;

	vector<Mat> q(m.rows);
	Mat z = m.clone();


	for (int k = 0; k < m.cols && k < m.rows - 1; k++) {
		vector<float> e(m.rows, 0);
		vector<float> x(m.rows, 0);
		float a = 0;

		Mat z1 = Mat::zeros(z.rows, z.cols, CV_32F);

		for (int i = 0; i < k; i++) {
			z1.at<float>(i, i) = 1;
		}
		for (int y = k; y < z1.rows; y++) {
			for (int x = k; x < z1.cols; x++) {
				z1.at<float>(y, x) = z.at<float>(y, x);
			}
		}
		z = z1.clone();

		for (int i = 0; i < z.rows; i++) {
			a += pow(z.at<float>(i, k), 2);
			x[i] = z.at<float>(i, k);
		}

		a = sqrt(a);
		if (m.at<float>(k, k) > 0) {
			a = -a;
		}
		for (int i = 0; i < m.rows; i++) {
			if (i == k) {
				e[i] = 1;
			}
			else {
				e[i] = 0;
			}
		}


		for (int i = 0; i < m.rows; i++) {
			e[i] = x[i] + a * e[i];
		}

		float norm = 0;
		for (int i = 0; i < e.size(); i++) {
			norm += pow(e[i], 2);
		}
		norm = sqrt(norm);
		for (int i = 0; i < e.size(); i++) {
			if (norm != 0) {
				e[i] /= norm;
			}
		}
		Mat E(e.size(), 1, CV_32F);
		for (int i = 0; i < e.size(); i++) {
			E.at<float>(i, 0) = e[i];
		}

		q[k] = Mat::eye(m.rows, m.rows, CV_32F) - 2 * E * E.t();
		z1 = q[k] * z;
		z = z1.clone();
	}

	Q = q[0].clone();
	R = q[0] * m;
	for (int i = 1; i < m.cols && i < m.rows - 1; i++) {
		Q = q[i] * Q;
	}
	R = Q * m;
	Q = Q.t();

	if (m.rows > m.cols) {
		R = R(Rect(0, 0, m.cols, m.cols)).clone();
		Q = Q(Rect(0, 0, m.cols, m.rows)).clone();
	}
}

}


