// Buffer.cpp
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

#include "buffer/Buffer.h"

#include <memory.h>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Buffer::Buffer (ssi_size_t size_)
: size (size_),
	_meta_size (0),
	_meta (0){

	// create buffer array and fill it with zeros
	buffer = (ssi_byte_t*)malloc(size);
	if(buffer == 0) ssi_err("Failed creating buffer (size = %.2f MB). Not enough memory.", size / 1048576.0f);

	for (ssi_size_t i = 0; i < size; i++) {
		buffer[i] = 0;
	}
};

Buffer::~Buffer () {

	// delete buffer
	if (buffer)
		free (buffer);
	delete[] _meta;
}

void Buffer::setMetaData (ssi_size_t size, const void *meta) {

	if (size > 0 && meta != 0) {
		_meta_size = size;
		_meta = new ssi_byte_t[_meta_size];
		memcpy (_meta, meta, _meta_size);
	}
}

bool Buffer::get (ssi_byte_t *data, ssi_lsize_t position, ssi_size_t bytes) {

	ssi_size_t begin_mod = position % size;

	// return false if requested data size
	// exceeds buffer size
	if (bytes > size) {
		return false;
	}

	// copy requested size from buffer to data
	if (begin_mod + bytes <= size) {
		// end of buffer not reached
		// copy data in one step
		memcpy (data, buffer + begin_mod, bytes);
	} else {
		// end of buffer reached
		// copy data in two steps:
		// 1. copy everything until the end of the buffer is reached
		// 2. copy remaining part from the beginning
		ssi_size_t size_until_end = size - begin_mod;
		ssi_size_t size_remaining = bytes - size_until_end;
		memcpy (data, buffer + begin_mod, size_until_end);
		memcpy (data + size_until_end, buffer, size_remaining);
	}

	return true;
}

bool Buffer::put (const ssi_byte_t *data, ssi_lsize_t position, ssi_size_t bytes) {

	ssi_size_t begin_mod = position % size;

	// return false if delivered data size
	// exceeds buffer size
	if (bytes > size) {
		return false;
	}

	// copy delivered data to buffer
	if (begin_mod + bytes <= size) {
		// end of buffer not reached
		// copy data in one step
		memcpy (buffer + begin_mod, data, bytes);
	} else {
		// end of buffer reached
		// copy data in two steps:
		// 1. fill until end of buffer is reached
		// 2. copy remaining part to the beginning
		ssi_size_t size_until_end = size - begin_mod;
		ssi_size_t size_remaining = bytes - size_until_end;
		memcpy (buffer + begin_mod, data, size_until_end);
		memcpy (buffer, data + size_until_end, size_remaining);
	}

	return true;
}

bool Buffer::putZeros (ssi_lsize_t position, ssi_size_t bytes) {

	ssi_size_t begin_mod = position % size;

	// return false if delivered data size
	// exceeds buffer size
	if (bytes > size) {
		return false;
	}

	// copy delivered data to buffer
	if (begin_mod + bytes <= size) {
		// end of buffer not reached
		// copy data in one step
		memset (buffer + begin_mod, 0, bytes);
	} else {
		// end of buffer reached
		// copy data in two steps:
		// 1. fill until end of buffer is reached
		// 2. copy remaining part to the beginning
		ssi_size_t size_until_end = size - begin_mod;
		ssi_size_t size_remaining = bytes - size_until_end;
		memset (buffer + begin_mod, 0, size_until_end);
		memset (buffer, 0, size_remaining);
	}

	return true;
}

}
