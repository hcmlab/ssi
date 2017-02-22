// ev_durationFeatures.c
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

#include "ev_emo.h"
#include "ev_segment.h"

#include "ev_extract.h"

int getDurationFeatures(mx_real_t *features, dsp_sample_t *signal, int nframes, mx_real_t* pitch, int pitchFrames) {
	int i, fcount=0, unvoiced=0, zerocrossings=0;
	
	
	//length 
	//in seconds
	features[fcount++]=1.0*nframes/SAMPLERATE;

	//pause
	//proportion of unvoiced frames
	for (i=0;i<pitchFrames;i++)
		if (pitch[i]==0)
			unvoiced++;
	if (pitchFrames >0)
		features[fcount++]=1.0*unvoiced/pitchFrames; 
	else
		features[fcount++]=0.0;

	//vad
	if (nframes >0) {
		int speechSegments=0, n_segments=0;
		dsp_sample_t **segments;
		int *segment_length;
		
		asegmentation_method_t *method = (asegmentation_method_t *) rs_malloc(sizeof(asegmentation_method_t),"Audio segmentation method");
		dsp_sample_t *_signal = (dsp_sample_t *) rs_malloc(nframes*sizeof(dsp_sample_t),"");
		for (i=0;i<nframes;i++)
			_signal[i]=signal[i];

		method->vad = dsp_vad_create(DSP_MK_VERSION(1,0),VAD_FRAME_LEN);
		emo_aset_output(0); 
		
		n_segments = emo_asegment(method, vad, _signal, nframes, &segments, &segment_length);
		for (i=0;i<n_segments;i++) 
			speechSegments+=segment_length[i];
		for (i=0;i<n_segments;i++) 
			rs_free(segments[i]);
		rs_free(segments);
		
		features[fcount++]=1-1.0*speechSegments/nframes;

    	rs_free(method->vad->ehist->idx_history);
		dsp_vad_destroy(method->vad);

		rs_free(_signal);
		rs_free(segment_length);
		rs_free(method);
	}
	else
		features[fcount++]=0.0;
	//speaking rate
	//zero-crossings rate
	if (nframes >0) {
		for (i=1;i<nframes;i++)
			if ((signal[i]==0 && signal[i-1]!=0) ||
				(signal[i]>0 && signal[i-1]<0) || (signal[i]<0 && signal[i-1]>0))
				zerocrossings++;
		features[fcount++]=1.0*zerocrossings/nframes;
	}
	else 
		features[fcount++]=0.0;

	return fcount;
}
