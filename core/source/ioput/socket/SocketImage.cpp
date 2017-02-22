// SocketImage.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/06/09
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

#include "ioput/socket/SocketImage.h"
#include "ioput/jpg/jpge.h"
#include "ioput/jpg/jpgd.h"
#if __gnu_linux__
#include <chrono>
#include <thread>
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

const ssi_char_t *SocketImage::ssi_log_name = "socketimg_";

SocketImage::SocketImage (Socket &socket,
	ssi_size_t capacity,
	ssi_size_t packet_send_delay_ms)
	: _socket (socket),
	_counter (0),
	_buffer (0),
	_n_buffer (capacity),
	_n_packet (0),
	_early_header (false),
	_n_compressed (0),
	_compressed (0),
	_packet_send_delay_ms (packet_send_delay_ms),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	_header.master = SSI_SOCKETIMAGE_HEADER_ID;
	_n_packet = _n_buffer - sizeof (DATA);
	_buffer = new ssi_byte_t[_n_buffer];
	ssi_pcast (DATA, _buffer)->master = SSI_SOCKETIMAGE_DATA_ID;
}

SocketImage::~SocketImage () {

	delete[] _compressed; _compressed = 0;
	delete[] _buffer; _buffer = 0;
}

int SocketImage::recvImage (ssi_video_params_t params, void *ptr, ssi_size_t size, long timeout_in_ms) {

	int result = 0;
	int n_recv = 0;

	// wait for next header
	if (!_early_header) {
		do {
			n_recv = _socket.recv (_buffer, _n_buffer, timeout_in_ms);
			if (n_recv == 0) {
				return 0;
			}
		} while (n_recv != sizeof (HEADER));
		result = n_recv;
		memcpy (&_header, _buffer, sizeof (HEADER));
		_early_header = false;
	}

	// read header
	if (params.widthInPixels != _header.params.widthInPixels ||
		params.heightInPixels != _header.params.heightInPixels ||
		params.depthInBitsPerChannel != _header.params.depthInBitsPerChannel ||
		(params.numOfChannels == 3 && (_header.params.numOfChannels != 3 && _header.params.numOfChannels != 4)) ||
		(params.numOfChannels != 3 && _header.params.numOfChannels != params.numOfChannels)) {
		ssi_wrn ("images do not fit");
		return 0;
	}
	ssi_size_t n_packets = _header.n_packets;
	ssi_size_t id = _header.id;
	COMPRESSION::VALUE compression = ssi_cast (COMPRESSION::VALUE, _header.compression);

	// clear image
	memset (ptr, 0, size);

	// receive image
	ssi_byte_t *image = 0;
	ssi_size_t n_image;
	switch (compression) {
		case COMPRESSION::NONE:
			image = (ssi_byte_t *) ptr;
			n_image = size;
			break;
		case COMPRESSION::JPG:
			if (_n_compressed < _header.bytes) {
				_n_compressed = _header.bytes;
				delete[] _compressed; _compressed = 0;
				_compressed = new ssi_byte_t[_n_compressed];
			}
			image = _compressed;
			n_image = _header.bytes;
			break;
	}

	ssi_size_t n = 0;
	while (n < n_packets) {

		n_recv = _socket.recv (_buffer, _n_buffer, timeout_in_ms);
		if (n_recv == 0) {
			return 0;
		}

		DATA *data = ssi_pcast (DATA, _buffer);
		if (data->master == SSI_SOCKETIMAGE_DATA_ID) {
			if (data->id == id) {
				memcpy (image + data->num * _n_packet, _buffer + sizeof (DATA), data->bytes);
				result += n_recv;
				n++;
			} else {
				ssi_wrn ("received incomplete image (new id)");
				return compression == COMPRESSION::NONE ? result : 0;
			}
		} else {
			ssi_wrn ("received incomplete image (new header)");
			_early_header = true;
			memcpy (&_header, _buffer, sizeof (HEADER));
			return compression == COMPRESSION::NONE ? result : 0;
		}
	}

	// uncompress
	switch (compression) {
		case COMPRESSION::NONE:
			break;
		case COMPRESSION::JPG:
			int actual_comps;
			int width;
			int height;
			if (!jpgd::decompress_jpeg_image_from_memory (ssi_pcast (jpgd::uint8, ptr), size, ssi_pcast (unsigned char, image), n_image, &width, &height, &actual_comps, params.numOfChannels)) {
				ssi_wrn ("jpg decompression failed");
				return 0;
			}
			break;
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "received image (%u)", id);

	return result;
}

int SocketImage::sendImage (ssi_video_params_t params, const void *ptr, ssi_size_t size, COMPRESSION::VALUE compression) {

	ssi_size_t result = 0;
	ssi_byte_t *image = 0;
	int n_image = 0;

	// apply compression
	switch (compression) {
		case COMPRESSION::NONE:
			image = (ssi_byte_t *) ptr;
			n_image = size;
			break;
		case COMPRESSION::JPG:
			if (_n_compressed < size) {
				_n_compressed = size;
				delete[] _compressed; _compressed = 0;
				_compressed = new ssi_byte_t[_n_compressed];
			}
			image = _compressed;
			n_image = size;

			// special case - depth image of kinect
			// we cheat by doubling height and setting depth to 8 bit
			if (params.numOfChannels == 1 && params.depthInBitsPerChannel == 16) {
				if (!jpge::compress_image_to_jpeg_file_in_memory (image, n_image, params.widthInPixels, params.heightInPixels*2, params.numOfChannels, ssi_pcast (const jpge::uint8, ptr), jpge::params())) {
					ssi_wrn ("jpg compression failed");
					return 0;
				}
			} else {
				if (!jpge::compress_image_to_jpeg_file_in_memory (image, n_image, params.widthInPixels, params.heightInPixels, params.numOfChannels, ssi_pcast (const jpge::uint8, ptr), jpge::params())) {
					ssi_wrn ("jpg compression failed");
					return 0;
				}
			}
			break;
	}

	// calc number of packets
	ssi_size_t n_packets = n_image / _n_packet;
	if (n_packets * _n_packet != n_image) {
		n_packets++;
	}

	// send header
	_header.id = _counter++;
	_header.compression = compression;
	_header.bytes = n_image;
	_header.n_packets = n_packets;
	_header.params = params;
	result += _socket.send (&_header, sizeof (HEADER));

	// send data
	DATA *data = ssi_pcast (DATA, _buffer);
	data->id = _header.id;
	ssi_size_t remaining = n_image;
	for (ssi_size_t i = 0; i < n_packets; i++) {
		ssi_size_t n_bytes = min (_n_packet, n_image - i * _n_packet);
		data->num = i;
		data->bytes = n_bytes;
		memcpy (_buffer + sizeof (DATA), image, n_bytes);
		result += _socket.send (_buffer, sizeof (DATA) + n_bytes);
		image += _n_packet;
ssi_sleep(_packet_send_delay_ms);

	}

	if (result != (sizeof (HEADER) + n_packets * sizeof (DATA) + n_image)) {
		ssi_wrn ("image not (completely) sent");
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "sent image (%u)", _header.id);

	return result;
}

const char *SocketImage::getRecvAddress () {
	return _socket.getRecvAddress ();
}

}
