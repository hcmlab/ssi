// RSPFeaturesTime.cpp
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

#include "RSPFeaturesTime.h"
#include "OverlapBuffer.h"
#include "../../signal/include/MvgAvgVar.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#define FLT_BIG 1000000

namespace ssi {

ssi_char_t *RSPFeaturesTime::ssi_log_name = "RSPFeaturesTime__";

RSPFeaturesTime::RSPFeaturesTime (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

RSPFeaturesTime::~RSPFeaturesTime () {


	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void RSPFeaturesTime::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

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

	ptr_prev = 0;
	state = RSP_NONE;
}



//based on http://iopscience.iop.org/article/10.1088/0967-3334/36/10/2027/pdf
void RSPFeaturesTime::transform( ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {
	
	std::vector<peaktrough> ptz;

	ssi_real_t sin = (float)stream_in.num;
	ssi_real_t mean = 0;

	ssi_real_t f_re_var = 0;
	ssi_real_t f_rfreq = 0;
	ssi_real_t f_rfreq_sd = 0;
	ssi_real_t f_bbc = 0;
	ssi_real_t f_bbc_sd = 0;
	ssi_real_t f_peak_mean = 0;
	ssi_real_t f_trough_mean = 0;
	ssi_real_t f_peak_median = 0;
	ssi_real_t f_trough_median = 0;
	ssi_real_t f_se_peaks = 0;
	ssi_real_t f_se_troughs = 0;
	ssi_real_t f_ptd_median = 0;
	ssi_real_t f_vol_br_median = 0;
	ssi_real_t f_vol_in_median = 0;
	ssi_real_t f_vol_ex_median = 0;
	ssi_real_t f_fr_br_median = 0;
	ssi_real_t f_fr_in_median = 0;
	ssi_real_t f_fr_ex_median = 0;

	ssi_real_t *ptr_in = ssi_pcast (ssi_real_t, stream_in.ptr);
	ptr_prev = *ptr_in;
	for (int nsamp = 0; nsamp < stream_in.num; nsamp++){
		
		if (*ptr_in < ptr_prev){
			if (state == RSP_RISING){ //peak
				peaktrough pte;
				pte.pos = nsamp;
				pte.value = *ptr_in;
				pte.type = PTZ_TYPE::TROUGH;
				ptz.push_back(pte);
			}
			state = RSP_FALLING;
		}
		else if (*ptr_in > ptr_prev){
			if (state == RSP_FALLING){ //trough
				peaktrough pte;
				pte.pos = nsamp;
				pte.value = *ptr_in;
				pte.type = PTZ_TYPE::PEAK;
				ptz.push_back(pte);
			}
			state = RSP_RISING;
		}

		if (*ptr_in >= 0 && ptr_prev < 0){
			peaktrough pte;
			pte.pos = nsamp;
			pte.value = *ptr_in;
			pte.type = PTZ_TYPE::ZC_RISING;
			ptz.push_back(pte);
		}
		else if(*ptr_in < 0 && ptr_prev >= 0){
			peaktrough pte;
			pte.pos = nsamp;
			pte.value = *ptr_in;
			pte.type = PTZ_TYPE::ZC_FALLING;
			ptz.push_back(pte);
		}

		mean += *ptr_in;

		ptr_prev = *ptr_in;
		ptr_in++;

	}

	mean /= sin;

	if (_options.print_info){
		ssi_print("[RSP] ------------------- \n");
		for (int i = 0; i < ptz.size(); i++){

			if (ptz[i].value == PTZ_TYPE::TROUGH){
				ssi_print("[RSP] trough at %i: %.2f\n", ptz[i].pos, ptz[i].value);
			}
			else if (ptz[i].value == PTZ_TYPE::PEAK){
				ssi_print("[RSP] peak  at %i: %.2f\n", ptz[i].pos, ptz[i].value);
			}
			else if (ptz[i].value == PTZ_TYPE::ZC_RISING){
				ssi_print("[RSP] zc (rising) at %i: %.2f\n", ptz[i].pos, ptz[i].value);
			}
			else if (ptz[i].value == PTZ_TYPE::ZC_FALLING){
				ssi_print("[RSP] zc (falling) at %i: %.2f\n", ptz[i].pos, ptz[i].value);
			}

		}
	}

	ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	for (int nsamp = 0; nsamp < stream_in.num; nsamp++){
		f_re_var += powf(*ptr_in - mean, 2);
		ptr_in += stream_in.dim;
	}
	f_re_var /= sin;

	std::vector<peaktrough> pt;
	for (int i = 0; i<ptz.size(); i++){
		if (ptz[i].type == PTZ_TYPE::PEAK || ptz[i].type == PTZ_TYPE::TROUGH){
			pt.push_back(ptz[i]);
		}
	}

	if (pt.size() > 1){
		ssi_real_t first_to_last = (ssi_real_t)(pt.back().pos - pt[0].pos);
		if (first_to_last > 0){
			f_rfreq = ((ssi_real_t)(pt.size()) / 2) / (first_to_last / stream_in.sr);

			if (pt.size() > 2){
				for (int i = 0; i < pt.size() - 1; i++){
					ssi_real_t subdiff = fabsf((ssi_real_t)(pt[i + 1].pos - pt[i].pos));
					float subfreq = (subdiff == 0) ? 0 : 1.0f / (subdiff / stream_in.sr);
					f_rfreq_sd += powf(subfreq - f_rfreq, 2);
				}
			}
		}
	}

	f_rfreq_sd /= (ssi_real_t)(pt.size() - 1);
	f_rfreq_sd = sqrtf(f_rfreq_sd);

	if (std::isnan(f_rfreq_sd) || std::isinf(f_rfreq_sd)) f_rfreq_sd = 0;

	std::vector<peaktrough> peaks;
	std::vector<peaktrough> troughs;
	for (int i = 0; i < pt.size(); i++){
		if (pt[i].type == PTZ_TYPE::TROUGH) troughs.push_back(pt[i]);
		if (pt[i].type == PTZ_TYPE::PEAK) peaks.push_back(pt[i]);
	}

	ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	getBreathByBreathCorreleation(ptr_in, troughs, &f_bbc, &f_bbc_sd);

	for (int i = 0; i < peaks.size(); i++) f_peak_mean += peaks[i].value;
	for (int i = 0; i < troughs.size(); i++) f_trough_mean += troughs[i].value;

	if (peaks.size() > 0) f_peak_mean /= (float)peaks.size();
	if (troughs.size() > 0) f_trough_mean /= (float)troughs.size();

	std::sort(peaks.begin(), peaks.end(), less_than_value());
	std::sort(troughs.begin(), troughs.end(), less_than_value());

	if (peaks.size() > 0) f_peak_median = peaks[(peaks.size() - 1) / 2].value;
	if (troughs.size() > 0) f_trough_median = troughs[(troughs.size() - 1) / 2].value;

	ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	getSampleEntropy(ptr_in, ptz, &f_se_peaks, &f_se_troughs);

	std::vector<ssi_real_t> ptd;
	if (pt.size() > 1){
		for (int i = 0; i < pt.size() - 1; i++){
			if (pt[i].type == PTZ_TYPE::PEAK && pt[i + 1].type == PTZ_TYPE::TROUGH){
				ptd.push_back(fabsf(pt[i + 1].value - pt[i].value));
			}
		}
	}

	if (ptd.size() > 0){
		std::sort(ptd.begin(), ptd.end());
		f_ptd_median = ptd[(ptd.size() - 1) / 2];
	}

	ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);
	getVolumeBasedFeatures(ptr_in, pt, &f_vol_br_median, &f_vol_in_median, &f_vol_ex_median, &f_fr_br_median, &f_fr_in_median, &f_fr_ex_median);

	if (f_bbc >((float)FLT_BIG - 1.0f) || f_bbc < ((float)-FLT_BIG + 1.0f)) f_bbc = 0;
	if (f_bbc_sd >((float)FLT_BIG - 1.0f) || f_bbc_sd < ((float)-FLT_BIG + 1.0f)) f_bbc_sd = 0;

	if (_options.print_info){
		ssi_print("[RSP] f_re_var:           %.2f\n", f_re_var);
		ssi_print("[RSP] f_rfreq:            %.2f\n", f_rfreq);
		ssi_print("[RSP] f_rfreq_sd:         %.2f\n", f_rfreq_sd);
		ssi_print("[RSP] f_bbc:              %.2f\n", f_bbc);
		ssi_print("[RSP] f_bbc sd:           %.2f\n", f_bbc_sd);
		ssi_print("[RSP] f_peak_mean:        %.2f\n", f_peak_mean);
		ssi_print("[RSP] f_trough_mean:      %.2f\n", f_trough_mean);
		ssi_print("[RSP] f_peak_median:      %.2f\n", f_peak_median);
		ssi_print("[RSP] f_trough_median:    %.2f\n", f_trough_median);
		ssi_print("[RSP] f_se_peaks:         %.2f\n", f_se_peaks);
		ssi_print("[RSP] f_se_troughs:       %.2f\n", f_se_troughs);
		ssi_print("[RSP] f_ptd_median:       %.2f\n", f_ptd_median);
		ssi_print("[RSP] f_vol_br_median:    %.2f\n", f_vol_br_median);
		ssi_print("[RSP] f_vol_in_median:    %.2f\n", f_vol_in_median);
		ssi_print("[RSP] f_vol_ex_median:    %.2f\n", f_vol_ex_median);
		ssi_print("[RSP] f_fr_br_median:     %.2f\n", f_fr_br_median);
		ssi_print("[RSP] f_fr_in_median:     %.2f\n", f_fr_in_median);
		ssi_print("[RSP] f_fr_ex_median:     %.2f\n", f_fr_ex_median);
	}

	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	*ptr_out = f_re_var; //variance of respiratory effort
	ptr_out++;
	*ptr_out = f_rfreq; // respiratory frequency
	ptr_out++;
	*ptr_out = f_rfreq_sd; // standard deviation of respiratory frequency
	ptr_out++;
	*ptr_out = f_bbc; // breath-by-breath correlation
	ptr_out++;
	*ptr_out = f_bbc_sd; // standard deviation of breath-by-breath correlation
	ptr_out++;
	*ptr_out = f_peak_mean; // peak mean value
	ptr_out++;
	*ptr_out = f_trough_mean; // trough mean value
	ptr_out++;
	*ptr_out = f_peak_median; // peak median value
	ptr_out++;
	*ptr_out = f_trough_median; // trough median value
	ptr_out++;
	*ptr_out = f_se_peaks; // peak sample entropy
	ptr_out++;
	*ptr_out = f_se_troughs; // trough sample entropy
	ptr_out++;
	*ptr_out = f_ptd_median; // median peak-to-trough difference
	ptr_out++;
	*ptr_out = f_vol_br_median; // median breath cycle volume
	ptr_out++;
	*ptr_out = f_vol_in_median; // median inhalation volume
	ptr_out++;
	*ptr_out = f_vol_ex_median; // median exhalation volume
	ptr_out++;
	*ptr_out = f_fr_br_median; // median breath cycle flow rate
	ptr_out++;
	*ptr_out = f_fr_in_median; // median inhalation flow rate
	ptr_out++;
	*ptr_out = f_fr_ex_median; // median exhalation flow rate

}


void RSPFeaturesTime::getBreathByBreathCorreleation(ssi_real_t* in, std::vector<peaktrough> troughs, ssi_real_t* bbc, ssi_real_t* bbc_sd){
	//http://download.springer.com/static/pdf/728/art%253A10.1007%252Fs11818-007-0314-8.pdf?originUrl=http%3A%2F%2Flink.springer.com%2Farticle%2F10.1007%2Fs11818-007-0314-8&token2=exp=1461151402~acl=%2Fstatic%2Fpdf%2F728%2Fart%25253A10.1007%25252Fs11818-007-0314-8.pdf%3ForiginUrl%3Dhttp%253A%252F%252Flink.springer.com%252Farticle%252F10.1007%252Fs11818-007-0314-8*~hmac=a3e17f884382f9f84765a8aad27508abc7b6adaf78ae1ec0d07d632a1fcafd75
	
	if (troughs.size() < 3){
		return;
	}

	float max_x = -FLT_BIG;
	float max_y = -FLT_BIG;
	std::vector<float> mmcs;

	for (int i = 0 ; i < troughs.size() - 2; i++){
		ssi_size_t tt_diff_x = troughs[i + 1].pos - troughs[i].pos;
		ssi_size_t tt_diff_y = troughs[i + 2].pos - troughs[i + 1].pos;
		ssi_size_t bigger_tt_diff = (tt_diff_x > tt_diff_y) ? tt_diff_x : tt_diff_y;

		float* tt_x = new float[bigger_tt_diff];
		float* tt_y = new float[bigger_tt_diff];

		for (int j = 0; j < bigger_tt_diff; j++){
			tt_x[j] = (j < tt_diff_x) ? in[troughs[i].pos + j] : 0;
			tt_y[j] = (j < tt_diff_y) ? in[troughs[i + 1].pos + j] : 0;

			if (tt_x[j] > max_x) max_x = tt_x[j];
			if (tt_y[j] > max_y) max_y = tt_y[j];
		}

		float mcc = getMaxCrossCorrelation(tt_x, tt_y, bigger_tt_diff);
		mmcs.push_back(mcc);

		delete tt_x;
		delete tt_y;
	}

	if (mmcs.size() < 2){
		*bbc = mmcs[0];
		*bbc_sd = 0;
	}
	else{
		float bbc_normalized_sum = 0;
		for (int i = 0; i < mmcs.size(); i++){
			bbc_normalized_sum += mmcs[i];
		}
		*bbc = bbc_normalized_sum / ((float)mmcs.size());

		for (int i = 0; i < mmcs.size(); i++){
			*bbc_sd += powf(mmcs[i] - bbc_normalized_sum, 2);
		}

		*bbc_sd /= (ssi_real_t)(mmcs.size());
		*bbc = sqrtf(*bbc_sd);

	}

	

}

void RSPFeaturesTime::getSampleEntropy(ssi_real_t* in, std::vector<peaktrough> _ptz, ssi_real_t* se_peaks, ssi_real_t* se_troughs){
	
	int prev_zc = -1;

	ssi_real_t se_p = 0;
	ssi_real_t se_t = 0;
	ssi_real_t n_se_p = 0;
	ssi_real_t n_se_t = 0;

	for (int i = 0; i < _ptz.size(); i++){

		if (prev_zc != -1){
			if ((_ptz[i].type == PTZ_TYPE::ZC_FALLING && _ptz[prev_zc].type == PTZ_TYPE::ZC_RISING) ||
				(_ptz[i].type == PTZ_TYPE::ZC_RISING && _ptz[prev_zc].type == PTZ_TYPE::ZC_FALLING)){

				int n_samples = _ptz[i].pos - _ptz[prev_zc].pos;
				double* series = new double[n_samples];
				ssi_time_t avg = 0;
				ssi_time_t sd = 0;
				for (int j = 0; j < n_samples; j++){
					series[j] = (double)in[_ptz[prev_zc].pos + j];
					avg += series[j];
				}
				avg /= (ssi_time_t)n_samples;

				for (int j = 0; j < n_samples; j++) sd += powf(series[j] - avg, 2);
				sd = sqrtf(sd / avg);
				if (sd < 1) sd = 1;

				if (_ptz[i].type == PTZ_TYPE::ZC_FALLING && _ptz[prev_zc].type == PTZ_TYPE::ZC_RISING){ //peak
					se_p += sampen(series, 0, 0.2 * sd, n_samples);
					n_se_p++;
				}

				if (_ptz[i].type == PTZ_TYPE::ZC_RISING && _ptz[prev_zc].type == PTZ_TYPE::ZC_FALLING){ //trough
					se_t += sampen(series, 0, 0.2 * sd, n_samples);
					n_se_t++;
				}

				delete series;

			}
			
		}

		if (_ptz[i].type == PTZ_TYPE::ZC_RISING || _ptz[i].type == PTZ_TYPE::ZC_FALLING){
			prev_zc = i;
		}
	}

	*se_peaks  = n_se_p > 0.000001 ? (se_p / n_se_p) : 0;
	*se_troughs = n_se_t > 0.000001 ? (se_t / n_se_t) : 0;
	
}

//from http://paulbourke.net/miscellaneous/correlate/
float RSPFeaturesTime::getMaxCrossCorrelation(float* x, float* y, int n){
	float mx, my, sx, sy, sxy, denom, r;
	float r_max = -999;
	float maxdelay = n;

	/* Calculate the mean of the two series x[], y[] */
	mx = 0;
	my = 0;
	for (int i = 0; i < n; i++) {
		mx += x[i];
		my += y[i];
	}
	mx /= n;
	my /= n;

	/* Calculate the denominator */
	sx = 0;
	sy = 0;
	for (int i = 0; i < n; i++) {
		sx += (x[i] - mx) * (x[i] - mx);
		sy += (y[i] - my) * (y[i] - my);
	}
	denom = sqrt(sx*sy);

	/* Calculate the correlation series */
	for (int delay = -maxdelay; delay < maxdelay; delay++) {
		sxy = 0;
		for (int i = 0; i < n; i++) {
			int j = i + delay;
			if (j < 0 || j >= n)
				continue;
			else
				sxy += (x[i] - mx) * (y[j] - my);
			
		}
		r = sxy / denom; /* r is the correlation coefficient at "delay" */

		if (r > r_max) r_max = r;
	}

	return r_max;
}

void RSPFeaturesTime::getVolumeBasedFeatures(ssi_real_t* in, std::vector<peaktrough> _pt, ssi_real_t* f_vol_br_median, ssi_real_t* f_vol_in_median, ssi_real_t* f_vol_ex_median, ssi_real_t* f_fr_br_median, ssi_real_t* f_fr_in_median, ssi_real_t* f_fr_ex_median){

	const ssi_size_t none = 9999999999;

	ssi_size_t prev_peak = none;
	ssi_size_t prev_trough = none;

	std::vector<ssi_real_t> vol_br;
	std::vector<ssi_real_t> vol_in;
	std::vector<ssi_real_t> vol_ex;
	std::vector<ssi_real_t> fr_br;
	std::vector<ssi_real_t> fr_in;
	std::vector<ssi_real_t> fr_ex;

	for (int i = 0; i < _pt.size(); i++){

		if (_pt[i].type == PTZ_TYPE::PEAK && prev_trough != none){
			vol_in.push_back(getVolume(in, _pt[prev_trough].pos, _pt[i].pos));
			fr_in.push_back(getFlowRate(in, _pt[prev_trough].pos, _pt[i].pos));
		}

		if (_pt[i].type == PTZ_TYPE::TROUGH){
			if (prev_peak != none){
				vol_ex.push_back(getVolume(in, _pt[prev_peak].pos, _pt[i].pos));
				fr_ex.push_back(getFlowRate(in, _pt[prev_peak].pos, _pt[i].pos));
			}
			if (prev_trough != none){
				vol_br.push_back(getVolume(in, _pt[prev_trough].pos, _pt[i].pos));
				fr_br.push_back(getFlowRate(in, _pt[prev_trough].pos, _pt[i].pos));
			}
		}

		if (_pt[i].type == PTZ_TYPE::PEAK)  prev_peak = i;
		if (_pt[i].type == PTZ_TYPE::TROUGH) prev_trough = i;
	}

	std::sort(vol_br.begin(), vol_br.end());
	std::sort(vol_in.begin(), vol_in.end());
	std::sort(vol_ex.begin(), vol_ex.end());

	std::sort(fr_br.begin(), fr_br.end());
	std::sort(fr_in.begin(), fr_in.end());
	std::sort(fr_ex.begin(), fr_ex.end());

	*f_vol_br_median = vol_br.size() > 0 ? vol_br[vol_br.size() / 2] : 0;
	*f_vol_in_median = vol_in.size() > 0 ? vol_in[vol_in.size() / 2] : 0;
	*f_vol_ex_median = vol_ex.size() > 0 ? vol_ex[vol_ex.size() / 2] : 0;

	*f_fr_br_median = fr_br.size() > 0 ? fr_br[fr_br.size() / 2] : 0;
	*f_fr_in_median = fr_in.size() > 0 ? fr_in[fr_in.size() / 2] : 0;
	*f_fr_ex_median = fr_ex.size() > 0 ? fr_ex[fr_ex.size() / 2] : 0;

}

ssi_real_t RSPFeaturesTime::getVolume(ssi_real_t* in, ssi_size_t from, ssi_size_t to){
	ssi_real_t sum = 0;
	for (int i = from; i < to; i++)	sum += in[i];
	return fabsf(sum);
}

ssi_real_t RSPFeaturesTime::getFlowRate(ssi_real_t* in, ssi_size_t from, ssi_size_t to){
	ssi_real_t sum = 0;
	ssi_real_t tau = (to - from) / _options.sr;
	for (int i = from; i < to; i++)	sum += in[i];
	return fabsf(sum / tau);
}


// **********************************************************************************************************************
// *************************************  Sample Entropy calculation by Doug Lake ***************************************
// **********************************  http://www.physionet.org/physiotools/sampen/c/  **********************************
// **********************************************************************************************************************



/* sampen() calculates an estimate of sample entropy but does NOT calculate
the variance of the estimate */
ssi_real_t RSPFeaturesTime::sampen(double *y, int M, double r, int n)
{
	M++;

	double *p = new double[M](); //init with zeroes, c++ equivalent of calloc
	//double *e = NULL;
	long *run = new long[n]();
	long *lastrun = new long[n]();
	long N;
	double *A = new double[M]();
	double *B = new double[M]();
	int M1, j, nj, jj, m;
	int i;
	double y1;

	
	/* start running */
	for (i = 0; i < n - 1; i++) {
		nj = n - i - 1;
		y1 = y[i];
		for (jj = 0; jj < nj; jj++) {
			j = jj + i + 1;
			if (((y[j] - y1) < r) && ((y1 - y[j]) < r)) {
				run[jj] = lastrun[jj] + 1;
				M1 = M < run[jj] ? M : run[jj];
				for (m = 0; m < M1; m++) {
					A[m]++;
					if (j < n - 1)
						B[m]++;
				}
			}
			else
				run[jj] = 0;
		}			/* for jj */
		for (j = 0; j < nj; j++)
			lastrun[j] = run[j];
	}				/* for i */

	N = (long)(n * (n - 1) / 2);
	p[0] = A[0] / N;

	/*printf("SampEn(0,%g,%d) = %lf (from %f)\n", r, n, -log(p[0]), p[0]);
	for (m = 1; m < M; m++) {
		p[m] = A[m] / B[m - 1];
		if (p[m] == 0)
			printf("No matches! SampEn((%d,%g,%d) = Inf!\n", m, r, n);
		else
			printf("SampEn(%d,%g,%d) = %lf\n", m, r, n, -log(p[m]));
	}*/


	ssi_real_t result = 0;
	if (fabsf(p[0]) > 0.0000001) result = (float)-log(p[0]);

	if (A) delete A;
	if (B) delete B;
	if (p) delete p;
	if (run) delete run;
	if (lastrun) delete lastrun;

	return result;

}


void RSPFeaturesTime::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	
}

void RSPFeaturesTime::printFeatures(){
	int af;
	af = 0;
	ssi_print("\n[RSPFeaturesTime] Activated Features:\n");

	ssi_print(" %i: variance of respiratory effort\n", af++);
	ssi_print(" %i: respiratory frequency\n", af++);
	ssi_print(" %i: standard deviation of respiratory frequency\n", af++);
	ssi_print(" %i: breath-by-breath correlation\n", af++);
	ssi_print(" %i: standard deviation of breath-by-breath correlation\n", af++);
	ssi_print(" %i: peak mean value\n", af++);
	ssi_print(" %i: trough mean value\n", af++);
	ssi_print(" %i: peak median value\n", af++);
	ssi_print(" %i: trough median value\n", af++);
	ssi_print(" %i: peak sample entropy\n", af++);
	ssi_print(" %i: trough sample entropy\n", af++);
	ssi_print(" %i: median peak-to-trough difference\n", af++);
	ssi_print(" %i: median breath cycle volume\n", af++);
	ssi_print(" %i: median inhalation volume\n", af++);
	ssi_print(" %i: median exhalation volume\n", af++);
	ssi_print(" %i: median breath cycle flow rate\n", af++);
	ssi_print(" %i: median inhalation flow rate\n", af++);
	ssi_print(" %i: median exhalation flow rate\n", af++);

	ssi_print("\n");
}





}
