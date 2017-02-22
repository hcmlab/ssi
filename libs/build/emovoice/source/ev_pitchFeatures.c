// ev_pitchFeatures.c
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

#include "ev_messages.h"
#include "ev_memory.h"

#include "ev_extract.h"
#include "ev_serien.h"
#include "ev_efeatures.h"

#define DERIV_LENGTH    5

int getPitchFeatures(fextract_t *fex, dsp_sample_t *signal, mx_real_t *features, mx_real_t **pitch_ret){
	mx_real_t *pitch;
	int i, fcount=0, nframes;
	int n_rising=0, n_falling=0, last_extreme=-1, extr_start=0;
	int overall_max_ind=-1, overall_min_ind=-1;
	mx_real_t overall_max=0, overall_min = fex->pitch->maximumPitch*2;

	*pitch_ret = pitch_calc (fex->pitch,signal,fex->frame_len);
	pitch = *pitch_ret;
	
    
	if (pitch) {
		const int series_size=8+7+7;
		// @begin_change_johannes
		//fextract_series_t *serien[series_size];
		fextract_series_t *serien[8+7+7];
		// @begin_change_johannes
		mx_real_t *derivation = (mx_real_t *) rs_malloc(3*sizeof(mx_real_t),"pitch derivation value");
		dsp_delay_t *deriv, *deriv_log;

		// fuer HT-Transformation
		mx_real_t median;

		nframes= fex->pitch->nframes;
		pitch_frame_candidates_destroy(fex->pitch);
				
		deriv = dsp_delay_create(DERIV_LENGTH,sizeof(mx_real_t),1);
		deriv_log = dsp_delay_create(DERIV_LENGTH,sizeof(mx_real_t),1);

		for (i=0;i<series_size;i++)
			serien[i]=series_create(MAX_SERIES);

		if (pitch[0]!=0) {
			series_add(serien[0],pitch[0]);
			series_add(serien[8],log(pitch[0]));
			last_extreme=0;
			extr_start=1;

	    	//Ableitung
			dsp_delay_push(deriv, pitch);
			derivation[0]=log(pitch[0]);
			dsp_delay_push(deriv_log, derivation);
	    
	    
		    // rising?
			if (pitch[1] >pitch[0])
				n_rising++;

		    // falling?
			if (pitch[0] > pitch[1] && pitch[1] !=0)
				n_falling++;
	    
			if (pitch[0] > overall_max) {
				overall_max_ind=0;
				overall_max=pitch[0];
			}
			if (pitch[0] <overall_min) {
				overall_min_ind=0;
				overall_min=pitch[0];
			}
		}

		for (i=1;i<nframes-1;i++) {
		    //basic ohne stimmlos
			if (pitch[i]!=0) {
				series_add(serien[0],pitch[i]);
				series_add(serien[8],log(pitch[i]));
			}

	    //nur maxima
			if (pitch[i-1]<pitch[i] && pitch[i]>pitch[i+1]) 
				if (pitch[i-1]!=0 && pitch[i]!=0 && pitch[i+1]!=0) {
				series_add(serien[1],pitch[i]);
				series_add(serien[9],log(pitch[i]));
				}
			
	    //nur minima
				if (pitch[i-1]>pitch[i] && pitch[i]<pitch[i+1]) 
					if (pitch[i-1]!=0 && pitch[i]!=0 && pitch[i+1]!=0) {
					series_add(serien[2],pitch[i]);
					series_add(serien[10],log(pitch[i]));
					}

					if (pitch[i-1]==0 && pitch[i]!=0) {
						extr_start=1;
						last_extreme=i;
					}
					else 
						if (extr_start && pitch[i]!=0 && pitch[i-1]!=0 && 
										((pitch[i-1] < pitch[i] && pitch[i+1] < pitch[i]) ||
										(pitch[i-1] > pitch[i] && pitch[i+1] > pitch[i]) ||
										pitch[i+1]==0)) {
		    		//Frequenzabstandsbetrag zw. Extrema
						series_add(serien[3],fabs(pitch[i]-pitch[last_extreme]));
						series_add(serien[11],fabs(log(pitch[i])-log(pitch[last_extreme])));
		    		//Steigungsbetrag zw. Extrema
						series_add(serien[4],fabs((pitch[i]-pitch[last_extreme])/(i-last_extreme)));
						series_add(serien[12],fabs((log(pitch[i])-log(pitch[last_extreme]))/(i-last_extreme)));
		    		//zeitl. Abstand zw. Extrema
						series_add(serien[7],i-last_extreme);
						if (pitch[i+1]==0) 
							extr_start=0;
						last_extreme=i;
										}

		    //Ableitung
										if (pitch[i] !=0) {
				//basic Ableitung
										derivation[0]=pitch[i];
										dsp_delay_push(deriv, derivation);
										if ((dsp_deriv(derivation+1, deriv, 1, DERIV_LENGTH) == 0) && 
										(dsp_deriv(derivation+2, deriv, 2, DERIV_LENGTH) == 0)) {
										dsp_tirol(derivation,deriv);
										series_add(serien[5],derivation[1]);
										series_add(serien[6],derivation[2]);
										}

				//log Ableitung
										derivation[0]=log(pitch[i]);
										dsp_delay_push(deriv_log, derivation);
				//basic Ableitung
										if ((dsp_deriv(derivation+1, deriv_log, 1, DERIV_LENGTH) == 0) && 
										(dsp_deriv(derivation+2, deriv_log, 2, DERIV_LENGTH) == 0)) {
										dsp_tirol(derivation,deriv_log);
										series_add(serien[13],derivation[1]);
										series_add(serien[14],derivation[2]);
										}
										}
										else 
										if (pitch[i-1]!=0) {
										dsp_delay_flush(deriv);
										dsp_delay_flush(deriv_log);
										}

		    // rising?
										if (pitch[i-1] <pitch[i] && (pitch[i]< pitch[i+1] || pitch[i+1]==0)) 
										n_rising++;
	    
	    	// falling?
										if (pitch[i+1] < pitch[i] && (pitch[i] < pitch[i-1] || pitch[i-1]==0)) 
										n_falling++;
	    	    
										if (pitch[i] > overall_max) {
										overall_max_ind=i;
										overall_max=pitch[i];
										}
										if (pitch[i] !=0 && pitch[i] <overall_min) {
										overall_min_ind=i;
										overall_min=pitch[i];
										}
		}
	
		if (nframes >1 && pitch[nframes-1]!=0) {
			i=nframes-1;
			series_add(serien[0],pitch[i]);
			series_add(serien[8],log(pitch[i]));
			if (extr_start) {
				//Frequenzabstandsbetrag zw. Extrema
				series_add(serien[3],fabs(pitch[i]-pitch[last_extreme]));
				series_add(serien[11],fabs(log(pitch[i])-log(pitch[last_extreme])));
				//Steigungsbetrag zw. Extrema
				series_add(serien[4],fabs((pitch[i]-pitch[last_extreme])/(i-last_extreme)));
				series_add(serien[12],fabs((log(pitch[i])-log(pitch[last_extreme]))/(i-last_extreme)));
				//zeitl. Abstand zw. Extrema
				series_add(serien[7],i-last_extreme);
			}

			//basic Ableitung
			derivation[0]=pitch[i];
			dsp_delay_push(deriv, derivation);
			if ((dsp_deriv(derivation+1, deriv, 1, DERIV_LENGTH) == 0) && 
						  (dsp_deriv(derivation+2, deriv, 2, DERIV_LENGTH) == 0)) {
				dsp_tirol(derivation,deriv);
				series_add(serien[5],derivation[1]);
				series_add(serien[6],derivation[2]);
						  }

			//log Ableitung
						  derivation[0]=log(pitch[i]);
						  dsp_delay_push(deriv_log, derivation);
			//basic Ableitung
						  if ((dsp_deriv(derivation+1, deriv_log, 1, DERIV_LENGTH) == 0) && 
										(dsp_deriv(derivation+2, deriv_log, 2, DERIV_LENGTH) == 0)) {
							  dsp_tirol(derivation,deriv_log);
							  series_add(serien[13],derivation[1]);
							  series_add(serien[14],derivation[2]);
										}

		    // rising?
										if (pitch[i-1] <pitch[i] && pitch[i-1]!=0)
										n_rising++;

		    // falling?
										if (pitch[i] < pitch[i-1])
										n_falling++;
	    
										if (pitch[i] > overall_max) {
										overall_max_ind=i;
										overall_max=pitch[i];
										}
										if (pitch[i] <overall_min) {
										overall_min_ind=i;
										overall_min=pitch[i];
										}
		}
	
		for (i=0; i<series_size-7; i++) 
			fcount+=getStatistics(features+fcount,serien[i]->series,serien[i]->nSeries); 

		//HT-Trans
		//1. Serie: index 15
		dsp_delay_destroy(deriv);
		deriv = dsp_delay_create(DERIV_LENGTH,sizeof(mx_real_t),1);
		median=features[8*STATS+5];
		last_extreme=-1;
		extr_start=0;
	
		if (pitch[0]!=0) {
			series_add(serien[15],log(pitch[0])-median);
			last_extreme=0;
			extr_start=1;

	    	//Ableitung
			derivation[0]=log(pitch[0])-median;
			dsp_delay_push(deriv, derivation);
		}
	
		for (i=1;i<nframes-1;i++) {
		    //basic ohne stimmlos
			if (pitch[i]!=0) 
				series_add(serien[15],log(pitch[i])-median);
	    
		    //nur maxima
			if (pitch[i-1]<pitch[i] && pitch[i]>pitch[i+1]) 
				if (pitch[i-1]!=0 && pitch[i]!=0 && pitch[i+1]!=0) 
					series_add(serien[16],log(pitch[i])-median);
		
	    	//nur minima
			if (pitch[i-1]>pitch[i] && pitch[i]<pitch[i+1]) 
				if (pitch[i-1]!=0 && pitch[i]!=0 && pitch[i+1]!=0) 
					series_add(serien[17],log(pitch[i])-median);
		
			if (pitch[i-1]==0 && pitch[i]!=0) {
				extr_start=1;
				last_extreme=i;
			}
			else 
				if (extr_start && pitch[i]!=0 && pitch[i-1]!=0 && 
								((pitch[i-1] < pitch[i] && pitch[i+1] < pitch[i]) ||
								(pitch[i-1] > pitch[i] && pitch[i+1] > pitch[i]) ||
								pitch[i+1]==0)) {
		    		//Frequenzabstandsbetrag zw. Extrema
				series_add(serien[18],fabs(log(pitch[i])-log(pitch[last_extreme])-2*median));
		    		//Steigungsbetrag zw. Extrema
				series_add(serien[19],fabs((log(pitch[i])-log(pitch[last_extreme])-2*median)/(i-last_extreme)));
				if (pitch[i+1]==0) 
					extr_start=0;
				last_extreme=i;
								}

		    //Ableitung
								if (pitch[i] !=0) {
				//basic Ableitung
									derivation[0]=log(pitch[i])-median;
									dsp_delay_push(deriv, derivation);
									if ((dsp_deriv(derivation+1, deriv, 1, DERIV_LENGTH) == 0) && 
										(dsp_deriv(derivation+2, deriv, 2, DERIV_LENGTH) == 0)) {
										dsp_tirol(derivation,deriv);
										series_add(serien[20],derivation[1]);
										series_add(serien[21],derivation[2]);
										}
								}
								else 
									if (pitch[i-1]!=0) 
										dsp_delay_flush(deriv);
		}
	
		if (nframes >1 && pitch[nframes-1]!=0) {
			i=nframes-1;
			series_add(serien[15],log(pitch[i])-median);
			if (extr_start) {
				//Frequenzabstandsbetrag zw. Extrema
				series_add(serien[18],fabs(log(pitch[i])-log(pitch[last_extreme])-2*median));
				//Steigungsbetrag zw. Extrema
				series_add(serien[19],fabs((log(pitch[i])-log(pitch[last_extreme])-2*median)/(i-last_extreme)));
			}
		    //Ableitung
			derivation[0]=log(pitch[i])-median;
			dsp_delay_push(deriv, derivation);
			if ((dsp_deriv(derivation+1, deriv, 1, DERIV_LENGTH) == 0) && 
						  (dsp_deriv(derivation+2, deriv, 2, DERIV_LENGTH) == 0)) {
				dsp_tirol(derivation,deriv);
				series_add(serien[20],derivation[1]);
				series_add(serien[21],derivation[2]);
						  }
		}
		
		//end HT-Trans
		dsp_delay_destroy(deriv);
		dsp_delay_destroy(deriv_log);
		rs_free(derivation);

        //pitch statistics features
		for (i=15; i<series_size; i++) 
			fcount+=getStatistics(features+fcount,serien[i]->series,serien[i]->nSeries); 
	      
     	// Anzahl Maxima pro Frames pro Segment
		features[fcount++]=1.0 * serien[1]->nSeries/nframes; 
		// Anzahl Minima pro Frames pro Segment
		features[fcount++]=1.0 * serien[2]->nSeries/nframes;
		// Position des globalen Maximums
		features[fcount++]=(overall_max_ind!=-1)?1.0*(overall_max_ind+1)/nframes:-1; 
		// Position des globalen Minimums
		features[fcount++]=(overall_min_ind!=-1)?1.0*(overall_min_ind+1)/nframes:-1; 
		// Anzahl falling
		features[fcount++]=1.0*n_falling/nframes; 
		// Anzahl rising
		features[fcount++]=1.0*n_rising/nframes; 

		// normierter Mittelwert, normierter Median, normiertes 1. Quartil, normiertes 3. Quartil
		if (features[1]-features[2]) {
			features[fcount++]=(features[0]-features[2])/(features[1]-features[2]);
			features[fcount++]=(features[5]-features[2])/(features[1]-features[2]);
			features[fcount++]=(features[6]-features[2])/(features[1]-features[2]);
			features[fcount++]=(features[7]-features[2])/(features[1]-features[2]);
		}
		else {
			features[fcount++]=0;
			features[fcount++]=0;
			features[fcount++]=0;
			features[fcount++]=0;
		}
	
		for (i=0;i<series_size;i++)
			series_destroy(serien[i]);
	}
	else {
		for (;fcount<N_PITCH_FEATURES;fcount++)
			features[fcount]=-1.0;
	}

	return fcount;
    
}

