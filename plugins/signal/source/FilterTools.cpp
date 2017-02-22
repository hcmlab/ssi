// FilterTools.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/12/20
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

#include "FilterTools.h"
#include "signal/MatrixOps.h"
#include "FFT.h"
#include "IFFT.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Matrix<ssi_real_t> *FilterTools::Singen (double sample_rate,
	ssi_time_t duration, 
	const Matrix<ssi_real_t> *freqs_and_ampls, 
	ssi_real_t noise_level) {

	/* matlab code:

	t = 0:1/sr:len-1/sr;
	signal = A' * sin (2*pi*f*t) + noise * randn(size(t));

	*/

	SSI_ASSERT (freqs_and_ampls->rows > 0 && freqs_and_ampls->cols == 2);

	ssi_real_t delta = 1 / static_cast<ssi_real_t> (sample_rate);
	Matrix<ssi_real_t> *t = MatrixOps<ssi_real_t>::Array (0, delta, static_cast<ssi_real_t> (duration) - delta, MATRIX_DIMENSION_ROW);
	Matrix<ssi_real_t> *f = MatrixOps<ssi_real_t>::GetSubMatrix (freqs_and_ampls, 0, 0, MATRIX_DIMENSION_COL);
	Matrix<ssi_real_t> *A = MatrixOps<ssi_real_t>::GetSubMatrix (freqs_and_ampls, 1, 1, MATRIX_DIMENSION_COL);
	Matrix<ssi_real_t> *noise = MatrixOps<ssi_real_t>::Rand (t->cols, 1, noise_level);

	MatrixOps<ssi_real_t>::Mult (f, static_cast<ssi_real_t> (2*PI));
	Matrix<ssi_real_t> *tmp = MatrixOps<ssi_real_t>::MultM (f, t);
	MatrixOps<ssi_real_t>::Sin (tmp);
	MatrixOps<ssi_real_t>::Transpose (A);
	Matrix<ssi_real_t> *signal = MatrixOps<ssi_real_t>::MultM (A, tmp);
	MatrixOps<ssi_real_t>::Transpose (signal);
	MatrixOps<ssi_real_t>::Plus (signal, noise);

	delete t;
	delete f;
	delete A;
	delete noise;
	delete tmp;

	return signal;
}

