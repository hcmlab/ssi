// ev_extract.c
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


#include <stdarg.h>
#include <string.h>

#include "ev_memory.h"
#include "ev_messages.h"
#include "ev_mfcc.h"

#include "ev_extract.h"
#include "ev_efeatures.h"
#include "ev_serien.h"

#define MAX_SAMPLES	1024
#define MAX_FEATURES	256
#define MAX_FRAMES      100000
#define FRAME_LENGTH	8000

const int pitch2mfcc_shift = 960;

void _channel_reset(dsp_channel_t *channel);
void _mfcc_reset(dsp_fextract_t *mfcc);
void _histogram_reset(mx_histogram_t *ehist);
void mfcc_destroy(dsp_fextract_t *fex);

int fextract_calc_v1_0(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal);
int fextract_calc_v1_1(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal, char *pfile);
int fextract_calc_v2_0(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal);

fextract_t *fextract_create(int frame_len, char *m_e_params,int maj, int min) {
	fextract_t *fex;
	dsp_fextract_t *mfcc;

	fex = (fextract_t *) rs_malloc(sizeof(fextract_t),"emotion feature extraction data");
  
	if (maj==1)
		fex->n_features = V1_N_FEATURES;
	else
		if (maj==2)
			fex->n_features = V2_N_FEATURES;
	else
		rs_error("Unrecognised feature extraction version!");

  
	/* Abtastrate etc. ... */
	fex->samplerate = SAMPLERATE;
	fex->frame_len = frame_len; /* global frame length (in frames) */
  
	fex->pitch = pitch_create(AC_GAUSS);
    
	fex->frame_shift = fex->frame_len - ((fex->pitch->method == AC_GAUSS? 2 : 1 ) * fex->pitch->periodsPerWindow / fex->pitch->minimumPitch - fex->pitch->dt) * fex->samplerate ; /* global shift (in frames) */
  

	fex->hnr = hnr_create();
  
	fex->vq = pitch_create(FCC_NORMAL);
  
	/* MFCCs ... */
	mfcc = (dsp_fextract_t *) rs_malloc(sizeof(dsp_fextract_t), "feature extraction data");
	mfcc->type = dsp_fextype_MFCC;
	mfcc->version = DSP_MK_VERSION(1, 4);
	if (!dsp_mfcc_create(mfcc, m_e_params)) {
		rs_free(mfcc);
		mfcc = NULL;
		rs_warning("Could not initialize MFCC configuration!");
	}
	fex->mfcc=mfcc;
  
	return(fex);
}


fextract_t *fextract_pitch_energy_create(int frame_len, char *m_e_params) {
	fextract_t *fex;
	dsp_fextract_t *mfcc;

	fex = (fextract_t *) rs_malloc(sizeof(fextract_t),"emotion feature extraction data");
  
	fex->n_features = V2_N_FEATURES;

  
	/* Abtastrate etc. ... */
	fex->samplerate = SAMPLERATE;
	fex->frame_len = frame_len; /* global frame length (in frames) */
  
	fex->pitch = pitch_create(AC_GAUSS);
    
	fex->frame_shift = fex->frame_len - ((fex->pitch->method == AC_GAUSS? 2 : 1 ) * fex->pitch->periodsPerWindow / fex->pitch->minimumPitch - fex->pitch->dt) * fex->samplerate ; /* global shift (in frames) */
  

	fex->hnr = NULL;
  
	fex->vq = NULL;
  
	/* MFCCs ... */
	mfcc = (dsp_fextract_t *) rs_malloc(sizeof(dsp_fextract_t), "feature extraction data");
	mfcc->type = dsp_fextype_MFCC;
	mfcc->version = DSP_MK_VERSION(1, 4);
	if (!dsp_mfcc_create(mfcc, m_e_params)) {
		rs_free(mfcc);
		mfcc = NULL;
		rs_warning("Could not initialize MFCC configuration!");
	}
	fex->mfcc=mfcc;
  
	return(fex);
}


