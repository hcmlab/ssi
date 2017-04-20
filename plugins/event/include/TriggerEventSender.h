// TriggerEventSender.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/11/15
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

#ifndef SSI_EVENT_TRIGGEREVENTSENDER_H
#define SSI_EVENT_TRIGGEREVENTSENDER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "thread/Lock.h"

namespace ssi {

class TriggerEventSender : public IConsumer {

public:

	struct TRIGGER
	{
		enum List
		{
			GREATER = 0,
			GREATER_EQUAL,
			LESSER,
			LESSER_EQUAL,
			EQUAL,
			NOT_EQUAL,
			IN_RANGE,
			IN_RANGE_EQUAL,
			NOT_IN_RANGE,
			NOT_IN_RANGE_EQUAL,
		};
	};

	class Options : public OptionList {

	public:

		Options ()
			: triggerType(TRIGGER::GREATER),
			thresholdIn(0), 
			thresholdOut(0), 
			thresholdInEnd(0),
			thresholdOutEnd(0),
			hangInDuration(0),
			hangOutDuration(0),
			hangInSamples(0), 
			hangOutSamples(0), 
			hardThreshold(false),
			minDuration(0.0),
			incDuration(0.0),
			maxDuration(5.0),
			offsetInDuration(0),
			offsetOutDuration(0),
			sendStartEvent(false),
			skipMaxDuration(false),
			sendIncomplete(false),
			eventType(SSI_ETYPE_EMPTY) {

				setAddress("");;
				eventString[0] = '\0';

				SSI_OPTIONLIST_ADD_ADDRESS(address);

				addOption("triggerType", &triggerType, 1, SSI_INT, "threshold type (0='>',1='>=',2='<',3='<=',4='==',5='!=',6=']..[',7='[..]',8='![..]',9='!]..[')");
				addOption("thresholdIn", &thresholdIn, 1, SSI_DOUBLE, "threshold to trigger onset", false);
				addOption("thresholdOut", &thresholdOut, 1, SSI_DOUBLE, "threshold to trigger offset", false);
				addOption("thresholdInEnd", &thresholdInEnd, 1, SSI_DOUBLE, "end of interval to trigger onset (only applied if type is 6-9)", false);
				addOption("thresholdOutEnd", &thresholdOutEnd, 1, SSI_DOUBLE, "end of interval to trigger offset (only applied if type is 6-9)", false);
				addOption("hardThreshold", &hardThreshold, 1, SSI_BOOL, "consider all dimensions to tigger on/offset");				
				addOption("hangInDuration", &hangInDuration, 1, SSI_TIME, "minimum duration with only positive samples before an onset is triggered", false);
				addOption("hangOutDuration", &hangOutDuration, 1, SSI_TIME, "minimum duration with only negative smaples before an offset is triggered", false);
				addOption("hangInSamples", &hangInSamples, 1, SSI_SIZE, "like 'hangInDuration' but given in #samples (overides if > 0)", false);
				addOption("hangOutSamples", &hangOutSamples, 1, SSI_SIZE, "like 'hangOutDuration' but given in #samples (overides if > 0)", false);
				addOption("minDuration", &minDuration, 1, SSI_TIME, "if > 0 minimum duration in seconds (skip all shorter events)", false);
				addOption("incDuration", &incDuration, 1, SSI_TIME, "if > 0 incremental duration in seconds (incomplete events are sent when duration has passed and the current event is not yet finished)", false);
				addOption("maxDuration", &maxDuration, 1, SSI_TIME, "if > 0 maximum duration in seconds (otherwise the start time of an event is moved forward)", false);
				addOption("offsetInDuration", &offsetInDuration, 1, SSI_TIME, "lower offset in seconds (will be substracted from event start time)", false);
				addOption("offsetOutDuration", &offsetOutDuration, 1, SSI_TIME, "upper offset in seconds (will be added to event end time)", false);
				addOption("sendStartEvent", &sendStartEvent, 1, SSI_BOOL, "send a start event if onset is triggered", false);
				addOption("skipMaxDuration", &skipMaxDuration, 1, SSI_BOOL, "skip events that exceed the max duration", false);				
				addOption("eventType", &eventType, 1, SSI_INT, "event type (1=empty, 2=string [see 'eventString'], 3=map [see 'eventString', values are set to mean of input stream], 4=tuple [set to mean of input stream])");							
				addOption("eventString", eventString, SSI_MAX_CHAR, SSI_CHAR, "if event type is 'string' this will be the string that is sent, if event type is 'map' this will be key (if multiple keys separate by ';')");
		};

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}
		void setEventString(const ssi_char_t *eventString) {
			if (eventString) {
				ssi_strcpy(this->eventString, eventString);
			}
		}

		TRIGGER::List triggerType;
		double thresholdIn, thresholdOut;
		double thresholdInEnd, thresholdOutEnd;
		ssi_size_t hangInSamples, hangOutSamples;
		ssi_time_t hangInDuration, hangOutDuration;
		bool hardThreshold;
		ssi_time_t minDuration, incDuration, maxDuration;		
		ssi_time_t offsetInDuration, offsetOutDuration;
		bool sendStartEvent, skipMaxDuration, sendIncomplete;
		ssi_etype_t eventType;
		ssi_char_t eventString[SSI_MAX_CHAR];
		ssi_char_t address[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "TriggerEventSender"; };
	static IObject *Create (const ssi_char_t *file) { return new TriggerEventSender (file); };
	~TriggerEventSender ();

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Detects events in a trigger stream (e.g. when values in the stream exceed a certain threshold)."; };

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

	TriggerEventSender (const ssi_char_t *file = 0);
	TriggerEventSender::Options _options;
	ssi_char_t *_file;

	void readOptions();

	ssi_size_t _n_map_ids;
	ssi_size_t *_map_ids;

	TRIGGER::List _trigger;
	bool _trigger_on;
	ssi_time_t _trigger_start, _trigger_stop;
	ssi_size_t _hangover_in, _hangover_out, _counter_in, _counter_out;
	ssi_size_t _samples_inc_dur, _counter_inc_dur;
	ssi_time_t _min_dur, _inc_dur, _max_dur;
	ssi_time_t _loffset, _uoffset;
	bool _skip_on_max_dur, _eager, _send_incomplete;
	bool _hard_threshold;

	ssi_type_t _sample_type;
	ssi_time_t _sample_rate;
	ssi_size_t _sample_dim;
	ssi_size_t _sample_num;
	ssi_real_t *_sample_sum;

	IEventListener *_elistener;
	ssi_event_t _event;
	EventAddress _event_address;
	ssi_size_t _thres_string_id;
	ssi_size_t _thresout_string_id;

	bool update_h (IConsumer::info info);

	void *_thres_in;
	void *_thres_out;
	void *_thres_in_end;
	void *_thres_out_end;
	
	void setThresholds(double thres_in, double thres_out, double thres_in_end, double thres_out_end, ssi_type_t type);
	void sendEvent();

	template <class T>
	bool check_thres(void *ptr, void *thres, void *thres_end, TRIGGER::List trigger);
	template <class T>
	void update_sum(void *ptr);
};

}

#endif