Matrix<ssi_real_t> *FilterTools::Window (int size, 
	WINDOW_TYPE type, 
	MATRIX_DIMENSION dimension) {

	Matrix<ssi_real_t> *window;

	if (size < 1) {

		window = new Matrix<ssi_real_t> (0,0,0);

	} else if (size == 1) {

		window = new Matrix<ssi_real_t> (1,1);
		window->data[0] = 1;

	} else {

		switch (type) {

			case WINDOW_TYPE_RECTANGLE:	

				window = MatrixOps<ssi_real_t>::Ones (1, size);
				break;

			case WINDOW_TYPE_TRIANGLE:

				/* matlab code
				
				if rem(n,2) % odd
					win = 2*(1:(n+1)/2)/(n+1);
					win = [win win((n-1)/2:-1:1)]';
				else % even
					win = (1:2:(n+1)-1)/n;
					win = [win win(n/2:-1:1)]';
				end

				*/    

				if (size % 2) {
					
					Matrix<ssi_real_t> *front = MatrixOps<ssi_real_t>::Array (1,1,static_cast<ssi_real_t> ((size+1)>>1), MATRIX_DIMENSION_ROW);
					ssi_real_t scalar = static_cast<ssi_real_t> (2.0 / (size + 1.0));
					MatrixOps<ssi_real_t>::Mult (front, scalar);

					Matrix<ssi_size_t> *inds = MatrixOps<ssi_size_t>::IndArray (((size-1)>>1)-1, 1, 0, MATRIX_DIMENSION_ROW);
					Matrix<ssi_real_t> *back = MatrixOps<ssi_real_t>::GetSubMatrix (front, inds);

					window = MatrixOps<ssi_real_t>::Concat (front, back, MATRIX_DIMENSION_COL);

					delete front;
					delete back;
					delete inds;

				} else {

					Matrix<ssi_real_t> *front = MatrixOps<ssi_real_t>::Array (1,2,static_cast<ssi_real_t> ((size+1)-1), MATRIX_DIMENSION_ROW);
					ssi_real_t scalar = static_cast<ssi_real_t> (1.0 / size);
					MatrixOps<ssi_real_t>::Mult (front, scalar);

					Matrix<ssi_size_t> *inds = MatrixOps<ssi_size_t>::IndArray ((size>>1) - 1, 1, 0, MATRIX_DIMENSION_ROW);
					Matrix<ssi_real_t> *back = MatrixOps<ssi_real_t>::GetSubMatrix (front, inds);

					window = MatrixOps<ssi_real_t>::Concat (front, back, MATRIX_DIMENSION_COL);

					delete front;
					delete back;
					delete inds;
				}

				break;

			case WINDOW_TYPE_GAUSS:

				/* matlab code

				k = -(n-1)/2:(n-1)/2;
				win = exp (-((5/(sqrt(2)*n)) * k).^2);

				*/

				{
				window = MatrixOps<ssi_real_t>::Array (static_cast<ssi_real_t> (-(size-1.0)/2.0), 1, static_cast<ssi_real_t> ((size-1.0)/2.0), MATRIX_DIMENSION_ROW);
				ssi_real_t scalar = static_cast<ssi_real_t> (5.0 / (SQRT2 * size));
				MatrixOps<ssi_real_t>::Mult (window, scalar);
				MatrixOps<ssi_real_t>::Pow (window, 2);
				MatrixOps<ssi_real_t>::Mult (window, -1);
				MatrixOps<ssi_real_t>::Exp (window);
				}
				break;

			case WINDOW_TYPE_HAMMING:

				/* matlab code
			
				win  = 0.54 - 0.46 * cos (2 * pi * ((0:n-1)) / n);

				*/

				{
				window = MatrixOps<ssi_real_t>::Array (0, 1, static_cast<ssi_real_t> (size-1), MATRIX_DIMENSION_ROW);		
				ssi_real_t scalar = static_cast<ssi_real_t> ((2.0 * PI) / size);
				MatrixOps<ssi_real_t>::Mult (window, scalar);
				MatrixOps<ssi_real_t>::Cos (window);
				MatrixOps<ssi_real_t>::Mult (window, static_cast<ssi_real_t> (-0.46));
				MatrixOps<ssi_real_t>::Plus (window, static_cast<ssi_real_t> (0.54));
				}
				break;
		}

		// swap dimension if column vector
		if (dimension == MATRIX_DIMENSION_COL) {
			window->rows = window->cols;
			window->cols = 1;
		}

	}

	return window;
}


Matrix<ssi_real_t> *FilterTools::Filterbank (int size,
	double sample_rate, 
	const Matrix<ssi_real_t> *intervals, WINDOW_TYPE type) {

	/* Matlab code

	fs = fs / 2;
	m = size (interval, 1);
	bands = zeros (m, n);
	for i = 1:m
		minind = round ((interval(i,1) / fs) * n);
		maxind = round ((interval(i,2) / fs) * n);
		maxind = min (maxind, n-1);
		bands(i, minind+1:maxind+1) = winfunc (1 + (maxind - minind), name);
	end
	bands = bands ./ repmat (sum (bands, 2), 1, n);
	
	*/

	Matrix<ssi_real_t> *filterbank = MatrixOps<ssi_real_t>::Zeros (intervals->rows, size);

	sample_rate /= 2; // convert sampling to nyquist rate

	ssi_real_t *intervalsptr = intervals->data;
	for (ssi_size_t i = 0; i < filterbank->rows; i++) {
		int minind = static_cast<int> ((*intervalsptr++ / sample_rate) * size + static_cast<ssi_real_t> (0.5));
		int maxind = static_cast<int> ((*intervalsptr++ / sample_rate) * size + static_cast<ssi_real_t> (0.5));
		maxind = min (maxind, size-1);
		Matrix<ssi_real_t> *winmat = Window (1 + (maxind - minind), type, MATRIX_DIMENSION_ROW);
		MatrixOps<ssi_real_t>::Div (winmat, MatrixOps<ssi_real_t>::Sum (winmat));
		MatrixOps<ssi_real_t>::SetSubMatrix (filterbank, i, minind, winmat);
		delete winmat;
	}

	return filterbank;
}