void fextract_destroy(fextract_t *fex) {
	if (fex->mfcc) {
		mfcc_destroy(fex->mfcc);
	}
	if (fex->pitch) 
		pitch_destroy(fex->pitch);
	if (fex->hnr)
		pitch_destroy(fex->hnr);
	if (fex->vq)
		pitch_destroy(fex->vq);
	rs_free(fex);
}


void mfcc_destroy(dsp_fextract_t *fex) {
	dsp_mfcc_t *cfg;
	
	/* Parameter pruefen ... */
	if (!fex || fex->type != dsp_fextype_MFCC)
		return;

	cfg = fex->config;
	/* ... und ggf. existierende Eintraege loeschen */
	if (cfg->wderiv)
		dsp_delay_destroy(cfg->wderiv);
	if (cfg->channel) {
		rs_free(cfg->channel->pMeans);
		rs_free(cfg->channel);
	}
	if (cfg->ehist) {
		rs_free(cfg->ehist->idx_history);
		mx_histogram_destroy(cfg->ehist);
	}
	rs_free(fex->config);

	rs_free(fex);
	fex=NULL;
}

fextract_t *fextract_hard_reset(fextract_t *fex, int frame_len) {
	_mfcc_reset(fex->mfcc);

	if (fex->pitch->last_frame)
		rs_free(fex->pitch->last_frame);
	if (fex->hnr->last_frame)
		rs_free(fex->hnr->last_frame);
	if (fex->vq->last_frame)
		rs_free(fex->vq->last_frame);
    
	fex->frame_len = frame_len;
	fex->frame_shift = fex->frame_len - ((fex->pitch->method == AC_GAUSS? 2 : 1 ) * fex->pitch->periodsPerWindow / fex->pitch->minimumPitch - fex->pitch->dt) * fex->samplerate;

	return fex;
}

fextract_t *fextract_soft_reset(fextract_t *fex, int frame_len) {
    
	if (fex->pitch->last_frame) {
		rs_free(fex->pitch->last_frame);
		fex->pitch->last_frame=0;
	}

	if (fex->hnr->last_frame) {
		rs_free(fex->hnr->last_frame);
		fex->hnr->last_frame=0;
	}

	if (fex->vq->last_frame) {
		rs_free(fex->vq->last_frame);
		fex->vq->last_frame=0;
	}

	fex->frame_len = frame_len;
	fex->frame_shift = fex->frame_len - ((fex->pitch->method == AC_GAUSS? 2 : 1 ) * fex->pitch->periodsPerWindow / fex->pitch->minimumPitch - fex->pitch->dt) * fex->samplerate;

	return fex;
}


void fextract_print(fextract_t *fex) {
	fprintf(stderr,"Feature extraction configuration:\nsamplerate: %d, frame length %d, frame shift %d, # features %d\n",fex->samplerate,fex->frame_len,fex->frame_shift,fex->n_features);
	fprintf(stderr,"Pitch: dt=%g, minPitch=%g, maxPitch=%g\nppw=%d, max # candidates %d, method %d\nlast frame? %d\n\n",fex->pitch->dt,fex->pitch->minimumPitch,fex->pitch->maximumPitch,fex->pitch->periodsPerWindow,fex->pitch->maxnCandidates,fex->pitch->method,fex->pitch->last_frame?1:0);

}


int fextract_calc(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal, int maj, int min, ...) {
	va_list  arg_zeiger; 
	if (maj==1 && min==0) {
		return fextract_calc_v1_0(fex,features,signal);
	}
	if (maj==1 && min==1) {
		char *filename=NULL;
		va_start(arg_zeiger,min);
		filename = va_arg(arg_zeiger,char *);
		if (!filename)
			rs_error("No file name given for feature extraction version %d.%d!",maj,min);
		va_end(arg_zeiger);
		return fextract_calc_v1_1(fex,features,signal,filename);
	}
	if (maj==2 && min==0)
		return fextract_calc_v2_0(fex,features,signal);
	rs_error("Unsupported feature extraction version: %d.%d!",maj,min);
	return 0;
}

