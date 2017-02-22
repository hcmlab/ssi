// ev_pitch.h
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

// @begin_add_johannes
#ifndef EV_PITCH_H
#define EV_PITCH_H
// @end_add_johannes

#include "ev_dsp.h"

#define MINPITCH 75.0
#define MAXPITCH 600.0
// fuer Kinder
//#define MINPITCH 100.0
//#define MAXPITCH 800.0

// @begin_add_johannes
#ifndef SAMPLERATE
#define SAMPLERATE 16000
#endif
// @end_add_johannes

typedef enum {AC_HANNING, AC_GAUSS, FCC_NORMAL, FCC_ACCURATE} pitch_method_t;

typedef struct {
  mx_real_t F;
  mx_real_t R;
} pitch_candidate_t ;

typedef struct {
  pitch_candidate_t* candidates;
  mx_real_t intensity;
  int nCandidates;
} pitch_frame_t;

typedef struct {
    pitch_frame_t* last_frame;
    mx_real_t dt; 
    mx_real_t minimumPitch;
    mx_real_t maximumPitch;
    int periodsPerWindow; 
    int maxnCandidates; 
    int nframes;
    mx_real_t silence_threshold;
    mx_real_t voicing_threshold;
    mx_real_t octave_cost;
    mx_real_t octave_jump_cost;
    mx_real_t voiced_unvoiced_cost;
    pitch_method_t method;
    pitch_frame_t **frame_candidates;
} pitch_t ;

struct improve_params {
  mx_real_t *y_f;
  mx_real_t *y_d;
  int depth;
  int ixmax;
  int isMaximum;
};


pitch_t *pitch_create (pitch_method_t method);
mx_real_t* pitch_calc (pitch_t *cfg, dsp_sample_t *signal, int frameLength);
void pitch_destroy (pitch_t *p);
pitch_t *pitch_hnr_configure(pitch_t *pitch);
void pitch_frame_candidates_destroy (pitch_t *p);

// @begin_add_johannes
int fitInFrame (mx_real_t windowDuration, mx_real_t timeStep, int *numberOfFrames, mx_real_t *startTime, mx_real_t x1, int frameLength);
// @end_add_johannes

// @begin_add_johannes
#endif EV_PITCH_H
// @end_add_johannes