Matrix<ssi_real_t> *FilterTools::Filterbank (int size,
	double sample_rate, 
	int banks_num, 
	double min_freq, 
	double max_freq, 
	WINDOW_TYPE type) {

	if (max_freq <= 0) {
		max_freq = sample_rate / 2;
	}

	double delta = (max_freq - min_freq) / banks_num;
	Matrix<ssi_real_t> *lower = MatrixOps<ssi_real_t>::Array (static_cast<ssi_real_t> (0), static_cast<ssi_real_t> (1), static_cast<ssi_real_t> (banks_num - 1), MATRIX_DIMENSION_COL);
	MatrixOps<ssi_real_t>::Mult (lower, static_cast<ssi_real_t> (delta));
	MatrixOps<ssi_real_t>::Plus (lower, static_cast<ssi_real_t> (min_freq));
	Matrix<ssi_real_t> *upper = MatrixOps<ssi_real_t>::Clone (lower);
	MatrixOps<ssi_real_t>::Plus (upper, static_cast<ssi_real_t> (delta));
	Matrix<ssi_real_t>* freqs = MatrixOps<ssi_real_t>::Concat (lower, upper, MATRIX_DIMENSION_COL);
	delete lower;
	delete upper;

	Matrix<ssi_real_t> *filterbank = FilterTools::Filterbank ((size >> 1) + 1, sample_rate, freqs, type);
	delete freqs;

	return filterbank;
}

Matrix<std::complex<double>> *FilterTools::ButterPoles (int sections, double frequency) {

	/* Matlab code:

	poles = zeros (sections, 1);

	% calculate angles
	w = pi * freq;
	tanw = sin (w) / cos (w);

	% calculate +im pole position for each section
	for m = sections:2*sections-1
		% Butterworth formula adapted to z-plane
		ang = (2*m + 1) * pi / (4 * sections);
		d = 1 - 2 * tanw * cos(ang) + tanw*tanw;
		poles(m-sections+1) = complex ((1 - tanw*tanw) / d, 2 * tanw * sin (ang) / d);
	end

	*/

	Matrix<std::complex<double>> *poles = MatrixOps<std::complex<double>>::Zeros (sections, 1);

	double w = PI * frequency;
	double tanw = sin (w) / cos (w);

	std::complex<double> *polesptr = poles->data;
	for (int m = sections; m <= 2*sections-1; m++) {
		double ang = (2.0 * m + 1.0) * PI / (4.0 * sections);
		double d = 1.0 - 2.0 * tanw * cos (ang) + tanw * tanw;
		(*polesptr).real ((1.0 - tanw * tanw) / d);
		(*polesptr).imag (2.0 * tanw * sin (ang) / d);
		polesptr++;
	}

	return poles;
}

