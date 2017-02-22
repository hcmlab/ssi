// author: Fabian Hertwig <fabian.hertwig@student.uni-augsburg.de>,
//		   Florian Obermayer <florian.obermayer@student.uni-augsburg.de>
// created: 2015/03/13
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************
// This class listens to any event and is able to output the values to a stream.
// 
//************************************************************************************************* 


#ifndef SSI_SIGNAL_EVENTTOSTREAM_H
#define SSI_SIGNAL_EVENTTOSTREAM_H

#pragma once

#define SSI_SIGNAL_EVENTTOSTREAM_PROVIDER_NAME "event output"
#define SSI_SIGNAL_EVENTTOSTREAM_CHANNEL_INDEX 0
#define SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS 1

#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_STARTTIME (-1)
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ENDTIME (-2)
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_DURATION (-3)
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ERROR (-4)
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_NREVENTS (-5)
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_IND_NAME (-6)

#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_STARTTIME_STR "starttime"
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ENDTIME_STR "endtime"
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_DURATION_STR "duration"
#define SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_NREVENTS_STR "number_of_events"




#include "base/ISensor.h"
#include "thread/ClockThread.h"
#include "ioput/option/OptionList.h"
#include "SSI_Tools.h"
#include "thread/Lock.h"
#include "base/Factory.h"

namespace ssi{
	class EventToStream : public ISensor, public ClockThread
	{
	public:

		// Possible statistical functions
		enum eventToStream_statistical_function_t{
			SUM = 0,
			MEAN = 1,
			VARIANCE = 2,
			STANDARD_DEVIATION = 3,
			MIN = 4,
			MAX = 5
		};


		struct event_with_ct_time{

			unsigned long clock_thread_time;
			ssi_event_t event;
		};

		class Options : public OptionList {

		public:
			Options() : sr(1.0), useWindow(false), window(0), statisticalFn(SUM){
				eventValue[0] = '\0';

				ssi_char_t description[SSI_MAX_CHAR];
				description[0] = '\0';
				ssi_strcpy(description, "Use \"");
				strcat(description, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_NREVENTS_STR);
				strcat(description, "\" to get the number of events in the defined Window. Use [");

				strcat(description, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_STARTTIME_STR);
				strcat(description, " | ");
				strcat(description, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ENDTIME_STR);
				strcat(description, " | ");
				strcat(description, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_DURATION_STR);
				strcat(description, " | (0 - n | name)] for the data index. Name only works if tuples are used.");

				addOption("sr", &sr, 1, SSI_TIME, "sampling rate of the output signal (Hz)");
				addOption("eventValue", eventValue, SSI_MAX_CHAR, SSI_CHAR, description);
				addOption("useWindow", &useWindow, 1, SSI_BOOL, "Use a window and a aggregation function (True) or output the raw data (False).");
				addOption("window", &window, 1, SSI_INT, "If useWindow is True, time span for the window in ms. 0 means no window (all prior events are used). This option competes with listener span.");
				addOption("statisticalFunction", &statisticalFn, 1, SSI_INT, "0-Sum, 1-Mean, 2-Variance, 3-StdDev, 4-Minimum, 5-Maximum");

			};

			ssi_time_t sr;
			ssi_char_t eventValue[SSI_MAX_CHAR];
			bool useWindow;
			ssi_size_t window;
			ssi_size_t statisticalFn;

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
			const ssi_char_t *getName() { return SSI_SIGNAL_EVENTTOSTREAM_PROVIDER_NAME; };
			const ssi_char_t *getInfo(){ return "continuously outputs the last value until a new event arrives."; };
			ssi_stream_t getStream() { return stream; };

		protected:
			ssi_stream_t stream;
		};


		~EventToStream();

		static ssi_char_t *ssi_log_name;



		static const ssi_char_t *GetCreateName() { return "EventToStream"; }
		virtual const ssi_char_t * getName()override{ return GetCreateName(); }
		static IObject *Create(const ssi_char_t *file) { return new EventToStream(file); };
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

		ssi_real_t calculateResult(ssi_int_t channel_idx);


		void listen_flush(){}

		IChannel * getChannel(ssi_size_t index);

	protected:
		EventToStream(const ssi_char_t *file = 0);
		EventToStream::Options _options;
		ssi_int_t _event_data_index_to_output;
		ssi_real_t _current_value;

		ssi_char_t *_file;

		IProvider * _provider[SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS];
		IChannel * _channels[SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS];

		static Mutex _mutex;

		/// Implements the clock thread function.
		void clock();

		unsigned long getClockThreadTime();

		/// checks if a channel is active
		bool hasChannel(ssi_size_t index);

		ssi_time_t getSampleRate();
		unsigned long  _clock_thread_time;

		std::vector<event_with_ct_time> events_in_window;



		/// Statistical functions
		ssi_real_t BaseSum(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseMean(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseStdD(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseVar(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseMin(ssi_real_t * arr, ssi_size_t arr_len);
		ssi_real_t BaseMax(ssi_real_t * arr, ssi_size_t arr_len);

		void adaptToWindow();
	private:
		ssi_int_t parseEventValueText(ssi_char_t * eventValue);

		ssi_real_t getEventRawData(ssi_event_t event_source, ssi_int_t data_index);
		ssi_real_t getEventInformationData(ssi_event_t event_source, ssi_int_t index_to_return);

	};


}
#endif //SSI_SIGNAL_EVENTTOSTREAM_H


