// TimeBuffer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/23 
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "buffer/TimeBuffer.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *TimeBuffer::STATUS_NAMES[7] = {
	"SUCCESS",
	"INPUT_ARRAY_TOO_SMALL",
	"DATA_EXCEEDS_BUFFER_SIZE",
	"DATA_NOT_IN_BUFFER_YET",
	"DATA_NOT_IN_BUFFER_ANYMORE",
	"DURATION_TOO_SMALL",
	"DURATION_TOO_LARGE"
};

TimeBuffer::TimeBuffer(ssi_size_t capacity, ssi_time_t sample_rate_, ssi_size_t sample_dimension_, ssi_size_t sample_bytes_, ssi_type_t sample_type_)
	: Buffer(capacity * sample_bytes_ * sample_dimension_),
    max_samples(ssi_cast(int32_t, capacity)),
	sample_rate(sample_rate_),
	sample_duration(1.0f / sample_rate_),
	sample_bytes(sample_bytes_),
	sample_type(sample_type_),
	sample_dimension(sample_dimension_),
	sample_total_bytes(sample_bytes_ * sample_dimension_),
	buffer_filled(false),
	next_sample(0),
	last_accessed_sample(0),
	offset_samples(0) {
}

TimeBuffer::TimeBuffer (ssi_time_t capacity, ssi_time_t sample_rate_, ssi_size_t sample_dimension_, ssi_size_t sample_bytes_, ssi_type_t sample_type_) 
: Buffer (ssi_cast (ssi_size_t, capacity * sample_rate_ + 0.5) * sample_bytes_ * sample_dimension_), 
      max_samples (ssi_cast (int32_t, capacity * sample_rate_)),
	  sample_rate (sample_rate_), 
	  sample_duration (1.0f / sample_rate_),
	  sample_bytes (sample_bytes_),
	  sample_type (sample_type_),
	  sample_dimension (sample_dimension_),
	  sample_total_bytes (sample_bytes_ * sample_dimension_),
	  buffer_filled (false),
	  next_sample (0),
	  last_accessed_sample (0),
	  offset_samples (0) {
}

TimeBuffer::~TimeBuffer () {
}

void TimeBuffer::sync (ssi_time_t sync_time) {

	setCurrentSampleTime (sync_time);
}

TimeBuffer::STATUS TimeBuffer::push (const ssi_byte_t *data, ssi_size_t samples) {

	// check if data fits in buffer
	if (samples > max_samples) {
		return TimeBuffer::DATA_EXCEEDS_BUFFER_SIZE;
	}

	// put data to buffer
	Buffer::put (data, next_sample * sample_total_bytes, samples * sample_total_bytes);

	// calculate next position
	next_sample += samples;

	// if buffer was not filled yet, check if filled now
	if (!buffer_filled) {
		if (next_sample >= max_samples) {
			buffer_filled = true;
		}
	}

	return TimeBuffer::SUCCESS;
}

TimeBuffer::STATUS TimeBuffer::pushZeros (ssi_size_t samples) {

	// check if data fits in buffer
	if (samples > max_samples) {
		return TimeBuffer::DATA_EXCEEDS_BUFFER_SIZE;
	}

	// put data to buffer
	Buffer::putZeros (next_sample * sample_total_bytes, samples * sample_total_bytes);

	// calculate next position
	next_sample += samples;

	// if buffer was not filled yet, check if filled now
	if (!buffer_filled) {
		if (next_sample >= max_samples) {
			buffer_filled = true;
		}
	}

	return TimeBuffer::SUCCESS;
}

TimeBuffer::STATUS TimeBuffer::get (ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples, ssi_time_t start_time, ssi_time_t duration) {

	// calculate position of first and last element and number of requested samples
	ssi_lsize_t start_sample = ssi_cast (ssi_lsize_t, (start_time * sample_rate) + 0.5);
	ssi_lsize_t stop_sample = ssi_cast (ssi_lsize_t, ((start_time + duration) * sample_rate) + 0.5);
	samples = (ssi_size_t) (stop_sample - start_sample);
	start_sample -= offset_samples;

	// check if requested duration is too small
	if (samples == 0) {
		return TimeBuffer::DURATION_TOO_SMALL;
	}

	// check if requested duration is too large
	if (samples > max_samples) {
		return TimeBuffer::DURATION_TOO_LARGE;
	}

	// check if requested data is still available
	if (start_sample + max_samples < next_sample) {
		return TimeBuffer::DATA_NOT_IN_BUFFER_ANYMORE;
	}

	// check if requested data is already available
	if (start_sample + samples > next_sample) {
		return TimeBuffer::DATA_NOT_IN_BUFFER_YET;
	}

	// check if output array is large enough
	// and make it larger if necessary
	if (samples > samples_in) {
		delete[] *data;
		*data = new ssi_byte_t[sample_bytes * sample_dimension * samples];
		samples_in = samples;
	}

	// get data from buffer
	Buffer::get (*data, start_sample * sample_total_bytes, samples * sample_total_bytes);

	// store last accessed sample
	last_accessed_sample = start_sample + samples - 1;

	return TimeBuffer::SUCCESS;
}

TimeBuffer::STATUS TimeBuffer::get (ssi_byte_t *data, ssi_size_t samples, ssi_lsize_t start_sample) {

	// calculate position of first and last element and number of requested samples
	start_sample -= offset_samples;

	// check if requested duration is too small
	if (samples == 0) {
		return TimeBuffer::DURATION_TOO_SMALL;
	}

	// check if requested duration is too large
	if (samples > max_samples) {
		return TimeBuffer::DURATION_TOO_LARGE;
	}

	// check if requested data is still available
	if (start_sample + max_samples < next_sample) {
		return TimeBuffer::DATA_NOT_IN_BUFFER_ANYMORE;
	}

	// check if requested data is already available
	if (start_sample + samples > next_sample) {
		return TimeBuffer::DATA_NOT_IN_BUFFER_YET;
	}

	// get data from buffer
	Buffer::get (data, start_sample * sample_total_bytes, samples * sample_total_bytes);

	// store last accessed sample
	last_accessed_sample = start_sample + samples - 1;

	return TimeBuffer::SUCCESS;
}

void TimeBuffer::reset (ssi_time_t offset) {

    offset_samples = ssi_cast (int32_t, offset * sample_rate);
	next_sample = 0;
	last_accessed_sample = 0;
	buffer_filled = false;
}

ssi_time_t TimeBuffer::getOffsetTime () {

	return offset_samples * sample_duration;
}

ssi_time_t TimeBuffer::getCurrentSampleTime () {

	return (offset_samples + next_sample) * sample_duration;
}

ssi_lsize_t TimeBuffer::getCurrentWritePos () {

	return offset_samples + next_sample;
}

void TimeBuffer::setCurrentSampleTime (ssi_time_t sync_time) {

	ssi_time_t delta = getCurrentSampleTime () - sync_time;
    offset_samples -= ssi_cast (int32_t, delta * sample_rate + 0.5);

	//ssi_print ("%d %lf\n", offset_samples, delta);

}

ssi_time_t TimeBuffer::getLastAccessedSampleTime () {

	return (offset_samples + last_accessed_sample) * sample_duration;
}

ssi_time_t TimeBuffer::getLastWrittenSampleTime() {

	return (offset_samples + next_sample) * sample_duration;
}

ssi_time_t TimeBuffer::getCapacity () {

	return max_samples * sample_duration;
}


}
