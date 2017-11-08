// SocketEventWriter.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/12/07
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

#ifndef SSI_SOCKET_SOCKETEVENTWRITER_H
#define SSI_SOCKET_SOCKETEVENTWRITER_H

#include "ioput/option/OptionList.h"
#include "base/IObject.h"
#include "ioput/socket/SocketOsc.h"
#include "base/ITheFramework.h"

namespace ssi {

class SocketEventWriter : public IObject {

public:

	class Options : public OptionList {

	public:

		Options ()
			: osc (false), xml (true), reltime (false), cdata(false) {

			setUrl("");

			addOption ("url", url, SSI_MAX_CHAR, SSI_CHAR, "url of the form 'scheme://host:port' (e.g. udp://172.0.0.1:1234)");
			addOption ("osc", &osc, 1, SSI_BOOL, "use osc format");
			addOption ("xml", &xml, 1, SSI_BOOL, "output in xml format (not for osc)");
			addOption ("cdata", &cdata, 1, SSI_BOOL, "wrap xml strings in cdata");
			addOption ("reltime", &reltime, 1, SSI_BOOL, "send relative time stamps (osc or xml)");

			// deprecated

			type = Socket::TYPE::UDP;
			port = 1234;
			setHost("localhost");

			addOption("host", host, SSI_MAX_CHAR, SSI_CHAR, "host name (empty for any) [deprecated use 'url']");
			addOption("port", &port, 1, SSI_INT, "port number (-1 for any) [deprecated use 'url']");
			addOption("type", &type, 1, SSI_UCHAR, "protocol type (0=UDP, 1=TCP) [deprecated use 'url']");
		};

		void setHost (const ssi_char_t *host) {
			this->host[0] = '\0';
			if (host) {
				ssi_strcpy (this->host, host);
			}
		}
		void setUrl(const ssi_char_t *url) {
			this->url[0] = '\0';
			if (url) {
				ssi_strcpy(this->url, url);
			}
		}
		void setUrl(Socket::TYPE::List type, const ssi_char_t *host, int port) {
			ssi_sprint(url, "%s://%s:%d", Socket::TYPE_NAMES[type], host, port);
		}

		ssi_char_t url[SSI_MAX_CHAR];
		ssi_char_t host[SSI_MAX_CHAR];
		int port;
		Socket::TYPE::List type;	
		bool osc;
		bool xml;
		bool reltime;
		bool cdata;
	};

public:

	static const ssi_char_t *GetCreateName () { return "SocketEventWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new SocketEventWriter (file); };
	~SocketEventWriter ();
	
	SocketEventWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Forwards events to external applications through a socket connection."; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}
	
protected:

	SocketEventWriter (const ssi_char_t *file = 0);
	SocketEventWriter::Options _options;
	ssi_char_t *_file;

	Socket *_socket;
	SocketOsc *_socket_osc;
	long _timeout;
	ssi_size_t _bytes_per_sample;

	ITheFramework *_frame;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
};

}

#endif
