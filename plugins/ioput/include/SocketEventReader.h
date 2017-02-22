// SocketEventReader.h
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

#ifndef SSI_SOCKET_SOCKETEVENTREADER_H
#define SSI_SOCKET_SOCKETEVENTREADER_H

#include "base/ISensor.h"
#include "thread/Thread.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/ITheFramework.h"

namespace ssi {

class SocketEventReader :  public SSI_IRunnableObject, public Thread, public SocketOscListener {

	class Options : public OptionList {

	public:

		Options ()
			: port (-1), type (Socket::UDP), size (Socket::DEFAULT_MTU_SIZE), timeout (1000), osc (false), reltime(false) {

			strcpy (sname, "socketreceiver");
			strcpy (ename, "socketevent");

			host[0] = '\0';

			SSI_OPTIONLIST_ADD_ADDRESS(address);

			addOption ("host", host, SSI_MAX_CHAR, SSI_CHAR, "host name (empty for any)");
			addOption ("port", &port, 1, SSI_INT, "port number (-1 for any)");		
			addOption ("size", &size, 1, SSI_UINT, "size of buffer");
			addOption ("type", &type, 1, SSI_UCHAR, "protocol type (0=UDP, 1=TCP)");
			addOption ("timeout", &timeout, 1, SSI_UINT, "time out in milliseconds");
			addOption ("osc", &osc, 1, SSI_BOOL, "use osc format");
			addOption ("reltime", &reltime, 1, SSI_BOOL, "send relative time stamps (osc or xml)");

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "sender name [deprecated use address]");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "event name [deprecated use address]");
		};

		void setHost (const ssi_char_t *host) {
			this->host[0] = '\0';
			if (host) {
				ssi_strcpy (this->host, host);
			}
		}
		
		void setSenderName (const ssi_char_t *sname) {
			this->sname[0] = '\0';
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}

		void setEventName (const ssi_char_t *ename) {
			this->ename[0] = '\0';
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}
		
		ssi_char_t host[SSI_MAX_CHAR];
		int port;
		ssi_size_t size;
		Socket::TYPE type;	
		ssi_size_t timeout;
		bool osc;
		bool reltime;
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
		ssi_char_t address[SSI_MAX_CHAR];
	};


public:

	static const ssi_char_t *GetCreateName () { return "SocketEventReader"; };
	static IObject *Create (const ssi_char_t *file) { return new SocketEventReader (file); };
	~SocketEventReader ();
	
	SocketEventReader::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Receives events through a UDP or TCP socket connection."; };

	bool start() {
		return Thread::start();
	}
	bool stop (){
		return Thread::stop();
	}
	void enter ();
	void run ();
	void flush ();

	void send_enter () {
		start ();
	}
	void send_flush () {
		stop ();
	}

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	static void setLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	SocketEventReader (const ssi_char_t *file = 0);
	SocketEventReader::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	IProvider *_provider;
	Socket *_socket;
	SocketOsc *_socket_osc;
	ssi_size_t _bytes_per_sample;
	ssi_byte_t *_buffer;

	ITheFramework *_frame;

	IEventListener *_elistener;
	ssi_event_t _event_string;
	EventAddress _event_address;

	void message (const char *from,
		const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		osc_int32 time,
		osc_int32 dur,
		const ssi_char_t *msg);
	void stream (const char *from,
		const ssi_char_t *id,
		osc_int32 time,
		float sr,
		osc_int32 num, 		
		osc_int32 dim,
		osc_int32 bytes,
		osc_int32 type,
		void *data) {};
	void event (const char *from,
		const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		osc_int32 time,
		osc_int32 dur,
		osc_int32 state,
		osc_int32 n_events,
		const ssi_char_t **events,
		const ssi_real_t *values);
};

}

#endif
