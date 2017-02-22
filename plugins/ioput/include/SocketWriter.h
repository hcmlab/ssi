// SocketWriter.h
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

#ifndef SSI_SOCKET_SOCKETWRITER_H
#define SSI_SOCKET_SOCKETWRITER_H

#include "base/IConsumer.h"
#include "ioput/file/FileMem.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/socket/SocketImage.h"
#include "ioput/option/OptionList.h"
#include "base/ITheFramework.h"

namespace ssi {

class SocketWriter : public IConsumer {

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
			: port (1234), type (Socket::UDP), osc (false), reltime (false), format (FORMAT::BINARY), compression (SocketImage::COMPRESSION::JPG), packet_delay (3) {

			host[0] = '\0';
			delim[0] = ',';
			delim[1] = '\0';
			setHost("localhost");

			addOption ("host", host, SSI_MAX_CHAR, SSI_CHAR, "host name (empty for any)");
			addOption ("port", &port, 1, SSI_INT, "port number (-1 for any)");		
			addOption ("type", &type, 1, SSI_UCHAR, "protocol type (0=UDP, 1=TCP)");	
			addOption ("osc", &osc, 1, SSI_BOOL, "use osc format (deprecated, use format instead)");
			addOption ("format", &format, 1, SSI_INT, "streaming format: 0=BINARY, 1=ASCII, 2=OSC, 3=IMAGE");
			addOption ("delim", &delim, SSI_MAX_CHAR, SSI_CHAR, "delim chars if streaming in ascii format");
			addOption ("id", id, SSI_MAX_CHAR, SSI_CHAR, "id if streaming in osc format");
			addOption ("compression", &compression, 1, SSI_INT, "compression if streaming images (0=NONE,1=JPG)");
			addOption ("packet_delay", &packet_delay, 1, SSI_SIZE, "delay between sending packets in milliseconds to give receiver some time to pick them up (IMAGE only)");
			addOption ("reltime", &reltime, 1, SSI_BOOL, "send relative time stamps (OSC only)");
		};

		void setHost (const ssi_char_t *host) {
			this->host[0] = '\0';
			if (host) {
				ssi_strcpy (this->host, host);
			}
		}
		void setId (const ssi_char_t *id) {
			this->id[0] = '\0';
			if (id) {
				ssi_strcpy (this->id, id);
			}
		}

		ssi_char_t host[SSI_MAX_CHAR];
		int port;
		Socket::TYPE type;	
		bool osc;
		ssi_char_t id[SSI_MAX_CHAR];
		bool reltime;
		FORMAT::VALUE format;
		char delim[SSI_MAX_CHAR];
		SocketImage::COMPRESSION::VALUE compression;
		ssi_size_t packet_delay;
	};

public:

	static const ssi_char_t *GetCreateName () { return "SocketWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new SocketWriter (file); };
	~SocketWriter ();
	
	SocketWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Forwards streams to external applications through a socket connection."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void setVideoFormat (ssi_video_params_t	video_format) { _video_format = video_format; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_wrn ("unknown meta size");
			return;
		}
		memcpy (&_video_format, meta, size);
	}
	ssi_video_params_t getVideoFormat () { return _video_format; };

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	SocketWriter (const ssi_char_t *file = 0);
	SocketWriter::Options _options;
	ssi_char_t *_file;

	Socket *_socket;
	SocketOsc *_socket_osc;
	long _timeout;
	ssi_size_t _bytes_per_sample;
	FileMem *_memory;

	ssi_video_params_t _video_format;
	SocketImage *_socket_img;

	ITheFramework *_frame;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
};

}

#endif
