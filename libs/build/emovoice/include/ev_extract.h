// ev_extract.h
// author: Thurid Vogt <thurid.vogt@informatik.uni-augsburg.de>
// created: 16.12.2003
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


#ifndef __EXTRACT_H_INCLUDED__
#define __EXTRACT_H_INCLUDED__

#include "ev_hnr.h"

#define N_PITCH_FEATURES 208
#define N_ENERGY_FEATURES 110
#define N_MFCC_FEATURES 1053
#define N_DURATION_FEATURES 4
#define N_SPECTRAL_FEATURES 45
#define N_VOICING_FEATURES 19
#define N_VOICE_QUALITY_FEATURES 12

#define MFCCS     12
#define ME_DERIV     3

#define V1_N_FEATURES    1316
#define V2_N_FEATURES    N_PITCH_FEATURES + N_ENERGY_FEATURES + N_MFCC_FEATURES + N_DURATION_FEATURES + N_SPECTRAL_FEATURES + N_VOICING_FEATURES + N_VOICE_QUALITY_FEATURES 

typedef struct {
  dsp_fextract_t *mfcc;
  pitch_t *pitch;
  pitch_t *hnr;
  pitch_t *vq;
  int samplerate;		/* Abtastrate ... */
  int frame_len;		/* ... # Abtastwerte pro Frame ... */
  int frame_shift;	/* ... sowie Frameverschiebung */
  int n_features;		/* Gesamtanzahl berechneter Merkmale */
} fextract_t;




fextract_t *fextract_create(int frame_len, char *m_e_params, int maj, int min);
fextract_t *fextract_pitch_energy_create(int frame_len, char *m_e_params);
void fextract_destroy(fextract_t *fex);
fextract_t *fextract_hard_reset(fextract_t *fex, int frame_len);
fextract_t *fextract_soft_reset(fextract_t *fex, int frame_len);
int fextract_calc(fextract_t *fex, mx_real_t *features, dsp_sample_t *samples, int maj, int min,...);
fextract_t *fextract_set_length(fextract_t *fex,int frame_len);
void fextract_print(fextract_t *fex);


int getPitchFeatures(fextract_t *fex, dsp_sample_t *signal, mx_real_t *features, mx_real_t **pitch_ret);
int getLoudnessFeatures(mx_real_t *features, mx_real_t** energy, int ser);
int getMFCCFeatures(mx_real_t *features, mx_real_t** energy, int ser);
int getDurationFeatures(mx_real_t *features, dsp_sample_t *signal, int nframes, mx_real_t* pitch, int pitchFrames);
int getSpectralFeatures(mx_real_t *features,dsp_sample_t *signal, int nframes);
int getVoicingFeatures(mx_real_t *features, mx_real_t *pitch, int nframes);
int getVoiceQualityFeatures(mx_real_t *features,fextract_t *fex, dsp_sample_t *signal);


void getEnergy_and_MFCC(dsp_fextract_t *mfcc, dsp_sample_t *signal, int frame_len, mx_real_t ***energy_series, mx_real_t ***mfcc_series, int *ser);
int get_basic_energy(fextract_t *fex, dsp_sample_t *signal, int frame_len, mx_real_t **energy_series);
fextract_series_t *getOnlyCoG(dsp_sample_t *signal, int nframes);
int next_mfcc_frame(dsp_sample_t *s, dsp_sample_t *last_s, int f_length, int f_shift, dsp_sample_t *signal,int s_length);



#endif /* __EXTRACT_H_INCLUDED__ */
