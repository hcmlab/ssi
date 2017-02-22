// ev_extract_v1.c
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
#include "ev_messages.h"
#include "ev_mfcc.h"

#include "ev_extract.h"
#include "ev_efeatures.h"
#include "ev_serien.h"

#define MAX_SAMPLES	1024
#define MAX_FEATURES	256
#define MAX_FRAMES      100000
#define MAX_SERIES      256
#define FRAME_LENGTH	8000
#define STATS           9        // mean, maximum, minimum, difference between min & max, variance, median, 
                                 // 1st quartile, 3rd quartile, interquartile range
#define MFCC_FEATURES     36 //vormals 39
#define ENERGY_FEATURES     3

int _fextract_calc(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal, char  *pfile);

int fextract_calc_v1_0(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal) {
	return _fextract_calc(fex,features,signal,NULL);
}

int fextract_calc_v1_1(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal, char *pfile) {
	return _fextract_calc(fex,features,signal,pfile);
}


int _fextract_calc(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal, char *pfile) {
	mx_real_t *pitch=NULL, *hnr=NULL;
	mx_real_t **_mfcc_series, **_energy_series;
	mx_real_t overall_max=0, overall_max_ind=0;
	int nframes=0, i, j, n_voiced_frames, max;
	int last_max=0, last_min=0, ser=0, max_sum=0, en_mean=0, fcount=0;
	fextract_series_t * serie=NULL;
	
	if (fex->frame_len==0) {
		rs_warning("Cannot compute features for segment of size 0!");
		return -1;
	}
	
	fex->n_features=V1_N_FEATURES;
	
	_mfcc_series= (mx_real_t **) rs_malloc(MFCC_FEATURES * sizeof(mx_real_t *), "mfcc feature series");
	for (i=0;i<MFCC_FEATURES;i++)
		_mfcc_series[i]= (mx_real_t *) rs_malloc(MAX_FRAMES * sizeof(mx_real_t), "mfcc features");
	_energy_series= (mx_real_t **) rs_malloc(ENERGY_FEATURES * sizeof(mx_real_t *), "energy feature series");
	for (i=0;i<3;i++)
		_energy_series[i]= (mx_real_t *) rs_malloc(MAX_FRAMES * sizeof(mx_real_t), "energy features");

	if (pfile) {
		FILE *pfile_fp= fopen(pfile,"r");
		if (!pfile_fp)
			rs_warning("pitch file %s not found!",pfile);
		/* read external pitch values */
		pitch = (mx_real_t *) rs_malloc(MAX_FRAMES * sizeof(mx_real_t),"external pitch values");
		j=MAX_FRAMES;
		i=0;
		while (fscanf(pfile_fp,"%g",&pitch[i])==1) {
			if (j == i+1) {
				j+=MAX_FRAMES;
				pitch = (mx_real_t *) rs_realloc(pitch,j*sizeof(mx_real_t),"external pitch values");
			}
			i++;
		}
		nframes=i;
		fclose(pfile_fp);
	}
	else {
		pitch= pitch_calc (fex->pitch,signal,fex->frame_len);
		nframes=fex->pitch->nframes;
	}

	if (!pitch) {
		for (;fcount<STATS*9+7;fcount++)
			features[fcount]=-1.0;
		n_voiced_frames=-1.0;
	}
	else {
		const int series_size=9;
		// @begin_change_johannes
		//fextract_series_t *serien[series_size];
		fextract_series_t *serien[9];
		// @end_change_johannes
	
//		printRawXML(pitch,nframes,"pitch");

		pitch_frame_candidates_destroy(fex->pitch);
		for (i=0;i<series_size;i++)
			serien[i]=series_create(MAX_SERIES);
	
		if (pitch[0]!=0)
			series_add(serien[0],pitch[0]);
	
		for (i=1;i<nframes-1;i++) {
			if (pitch[i]!=0)
				series_add(serien[0],pitch[i]);
	    
			if (pitch[i-1]<pitch[i] && pitch[i]>pitch[i+1]) {
				series_add(serien[1],pitch[i]);
				if (pitch[i] > overall_max) {
					overall_max=pitch[i];
					overall_max_ind=i;
				}
				series_add(serien[2],i-last_min);
				series_add(serien[3],pitch[i]-pitch[last_min]);
				series_add(serien[4],(pitch[i]-pitch[last_min])/(i-last_min));
				last_max=i;
			}
			else
				if (pitch[i-1]>pitch[i] && pitch[i]<pitch[i+1]) {
				series_add(serien[5],pitch[i]);
				series_add(serien[6],i-last_max);
				series_add(serien[7],pitch[i]-pitch[last_max]);
				series_add(serien[8],(pitch[i]-pitch[last_max])/(i-last_max));
				last_min=i;
				}
		}
	
		overall_max_ind = 1.0 * (overall_max_ind+1) / nframes;
	
		if (nframes >1 && pitch[nframes-1]!=0)
			series_add(serien[0],pitch[nframes-1]);
	
		for (i=0; i<series_size; i++) {
			getStatistics(features+fcount,serien[i]->series,serien[i]->nSeries); //pitch statistics features
			fcount+=STATS;
		}
      
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
	
		features[fcount++]=1.0 * serien[1]->nSeries/nframes; // Anzahl Maxima pro Frames pro Segment
		features[fcount++]=1.0 * serien[5]->nSeries/nframes; // Anzahl Minima pro Frames pro Segment
		features[fcount++]=overall_max_ind; // Position des Maximums
		n_voiced_frames=serien[0]->nSeries;
	
		for (i=0;i<series_size;i++)
			series_destroy(serien[i]);
	}

	getEnergy_and_MFCC(fex->mfcc, signal, fex->frame_len, &_energy_series, &_mfcc_series, &ser);
//  ser=0;
	if (ser) {
		const int series_size = 8;
		// @begin_change_johannes
		//fextract_series_t *serien[series_size];
		fextract_series_t *serien[8];
		// @end_change_johannes
		for (i=0;i<series_size;i++)
			serien[i]=series_create(MAX_SERIES);

		fcount+=getMFCCFeatures(features+fcount, _mfcc_series,ser);

		/* Energie-Merkmale: */

		last_max=last_min=0;

		for (i=1;i<ser-1;i++) {
			if (_energy_series[0][i-1]<_energy_series[0][i] && _energy_series[0][i]>_energy_series[0][i+1]) {
				series_add(serien[0], _energy_series[0][i]); //Maxima
				max_sum += _energy_series[0][i];
				series_add(serien[1], i-last_min); // Laenge der Maxima
				series_add(serien[2], _energy_series[0][i]-_energy_series[0][last_min]); // Groesse der Maxima
				series_add(serien[3], (_energy_series[0][i]-_energy_series[0][last_min])/(i-last_min)); // Steigung der Maxima
				last_max=i;
			}
			else
				if (_energy_series[0][i-1]>_energy_series[0][i] && _energy_series[0][i]<_energy_series[0][i+1]) {
				series_add(serien[4], _energy_series[0][i]); //Maxima
				series_add(serien[5], i-last_max); // Laenge der Maxima
				series_add(serien[6], _energy_series[0][i]-_energy_series[0][last_max]); // Groesse der Maxima
				series_add(serien[7], (_energy_series[0][i]-_energy_series[0][last_min])/(i-last_max)); // Steigung der Maxima
				last_max=i;
				}
		}

		getStatistics(features+fcount,_energy_series[0],ser); // energy statistics
		en_mean=features[fcount+1];
		fcount+=STATS;

		for (i=0;i<series_size;i++) {
			getStatistics(features+fcount,serien[i]->series,serien[i]->nSeries);
			fcount+=STATS;
		}

		max=serien[0]->nSeries;

		for (j=1;j<3;j++) {

			series_reset(serien[0]); series_reset(serien[1]);

			for (i=1;i<ser-1;i++) {
				if (_energy_series[j][i-1]<_energy_series[j][i] && _energy_series[j][i]>_energy_series[j][i+1]) {
					series_add(serien[0], _energy_series[j][i]);
				}
				else
					if (_energy_series[j][i-1]>_energy_series[j][i] && _energy_series[j][i]<_energy_series[j][i+1]) {
					series_add(serien[1], _energy_series[j][i]);
					}
			}

			getStatistics(features+fcount,_energy_series[j],ser); // energy delta statistics
			fcount+=STATS;
			getStatistics(features+fcount,serien[0]->series,serien[0]->nSeries);
			fcount+=STATS;
			getStatistics(features+fcount,serien[1]->series,serien[1]->nSeries);
			fcount+=STATS;
		}


  
		features[fcount++]= max? (1.0 * max_sum/max) - en_mean:0; // Mittelwert der Maxima minus globaler Mittelwert
		features[fcount++]= (1.0 * max)/ser; // Anzahl Maxima pro Frames pro Segment
		for (i=0;i<series_size;i++)
			series_destroy(serien[i]);
	} 
	else {
		for (;fcount < 141*STATS+9;fcount++)
			features[fcount]=-1;
	}

	/* Voiced/Unvoiced-Merkmal: */
	features[fcount++]=nframes ? (1.0 * n_voiced_frames)/nframes : -1;  // Anzahl stimmloser Frames pro Frames pro Segment
  
	/* Spectral Center of gravity-Merkmale: */
	serie = getOnlyCoG(signal,fex->frame_len);
	if (serie) {
		getStatistics(features+fcount,serie->series,serie->nSeries);
		series_destroy(serie);
		fcount+=STATS;
	}
	else
		for (;fcount< 142*STATS+10;fcount++)
			features[fcount]=-1;

	/* Dauer-Merkmal: */
	features[fcount++]=fex->frame_len;

	/* Harmonics-to-Noise-Ratio-Merkmale: */
	hnr=hnr_calc(fex->hnr,signal,fex->frame_len);
	nframes=fex->hnr->nframes;

	if (!hnr) 
		for (;fcount<145*STATS+11;fcount++) 
			features[fcount]=-1.0;
	else {
		const int series_size=3;
		// @begin_change_johannes
		//fextract_series_t *serien[series_size];
		fextract_series_t *serien[3];
		// @end_change_johannes

		pitch_frame_candidates_destroy(fex->hnr);
		for (i=0;i<series_size;i++)
			serien[i]=series_create(MAX_SERIES);

		if (hnr[0]!=0)
			series_add(serien[0],hnr[0]);

		for (i=1;i<nframes-1;i++) {
			if (hnr[i]!=-200) {
				series_add(serien[0],hnr[i]);
 
				if ((hnr[i-1]!=-200) && (hnr[i+1]!=200)) { // stimmlose Bereiche nicht beruecksichtigen
					if (hnr[i-1]<hnr[i] && hnr[i]>hnr[i+1])
						series_add(serien[1],hnr[i]);
					else
						if (hnr[i-1]>hnr[i] && hnr[i]<hnr[i+1])
							series_add(serien[2],hnr[i]);
				}
			}
		}

		if (nframes >1 && hnr[nframes-1]!=0)
			series_add(serien[0],hnr[nframes-1]);

		for (i=0; i<series_size; i++) {
			getStatistics(features+fcount,serien[i]->series,serien[i]->nSeries); //hnr statistics features
			fcount+=STATS;
		}
	}
  
	/* Aufraeumen... */
	for (i=0;i<MFCC_FEATURES;i++)
		if (_mfcc_series[i])
			rs_free(_mfcc_series[i]);
	if (_mfcc_series)
		rs_free(_mfcc_series);
  
	for (i=0;i<ENERGY_FEATURES;i++)
		if (_energy_series[i])
			rs_free(_energy_series[i]);
	if (_energy_series)
		rs_free(_energy_series);

	if (pitch)
		rs_free(pitch);
	if (hnr)
		rs_free(hnr);

	/*   for (;fcount<V1_N_FEATURES;fcount++) */
	/*       features[fcount]=0.0; */

	if (fcount != V1_N_FEATURES)
		rs_error("number of features is not correct (%d expected, %d calculated)!",V1_N_FEATURES,fcount);

	return V1_N_FEATURES;
} 

