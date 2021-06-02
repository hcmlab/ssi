// exitstream.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2021/03/08
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


#ifndef EXITSTREAM_H
#define EXITSTREAM_H

#pragma once

#define EXITSTREAM_PROVIDER_NAME "event output"
#define EXITSTREAM_CHANNEL_INDEX 0
#define EXITSTREAM_NUMBER_OF_CHANNELS 1

#define EXITSTREAM_EVENT_VALUE_STARTTIME (-1)
#define EXITSTREAM_EVENT_VALUE_ENDTIME (-2)
#define EXITSTREAM_EVENT_VALUE_DURATION (-3)
#define EXITSTREAM_EVENT_VALUE_ERROR (-4)
#define EXITSTREAM_EVENT_VALUE_NREVENTS (-5)
#define EXITSTREAM_EVENT_VALUE_IND_NAME (-6)

#define EXITSTREAM_EVENT_VALUE_STARTTIME_STR "starttime"
#define EXITSTREAM_EVENT_VALUE_ENDTIME_STR "endtime"
#define EXITSTREAM_EVENT_VALUE_DURATION_STR "duration"
#define EXITSTREAM_EVENT_VALUE_NREVENTS_STR "number_of_events"

#include "base/ISensor.h"
#include "thread/ClockThread.h"
#include "ioput/option/OptionList.h"
#include "SSI_Tools.h"
#include "thread/Lock.h"
#include "base/Factory.h"

#include <deque>

namespace ssi{
	class ExitStream : public ISensor, public ClockThread
	{
	public:

		struct event_with_ct_time{

			unsigned long clock_thread_time;
			ssi_event_t event;
		};

		class Options : public OptionList {

		public:
			Options() : sr(1.0) {

				eventValue[0] = '\0';

				/*ssi_char_t description[SSI_MAX_CHAR];
				description[0] = '\0';
				ssi_strcpy(description, "Use \"");
				strcat(description, EXITSTREAM_EVENT_VALUE_NREVENTS_STR);
				strcat(description, "\" to get the number of events in the defined Window. Use [");

				strcat(description, EXITSTREAM_EVENT_VALUE_STARTTIME_STR);
				strcat(description, " | ");
				strcat(description, EXITSTREAM_EVENT_VALUE_ENDTIME_STR);
				strcat(description, " | ");
				strcat(description, EXITSTREAM_EVENT_VALUE_DURATION_STR);
				strcat(description, " | (0 - n | name)] for the data index. Name only works if tuples are used.");*/

				addOption("sr", &sr, 1, SSI_TIME, "sampling rate of the output signal (Hz)");
				addOption("eventValue", eventValue, SSI_MAX_CHAR, SSI_CHAR, /*description*/ "...");

			};

			ssi_time_t sr;
			ssi_char_t eventValue[SSI_MAX_CHAR];

			void setEventValue(const ssi_char_t *value) {
				ssi_strcpy(this->eventValue, value);
			}

		};


		class OutputChannel : public IChannel {

		public:
			OutputChannel(ssi_time_t sr) {
				ssi_stream_init(stream, 0, 1, sizeof(ssi_real_t), SSI_REAL, sr);
			}
			~OutputChannel() {
				ssi_stream_destroy(stream);
			}
			const ssi_char_t *getName() { return EXITSTREAM_PROVIDER_NAME; };
			const ssi_char_t *getInfo(){ return "continuously outputs the last value until a new event arrives."; };
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};


		~ExitStream();

		static ssi_char_t *ssi_log_name;



		static const ssi_char_t *GetCreateName() { return "ExitStream"; }
		virtual const ssi_char_t * getName()override{ return GetCreateName(); }
		static IObject *Create(const ssi_char_t *file) { return new ExitStream(file); };
		const ssi_char_t *getInfo() {
			return "Listens to the set events and continuously outputs the last value until a new event arrives.";
		}

		Options * getOptions() override
		{
			return &_options;
		}

		ssi_size_t getChannelSize();
		bool setProvider(const ssi_char_t *name, IProvider *provider);
		bool connect();
		bool disconnect();

		bool start();
		bool stop();


		void listen_enter(){}
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);

		ssi_real_t getEventDataOrInformation(ssi_event_t event_source, ssi_int_t index);

		void listen_flush(){}

		IChannel * getChannel(ssi_size_t index);

	protected:
		ExitStream(const ssi_char_t *file = 0);
		ExitStream::Options _options;



		ssi_int_t _event_data_index_to_output;
		ssi_real_t _current_value;

		ssi_char_t *_file;

		IProvider * _provider[EXITSTREAM_NUMBER_OF_CHANNELS];
		IChannel * _channels[EXITSTREAM_NUMBER_OF_CHANNELS];

		static Mutex _mutex;

		/// Implements the clock thread function.
		void clock();

		unsigned long getClockThreadTime();

		/// checks if a channel is active
		bool hasChannel(ssi_size_t index);

		ssi_time_t getSampleRate();
		unsigned long  _clock_thread_time;

		std::vector<event_with_ct_time> events_in_window;

		std::deque<FLOAT> _receive;
		std::deque<FLOAT> _store;
		std::deque<FLOAT> _send;

	private:
		ssi_int_t parseEventValueText(ssi_char_t * eventValue);

		ssi_real_t getEventRawData(ssi_event_t event_source, ssi_int_t data_index);
		ssi_real_t getEventInformationData(ssi_event_t event_source, ssi_int_t index_to_return);

	};


}
#endif //EXITSTREAM_H


