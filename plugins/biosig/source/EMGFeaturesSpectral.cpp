// EMGFeaturesSpectral.cpp
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

#include "EMGFeaturesSpectral.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *EMGFeaturesSpectral::ssi_log_name = "EMGFeaturesSpectral__";


EMGFeaturesSpectral::EMGFeaturesSpectral (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

}

EMGFeaturesSpectral::~EMGFeaturesSpectral () {


	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void EMGFeaturesSpectral::transform_enter(ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num,	ssi_stream_t xtra_stream_in[]) {

	if (stream_in.type != SSI_REAL) {
		ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[stream_in.type]);
	}
		
	if (_options.print_features){
		printFeatures();
		_options.print_features = false;
	}

}


//based on https://www.researchgate.net/publication/268363764_Stages_for_Developing_Control_Systems_using_EMG_and_EEG_Signals_A_survey
void EMGFeaturesSpectral::transform(ITransformer::info info, ssi_stream_t &stream_in, ssi_stream_t &stream_out, ssi_size_t xtra_stream_in_num, ssi_stream_t xtra_stream_in[]) {

	ssi_real_t *ptr_in = ssi_pcast(ssi_real_t, stream_in.ptr);

	float* sf_ar_coeffs = new float[_options.n_ar_coeffs];
	calcARcoefficients(ptr_in, stream_in.num, _options.n_ar_coeffs, sf_ar_coeffs, _options.ar_method);
		
	float sf_fmd = calcFrequencyMedian(ptr_in, stream_in.num, false);
	float sf_fmn = calcFrequencyMean(ptr_in, stream_in.num, false);
	float sf_mfmd = calcFrequencyMedian(ptr_in, stream_in.num, true);
	float sf_mfmn = calcFrequencyMean(ptr_in, stream_in.num, true);
	float sf_fr = calcFrequencyRatio(ptr_in, stream_in.num, true, _options.fr_threshold);

	ssi_real_t *ptr_out = ssi_pcast(ssi_real_t, stream_out.ptr);
	for (int i = 0; i < _options.n_ar_coeffs; i++){
		*ptr_out = sf_ar_coeffs[i];
		ptr_out++;
	}

	*ptr_out = sf_fmd; // FMD: Frequency Median (power spectrum)
	ptr_out++;
	*ptr_out = sf_fmn; // FMN: Frequency Mean (power spectrum)
	ptr_out++;
	*ptr_out = sf_mfmd; //MFMD: Modified Frequency Median (amplitude spectrum)
	ptr_out++;
	*ptr_out = sf_mfmn; //MFMN: Modified Frequency Mean (amplitude spectrum)
	ptr_out++;
	*ptr_out = sf_fr; //FR: Frequency Ratio
	ptr_out++;

	delete sf_ar_coeffs;

}

void EMGFeaturesSpectral::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

}

float EMGFeaturesSpectral::calcFrequencyMedian(ssi_real_t* in, ssi_size_t num, bool modified){

	ssi_size_t split = num / 2;
	ssi_size_t split_step = split / 2;
	ssi_real_t sum1 = 0;
	
	ssi_real_t sum_total = 0;
    for (int i = 0; i < num; i++) sum_total += (modified ? in[i] : std::pow(in[i], 2));
	ssi_real_t sum_total_half = sum_total/2;

	//int it = 0;
	//ssi_print("[EFS] CFM2 --------------------------------\n", it, split, sum1, sum_total_half);
	while (true){

		sum1 = 0;
        for (int i = 0; i < split; i++) sum1 += (modified ? in[i] : std::pow(in[i], 2));

		//ssi_print("[EFS] median it: %i, split: %i (%.1f to %.1f)\n", it, split, sum1, sum_total_half);
		//it++;

		if (fabsf(sum1 - sum_total_half) < 1 || split_step < 1){
			return ((float)split / (float)num);
		}
		else{
			if (sum1 > sum_total_half) split -= split_step;
			else                       split += split_step;

			split_step /= 2;
		}
	}

}

float EMGFeaturesSpectral::calcFrequencyMean(ssi_real_t* in, ssi_size_t num, bool modified){

	ssi_real_t sum1 = 0;
	ssi_real_t sum2 = 0;

	for (int i = 0; i < num; i++){
        sum1 += (i + 1) * (modified ? in[i] : std::pow(in[i], 2));
        sum2 += (modified ? in[i] : std::pow(in[i], 2));
	}

	if (sum2 > 0){
		return sum1 / sum2;
	}
	else{
		return 0;
	}
}

