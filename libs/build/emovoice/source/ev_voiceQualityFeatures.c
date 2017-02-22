// ev_voiceQualityFeatures.c
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
#include "ev_efeatures.h"
#include "ev_serien.h"

int getVoiceQualityFeatures(mx_real_t *features,fextract_t *fex, dsp_sample_t *signal) {
	int fcount=0, nframes, i, nsamples=fex->frame_len;
	mx_real_t *hnr=NULL;
	mx_real_t jitter, shimmer;
	fextract_series_t *pulses;
	mx_real_t *_signal, *pitch_values;
	int vqframes;
	hnr=hnr_calc(fex->hnr,signal,nsamples);
	nframes=fex->hnr->nframes;

	if (!hnr || nframes <=0) 
		for (;fcount<STATS;fcount++) 
            features[fcount]=-1.0;
	else {
		fextract_series_t *hseries = series_create(MAX_SERIES);
		pitch_frame_candidates_destroy(fex->hnr);
		for (i=0;i<nframes;i++) {
			if (hnr[i]!=-200)
				series_add(hseries,hnr[i]);
		}
		fcount+=getStatistics(features+fcount,hseries->series,hseries->nSeries); //hnr statistics features
		series_destroy(hseries);
		rs_free(hnr);
	}
	
	pitch_values=pitch_calc(fex->vq,signal,nsamples);
	rs_free(pitch_values);
	vqframes=fex->vq->nframes;

	if (!fex->vq || vqframes <=0)
		for (;fcount<N_VOICE_QUALITY_FEATURES;fcount++) 
			features[fcount]=-1.0;
	else {
		_signal = (mx_real_t *) rs_malloc(sizeof(mx_real_t)*nsamples,"signal divided by 32768");
		for (i=0;i<nsamples;i++) 
			_signal[i]=signal[i]/32768.0;
	
		pulses = pulses_calc(fex->vq,vqframes,_signal,nsamples);
		pitch_frame_candidates_destroy(fex->vq);
		features[fcount++]=pulses->nSeries/(1.0*nsamples/SAMPLERATE);
		
		jitter= getJitter(pulses,fex->vq->maximumPitch,fex->vq->minimumPitch);
		features[fcount++]=jitter;
	
		shimmer= getShimmer(pulses, _signal, nsamples, fex->vq->maximumPitch,  fex->vq->minimumPitch);
		features[fcount++]=shimmer;
	
		series_destroy(pulses);
		rs_free(_signal);
	}
	return fcount;
}
