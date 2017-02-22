// TransformedEventSender.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/05/23
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

#ifndef SSI_TRANSFORMEDEVENTSENDER_H
#define SSI_TRANSFORMEDEVENTSENDER_H

#include "base/IConsumer.h"
#include "base/IEvents.h"
#include "ioput/option/OptionList.h"
#include "base/IFeature.h"
#include "base/ITheEventBoard.h"
#include "event/EventAddress.h"
#include "base/ITheEventBoard.h"

namespace ssi {

class TransformedEventSender : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options () {

			setSenderName ("te_sender");
			setEventName ("te_event");

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");				
		};

		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEventName (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}

		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];		
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "TransformedEventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new TransformedEventSender (file); };
	~TransformedEventSender ();

	TransformedEventSender::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "applies transformations and sends them as an event to the board"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	virtual void setTransformer(ITransformer *transformer){
		_transformer = transformer;
	}



protected:

	TransformedEventSender (const ssi_char_t *file = 0);
	TransformedEventSender::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _event;
	ITheEventBoard *_board;

	ITransformer *_transformer;
	ssi_stream_t _fstream;
	
};

}

#endif