Matrix<ssi_real_t> *FilterTools::LPButter (ssi_size_t order, double cutoff) {

	/* Matlab code:

		sections = ceil (order/2);
		sos = ones (sections, 6);
		freq = cutoff / sr;
        
        % get pole positions
        poles = butpoles (sections, freq);        

        % convert each conjugate pole pair to difference equation
        for i = 1:sections
            % put in conjugate pole pair
            sos(i,5) = -2 * ssi_real_t (poles(i));
            sos(i,6) = ssi_real_t (poles(i)) * ssi_real_t (poles(i)) + imag (poles(i)) * imag (poles(i));
            % put 2 zeros at (-1,0) and normalize to unity gain
            gain = 4 / (1 + sos(i,5) + sos(i,6));
            sos(i,1) = 1 / gain;
            sos(i,2) = 2 / gain;
            sos(i,3) = 1 / gain;
        end

	*/

	int sections = (order + 1) / 2;
	Matrix<ssi_real_t> *sos = MatrixOps<ssi_real_t>::Ones (sections, 6);
	double freq = cutoff / 2.0;

	Matrix<std::complex<double>> *poles = ButterPoles (sections, freq);
	ssi_real_t *sosptr = sos->data;
	for (int i = 0; i < sections; i++) {
		double pole_ssi_real_t = poles->data[i].real ();
		double pole_imag = poles->data[i].imag ();
		double a1 = -2.0 * pole_ssi_real_t;
		double a2 = pole_ssi_real_t * pole_ssi_real_t + pole_imag * pole_imag;
		double gain = 4.0 / (1.0 + a1 + a2);
		*sosptr++ = static_cast<ssi_real_t> (1.0/gain);
		*sosptr++ = static_cast<ssi_real_t> (2.0/gain);
		*sosptr++ = static_cast<ssi_real_t> (1.0/gain);
		*sosptr++ = static_cast<ssi_real_t> (1.0);
		*sosptr++ = static_cast<ssi_real_t> (a1);
		*sosptr++ = static_cast<ssi_real_t> (a2);
	}
	delete poles;

	return sos;

}

Matrix<ssi_real_t> *FilterTools::HPButter (ssi_size_t order, double cutoff) {

	/* Matlab code:

		sections = ceil (order/2);
		sos = ones (sections, 6);
		freq = cutoff / sr;     
        
        % get pole positions for equivalent low-pass
        poles = butpoles (sections, 0.5 - freq);

        % flip all the poles over to get high pass
        for i = 1:sections
            poles(i) = complex (- ssi_real_t (poles(i)), imag (poles(i)));
        end

        % convert each conjugate pole pair to difference equation
        for i = 1:sections
            % put in conjugate pole pair
            sos(i,5) = -2 * ssi_real_t (poles(i));
            sos(i,6) = ssi_real_t (poles(i)) * ssi_real_t (poles(i)) + imag (poles(i)) * imag (poles(i));
            % put 2 zeros at (1,0) and normalize to unity gain
            gain = abs (2 / sum (sos(i,4:6) .* exp(1).^(2*pi*1i * (0:2) * 0.25)));            
            sos(i,1) = 1 / gain;
            sos(i,2) = -2 / gain;
            sos(i,3) = 1 / gain;
        end

	*/

	int sections = (order + 1) / 2;
	Matrix<ssi_real_t> *sos = MatrixOps<ssi_real_t>::Ones (sections, 6);
	double freq = cutoff / 2.0;

	Matrix<std::complex<double>> *poles = ButterPoles (sections, 0.5 - freq);
	ssi_real_t *sosptr = sos->data;
	for (int i = 0; i < sections; i++) {

		double pole_ssi_real_t = - poles->data[i].real ();
		double pole_imag = poles->data[i].imag ();
		double a1 = -2.0 * pole_ssi_real_t;
		double a2 = pole_ssi_real_t * pole_ssi_real_t + pole_imag * pole_imag;

		Matrix<std::complex<double>> *tmp = MatrixOps<std::complex<double>>::Zeros (3,1);
		tmp->data[0] = std::complex<double> (1.0, 0.0);
		tmp->data[1] = std::complex<double> (cos(0.5*PI), sin(0.5*PI));
		tmp->data[2] = std::complex<double> (cos(PI), sin(PI));
		Matrix<std::complex<double>> *tmp2 = MatrixOps<std::complex<double>>::Zeros (1,3);
		tmp2->data[0] = std::complex<double> (1.0, 0.0);
		tmp2->data[1] = std::complex<double> (a1, 0.0);
		tmp2->data[2] = std::complex<double> (a2, 0.0);
		double gain = abs (2.0 / MatrixOps<std::complex<double>>::MultV (tmp2, tmp));
		delete tmp;
		delete tmp2;

		*sosptr++ = static_cast<ssi_real_t> (1.0/gain);
		*sosptr++ = static_cast<ssi_real_t> (-2.0/gain);
		*sosptr++ = static_cast<ssi_real_t> (1.0/gain);
		*sosptr++ = static_cast<ssi_real_t> (1.0);
		*sosptr++ = static_cast<ssi_real_t> (a1);
		*sosptr++ = static_cast<ssi_real_t> (a2);
	}
	delete poles;

	return sos;
}


