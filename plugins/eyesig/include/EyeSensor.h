// EyeSensor.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/08
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

#ifndef SSI_EYE_SENSOR_H
#define	SSI_EYE_SENSOR_H

// link libraries
#ifdef _MSC_VER 
#pragma comment (lib, "Ws2_32.lib")
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "ioput/option/OptionList.h"
#include "thread/Thread.h"

#define SSI_EYE_PACKET_SIZE			256
#define SSI_EYE_SAMPLE_TYPE			int
#define SSI_EYE_PROVIDER_NAME		"position"

namespace ssi {

class EyeSensor : public ISensor, public Thread {

public:

	class EyeChannel : public IChannel {

		friend class EyeSensor;

		public:

			EyeChannel () {
				ssi_stream_init (stream, 0, 2, sizeof(SSI_EYE_SAMPLE_TYPE), SSI_INT, 50.0);
			}
			~EyeChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_EYE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Provides 2d position (X,Y) as stream of signed int values at a rate of 50 Hz."; };
			ssi_stream_t getStream () { return stream; };

		protected:

			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options () {

			addOption ("size", &size, 1, SSI_TIME, "block size in seconds");
			addOption ("port", &port, 1, SSI_INT, "port number of eye tracker");
		};

		ssi_time_t size;
		int port;
	};

public:

	static const ssi_char_t *GetCreateName () { return "EyeSensor"; };
	static IObject *Create (const ssi_char_t *file) { return new EyeSensor (file); };
	~EyeSensor ();
	EyeSensor::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "eye tracker sensor"; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_eye_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *_provider);
	bool connect ();
	bool start () { return Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	EyeSensor (const ssi_char_t *file = 0);
	EyeSensor::Options _options;
	ssi_char_t *_file;

	int ssi_log_level;
	bool strtoken (char *&str, char token, char *substr, int maxlen);

	EyeChannel _eye_channel;
	IProvider *_provider;
	ssi_size_t _buffer_size;
	int *_buffer;
	ssi_byte_t _socket_buffer[SSI_EYE_PACKET_SIZE];
	SOCKET _socket;
	int _counter;
};

}

#endif

