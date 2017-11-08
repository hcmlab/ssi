// NotifySender.h
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

#ifndef SSI_IOPUT_NOTIFYSENDER_H
#define SSI_IOPUT_NOTIFYSENDER_H

#include "ioput/option/OptionList.h"
#include "base/IObject.h"
#include "ioput/socket/Socket.h"

namespace ssi {

class NotifySender : public SSI_IRunnableObject {

public:

	class Options : public OptionList {

	public:

		Options () {

			setUrl ("upd://172.0.0.1:1234");	

			addOption("url", url, SSI_MAX_CHAR, SSI_CHAR, "url of the form 'scheme://host:port' (e.g. upd://:-1, tcp://172.0.0.1:1234)");
		};

		void setUrl (const ssi_char_t *url) {
			this->url[0] = '\0';
			if (url) {
				ssi_strcpy (this->url, url);
			}
		}

		ssi_char_t url[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "NotifySender"; };
	static IObject *Create (const ssi_char_t *file) { return new NotifySender (file); };
	~NotifySender ();
	
	NotifySender::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Forwards notifictions through a socket connection."; };

	bool start();
	bool stop();
	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);
	void setEnabled(bool enabled);

	void setLogLevel (int level) 
	{
		ssi_log_level = level;
	}
	
protected:

	NotifySender (const ssi_char_t *file = 0);
	NotifySender::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	Socket *_socket;
	ssi_byte_t *_buffer;
	ssi_size_t _n_buffer;

};

}

#endif
