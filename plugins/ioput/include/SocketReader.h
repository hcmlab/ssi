// SocketReader.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/06/02
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

#ifndef SSI_SOCKET_SOCKETREADER_H
#define SSI_SOCKET_SOCKETREADER_H

#include "base/ISensor.h"
#include "thread/Thread.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/socket/SocketImage.h"
#include "ioput/option/OptionList.h"
#include "ioput/file/FileMem.h"

#define SSI_SOCKETREADER_PROVIDER_NAME "socket"

namespace ssi {

class SocketReader :  public ISensor, public Thread, public SocketOscListener {

	class SocketChannel : public IChannel {

		friend class SocketReader;

		public:

			SocketChannel () {
				ssi_stream_init (stream, 0, 0, 0, SSI_UNDEF, 0);
			}
			~SocketChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_SOCKETREADER_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Properties are determined from the options."; };
			ssi_stream_t getStream () { return stream; };

		protected:

			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		struct FORMAT {
			enum VALUE {
				BINARY,
				ASCII,
				OSC,
				IMAGE
			};
		};

	public:

		Options ()
			: port (-1), type (Socket::UDP), ssr (0), sdim (0), sbyte (0), stype (SSI_UNDEF), swidth (640), sheight (480), sdepth (8), schannels (3), size (Socket::MAX_MTU_SIZE), timeout (1000), osc (false), format (FORMAT::BINARY) {

			delim[0] = ',';
			delim[1] = '\0';
			setHost("");

			addOption ("host", host, SSI_MAX_CHAR, SSI_CHAR, "host name (empty for any)");
			addOption ("port", &port, 1, SSI_INT, "port number (-1 for any)");		
			addOption ("size", &size, 1, SSI_UINT, "size of buffer");
			addOption ("type", &type, 1, SSI_UCHAR, "protocol type (0=UDP, 1=TCP)");
			addOption ("osc", &osc, 1, SSI_BOOL, "use osc format (deprecated, use format instead)");
			addOption ("format", &format, 1, SSI_INT, "streaming format: 0=BINARY, 1=ASCII, 2=OSC, 3=IMAGE");
			addOption ("delim", &delim, SSI_MAX_CHAR, SSI_CHAR, "delim chars if streaming in ascii format");
			addOption ("ssr", &ssr, 1, SSI_DOUBLE, "sample rate in Hz");
			addOption ("sdim", &sdim, 1, SSI_UINT, "sample dimension (if format is set to BINARY, ASCII or OSC)");
			addOption ("sbyte", &sbyte, 1, SSI_UINT, "sample bytes (if format is set to BINARY, ASCII or OSC)");
			addOption ("stype", &stype, 1, SSI_UCHAR, "sample type (0=UNDEF, 1=CHAR, 2=UCHAR, 3=SHORT 4=USHORT, 5=INT, 6=UINT, 7=LONG, 8=ULONG, 9=FLOAT, 10=DOUBLE, 11=LDOUBLE, 12=STRUCT, 13=IMAGE, 14=BOOL)");			
			addOption ("swidth", &swidth, 1, SSI_INT, "image width in pixels (if format is set to IMAGE)");
			addOption ("sheight", &sheight, 1, SSI_INT, "image height in pixels (if format is set to IMAGE)");
			addOption ("sdepth", &sdepth, 1, SSI_INT, "number of bits per pixel (if format is set to IMAGE)");
			addOption ("schannels", &schannels, 1, SSI_INT, "number of channels per pixel (if format is set to IMAGE)");
			addOption ("timeout", &timeout, 1, SSI_UINT, "time out in milliseconds");			
		};

		void setHost (const ssi_char_t *host) {
			this->host[0] = '\0';
			if (host) {
				ssi_strcpy (this->host, host);
			}
		}
		void setSampleInfo (ssi_time_t sr, ssi_size_t dim, ssi_size_t byte, ssi_type_t type) {
			ssr = sr; sdim = dim; sbyte = byte; stype = type;
		}
		void setSampleInfo (ssi_video_params_t vparams) {
			ssr = vparams.framesPerSecond;
			schannels = vparams.numOfChannels;
			sdepth = vparams.depthInBitsPerChannel;
			swidth = vparams.widthInPixels;
			sheight = vparams.heightInPixels;
		}

		ssi_char_t host[SSI_MAX_CHAR];
		int port;
		ssi_size_t size;
		Socket::TYPE type;	
		FORMAT::VALUE format;
		char delim[SSI_MAX_CHAR];
		ssi_time_t ssr;
		ssi_size_t sdim;
		ssi_size_t sbyte;
		ssi_type_t stype;
		int swidth, sheight, sdepth, schannels;
		ssi_size_t timeout;		
		bool osc;
	};


public:

	static const ssi_char_t *GetCreateName () { return "SocketReader"; };
	static IObject *Create (const ssi_char_t *file) { return new SocketReader (file); };
	~SocketReader ();
	
	SocketReader::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Receives a signal through a UDP or TCP socket connection either as a raw stream or in osc format."; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_socket_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	void terminate ();
	bool disconnect ();

	ssi_video_params_t getFormat () { return _vformat; };
	const void *getMetaData (ssi_size_t &size) { 
		if (_options.format == Options::FORMAT::IMAGE) { 
			size = sizeof (_vformat); 
			return &_vformat; 
		} else {
			size = 0;
			return 0;
		}
	};

	static void setLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	SocketReader (const ssi_char_t *file = 0);
	SocketReader::Options _options;
	ssi_char_t *_file;

	SocketChannel _socket_channel;
	void setProvider (IProvider *provider);

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	IProvider *_provider;
	Socket *_socket;
	SocketOsc *_socket_osc;
	SocketImage *_socket_img;
	ssi_size_t _bytes_per_sample;
	ssi_video_params_t _vformat;
	ssi_size_t _n_buffer;
	ssi_byte_t *_buffer, *_buffer2;
	FileMem *_memory;

	void message (const char *from,
		const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		osc_int32 time,
		osc_int32 dur,
		const ssi_char_t *msg) {};
	void stream (const char *from,
		const ssi_char_t *id,
		osc_int32 time,
		float sr,
		osc_int32 num, 		
		osc_int32 dim,
		osc_int32 bytes,
		osc_int32 type,
		void *data);
	void event (const char *from,
		const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		osc_int32 time,
		osc_int32 dur,
		osc_int32 state,
		osc_int32 n_events,
		const ssi_char_t **events,
		const ssi_real_t *values) {};
};

}

#endif

