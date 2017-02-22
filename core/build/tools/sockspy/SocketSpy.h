// SocketSpy.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/18
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

#ifndef SSI_SOCKETSPY_H
#define SSI_SOCKETSPY_H

#include "ssi.h"
using namespace ssi;

class SocketSpy : public Thread, public SocketOscListener {

public:

	struct FORMAT {
		enum VALUE {
			PEEK,
			BINARY,
			ASCII,
			OSC,
			IMAGE,
			FORK
		};
	};

	struct options {
		FORMAT::VALUE format;
		Socket::TYPE protocol;
		int buffer_size;
		ssi_char_t *work_dir;
		ssi_char_t *now;
		int port;
		ssi_char_t *host;
		bool log_ascii;
		bool log_binary;
		bool log_console;
		ssi_char_t *log_delim;
		ssi_char_t *log_format;
		bool log_header;	
		ssi_type_t type;
		ssi_size_t dim;
		ssi_video_params_t vparams;		
		const char *fork_urls;
	};


public:

	SocketSpy (options opts);
	~SocketSpy ();

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	void message (const char *from,
		const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		osc_int32 time,
		osc_int32 dur,
		const ssi_char_t *msg);

	void stream (const ssi_char_t *from,
		const ssi_char_t *id,
		ssi::osc_int32 time,
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
		const ssi_char_t **event_name,
		const ssi_real_t *event_value);

	void init_log ();

	void enter ();
	void run ();
	void flush ();

	ssi_time_t _sample_rate;
	ssi_size_t _sample_dimension;
	ssi_size_t _sample_bytes;
	ssi_type_t _sample_type;

	options _opts;

	File *_cfile;
	File *_bfile;
	File *_afile;

	bool _first_call;
	ssi_char_t _string[10000];

	Socket *_socket;
	SocketOsc *_socket_osc;
	SocketImage *_socket_img;

	IObject *_painter;
	int _plot_id;

	ssi_byte_t *_buffer;
	int _buffer_size;

	ssi_size_t _n_fork;
	Socket **_fork;
};

#endif
