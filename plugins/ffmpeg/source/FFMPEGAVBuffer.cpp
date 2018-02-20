// FFMPEGAVBuffer.cpp
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

#include "FFMPEGAVBuffer.h"

namespace ssi {

FFMPEGVideoBuffer::FFMPEGVideoBuffer (ssi_time_t size_s, ssi_time_t fps, ssi_size_t n_bytes_per_frame) 
: _n_frame_bytes (n_bytes_per_frame) {	

	_n_frames = ssi_cast (ssi_size_t, size_s * fps + 0.5);

	_n_buffer = n_bytes_per_frame * _n_frames;
	_buffer = new ssi_byte_t[_n_buffer];
	memset (_buffer, 0, _n_buffer);

	reset ();
}

FFMPEGVideoBuffer::~FFMPEGVideoBuffer () {

	delete[] _buffer;
}

void FFMPEGVideoBuffer::reset () {
	
	_ready = false;		
	_n_pushed = 0;	
	_push_pos = 0;
	_pop_pos = 0;
}

bool FFMPEGVideoBuffer::push (ssi_byte_t *frame) {

	Lock lock (_mutex);

	if (_n_pushed == _n_frames) {		
		return false;
	}

	memcpy (_buffer + _push_pos, frame, _n_frame_bytes);
	_push_pos = (_push_pos + _n_frame_bytes)  % _n_buffer;
	++_n_pushed;

	if (!_ready) {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "video buffer ready to pop");
		_ready = true;
	}

	return true;
}

ssi_byte_t *FFMPEGVideoBuffer::pop (ssi_size_t &n_bytes, bool &is_old) {
	
	Lock lock (_mutex);

	n_bytes = _n_frame_bytes;
	if (_ready) {		
		is_old = false;
		return _buffer + _pop_pos;
	} else {
		is_old = true;
		return _buffer + _pop_pos;
	}
}

void FFMPEGVideoBuffer::pop_done () {

	Lock lock (_mutex);

	if (_ready) {
		_pop_pos = (_pop_pos + _n_frame_bytes)  % _n_buffer;
		if (--_n_pushed == 0) {				
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "re-filling video buffer");
			_ready = false;
		}
	}
}

ssi_real_t FFMPEGVideoBuffer::getFillState () {

	Lock lock (_mutex);

	return ((ssi_real_t) _n_pushed) / _n_frames; 	
}

ssi_size_t FFMPEGVideoBuffer::getPushed () {

	Lock lock (_mutex);
	return _n_pushed; 	
}


FFMPEGAudioBuffer::FFMPEGAudioBuffer (ssi_time_t size_s, ssi_time_t sr, ssi_time_t chunk_s) 
: _chunk_s (chunk_s),
	_n_samples_per_chunk (0),
	_temp_chunk (0),
	_n_temp_chunk (0),
	_sr (sr) {

	SSI_ASSERT (chunk_s < size_s);

	_n_buffer = ssi_cast (ssi_size_t, size_s * sr + 0.5);
	_buffer = new ssi_real_t[_n_buffer];

	if (_chunk_s > 0) {
		_n_samples_per_chunk = ssi_cast (ssi_size_t, _chunk_s * sr) + 1;
		_n_temp_chunk = _n_samples_per_chunk;
		_temp_chunk = new ssi_real_t[_n_temp_chunk];	
	}
	
	reset ();
}

FFMPEGAudioBuffer::~FFMPEGAudioBuffer () {

	delete[] _temp_chunk;
	delete[] _buffer;
}

void FFMPEGAudioBuffer::reset () {
	
	_ready = false;		
	_n_pushed = 0;	
	_push_pos = 0;
	_pop_pos = 0;
	_n_delivered_chunks = 0;
	_n_delivered_samples = 0;
}

bool FFMPEGAudioBuffer::push (ssi_size_t n_samples, ssi_real_t *chunk) {

	Lock lock (_mutex);	

	if (_n_pushed + n_samples > _n_buffer) {		
		return false;
	}

	ssi_size_t n_remaining = _push_pos + n_samples > _n_buffer ? (_push_pos + n_samples) - _n_buffer : 0; 

	memcpy (_buffer + _push_pos, chunk, (n_samples - n_remaining) * sizeof (ssi_real_t));	
	if (n_remaining > 0) {
		memcpy (_buffer, chunk + (n_samples - n_remaining), n_remaining * sizeof (ssi_real_t));		
	}

	_push_pos = (_push_pos + n_samples) % _n_buffer;
	_n_pushed += n_samples;

	if (!_ready) {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "audio buffer ready to pop");
		_ready = true;
	}

	return true;
}

ssi_real_t *FFMPEGAudioBuffer::pop_all(ssi_size_t &n_samples)
{
	n_samples = _n_pushed;
	return _buffer + _pop_pos;
}

ssi_real_t *FFMPEGAudioBuffer::pop (ssi_size_t &n_samples, bool &is_old) {
	
	Lock lock (_mutex);
	
	if (_chunk_s > 0) {
		// calculate exact number of samples we need to deliver
		n_samples = ssi_cast (ssi_size_t, (_n_delivered_chunks + 1) * _chunk_s * _sr + 0.5) - _n_delivered_samples;
	} else if (n_samples == 0) {
		n_samples = _n_pushed;		
	}

	if (n_samples == 0) {
		return 0;
	} else if (_n_temp_chunk < n_samples) {
		delete[] _temp_chunk;
		_n_temp_chunk = n_samples;
		_temp_chunk = new ssi_real_t[_n_temp_chunk];
	}

	if (_ready && n_samples <= _n_pushed) {
		ssi_size_t n_remaining = _pop_pos + n_samples > _n_buffer ? (_pop_pos + n_samples) - _n_buffer : 0; 
		memcpy (_temp_chunk, _buffer + _pop_pos, (n_samples - n_remaining) * sizeof (ssi_real_t));
		if (n_remaining > 0) {
			memcpy (_temp_chunk + (n_samples - n_remaining), _buffer, n_remaining * sizeof (ssi_real_t));
		}
		is_old = false;
	} else {		
		memset (_temp_chunk, 0, n_samples * sizeof (ssi_real_t));			
		is_old = true;
		_ready = false;
	}

	return _temp_chunk;
}

void FFMPEGAudioBuffer::pop_done (ssi_size_t n_samples) {

	Lock lock (_mutex);

	if (_ready) {		
		_pop_pos = (_pop_pos + n_samples) % _n_buffer;
		SSI_ASSERT (_n_pushed >= n_samples);
		_n_pushed -= n_samples;	
		_n_delivered_chunks++;
		_n_delivered_samples += n_samples;
	}

}

ssi_size_t FFMPEGAudioBuffer::getPushed () {

	Lock lock (_mutex);
	return _n_pushed; 	
}

ssi_real_t FFMPEGAudioBuffer::getFillState () {

	Lock lock (_mutex);
	return ((ssi_real_t) _n_pushed) / _n_buffer; 	
}



}
