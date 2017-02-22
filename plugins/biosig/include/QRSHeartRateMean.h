// QRSHeartRateMean.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/02/13
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

#ifndef SSI_EVENT_QRSHEARTRATEMEAN_H
#define SSI_EVENT_QRSHEARTRATEMEAN_H

#include "base/IConsumer.h"
#include "base/IEvents.h"
#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class QRSHeartRateMean : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options () 
			: winsize (5.0), cap (true), amplifier (1.0){

			setSenderName ("heartrate");
			setEventName ("mean");			

			addOption ("winsize", &winsize, 1, SSI_REAL, "size in seconds of mean window");
			addOption ("cap", &cap, 1, SSI_BOOL, "cap change value [-1.0 .. 1.0]");
			addOption ("amplifier", &amplifier, 1, SSI_REAL, "multiply value of change with amplifier");
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
		ssi_time_t winsize;
		bool cap;
		ssi_real_t amplifier;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "QRSHeartRateMean"; };
	static IObject *Create (const ssi_char_t *file) { return new QRSHeartRateMean (file); };
	~QRSHeartRateMean ();

	QRSHeartRateMean::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "sends hr-mean events to the board"; };

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

protected:

	QRSHeartRateMean (const ssi_char_t *file = 0);
	QRSHeartRateMean::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _event;

	bool firstrun;

	ssi_time_t _src_sr;
	ssi_real_t _sample_counter;
	ssi_real_t _win_samples;
	ssi_real_t _sample_sum;
	ssi_real_t _last_mean;

};

}

#endif
