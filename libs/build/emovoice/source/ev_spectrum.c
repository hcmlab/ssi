// ev_spectrum.c
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

#include "ev_real.h"
#include "ev_complex.h"
#include "ev_memory.h"
#include "ev_messages.h"

#include "ev_spectrum.h"
#include "ev_efeatures.h"

mx_real_t get_CoG(mx_complex_t *samples, int n_samples) {
    int i;
    mx_real_t sumenergy = 0.0, sumfenergy = 0.0, rate = 1.0*SAMPLERATE/n_samples, result;
 
    for (i = 1; i < n_samples / 2; i ++) {
	mx_real_t re = mx_re(samples[i]), im = mx_im(samples[i]), energy = re * re + im * im;
	mx_real_t f = i * rate;
	energy = sqrt (energy);
	sumenergy += energy;
	sumfenergy += f * energy;
    }
    if (!sumenergy) 
	return 0;
    
    result = sumfenergy / sumenergy;
    return result;
}

mx_real_t get_quantile_range(mx_complex_t *samples, int n_samples) {
	int p1, p9, length;
	mx_real_t re1, im1, energy1, re9, im9, energy9;

	length=n_samples/2-1;
	p1 = 0.25*length;
	p9 = 0.75*length;

	if (length <=0 || p9==0)
		return 0;
		
	re1 = mx_re(samples[p1]);
	im1 = mx_im(samples[p1]);
	energy1 = sqrt(re1 * re1 + im1 * im1);
	re9 = mx_re(samples[p9]);
	im9 = mx_im(samples[p9]);
	energy9 = sqrt(re9 * re9 + im9 * im9);

	return energy1 - energy9;
}

mx_real_t get_minmax_slope(mx_complex_t *samples, int n_samples) {
	int i, max_ind=-1, min_ind=-1;
	mx_real_t max=-MX_REAL_MAX, min=MX_REAL_MAX, slope;
	
    for (i = 1; i < n_samples / 2; i ++) {
		mx_real_t re = mx_re(samples[i]), im = mx_im(samples[i]), energy = re * re + im * im;
		energy = sqrt (energy);
		if (energy > max) {
			max=energy;
			max_ind=i;
		}
		else 
			if (energy <min) {
				min=energy;
				min_ind=i;
			}
    }
    
    if (min_ind==-1 || max_ind==-1)
    	return 0;
    
    	
    if (min_ind < max_ind)
    	slope = (max-min)/(max_ind - min_ind);	
    else
    	slope = (min-max)/(min_ind - max_ind);	

    	
    return slope;
}

mx_real_t *get_regression(mx_complex_t *samples, int n_samples) {
	int i, length = n_samples/2;
	mx_real_t x_mean=0, y_mean=0, rate = 1.0*SAMPLERATE/n_samples, up_sum=0, down_sum=0;
	mx_real_t *reg = (mx_real_t *) rs_malloc(2*sizeof(mx_real_t),"regression coefficients");  

	mx_real_t *en = (mx_real_t *) rs_malloc((length-1)*sizeof(mx_real_t),"regression coefficients");  
	mx_real_t *ra = (mx_real_t *) rs_malloc((length-1)*sizeof(mx_real_t),"regression coefficients");  
	
	for (i = 1; i < length; i ++) {
		mx_real_t re = mx_re(samples[i]), im = mx_im(samples[i]), energy;
		mx_real_t f = i * rate; 
		energy = re * re + im * im;
		energy = sqrt(energy);

		en[i-1]=energy;
		ra[i-1]=f;
		x_mean+=energy;
		y_mean+=f;
	} 
	
	x_mean/=length-1;
	y_mean/=length-1;
	
	for (i=0;i<length-1;i++) {
		up_sum+=(ra[i]-y_mean)*(en[i]-x_mean);
		down_sum+=(ra[i]-y_mean)*(ra[i]-y_mean);
	}
	
	reg[0]=up_sum/down_sum;
	reg[1]=x_mean-reg[0]*y_mean;
	
	rs_free(en);
	rs_free(ra);
	
	return reg;
}
