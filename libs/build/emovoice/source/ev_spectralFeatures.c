// ev_spectralFeatures.c
// author: Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// created: 
// Copyright (C) 2003-9 University of Augsburg, Thurid Vogt
//
// *************************************************************************************************
//
// This file is part of EmoVoice/SSI developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "ev_memory.h"

#include "ev_extract.h"
#include "ev_serien.h"
#include "ev_spectrum.h"
#include "ev_efeatures.h"

#define SP_FRAME_LEN 256
#define SP_FRAME_RATE 160
#define MAX_SAMPLES	1024
#define N_BASIC_SERIES 5

fextract_series_t **getBasicSeries(dsp_sample_t *signal, int nframes);


int getSpectralFeatures(mx_real_t *features,dsp_sample_t *signal, int nframes) { 
	int fcount=0,i;
	fextract_series_t **spectral_series;
	
	spectral_series = getBasicSeries(signal,nframes);

	for (i=0;i<N_BASIC_SERIES;i++) {
		fcount+=getStatistics(features+fcount,spectral_series[i]->series,spectral_series[i]->nSeries);
		series_destroy(spectral_series[i]);
	}
	rs_free(spectral_series);
		
	return fcount;
}


fextract_series_t **getBasicSeries(dsp_sample_t *signal, int nframes) {
	int n_samples, offset=0, frames=0, i;
	dsp_sample_t s[MAX_SAMPLES], last_s[MAX_SAMPLES];
	dsp_sample_t *frame, *last_frame, *__tmp;
	mx_real_t *reg=NULL, *window;
	mx_complex_t *z;
	fextract_series_t **serien;
	
	serien = (fextract_series_t **) rs_malloc(sizeof(fextract_series_t *) *N_BASIC_SERIES,"basic spectral series");

	for (i=0;i<N_BASIC_SERIES;i++)
    	serien[i]=series_create(MAX_SERIES);

	frame = s; 
	last_frame = last_s;

	window = (mx_real_t *) rs_malloc(sizeof(mx_real_t) * SP_FRAME_LEN, "w");
	z = (mx_complex_t *) rs_malloc(sizeof(mx_complex_t) * SP_FRAME_LEN, "z");

	n_samples = next_mfcc_frame(frame, NULL, SP_FRAME_LEN, SP_FRAME_RATE, signal,nframes);
	offset += SP_FRAME_LEN;

	while (n_samples > 0) {
    	if (n_samples < SP_FRAME_LEN)
    		break;

    	frames++;
	
		// FFT: ev. mit prae-emphase und zero-padding
		/* ... Hamming-Fenstern ... */
		dsp_window_hamming(window, frame, SP_FRAME_LEN);
		
 		for (i = 0; i < SP_FRAME_LEN; i++) {
			mx_re(z[i]) = window[i];
			mx_im(z[i]) = 0.0;
		}
				
		/* FFT */
		dsp_xfft(z, SP_FRAME_LEN, 0);
    
    	series_add(serien[0],get_CoG(z,SP_FRAME_LEN));
    	series_add(serien[1],get_quantile_range(z,SP_FRAME_LEN));
    	series_add(serien[2],get_minmax_slope(z,SP_FRAME_LEN));
    	reg=get_regression(z,SP_FRAME_LEN);
    	series_add(serien[3],reg[0]);
    	series_add(serien[4],reg[1]);
		rs_free(reg);
    
    
	    /* ... und naechsten Frame einlesen */
    	__tmp = last_frame;
    	last_frame = frame;
    	frame = __tmp;
    	n_samples = next_mfcc_frame(frame, last_frame, SP_FRAME_LEN, SP_FRAME_RATE, signal+offset,nframes-offset);
    	offset += SP_FRAME_RATE;
	}

	rs_free(window);
	rs_free(z);

	return serien;
}

fextract_series_t *getOnlyCoG(dsp_sample_t *signal, int nframes) {
	int n_samples, offset=0, frames=0, i;
	dsp_sample_t s[MAX_SAMPLES], last_s[MAX_SAMPLES];
	dsp_sample_t *frame, *last_frame, *__tmp;
	mx_real_t *window;
	mx_complex_t *z;
	fextract_series_t *serie=NULL;
	
   	serie=series_create(MAX_SERIES);

	frame = s; 
	last_frame = last_s;

	window = (mx_real_t *) rs_malloc(sizeof(mx_real_t) * SP_FRAME_LEN, "w");
	z = (mx_complex_t *) rs_malloc(sizeof(mx_complex_t) * SP_FRAME_LEN, "z");

	n_samples = next_mfcc_frame(frame, NULL, SP_FRAME_LEN, SP_FRAME_RATE, signal,nframes);
	offset += SP_FRAME_LEN;

	while (n_samples > 0) {
    	if (n_samples < SP_FRAME_LEN)
    		break;

    	frames++;
	
		// FFT: ev. mit prae-emphase und zero-padding
		/* ... Hamming-Fenstern ... */
		dsp_window_hamming(window, frame, SP_FRAME_LEN);
		
 		for (i = 0; i < SP_FRAME_LEN; i++) {
			mx_re(z[i]) = window[i];
			mx_im(z[i]) = 0.0;
		}
				
		/* FFT */
		dsp_xfft(z, SP_FRAME_LEN, 0);
    
    	series_add(serie,get_CoG(z,SP_FRAME_LEN));
    
    
	    /* ... und naechsten Frame einlesen */
    	__tmp = last_frame;
    	last_frame = frame;
    	frame = __tmp;
    	n_samples = next_mfcc_frame(frame, last_frame, SP_FRAME_LEN, SP_FRAME_RATE, signal+offset,nframes-offset);
    	offset += SP_FRAME_RATE;
	}

	rs_free(window);
	rs_free(z);

	return serie;
}