int fextract_calc_v2_0(fextract_t *fex, mx_real_t *features, dsp_sample_t *signal) {
	mx_real_t *pitch=NULL;
	mx_real_t **_mfcc_series, **_energy_series;
	int nframes=0, i;
	int ser=0, fcount=0;

	if (fex->frame_len==0) {
		rs_warning("Cannot compute features for segment of size 0!");
		return -1;
	}
	
	_mfcc_series= (mx_real_t **) rs_malloc(MFCCS*ME_DERIV * sizeof(mx_real_t *), "mfcc feature series");
	for (i=0;i<MFCCS*ME_DERIV;i++)
		_mfcc_series[i]= (mx_real_t *) rs_malloc(MAX_FRAMES * sizeof(mx_real_t), "mfcc features");
	_energy_series= (mx_real_t **) rs_malloc(ME_DERIV * sizeof(mx_real_t *), "energy feature series");
	for (i=0;i<ME_DERIV;i++)
		_energy_series[i]= (mx_real_t *) rs_malloc(MAX_FRAMES * sizeof(mx_real_t), "energy features");
    
	/* Pitch-Merkmale */
	fcount+=getPitchFeatures(fex,signal,features,&pitch);
	nframes=fex->pitch->nframes;

	getEnergy_and_MFCC(fex->mfcc, signal, fex->frame_len, &_energy_series, &_mfcc_series, &ser);

	/* Energie-Merkmale */
	fcount+=getLoudnessFeatures(features+fcount,_energy_series,ser);

	/* MFCC-Merkmale */
	fcount+=getMFCCFeatures(features+fcount,_mfcc_series,ser);

	/* Dauer-Merkmale: */
	fcount+=getDurationFeatures(features+fcount,signal,fex->frame_len,pitch,nframes);

	/* Spectral-Merkmale */
	fcount+=getSpectralFeatures(features+fcount,signal,fex->frame_len);

	/* Voicing-Merkmale */
	fcount+=getVoicingFeatures(features+fcount,pitch,nframes);

	/* Voice-Quality-Merkmale: */
    fcount+=getVoiceQualityFeatures(features+fcount,fex,signal);
  
	/* Aufraeumen... */
	for (i=0;i<MFCCS*ME_DERIV;i++)
		if (_mfcc_series[i])
			rs_free(_mfcc_series[i]);
	if (_mfcc_series)
		rs_free(_mfcc_series);
  
	for (i=0;i<ME_DERIV;i++)
		if (_energy_series[i])
			rs_free(_energy_series[i]);
	if (_energy_series)
		rs_free(_energy_series);

	if (pitch)
		rs_free(pitch);


	if (fcount != V2_N_FEATURES)
		rs_error("number of features is not correct (%d expected, %d calculated)!",V2_N_FEATURES,fcount);

	return V2_N_FEATURES;
} 

fextract_t *fextract_set_length(fextract_t *fex,int frame_len) {
	fex->frame_len = frame_len;
	fex->frame_shift = fex->frame_len - ((fex->pitch->method == AC_GAUSS? 2 : 1 ) * fex->pitch->periodsPerWindow / fex->pitch->minimumPitch - fex->pitch->dt) * fex->samplerate; 

	return fex;
}


