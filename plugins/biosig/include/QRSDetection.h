// QRSDetection.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/04/08
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

#ifndef SSI_BIOSIG_QRSDETECTION_H
#define SSI_BIOSIG_QRSDETECTION_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class QRSDetection : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options () 
			: depthRR (8), lowerLimitRR (0.5f), upperLimitRR (1.25f), sendEvent(false), tuple (false) {

			setSenderName ("qrs");
			setEventName ("rspike");			

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");				
			addOption ("depthRR", &depthRR, 1, SSI_INT, "Depth of RR-interval consideration.");	
			addOption ("lowerLimitRR", &lowerLimitRR, 1, SSI_REAL, "% of demanded RR-interval length for next R-spike (avoids recognition of T-waves).");	
			addOption ("upperLimitRR", &upperLimitRR, 1, SSI_REAL, "% of demanded RR-interval length for expecting next R-spike (temporarily lowers threshold for R-spike detection).");	
			addOption ("sendEvent", &sendEvent, 1, SSI_BOOL, "send an event to the board whenever a R-spike is detected");	
			addOption ("tuple", &tuple, 1, SSI_BOOL, "send tuple events instead of array");	
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
		ssi_size_t depthRR;
		ssi_real_t lowerLimitRR;
		ssi_real_t upperLimitRR;
		bool sendEvent;
		bool tuple;

	};

public:

	static const ssi_char_t *GetCreateName () { return "QRSDetection"; };
	static IObject *Create (const ssi_char_t *file) { return new QRSDetection (file); };
	~QRSDetection ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "..."; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		return 1;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type '%s' not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}
	void sendEvent_h(ITransformer::info &info, ssi_time_t sample_rate, ssi_size_t frame_count, ssi_size_t sample_number);

protected:

	QRSDetection (const ssi_char_t *file = 0);
	QRSDetection::Options _options;
	ssi_char_t *_file;

	ITransformer *_bandpass;
	ssi_stream_t _bandpass_stream;
	ITransformer *_diff;
	ssi_stream_t _diff_stream;
	ITransformer *_pre;
	ssi_stream_t _pre_stream;
	ITransformer *_pre_low;
	ssi_stream_t _pre_low_stream;

	IEventListener *_listener;
	EventAddress _event_address;
	ssi_event_t _r_event;
	bool _send_etuple;

	ssi_size_t _frame_count;

	ssi_real_t _signal_level;
	ssi_real_t _noise_level;

	ssi_real_t _thres1;
	ssi_real_t _thres2;

	bool _pulsed;
	ssi_size_t _n_R;
	ssi_size_t _samples_since_last_R;
	ssi_size_t _last_R;

	ssi_size_t _head_RR;
	ssi_size_t *_history_RR;
	ssi_size_t _sum_RR;
	ssi_real_t _average_RR;

	ssi_real_t _low_limit_RR;
	ssi_real_t _high_limit_RR;

	bool _rising;

	bool _first_call;

};

}

#endif
