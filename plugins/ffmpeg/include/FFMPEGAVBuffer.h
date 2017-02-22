// FFMPEGReader.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/04/15
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_FFMPEG_AVBUFFER_H
#define SSI_FFMPEG_AVBUFFER_H

#include "thread/Lock.h"

namespace ssi {

class FFMPEGVideoBuffer {

public:

	// a video buffer can store frames for 'size_s' seconds
	// frames are supposed to be delivered at an average frame rate of 'fps' hz
	// pop() returns a single frame with a fixed size of 'n_bytes_per_frame'
	FFMPEGVideoBuffer (ssi_time_t size_s, ssi_time_t fps, ssi_size_t n_bytes_per_frame);
	virtual ~FFMPEGVideoBuffer ();
		
	void reset ();
	bool push (ssi_byte_t *frame);
	ssi_byte_t *pop (ssi_size_t &n_bytes, bool &is_new);
	void pop_done ();
	ssi_real_t getFillState ();
	ssi_size_t getPushed ();

protected:
		
	Mutex _mutex;
	ssi_size_t _n_frame_bytes;
	ssi_size_t _n_frames;		
	bool _ready;
	ssi_size_t _n_pushed;		
	ssi_byte_t *_buffer;
	ssi_size_t _n_buffer;
	ssi_size_t _push_pos;
	ssi_size_t _pop_pos;
};

class FFMPEGAudioBuffer {

public:

	// an audio buffer can store 'size_s' seconds audio samples
	// samples are supposed to be delivered at an average frame rate of 'sr' hz
	// pop() returns a chunk of length 'chunk_s' seconds if 'chunk_s' != 0
	FFMPEGAudioBuffer (ssi_time_t size_s, ssi_time_t sr, ssi_time_t chunk_s);
	virtual ~FFMPEGAudioBuffer ();
		
	void reset ();
	bool push (ssi_size_t n_samples, ssi_real_t *chunk);
	// returns chunk of length 'chunk_s' seconds if 'chunk_s'
	// otherwise n_samples if n_samples > 0
	// full buffer if n_samples == 0
	ssi_real_t *pop (ssi_size_t &n_samples, bool &is_new);
	void pop_done (ssi_size_t n_samples);
	ssi_real_t getFillState ();
	ssi_size_t getPushed ();

protected:
		
	Mutex _mutex;
	ssi_real_t *_buffer;
	ssi_size_t _n_buffer;
	ssi_time_t _sr;
	ssi_time_t _chunk_s;
	ssi_size_t _n_samples_per_chunk; // max length, actually delivered might be -1
	ssi_size_t _n_delivered_chunks;
	ssi_size_t _n_delivered_samples;		
	bool _ready;
	ssi_size_t _n_pushed;			
	ssi_real_t *_temp_chunk;
	ssi_size_t _n_temp_chunk;
	ssi_size_t _push_pos;
	ssi_size_t _pop_pos;
};

};

#endif