Matrix<ssi_real_t> *FilterTools::BPButter (ssi_size_t order, double lcutoff, double hcutoff) {

	/* Matlab code:

		sections = ceil (order/2);
		sos = ones (sections, 6);
		freq = cutoff ./ sr;     
        
        % get pole positions for equivalent low-pass
        poles_tmp = butpoles (sections/2, freq(2) - freq(1));
        
        % translate the poles to band-pass position
        wlo = 2 * pi * freq(1);
        whi = 2 * pi * freq(2);
        ang = cos ((whi + wlo)/2) / cos((whi-wlo)/2);
        poles = zeros (sections,1);
        for i = 1:sections/2
            p1 = complex (ssi_real_t (poles_tmp(i)) + 1, imag (poles_tmp(i)));
            tmp = sqrt (p1*p1*ang*ang*0.25 - poles_tmp(i));
            poles(2*i-1) = p1 * ang * 0.5 + tmp;
            poles(2*i) = p1 * ang * 0.5 - tmp;
        end
        
         % convert each conjugate pole pair to difference equation
         for i = 1:sections
            % put in conjugate pole pair
            sos(i,5) = -2 * ssi_real_t (poles(i));
            sos(i,6) = ssi_real_t (poles(i)) * ssi_real_t (poles(i)) + imag (poles(i)) * imag (poles(i));
            % put 2 zeros at (1,0) and (-1,0) and normalise to unity gain
            gain = abs ((0.1685 + 0.5556i) / sum (sos(i,4:6) .* exp(1).^(2*pi*1i * (0:2) * (freq(1)+freq(2))*0.5)));
            sos(i,1) = 1 / gain;
            sos(i,2) = 0;
            sos(i,3) = -1 / gain;

        end

	*/

	int sections = (order + 1) / 2;
	Matrix<ssi_real_t> *sos = MatrixOps<ssi_real_t>::Ones (sections, 6);
	double lfreq = lcutoff / 2.0;
	double hfreq = hcutoff / 2.0;

	Matrix<std::complex<double>> *poles_tmp = ButterPoles (sections/2, hfreq - lfreq);

	double wlo = 2 * PI * lfreq;
	double whi = 2 * PI * hfreq;
	double ang = cos ((whi + wlo)/2) / cos ((whi - wlo)/2);
	Matrix<std::complex<double>> *poles = MatrixOps<std::complex<double>>::Zeros (sections, 1);
	for (int i = 0; i < sections / 2; i++) {
		std::complex<double> p1 (poles_tmp->data[i].real () + 1, poles_tmp->data[i].imag ());
		std::complex<double> tmp = sqrt (p1*p1*ang*ang*0.25 - poles_tmp->data[i]);
		poles->data[2*i] = p1 * ang * 0.5 + tmp;
		poles->data[2*i+1] = p1 * ang * 0.5 - tmp;
	}
	delete poles_tmp;

	ssi_real_t *sosptr = sos->data;
	for (int i = 0; i < sections; i++) {
		double pole_ssi_real_t = poles->data[i].real ();
		double pole_imag = poles->data[i].imag ();
		double a1 = -2.0 * pole_ssi_real_t;
		double a2 = pole_ssi_real_t * pole_ssi_real_t + pole_imag * pole_imag;

		Matrix<std::complex<double>> *tmp = MatrixOps<std::complex<double>>::Zeros (3,1);
		tmp->data[0] = std::complex<double> (1.0, 0.0);
		tmp->data[1] = std::complex<double> (cos((lfreq+hfreq)*PI), sin((lfreq+hfreq)*PI));
		tmp->data[2] = std::complex<double> (cos(2*(lfreq+hfreq)*PI), sin(2*(lfreq+hfreq)*PI));
		Matrix<std::complex<double>> *tmp2 = MatrixOps<std::complex<double>>::Zeros (1,3);
		tmp2->data[0] = std::complex<double> (1.0, 0.0);
		tmp2->data[1] = std::complex<double> (a1, 0.0);
		tmp2->data[2] = std::complex<double> (a2, 0.0);
		double gain = abs (std::complex<double> (0.1685, 0.5556) / MatrixOps<std::complex<double>>::MultV (tmp2, tmp));
		delete tmp;
		delete tmp2;
		
		*sosptr++ = static_cast<ssi_real_t> (1.0/gain);
		*sosptr++ = 0;
		*sosptr++ = static_cast<ssi_real_t> (-1.0/gain);
		*sosptr++ = static_cast<ssi_real_t> (1.0);
		*sosptr++ = static_cast<ssi_real_t> (a1);
		*sosptr++ = static_cast<ssi_real_t> (a2);
	}
	delete poles;

	return sos;
}

