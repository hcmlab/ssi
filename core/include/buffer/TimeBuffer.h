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

#pragma once

#ifndef SSI_BUFFER_TIMEBUFFER_H
#define SSI_BUFFER_TIMEBUFFER_H

#include "Buffer.h"

namespace ssi {

class TimeBuffer : public Buffer {

public:

	//! timebuffer status
	enum STATUS {
		SUCCESS = 0,
		INPUT_ARRAY_TOO_SMALL,
		DATA_EXCEEDS_BUFFER_SIZE,
		DATA_NOT_IN_BUFFER_YET,
		DATA_NOT_IN_BUFFER_ANYMORE,
		DURATION_TOO_SMALL,
		DURATION_TOO_LARGE
	};
	static ssi_char_t *STATUS_NAMES[7];

	TimeBuffer (ssi_size_t capacity_in_samples, ssi_time_t sample_rate, ssi_size_t sample_dimension, ssi_size_t sample_bytes, ssi_type_t sample_type);
	TimeBuffer (ssi_time_t capacity_in_sec, ssi_time_t sample_rate, ssi_size_t sample_dimension, ssi_size_t sample_bytes, ssi_type_t sample_type);
	virtual ~TimeBuffer ();

	TimeBuffer::STATUS push (const ssi_byte_t *data, ssi_size_t samples);
	TimeBuffer::STATUS pushZeros (ssi_size_t samples);
	TimeBuffer::STATUS get (ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration);
	TimeBuffer::STATUS get (ssi_byte_t *data, ssi_size_t samples, ssi_lsize_t position);
	void sync (ssi_time_t sync_time);

	void reset (ssi_time_t time);

	ssi_time_t getOffsetTime ();
	ssi_time_t getCurrentSampleTime ();
	ssi_lsize_t getCurrentWritePos ();
	void setCurrentSampleTime (ssi_time_t time);
	ssi_time_t getLastAccessedSampleTime();
	ssi_time_t getLastWrittenSampleTime();
	ssi_time_t getCapacity ();

	const ssi_size_t max_samples; // max number of samples that can be buffered
	const ssi_time_t sample_rate; // sample rate
	const ssi_time_t sample_duration; // 1 / sample rate = duration of one sample
	const ssi_size_t sample_bytes; // size of one sample element in bytes
	const ssi_type_t sample_type; // type of one sample element
	const ssi_size_t sample_dimension; // number of elements per sample
	const ssi_size_t sample_total_bytes; // size of one sample in bytes = sample_element_bytes * sample_dimension

private:

	bool buffer_filled; // false until buffer is completely filled
	ssi_lsize_t next_sample; // write position
	ssi_lsize_t last_accessed_sample; // last accessed sample index
	int offset_samples; // offset

};

}

#endif