float EMGFeaturesSpectral::calcFrequencyRatio(ssi_real_t* in, ssi_size_t num, bool modified, ssi_real_t threshold){
	
	ssi_real_t sum1 = 0;
	ssi_real_t sum2 = 0;
	ssi_size_t t = ((float)num * threshold);


	for (int i = 0; i < t; i++){
        sum1 += (modified ? in[i] : std::pow(in[i], 2));
	}

	for (int i = t; i < num; i++){
        sum2 += (modified ? in[i] : std::pow(in[i], 2));
	}

	if (sum2 > 0){
		return sum1 / sum2;
	}
	else{
		return 0;
	}
}

void EMGFeaturesSpectral::printFeatures(){

	ssi_print("\n[EMG Features Spectral] Activated Features:\n");

	for (int i = 0; i < _options.n_ar_coeffs; i++){
		ssi_print("%i: arc%i\n", i, i + 1);
	}

	ssi_print("%i: fmd\n", _options.n_ar_coeffs + 0);
	ssi_print("%i: fmn\n", _options.n_ar_coeffs + 1);
	ssi_print("%i: mfmd\n", _options.n_ar_coeffs + 2);
	ssi_print("%i: mfmn\n", _options.n_ar_coeffs + 3);

	ssi_print("\n");

}


// *************************************************************************************************
// *************************** AR coefficient calculation by Paul Burke ****************************
// *************************** http://paulbourke.net/miscellaneous/ar/  ****************************
// *************************************************************************************************

int EMGFeaturesSpectral::calcARcoefficients(float *inputseries, int length, int degree, float *coefficients, int method)
{
	float mean;
	int i, t;
	float *w = new float[length];         //Input series - mean
	float *h = new float[degree + 1];
	float *g = new float[degree + 2];     //Used by mempar()
	float *per = new float[length + 1];
	float *pef = new float[length + 1];   //Used by mempar()
	float **ar = new float*[degree + 1];  //AR coefficients, all degrees
	for (int i = 0; i < (degree + 1); i++) ar[i] = new float[degree + 1];

	/* Determine and subtract the mean from the input series */
	mean = 0.0;
	for (t = 0; t<length; t++)
		mean += inputseries[t];
	mean /= (float)length;
	for (t = 0; t<length; t++)
		w[t] = inputseries[t] - mean;

	/* Perform the appropriate AR calculation */
	if (method == AR_METHOD::MAXENTROPY) {

		if (!arMaxEntropy(w, length, degree, ar, per, pef, h, g)) {
			ssi_wrn("Max entropy failed - fatal!\n");
			getchar();
		}
		for (i = 1; i <= degree; i++)
			coefficients[i - 1] = -ar[degree][i];

	}
	else if (method == AR_METHOD::LEASTSQUARES) {

		if (!arLeastSquare(w, length, degree, coefficients)) {
			ssi_wrn("Least squares failed - fatal!\n");
			getchar();
		}

	}
	else {
		ssi_wrn("Unknown AR method\n");
		getchar();
	}

	if (w) delete w;
	if (h) delete h;
	if (g) delete g;
	if (per) delete per;
	if (pef) delete pef;
	if (ar) {
		for (i = 0; i < degree + 1; i++){
			if (ar[i]) delete ar[i];
		}
		delete ar;
	}

    return(true);
}

/*
Previously called mempar()
Originally in FORTRAN, hence the array offsets of 1, Yuk.
Original code from Kay, 1988, appendix 8D.

Perform Burg's Maximum Entropy AR parameter estimation
outputting successive models en passant. Sourced from Alex Sergejew

Two small changes made by NH in November 1998:
tstarz.h no longer included, just say "typedef double REAL" instead
Declare ar by "REAL **ar" instead of "REAL ar[MAXA][MAXA]

Further "cleaning" by Paul Bourke.....for personal style only.
*/