Matrix<ssi_real_t> *FilterTools::DCBlocker (double R) {

	SSI_ASSERT (R >= 0 && R <= 1);

	Matrix<ssi_real_t> *sos = MatrixOps<ssi_real_t>::Zeros (1, 6);
	sos->data[0] = 1;
	sos->data[1] = -1;
	sos->data[2] = 0;
	sos->data[3] = 1;
	sos->data[4] = - static_cast<ssi_real_t> (R);
	sos->data[5] = 0;

	return sos;
}

Matrix<ssi_real_t> *FilterTools::Boost (double sample_rate, double gain, double center_freq, double bandwidth) {

	/* see Julius O. Smith, Introduction to Digital Filters with Audio Application, p. 280 */

	double Q = sample_rate / bandwidth;
	double wcT = 2.0 * PI * (center_freq / sample_rate);

	double K = tan (wcT / 2.0);
	double V = gain;
	double norm = 1.0 + K / Q + K*K;

	Matrix<ssi_real_t> *sos = MatrixOps<ssi_real_t>::Zeros (1, 6);

	sos->data[0] = static_cast<ssi_real_t> ((1.0 + V * K / Q + K*K) / norm);
	sos->data[1] = static_cast<ssi_real_t> ((2.0 * (K*K - 1)) / norm);
	sos->data[2] = static_cast<ssi_real_t> ((1.0 - V * K / Q + K*K) / norm);
	sos->data[3] = 1;
	sos->data[4] = static_cast<ssi_real_t> ((2.0 * (K*K - 1)) / norm);
	sos->data[5] = static_cast<ssi_real_t> ((1.0 - K / Q + K*K) / norm);

	return sos;
}


ssi_real_t FilterTools::MelScale (ssi_real_t in, bool inverse) {

	/* Matlab code

	if inverse == 0
		out = 1127.01048 * log (1 + in ./ 700);
	else
		out = 700 * (exp (in ./ 1127.01048) - 1);
	end

	*/

	return inverse ? 700 * (exp (in / 1127) - 1) : 1127 * log (1 + in / 700);
}

void FilterTools::MelScale (Matrix<ssi_real_t> *in, bool inverse) {

	if (MatrixOps<ssi_real_t>::IsEmpty (in)) {
		return;
	}

	ssi_real_t *dataptr = in->data;
	int elems  = in->rows * in->cols;

	for (int i = 0; i < elems; i++) {
		*(dataptr++) = MelScale (*dataptr, inverse);
	}
}

Matrix<ssi_real_t> *FilterTools::MelBank (int n, 
	ssi_time_t sample_rate, 
	WINDOW_TYPE win_type) {

	int filt_num;
	ssi_real_t min_freq;
	ssi_real_t max_freq;

	if (sample_rate == 22050.0) {
		filt_num = 45;
		min_freq = 130;
		max_freq = 10000;
	} else if (sample_rate == 16000.0) {
		filt_num = 40;
		min_freq = 130;
		max_freq = 6800;
	} else if (sample_rate == 11025.0) {
		filt_num = 36;
		min_freq = 130;
		max_freq = 5400;
	} else if (sample_rate == 8000.0) {
		filt_num = 31;
		min_freq = 200;
		max_freq = 3500;
	} else {
		filt_num = 50;
		min_freq = 0;
		max_freq = static_cast<ssi_real_t> (sample_rate / 2);
	}

	return MelBank (n, sample_rate, filt_num, min_freq, max_freq, win_type);
}

