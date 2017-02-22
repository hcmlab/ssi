// QRSHrvEventListener.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/01/17
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

#ifndef SSI_LISTENER_QRSHRVEVENTLISTENER_H
#define SSI_LISTENER_QRSHRVEVENTLISTENER_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class QRSHrvEventListener : public IObject {

public:

	class Options : public OptionList {

	public:

		Options () : span(60000), update_ms(5000),  amplifier (1.0) {

			setSenderName ("qrshrvlistener");
			setEventName ("hrv");			

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");
			addOption ("span", &span, 1, SSI_INT, "Time span for hrv calculation in ms.");
			addOption("update_ms", &update_ms, 1, SSI_INT, "Time interval the updated fusion vector is sent via event.");
			addOption ("amplifier", &amplifier, 1, SSI_REAL, "multiply value of change with amplifier.");

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
		ssi_size_t span;
		ssi_size_t update_ms;
		ssi_real_t amplifier;

	};

public:

	static const ssi_char_t *GetCreateName () { return "QRSHrvEventListener"; };
	static IObject *Create (const ssi_char_t *file) { return new QRSHrvEventListener (file); };
	~QRSHrvEventListener ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Listens to events from QRS-Detection and calculates heart rate variability."; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

protected:

	QRSHrvEventListener (const ssi_char_t *file = 0);
	QRSHrvEventListener::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;	

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _hrv_event;
	ssi_size_t _update_counter;
	ssi_size_t _update_ms;
	ssi_size_t _last_event_ms;
	ssi_real_t _last_hrv;

};

}

#endif
