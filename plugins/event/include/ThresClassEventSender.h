// ThresClassEventSender.h
// author: Ionut Damianr <damian@hcm-lab.de>
// created: 2013/08/21
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

#ifndef SSI_EVENT_THRESCLASSEVENTSENDER_H
#define SSI_EVENT_THRESCLASSEVENTSENDER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class ThresClassEventSender : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: minDiff (0.1f), mean(false) {

				setAddress("");
				setSender ("tcsender");
				setEvent ("class");

				setClasses ("low, medium, high");
				setThresholds ("0.1, 0.3, 0.8");

				SSI_OPTIONLIST_ADD_ADDRESS(address);

				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");
				addOption ("classes", classes, SSI_MAX_CHAR, SSI_CHAR, "names of the classes event (e.g. low,medium,high)");	
				addOption ("thres", &thres, SSI_MAX_CHAR, SSI_CHAR, "thresholds (e.g. 0.1,0.3,0.8)");
				addOption ("minDiff", &minDiff, 1, SSI_FLOAT, "minimum difference to previous value");
				addOption ("mean", &mean, 1, SSI_BOOL, "classify based on mean value of entire frame");
		};

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}

		void setSender (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEvent (const ssi_char_t *ename) {			
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}

		void setClasses (const ssi_char_t *classes) {			
			if (classes) {
				ssi_strcpy (this->classes, classes);
			}
		}

		void setThresholds (const ssi_char_t *thres) {			
			if (thres) {
				ssi_strcpy (this->thres, thres);
			}
		}

		void setThresholds (ssi_size_t n_thres, ssi_real_t *thres) {
			thres[0] = '\0';
			if (n_thres > 0) {
				ssi_char_t c[SSI_MAX_CHAR];
				ssi_sprint (c, "%f", thres[0]);
				strcat (classes, c);
				for (ssi_size_t i = 1; i < n_thres; i++) {
					ssi_sprint (c, ",%f", thres[i]);
					strcat (classes, c);
				}
			}
		}

		bool mean;
		ssi_real_t minDiff;
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
		ssi_char_t classes[SSI_MAX_CHAR];	
		ssi_char_t thres[SSI_MAX_CHAR];	
	};

public:

	static const ssi_char_t *GetCreateName () { return "ThresClassEventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new ThresClassEventSender (file); };
	~ThresClassEventSender ();

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Classifies stream using thresholds."; };

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

protected:

	ThresClassEventSender (const ssi_char_t *file = 0);
	
	int classify(ssi_real_t value, ssi_real_t* thresholds, ssi_size_t n_thresholds);
	bool handleEvent(IEventListener *listener, ssi_event_t* ev, const ssi_char_t* class_name, ssi_time_t time);

	ThresClassEventSender::Options _options;
	ssi_char_t *_file;
	
	IEventListener *_elistener;
	EventAddress _event_address;
	ssi_event_t _event;
	
	ssi_real_t *_thres;
	ssi_size_t _num_classes;
	const ssi_char_t **_classes;

	int _lastClass;
	ssi_real_t _lastValue;

	ssi_real_t *parseFloats (const ssi_char_t *str, ssi_size_t &n_indices, bool sort = false, const ssi_char_t *delims = " ,");
	const ssi_char_t **parseStrings (const ssi_char_t *str, ssi_size_t &n_indices, const ssi_char_t *delims = " ,");
};

}

#endif