int EMGFeaturesSpectral::arMaxEntropy(float *inputseries, int length, int degree, float **ar, float *per, float *pef, float *h, float *g)
{
	int j, n, nn, jj;
	float sn, sd;
	float t1, t2;

	for (j = 1; j <= length; j++) {
		pef[j] = 0;
		per[j] = 0;
	}

	for (nn = 2; nn <= degree + 1; nn++) {
		n = nn - 2;
		sn = 0.0;
		sd = 0.0;
		jj = length - n - 1;
		for (j = 1; j <= jj; j++) {
			t1 = inputseries[j + n] + pef[j];
			t2 = inputseries[j - 1] + per[j];
			sn -= 2.0 * t1 * t2;
			sd += (t1 * t1) + (t2 * t2);
		}
		g[nn] = sn / sd;
		t1 = g[nn];
		if (n != 0) {
			for (j = 2; j<nn; j++)
				h[j] = g[j] + (t1 * g[n - j + 3]);
			for (j = 2; j<nn; j++)
				g[j] = h[j];
			jj--;
		}
		for (j = 1; j <= jj; j++) {
			per[j] += (t1 * pef[j]) + (t1 * inputseries[j + nn - 2]);
			pef[j] = pef[j + 1] + (t1 * per[j + 1]) + (t1 * inputseries[j]);
		}

		for (j = 2; j <= nn; j++)
			ar[nn - 1][j - 1] = g[j];
	}

    return(true);
}

/*
Least squares method
Original from Rainer Hegger, Last modified: Aug 13th, 1998
Modified (for personal style and context) by Paul Bourke
*/
int EMGFeaturesSpectral::arLeastSquare(float *inputseries, int length, int degree, float *coefficients)
{
	int i, j, k, hj, hi;
	float **mat = new float*[degree];
	for (int i = 0; i < (degree); i++) mat[i] = new float[degree];
	
	for (i = 0; i<degree; i++) {
		coefficients[i] = 0.0;
		for (j = 0; j<degree; j++)
			mat[i][j] = 0.0;
	}
	for (i = degree - 1; i<length - 1; i++) {
		hi = i + 1;
		for (j = 0; j<degree; j++) {
			hj = i - j;
			coefficients[j] += (inputseries[hi] * inputseries[hj]);
			for (k = j; k<degree; k++)
				mat[j][k] += (inputseries[hj] * inputseries[i - k]);
		}
	}
	for (i = 0; i<degree; i++) {
		coefficients[i] /= (length - degree);
		for (j = i; j<degree; j++) {
			mat[i][j] /= (length - degree);
			mat[j][i] = mat[i][j];
		}
	}

	/* Solve the linear equations */
	if (!arSolveLE(mat, coefficients, degree)) {
		ssi_wrn("Linear solver failed - fatal!\n");
		getchar();
	}

	for (i = 0; i < degree; i++){
		if (mat[i]) delete mat[i];
	}
	
	if (mat) delete mat;
    return(true);
}

/*
Gaussian elimination solver
Author: Rainer Hegger Last modified: Aug 14th, 1998
Modified (for personal style and context) by Paul Bourke
*/
int EMGFeaturesSpectral::arSolveLE(float **mat, float *vec, unsigned int n)
{
	int i, j, k, maxi;
	float vswap, *mswap, *hvec, max, h, pivot, q;

	for (i = 0; i<n - 1; i++) {
		max = fabs(mat[i][i]);
		maxi = i;
		for (j = i + 1; j<n; j++) {
			if ((h = fabs(mat[j][i])) > max) {
				max = h;
				maxi = j;
			}
		}
		if (maxi != i) {
			mswap = mat[i];
			mat[i] = mat[maxi];
			mat[maxi] = mswap;
			vswap = vec[i];
			vec[i] = vec[maxi];
			vec[maxi] = vswap;
		}

		hvec = mat[i];
		pivot = hvec[i];
		if (fabs(pivot) == 0.0) {
			ssi_wrn("Singular matrix - fatal!\n");
			getchar();
            return(false);
		}
		for (j = i + 1; j<n; j++) {
			q = -mat[j][i] / pivot;
			mat[j][i] = 0.0;
			for (k = i + 1; k<n; k++)
				mat[j][k] += q * hvec[k];
			vec[j] += (q * vec[i]);
		}
	}
	vec[n - 1] /= mat[n - 1][n - 1];
	for (i = n - 2; i >= 0; i--) {
		hvec = mat[i];
		for (j = n - 1; j>i; j--)
			vec[i] -= (hvec[j] * vec[j]);
		vec[i] /= hvec[i];
	}

    return(true);
}

}