void getEnergy_and_MFCC(dsp_fextract_t *mfcc, dsp_sample_t *signal, int frame_len, mx_real_t ***energy_series, mx_real_t ***mfcc_series, int *ser){
	/* fuer MFCC-Berechnung */
	int frames, n_samples, n_mfcc_features;
	int mfcc_frame_len=mfcc->frame_len;
	int mfcc_frame_shift=mfcc->frame_shift;
	int mfcc_offset, i, j;
	dsp_sample_t s[MAX_SAMPLES], last_s[MAX_SAMPLES];
	dsp_sample_t *frame, *last_frame, *__tmp, *flush;
	mx_real_t mfcc_features[MAX_FEATURES];
	dsp_mfcc_t* mfcc_cfg = mfcc->config;

	*ser=0;

	/* ersten Frame einlesen ... */
	frames = 0;
	frame = s; 
	last_frame = last_s;
  
	mfcc_offset = mfcc_cfg->wderiv->need_elems != 0 ? 0 : pitch2mfcc_shift;
	n_samples = next_mfcc_frame(frame, NULL, mfcc->frame_len, mfcc->frame_shift, signal+ mfcc_offset,frame_len);

	mfcc_offset += mfcc_frame_len ;
  
	while (n_samples > 0) {
		frames++;
		n_mfcc_features = dsp_fextract_calc(mfcc, mfcc_features, frame);
		if (n_samples < mfcc_frame_len)
			break;
		if (n_mfcc_features > 0) { 
			/* MFCCs */
			for (j=0;j<3;j++) {
				for (i=0;i<MFCCS;i++)
					(*mfcc_series)[j*MFCCS+i][*ser]=mfcc_features[j*(MFCCS+1)+1+i];
				
				/* Energie */
				(*energy_series)[j][*ser]=mfcc_features[j*(MFCCS+1)]; 
			}
			(*ser)++;
		}
    
		/* ... und naechsten Frame einlesen */
		__tmp = last_frame;
		last_frame = frame;
		frame = __tmp;
		n_samples = next_mfcc_frame(frame, last_frame, mfcc_frame_len, mfcc_frame_shift, signal+mfcc_offset,frame_len-mfcc_offset);
		mfcc_offset += mfcc_frame_shift;
	}


	
	flush = (dsp_sample_t *) rs_calloc(mfcc_frame_len,sizeof(dsp_sample_t),"flush MFCCs");
	dsp_fextract_calc(mfcc, mfcc_features, flush);
	rs_free(flush);
}




int get_basic_energy(fextract_t *fex, dsp_sample_t *signal, int frame_len, mx_real_t **energy_series){
	/* fuer MFCC-Berechnung */
	int frames, n_samples, n_mfcc_features;
	dsp_fextract_t *mfcc = fex->mfcc;
	int mfcc_frame_len=mfcc->frame_len;
	int mfcc_frame_shift=mfcc->frame_shift;
	int mfcc_offset;
	dsp_sample_t s[MAX_SAMPLES], last_s[MAX_SAMPLES];
	dsp_sample_t *frame, *last_frame, *__tmp, *flush;
	mx_real_t mfcc_features[MAX_FEATURES];
	dsp_mfcc_t* mfcc_cfg = mfcc->config;
	int ser=0;

	/* ersten Frame einlesen ... */
	frames = 0;
	frame = s; 
	last_frame = last_s;
  
	mfcc_offset = mfcc_cfg->wderiv->need_elems != 0 ? 0 : pitch2mfcc_shift;
	n_samples = next_mfcc_frame(frame, NULL, mfcc->frame_len, mfcc->frame_shift, signal+ mfcc_offset,frame_len);

	mfcc_offset += mfcc_frame_len ;
  
	while (n_samples > 0) {
		frames++;
		n_mfcc_features = dsp_fextract_calc(mfcc, mfcc_features, frame);
		/* ... falls aktueller Frame unvollstaendig, abbrechen! */
		if (n_samples < mfcc_frame_len)
			break;
		if (n_mfcc_features > 0) { 
			/* Energie */
			(*energy_series)[ser]=mfcc_features[0]; 
			ser++;
		}
    
		/* ... und naechsten Frame einlesen */
		__tmp = last_frame;
		last_frame = frame;
		frame = __tmp;
		n_samples = next_mfcc_frame(frame, last_frame, mfcc_frame_len, mfcc_frame_shift, signal+mfcc_offset,frame_len-mfcc_offset);
		mfcc_offset += mfcc_frame_shift;
	}

	flush = (dsp_sample_t *) rs_calloc(mfcc_frame_len,sizeof(dsp_sample_t),"flush MFCCs");
	dsp_fextract_calc(mfcc, mfcc_features, flush);
	rs_free(flush);
	return ser;
}




