// NotifyReceiver.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/11/06
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

#ifndef SSI_IOPUT_NOTIFYRECEIVER_H
#define SSI_IOPUT_NOTIFYRECEIVER_H

#include "base/ISensor.h"
#include "thread/Thread.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/ITheFramework.h"

namespace ssi {

class NotifyReceiver : public SSI_IRunnableObject, public Thread {

	class Options : public OptionList {

	public:

		Options()
		: size(Socket::DEFAULT_MTU_SIZE), timeout(1000) {

			setUrl("upd://172.0.0.1:1234");
			setId("");

			addOption("url", url, SSI_MAX_CHAR, SSI_CHAR, "url of the form 'scheme://host:port' (e.g. upd://:-1, tcp://172.0.0.1:1234)");
			addOption("id", id, SSI_MAX_CHAR, SSI_CHAR, "object id(s) (if several separate by comma)");
			addOption("size", &size, 1, SSI_UINT, "size of buffer");
			addOption("timeout", &timeout, 1, SSI_UINT, "time out in milliseconds");
		};

		void setUrl(const ssi_char_t *url) {
			this->url[0] = '\0';
			if (url) {
				ssi_strcpy(this->url, url);
			}
		}
		void setId(const ssi_char_t *string) {
			if (string) {
				ssi_strcpy(this->id, string);
			}
		}

		ssi_char_t url[SSI_MAX_CHAR];
		ssi_char_t id[SSI_MAX_CHAR];			
		ssi_size_t size;		
		ssi_size_t timeout;
	};


public:

	static const ssi_char_t *GetCreateName () { return "NotifyReceiver"; };
	static IObject *Create (const ssi_char_t *file) { return new NotifyReceiver (file); };
	~NotifyReceiver ();
	
	NotifyReceiver::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Receives events through a UDP or TCP socket connection."; };

	bool start()
	{
		return Thread::start();
	}
	bool stop ()
	{
		return Thread::stop();
	}
	void enter ();
	void run ();
	void flush ();

	void setLogLevel (int level)
	{
		ssi_log_level = level;
	}
	
protected:

	NotifyReceiver (const ssi_char_t *file = 0);
	NotifyReceiver::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	Socket *_socket;	
	ssi_byte_t *_buffer;

	ssi_size_t _n_targets;
	IObject **_targets;
	ssi_char_t **_target_ids;

};

}

#endif
