// exitstream.cpp
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


#include "../include/ExitStream.h"


namespace ssi{

	ssi_char_t *ExitStream::ssi_log_name = "exitstream";

	ExitStream::~ExitStream()
	{
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		for (ssi_size_t i = 0; i < EXITSTREAM_NUMBER_OF_CHANNELS; i++){
			delete _channels[i]; _channels[i] = 0;
		}
	}

	ssi_size_t ExitStream::getChannelSize()
	{
		return EXITSTREAM_NUMBER_OF_CHANNELS;
	}


	bool ExitStream::setProvider(const ssi_char_t *name, IProvider *provider)
	{
		ssi_size_t index = 0;

		if (strcmp(name, EXITSTREAM_PROVIDER_NAME) == 0) {
			index = EXITSTREAM_CHANNEL_INDEX;
		}
		else {
			ssi_wrn("channel with name '%s' does not exist", name);
			return false;
		}

		if (_provider[index]) {
			ssi_wrn("provider with name '%s' was already set", name);
			return false;
		}

		IChannel *channel = getChannel(index);
		_provider[index] = provider;
		_provider[index]->init(channel);
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "provider '%s' set", channel->getName());

		return true;
	}

	bool ExitStream::connect()
	{
		_event_data_index_to_output = parseEventValueText(_options.eventValue);

		setClockHz(getSampleRate());
		return true;
	}

	bool ExitStream::disconnect()
	{
		return true;
	}

	bool ExitStream::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
	{
		//ssi_print("\nEvents received:\t%d", n_new_events);

		/*if (!events.getSize() || !n_new_events){
			return false;
		}*/
		// events.reset();
		ssi_event_t * event_source = 0;
		std::deque<FLOAT> tmp;
		for (ssi_size_t i = 0; i < n_new_events; i++){
			//Go to the last new event
			event_source = events.next();
			// ssi_print("\t%d", event_source->time);
			// Lock lock(_mutex);
			_receive.push_back(getEventDataOrInformation(*event_source, _event_data_index_to_output));

			/*ssi_print("\nReceive: ");
			for (int n = 0; n < _data.size(); n++) {
				ssi_print("%.2f ", _data.at(n));
			}*/
		}

		for (ssi_size_t i = 0; i < n_new_events; i++) {
			Lock lock(_mutex);
			_store.push_back(_receive.at(i));
		}
		_receive.clear();

		// Lock lock(_mutex);

		// _current_value = getEventDataOrInformation(*event_source, _event_data_index_to_output);

		// _data.push_front(getEventDataOrInformation(*event_source, _event_data_index_to_output));
		
		return true;
	}


	ssi_real_t ExitStream::getEventDataOrInformation(ssi_event_t event_source, ssi_int_t index)
	{
		//Use the tuple name
		if (index == EXITSTREAM_EVENT_VALUE_IND_NAME){
			if (event_source.type == SSI_ETYPE_MAP){
				ssi_size_t maxindex = event_source.tot / sizeof(ssi_event_map_t);
				for (ssi_size_t i = 0; i < maxindex; i++)
				{
					if (strcmp( Factory::GetString((ssi_pcast(ssi_event_map_t, event_source.ptr)[i].id)), _options.eventValue) == 0){
						return ssi_pcast(ssi_event_map_t, event_source.ptr)[i].value;
					}
				}
				ssi_wrn("ExitStream: Could not find an event value with the id %s", _options.eventValue);
			}
			else
			{
				ssi_wrn("ExitStream: Event is not in the tuple format. ExitStream expects a tuple id if a name instead of an index is given.");
			}
		}
		//Use the data index
		else if (index >= 0){
				return getEventRawData(event_source, index);

		}
		else{
			//Use the event information
			return getEventInformationData(event_source, index);
		}
		return 0;
	}

	ssi_real_t ExitStream::getEventInformationData(ssi_event_t event_source, ssi_int_t index_to_return)
	{
		if (index_to_return == EXITSTREAM_EVENT_VALUE_NREVENTS){
			return 1;
		}
		else if (index_to_return == EXITSTREAM_EVENT_VALUE_STARTTIME){
			return ssi_cast(ssi_real_t, event_source.time);
		}
		else if (index_to_return == EXITSTREAM_EVENT_VALUE_ENDTIME){
			return ssi_cast(ssi_real_t, event_source.time + event_source.dur);
		}
		else if (index_to_return == EXITSTREAM_EVENT_VALUE_DURATION){
			return ssi_cast(ssi_real_t, event_source.dur);
		}
		else if (index_to_return == EXITSTREAM_EVENT_VALUE_ERROR){
			return 0;
		}
		return 0;
	}

	ssi_int_t ExitStream::parseEventValueText(ssi_char_t * eventValue)
	{
		ssi_char_t* string = ssi_strcpy(eventValue);
		ssi_int_t i = 0;

		if (sscanf(string, "%d", &i) != EOF && strlen(string) == (i/10) + 1)
		{
			return i;
		}

		//if (strlen(string) == 1){
		//	//maybe its only a number
		//	if (string[0] >= '0' && string[0] <= '9'){
		//		//convert ASCII to int
		//		return string[0] - '0';
		//	}
		//}

		if (strcmp(string, EXITSTREAM_EVENT_VALUE_STARTTIME_STR) == 0){
			return EXITSTREAM_EVENT_VALUE_STARTTIME;
		}
		else if (strcmp(string, EXITSTREAM_EVENT_VALUE_ENDTIME_STR) == 0){
			return EXITSTREAM_EVENT_VALUE_ENDTIME;
		}
		else if (strcmp(string, EXITSTREAM_EVENT_VALUE_DURATION_STR) == 0){
			return EXITSTREAM_EVENT_VALUE_DURATION;
		}
		else if (strcmp(string, EXITSTREAM_EVENT_VALUE_NREVENTS_STR) == 0){
			return EXITSTREAM_EVENT_VALUE_NREVENTS;
		}
		else{
			return EXITSTREAM_EVENT_VALUE_IND_NAME;
		}
	}

	ssi_real_t ExitStream::getEventRawData(ssi_event_t event_source, ssi_int_t data_index)
	{
		if (event_source.type == SSI_ETYPE_TUPLE){
			//check the index is out of range
			ssi_int_t max_index = event_source.tot / sizeof(ssi_event_tuple_t) - 1;
			if (data_index > max_index){
				ssi_wrn("ExitStream: event index is out of range. Index: %i, Range [0 - %i]", data_index, max_index);
			}
			//get value
			return ssi_pcast(ssi_event_tuple_t, event_source.ptr)[data_index];
		}
		else if (event_source.type == SSI_ETYPE_MAP){
			//check the index is out of range
			ssi_int_t max_index = event_source.tot / sizeof(ssi_event_map_t) - 1;
			if (data_index > max_index){
				ssi_wrn("ExitStream: event index is out of range. Index: %i, Range [0 - %i]", data_index, max_index);
			}
			//get value
			return ssi_pcast(ssi_event_map_t, event_source.ptr)[data_index].value;
		}
		ssi_wrn("ExitStream: Its only possible to read numerical values");
		return 0;
	}

	bool ExitStream::start()
	{
		return ClockThread::start();
	}

	bool ExitStream::stop()
	{
		return ClockThread::stop();
	}

	ExitStream::ExitStream(const ssi_char_t *file) : _file(0), _clock_thread_time(0l), _event_data_index_to_output(EXITSTREAM_EVENT_VALUE_DURATION), _current_value(0)
	{
		// _data.clear();

		for (ssi_size_t i = 0; i < EXITSTREAM_NUMBER_OF_CHANNELS; i++){
			_channels[i] = 0;
			_provider[i] = 0;
		}


		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	ssi::Mutex ExitStream::_mutex;

	void ExitStream::clock()
	{
		{
			// Lock lock(_mutex);
			#if _WIN32|_WIN64
			_clock_thread_time = ::timeGetTime();
			#else
			uint32_t ms=0;
			timespec ts;
			clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
			ms= ts.tv_sec*1000+ (uint32_t)(ts.tv_nsec/1000000);
			_clock_thread_time =ms;
			#endif
		}
		ssi_real_t tmp[1];
		for (ssi_size_t i = 0; i < EXITSTREAM_NUMBER_OF_CHANNELS; i++)
		{
			if (_provider[i])
			{
				// Lock lock(_mutex);

				// ssi_print("\nSize:\t%d", _data.size());

				// tmp[0] = _current_value;

				for (int i = 0; i < _store.size(); i++) {
					_send.push_front(_store.at(i));
				}

				{
					Lock lock(_mutex);
					_store.clear();
				}

				if (_send.size() > 0)
				{
					tmp[0] = _send.at(_send.size() - 1);
					// ssi_print("\nProvide: %.2f", tmp[0]);
					_send.pop_back();
					/*ssi_print("\nRemain: ");
					for (int n = 0; n < _send.size(); n++) {
						ssi_print("%.2f ", _send.at(n));
					}*/
				}
				_provider[i]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
			}
		}
	}

	unsigned long ExitStream::getClockThreadTime()
	{
		// Lock Lock(_mutex);
		
		return _clock_thread_time;
	}


	bool ExitStream::hasChannel(ssi_size_t index)
	{
		if (index < 0 || index >= EXITSTREAM_NUMBER_OF_CHANNELS){
			return false;
		}
		return _channels[index] != 0;
	}


	ssi_time_t ExitStream::getSampleRate()
	{
		return _options.sr;
	}

	IChannel * ExitStream::getChannel(ssi_size_t index)
	{

		if (index >= EXITSTREAM_NUMBER_OF_CHANNELS) {
			ssi_wrn("requested index '%u' exceeds maximum number of channels '%u'", index, EXITSTREAM_NUMBER_OF_CHANNELS);
			return 0;
		}

		if (!_channels[index]) {

			switch (index) {
			case EXITSTREAM_CHANNEL_INDEX:
				_channels[EXITSTREAM_CHANNEL_INDEX] = new OutputChannel(_options.sr);
				break;
			}
		}

		return _channels[index];
	}
}