Matrix<ssi_real_t> *FilterTools::MelBank (int n, 
	ssi_time_t sample_rate, 
	int filt_num, 
	ssi_real_t min_freq, 
	ssi_real_t max_freq, 
	WINDOW_TYPE win_type) {

	/* Matlab code:

	min_mel = melscale (min_freq);
	max_mel = melscale (max_freq);
	delta = (max_mel - min_mel) / (filt_num + 1);
	lower = min_mel + delta * (0:filt_num-1);
	upper = lower + 2*delta;
	freqs = melscale ([lower' upper'], 1);
	_filterbank = bandfunc (n, fs, freqs, name);

	*/

	ssi_real_t min_mel = MelScale (min_freq);
	ssi_real_t max_mel = MelScale (max_freq);
	ssi_real_t delta = (max_mel - min_mel) / (filt_num + 1);
	Matrix<ssi_real_t> *lower = MatrixOps<ssi_real_t>::Array (static_cast<ssi_real_t> (0), static_cast<ssi_real_t> (1), static_cast<ssi_real_t> (filt_num - 1), MATRIX_DIMENSION_COL);
	MatrixOps<ssi_real_t>::Mult (lower, delta);
	MatrixOps<ssi_real_t>::Plus (lower, min_mel);
	Matrix<ssi_real_t> *upper = MatrixOps<ssi_real_t>::Clone (lower);
	MatrixOps<ssi_real_t>::Plus (upper, 2 * delta);
	Matrix<ssi_real_t>* freqs = MatrixOps<ssi_real_t>::Concat (lower, upper, MATRIX_DIMENSION_COL);
	MelScale (freqs, true);
	Matrix<ssi_real_t> *_filterbank = FilterTools::Filterbank (n, sample_rate, freqs, win_type);
	
	delete lower;
	delete upper;
	delete freqs;

	return _filterbank;
}

Matrix<ssi_real_t> *FilterTools::DCTMatrix (int size, int cnum) {
	return DCTMatrix (size, 0, cnum);
}

Matrix<ssi_real_t> *FilterTools::DCTMatrix (int size, 
	int cfirst, int clast) {

	int cnum = clast - cfirst;

	/* Matlab code

	[M, N] = size (data);
	_dctmat = cos ((pi/(2*N):pi/N:pi-pi/(2*N))' * (0:cnum-1));
	_dctmat = sqrt (2 / N) * _dctmat; 
	_dctmat(:,1) = (sqrt (2) / 2) * _dctmat(:,1); 

	*/

	ssi_real_t start = static_cast<ssi_real_t> (PI / (size << 1));
	ssi_real_t delta = static_cast<ssi_real_t> (PI / size);
	ssi_real_t stop  = static_cast<ssi_real_t> (PI - PI / (size << 1));
	Matrix<ssi_real_t> *matrixA = MatrixOps<ssi_real_t>::Array (start, delta, stop, MATRIX_DIMENSION_COL);
	Matrix<ssi_real_t> *matrixB = MatrixOps<ssi_real_t>::Array (static_cast<ssi_real_t> (cfirst), static_cast<ssi_real_t> (1), static_cast<ssi_real_t> (clast-1), MATRIX_DIMENSION_ROW);
	Matrix<ssi_real_t> *dctmatrix = MatrixOps<ssi_real_t>::MultM (matrixA, matrixB);
	MatrixOps<ssi_real_t>::Cos (dctmatrix);
	ssi_real_t scalar = sqrt (static_cast<ssi_real_t> (2) / size);
	MatrixOps<ssi_real_t>::Mult (dctmatrix, scalar);
	scalar = static_cast<ssi_real_t> (SQRT2 / 2);
	ssi_real_t *dctmatrixptr = dctmatrix->data;
	for (int i = 0; i < size; i++) {
		*dctmatrixptr *= scalar;
		dctmatrixptr += cnum;
	}
		
	delete matrixA;
	delete matrixB;

	return dctmatrix;
}

