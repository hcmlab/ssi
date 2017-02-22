// ev_hnr.h
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

#include "ev_dsp.h"

#include "ev_pitch.h"
#include "ev_serien.h"


pitch_t *hnr_create();
mx_real_t *hnr_calc(pitch_t *hnr, dsp_sample_t *signal, int frameLength);
fextract_series_t *pulses_calc(pitch_t *p, int nframes, mx_real_t *signal, int nsamples);
mx_real_t getJitter(fextract_series_t *pulses, mx_real_t maximumPitch, mx_real_t minimumPitch);
mx_real_t getShimmer(fextract_series_t *pulses, mx_real_t *signal, int n_samples, mx_real_t maxPitch, mx_real_t minPitch);
