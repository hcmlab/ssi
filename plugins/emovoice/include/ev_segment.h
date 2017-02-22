// ev_segment.h
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

#include "ev_emo.h"
#include "ev_vad.h"

/// the frame shift for pitch calculation
#define PITCH_FRAME_SHIFT  1120

/**
 *  types of audio segmentation
 */
typedef enum { vad, fixed, info, undefined} asegmentation_type_t;


/// segmentation type can be voice activity detection
typedef union {
/// voice activity detection for audio segmentation
    dsp_vad_t *vad;
/// segmentation is specified by number of samples
    char *segmentation_info;
} asegmentation_method_t;

/**
 * segment one audio file
 * @param file the file name
 * @param method the audio segmentation method
 * @param type the audio segmentation types
 * @param signal_segment returned array with samples of the individual segments
 * @param segment_length returned array with individual segment lengths
 * @return number of segments
 */
int emo_afile_segment(char *file, asegmentation_method_t *method, asegmentation_type_t type, dsp_sample_t ***signal_segment, int **segment_length);
int emo_afilelist_segment(char *filelist, asegmentation_method_t *method, asegmentation_type_t type,int audio_output);
int emo_asegment(asegmentation_method_t *method, asegmentation_type_t type, dsp_sample_t *signal, int n_samples, dsp_sample_t ***signal_segment, int **length);
int emo_afile_output(char *file, char *outdir, int n_segments, dsp_sample_t **signal_segment, int *segment_length);

void emo_aset_output(int output);