void FilterTools::Noise (ssi_stream_t &series,
	ssi_real_t *amplitude) {

	ssi_real_t *ptr = ssi_pcast (ssi_real_t, series.ptr);
	ssi_real_t r;

	for (ssi_size_t i = 0; i < series.num; ++i) {
		for (ssi_size_t j = 0; j < series.dim; ++j) {
			// generate value in range [0..1]
			r = static_cast<ssi_real_t> (static_cast<double> (rand ()) / (static_cast<double> (RAND_MAX) + 1.0));
			// shift value in range [-amp..amp]
			r = amplitude[j] * (2.0f * r - 1.0f);
			// now add value
			*ptr += r;
			++ptr;
		}
	}
}

void FilterTools::Noise (ssi_stream_t &series,
	ssi_real_t noise_ampl,
	double noise_mean,
	double noise_std,
	ssi_time_t cutoff,
	ssi_time_t width) {

	/*

	y = m + s * randn (N,dim); 

	% apply fft
	NFFT = 2^nextpow2 (N);
	fy = fft (y, NFFT);

	% remove frequency in frange
	fdt = (sr/2) / (NFFT/2+1);
	fr = round (frange / fdt);
	fy(1:max(1,fr(1)),:) = 0;
	fy(min(fr(2),NFFT/2+1):NFFT-min(fr(2),NFFT/2),:) = 0;
	fy((NFFT-max(1,fr(1)):NFFT),:) = 0;

	% apply ifft
	y = ifft (fy, N);

	*/

	// determine nfft
	ssi_size_t nfft = log2 (series.num);
	nfft = pow (ssi_cast (ssi_size_t, 2),nfft);
	if (nfft < series.num) {
		nfft = log2 (series.num) + 1;
		nfft = pow (ssi_cast (ssi_size_t, 2), nfft);
	}

	// prepare noise
	ssi_real_t *noise = new ssi_real_t[nfft * series.dim];
	ssi_real_t *pnoise = noise;
	for (ssi_size_t i = 0; i < nfft; ++i) {
		for (ssi_size_t j = 0; j < series.dim; ++j) {
			*pnoise++ = ssi_cast (ssi_real_t, noise_mean + noise_std * ssi_random_distr (0.0,1.0));
		}
	}

	// remove frequeny range
	if (cutoff > 0) {

		FFT fft (nfft, series.dim);
		IFFT ifft (fft.rfft, series.dim);
		std::complex<ssi_real_t> *fnoise = new std::complex<ssi_real_t>[fft.rfft * series.dim];
		fft.transform (nfft, noise, fnoise);

		double fdt = (series.sr / 2.0) / (nfft / 2.0 + 1.0);
		ssi_size_t fmin = ssi_cast (ssi_size_t, cutoff / fdt + 0.5);	
		for (ssi_size_t i = 0; i < fmin; i++) {
			fnoise[i].real (0);
			fnoise[i].imag (0);
		}
		if (width > 0) {
			ssi_size_t fmax = fmin + ssi_cast (ssi_size_t, width / fdt + 0.5);
			for (ssi_size_t i = fmax - 1; i < fft.rfft; i++) {
				fnoise[i].real (0);
				fnoise[i].imag (0);
			}
		}
		ifft.transform (fnoise, noise);

		delete[] fnoise;
	}

 	// add noise to series
	ssi_real_t *pseries = ssi_pcast (ssi_real_t, series.ptr);
	pnoise = noise;
	for (ssi_size_t i = 0; i < series.num; ++i) {
		for (ssi_size_t j = 0; j < series.dim; ++j) {
			*pseries++ += noise_ampl * *pnoise++;
		}
	}

	delete[] noise;
}

}
