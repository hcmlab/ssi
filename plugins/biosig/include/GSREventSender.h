// GSREventSender.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/14
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

#ifndef SSI_EVENT_GSREVENTSENDER_H
#define SSI_EVENT_GSREVENTSENDER_H

#include "base/IConsumer.h"
#include "base/IEvents.h"
#include "base/ITransformer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "GSRFindPeaks.h"
#include "GSRFindSlopes.h"

namespace ssi {

class OverlapBuffer;

class GSREventSender : public IConsumer, public GSRFindPeaks::ICallback, public GSRFindSlopes::ICallback {

public:

	class Options : public OptionList {

	public:

		Options () 
			: winsize (15.0), peaknstd ( 0.25f), peakmind (1.5f), peakmaxd (4.0f), slopenstd (2.0f), slopemind (0.5f), slopemaxd (3.0f), tuple (false) {

			setSenderName ("gsr");
			setPeakEventName ("peak");			
			setSlopeEventName ("slope");			
			setDropEventName ("drop");			

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("epeakname", epeakname, SSI_MAX_CHAR, SSI_CHAR, "name of peak event");					
			addOption ("eslopename", eslopename, SSI_MAX_CHAR, SSI_CHAR, "name of slope event");	
			addOption ("edropname", edropname, SSI_MAX_CHAR, SSI_CHAR, "name of drop event");
			addOption ("winsize", &winsize, 1, SSI_REAL, "size in seconds of detrend window");		
			addOption ("peakmind", &peakmind, 1, SSI_REAL, "peak minimum duration in seconds");		
			addOption ("peakmaxd", &peakmaxd, 1, SSI_REAL, "peak maximum duration in seconds");		
			addOption ("peaknstd", &peaknstd, 1, SSI_REAL, "peak threshold = n * standard deviation");		
			addOption ("slopemind", &slopemind, 1, SSI_REAL, "slope minimum duration in seconds");		
			addOption ("slopemaxd", &slopemaxd, 1, SSI_REAL, "slope maximum duration in seconds");		
			addOption ("slopenstd", &slopenstd, 1, SSI_TIME, "slope threshold = n * standard deviation");	
			addOption ("tuple", &tuple, 1, SSI_BOOL, "send tuple events instead of array");	
		};

		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setPeakEventName (const ssi_char_t *epeakname) {
			if (epeakname) {
				ssi_strcpy (this->epeakname, epeakname);
			}
		}
		void setSlopeEventName (const ssi_char_t *eslopename) {
			if (eslopename) {
				ssi_strcpy (this->eslopename, eslopename);
			}
		}
		void setDropEventName (const ssi_char_t *edropname) {
			if (edropname) {
				ssi_strcpy (this->edropname, edropname);
			}
		}

		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t epeakname[SSI_MAX_CHAR];		
		ssi_char_t eslopename[SSI_MAX_CHAR];	
		ssi_char_t edropname[SSI_MAX_CHAR];	
		ssi_time_t winsize;
		ssi_real_t peaknstd;
		ssi_real_t peakmind;
		ssi_real_t peakmaxd;
		ssi_real_t slopenstd;
		ssi_real_t slopemind;
		ssi_real_t slopemaxd;
		bool tuple;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "GSREventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new GSREventSender (file); };
	~GSREventSender ();

	GSREventSender::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "sends gsr events"; };

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

	void peak (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area);
	void slope (ssi_time_t from, ssi_time_t to, ssi_real_t amplitude, ssi_real_t area, ssi_real_t slope);

protected:

	GSREventSender (const ssi_char_t *file = 0);
	GSREventSender::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _peak_event;
	ssi_event_t _slope_event;
	ssi_event_t _drop_event;
	ssi_size_t _etuple_amplitude_id;
	ssi_size_t _etuple_area_id;
	bool _send_etuple;
	
	ITransformer *_mvgvar;
	ssi_stream_t _var_stream;
	GSRFindPeaks *_findpeaks;	

	GSRFindSlopes *_findslopes;	
};

}

#endif