/**
		* next_mfcc_frame(s[], last_s[], f_length, f_shift, fp)
 *	Liest Abtastwerte fuer den naechsten 'f_length' Samples langen Frame
 *	aus der Datei 'fp' in 's[]' ein. Der sich bei einer Fortschaltrate
 *	'f_shift', die kleiner als die Frame-Laenge 'f_length' ist,
 *	ergebende Ueberlappungsbereich wird dabei aus dem
 *	Vorgaenger-Frame 'last_s[]' uebernommen, sofern dieser angegeben ist.
 *
 *	Liefert die Anzahl der tatsaechlich gelesenen Abtastwerte oder Null,
 *	wenn nicht ausreichend, d.h. < 'f_shift' Werte gelesen wurden.
 **/
int next_mfcc_frame(dsp_sample_t *s, dsp_sample_t *last_s, int f_length, int f_shift, dsp_sample_t *signal,int s_length) {
	int i=0, n_samples = 0, c_n_samples;
  
	/* Falls ein Vorgaenger-Frame existiert ... */
	if (last_s && f_shift < f_length) {
		/* ... Ueberlappungsbereich kopieren ... */
		memcpy(s, last_s + f_shift, (f_length - f_shift) * sizeof(dsp_sample_t));
		n_samples = (f_length - f_shift);
	}

	c_n_samples=n_samples;
	/* ... Frame mit Abtastwerten auffuellen ... */
	while (i<f_length-c_n_samples && i<s_length) {
		s[c_n_samples+i]=signal[i]; // Achtung: nur moeglich solange nicht ueber Laenge von Signal
		i++;
		n_samples++;
	}
  
	/* ... bei nicht ausreichender "Fuellung" -> Ende ... */
	if (n_samples < f_shift)
		return(0);
  
	/* ... sonst evtl. verbleibenden Bereich loeschen ... */
	for (i = n_samples; i < f_length; i++)
		s[i] = 0;
  
	/* ... und # tatsaechlich gelesener Abtastwerte zurueckliefern */
	return(n_samples);
}


void _channel_reset(dsp_channel_t *channel) {
	int i;
	mx_real_t chparam_desklab[1 + 2 + MFCCS+1] = {
		0,
		0, 6,
		0, 2.93657, 0.312502, 1.38028, -0.201216, 0.315178, -0.22886,
		0.76584, -0.312766, 0.491137, -0.380584, 0.513988, -0.619026
	};


	channel->fMin[1] =  1.0e20;
	channel->fMax[1] = -1.0e20;
	channel->nUsed[0] = channel->nUsed[1] = 0;
	channel->nTime = chparam_desklab[0];
	channel->fMin[0] = chparam_desklab[1];
	channel->fMax[0] = chparam_desklab[2];

	for (i = 0; i < channel->nDim; i++)
		channel->pMeans[i] = chparam_desklab[3 + i];
}

void _histogram_reset(mx_histogram_t *ehist) {
	int i, en_hist_iweight=100;
	mx_real_t en_hist_min=2.0, en_hist_max=6.0;

	mx_histogram_reset(ehist);  
	ehist->tot_samples=0;
	ehist->idx_history_top =0;
	for (i = 0; i < ehist->sample_limit; i++)
		ehist->idx_history[i] = -2;
	mx_histogram_update_urange(ehist,en_hist_min, en_hist_max, en_hist_iweight);
}

void _mfcc_reset(dsp_fextract_t *mfcc) {
	dsp_mfcc_t *cfg;
    

	if (mfcc->version != DSP_MK_VERSION(1,4))
		rs_warning("Cannot handle MFCC and energy feature extraction other than version 1.4!");
	cfg = mfcc->config;
    
	if (cfg->wderiv) 
		dsp_delay_flush(cfg->wderiv);

	if (cfg->channel)
		_channel_reset(cfg->channel);

	if (cfg->ehist) 
		_histogram_reset(cfg->ehist);

}
