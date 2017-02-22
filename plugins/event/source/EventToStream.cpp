#include "../include/EventToStream.h"


namespace ssi{

	ssi_char_t *EventToStream::ssi_log_name = "EventToStream___";

	EventToStream::~EventToStream()
	{
		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file;
		}

		for (ssi_size_t i = 0; i < SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS; i++){
			delete _channels[i]; _channels[i] = 0;
		}
	}

	ssi_size_t EventToStream::getChannelSize()
	{
		return SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS;
	}


	bool EventToStream::setProvider(const ssi_char_t *name, IProvider *provider)
	{
		ssi_size_t index = 0;

		if (strcmp(name, SSI_SIGNAL_EVENTTOSTREAM_PROVIDER_NAME) == 0) {
			index = SSI_SIGNAL_EVENTTOSTREAM_CHANNEL_INDEX;
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

	bool EventToStream::connect()
	{
		_event_data_index_to_output = parseEventValueText(_options.eventValue);

		setClockHz(getSampleRate());
		return true;
	}

	bool EventToStream::disconnect()
	{
		return true;
	}

	bool EventToStream::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
	{
		if (!events.getSize() || !n_new_events){
			return false;
		}
		events.reset();
		ssi_event_t * event_source = 0;

		for (ssi_size_t i = 0; i < n_new_events; i++){

			//Go to the last new event
			event_source = events.next();

			if (_options.useWindow){
				event_with_ct_time event;
				event.clock_thread_time = getClockThreadTime();
				event.event = *event_source;
				events_in_window.push_back(event);
			}
		}

		Lock lock(_mutex);

		if (!_options.useWindow){
			_current_value = getEventDataOrInformation(*event_source, _event_data_index_to_output);
		}
		
		return true;
	}


	ssi_real_t EventToStream::getEventDataOrInformation(ssi_event_t event_source, ssi_int_t index)
	{
		//Use the tuple name
		if (index == SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_IND_NAME){
			if (event_source.type == SSI_ETYPE_MAP){
				ssi_size_t maxindex = event_source.tot / sizeof(ssi_event_map_t);
				for (ssi_size_t i = 0; i < maxindex; i++)
				{
					if (strcmp( Factory::GetString((ssi_pcast(ssi_event_map_t, event_source.ptr)[i].id)), _options.eventValue) == 0){
						return ssi_pcast(ssi_event_map_t, event_source.ptr)[i].value;
					}
				}
				ssi_wrn("EventToStream: Could not find an event value with the id %s", _options.eventValue);
			}
			else
			{
				ssi_wrn("EventToStream: Event is not in the tuple format. EventToStream expects a tuple id if a name instead of an index is given.");
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

	ssi_real_t EventToStream::getEventInformationData(ssi_event_t event_source, ssi_int_t index_to_return)
	{
		if (index_to_return == SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_NREVENTS){
			return 1;
		}
		else if (index_to_return == SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_STARTTIME){
			return ssi_cast(ssi_real_t, event_source.time);
		}
		else if (index_to_return == SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ENDTIME){
			return ssi_cast(ssi_real_t, event_source.time + event_source.dur);
		}
		else if (index_to_return == SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_DURATION){
			return ssi_cast(ssi_real_t, event_source.dur);
		}
		else if (index_to_return == SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ERROR){
			return 0;
		}
		return 0;
	}

	ssi_int_t EventToStream::parseEventValueText(ssi_char_t * eventValue)
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

		if (strcmp(string, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_STARTTIME_STR) == 0){
			return SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_STARTTIME;
		}
		else if (strcmp(string, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ENDTIME_STR) == 0){
			return SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_ENDTIME;
		}
		else if (strcmp(string, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_DURATION_STR) == 0){
			return SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_DURATION;
		}
		else if (strcmp(string, SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_NREVENTS_STR) == 0){
			return SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_NREVENTS;
		}
		else{
			return SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_IND_NAME;
		}
	}

	ssi_real_t EventToStream::getEventRawData(ssi_event_t event_source, ssi_int_t data_index)
	{
		if (event_source.type == SSI_ETYPE_TUPLE){
			//check the index is out of range
			ssi_int_t max_index = event_source.tot / sizeof(ssi_event_tuple_t) - 1;
			if (data_index > max_index){
				ssi_wrn("EventToStream: event index is out of range. Index: %i, Range [0 - %i]", data_index, max_index);
			}
			//get value
			return ssi_pcast(ssi_event_tuple_t, event_source.ptr)[data_index];
		}
		else if (event_source.type == SSI_ETYPE_MAP){
			//check the index is out of range
			ssi_int_t max_index = event_source.tot / sizeof(ssi_event_map_t) - 1;
			if (data_index > max_index){
				ssi_wrn("EventToStream: event index is out of range. Index: %i, Range [0 - %i]", data_index, max_index);
			}
			//get value
			return ssi_pcast(ssi_event_map_t, event_source.ptr)[data_index].value;
		}
		ssi_wrn("EventToStream: Its only possible to read numerical values");
		return 0;
	}

	bool EventToStream::start()
	{
		return ClockThread::start();
	}

	bool EventToStream::stop()
	{
		return ClockThread::stop();
	}

	EventToStream::EventToStream(const ssi_char_t *file) : _file(0), _clock_thread_time(0l), _event_data_index_to_output(SSI_SIGNAL_EVENTTOSTREAM_EVENT_VALUE_DURATION), _current_value(0)
	{
		for (ssi_size_t i = 0; i < SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS; i++){
			_channels[i] = 0;
			_provider[i] = 0;
		}


		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}
	}

	ssi::Mutex EventToStream::_mutex;

	void EventToStream::clock()
	{
		{
			Lock lock(_mutex);
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
		for (ssi_size_t i = 0; i < SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS; i++)
		{
			if (_provider[i])
			{
				Lock lock(_mutex);
				if (_options.useWindow){
					adaptToWindow();
					_current_value = calculateResult(_event_data_index_to_output);
				}

				tmp[0] = _current_value;
				_provider[i]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
			}
		}
	}

	unsigned long EventToStream::getClockThreadTime()
	{
		Lock Lock(_mutex);
		return _clock_thread_time;
	}


	bool EventToStream::hasChannel(ssi_size_t index)
	{
		if (index < 0 || index >= SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS){
			return false;
		}
		return _channels[index] != 0;
	}


	ssi_time_t EventToStream::getSampleRate()
	{
		return _options.sr;
	}

	IChannel * EventToStream::getChannel(ssi_size_t index)
	{

		if (index >= SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS) {
			ssi_wrn("requested index '%u' exceeds maximum number of channels '%u'", index, SSI_SIGNAL_EVENTTOSTREAM_NUMBER_OF_CHANNELS);
			return 0;
		}

		if (!_channels[index]) {

			switch (index) {
			case SSI_SIGNAL_EVENTTOSTREAM_CHANNEL_INDEX:
				_channels[SSI_SIGNAL_EVENTTOSTREAM_CHANNEL_INDEX] = new OutputChannel(_options.sr);
				break;
			}
		}

		return _channels[index];
	}

	ssi_real_t EventToStream::BaseSum(ssi_real_t * arr, ssi_size_t arr_len)
	{
		ssi_real_t res = 0;
		for (ssi_size_t i = 0; i < arr_len; i++){
			res += arr[i];
		}
		return res;
	}

	ssi_real_t EventToStream::BaseMean(ssi_real_t * arr, ssi_size_t arr_len)
	{
		if (arr_len == 0){
			return 0.0;
		}
		return BaseSum(arr, arr_len) / arr_len;
	}

	ssi_real_t EventToStream::BaseStdD(ssi_real_t * arr, ssi_size_t arr_len)
	{
		return sqrt(BaseVar(arr, arr_len));
	}

	ssi_real_t EventToStream::BaseVar(ssi_real_t * arr, ssi_size_t arr_len)
	{
		ssi_real_t res = 0;
		ssi_var(arr_len, 1, arr, &res);
		return res;
	}

	ssi_real_t EventToStream::BaseMin(ssi_real_t * arr, ssi_size_t arr_len)
	{
		ssi_real_t res = 0;
		if (arr_len > 0){
			res = arr[0];
			for (ssi_size_t i = 1; i < arr_len; i++){
				res = (res < arr[i] ? res : arr[i]);
			}
		}
		return res;
	}

	ssi_real_t EventToStream::BaseMax(ssi_real_t * arr, ssi_size_t arr_len)
	{
		ssi_real_t res = 0;
		if (arr_len > 0){
			res = arr[0];
			for (ssi_size_t i = 1; i < arr_len; i++){
				res = (res > arr[i] ? res : arr[i]);
			}
		}
		return res;
	}

	ssi_real_t EventToStream::calculateResult(ssi_int_t event_data_idx)
	{
		Lock lock(_mutex);
		// copy regarded channel/feature into an array
		ssi_real_t * arr = new ssi_real_t[events_in_window.size()];

		for (ssi_size_t i = 0; i < events_in_window.size(); i++){
			arr[i] = getEventDataOrInformation(events_in_window[i].event, event_data_idx);
		}

		ssi_real_t res = 0;

		// call statistical function
		switch (_options.statisticalFn)
		{
		case SUM:
			res = BaseSum(arr, ssi_size_t (events_in_window.size()));
			break;
		case MEAN:
			res = BaseMean(arr, ssi_size_t(events_in_window.size()));
			break;
		case VARIANCE:
			res = BaseVar(arr, ssi_size_t(events_in_window.size()));
			break;
		case STANDARD_DEVIATION:
			res = BaseStdD(arr, ssi_size_t(events_in_window.size()));
			break;
		case MIN:
			res = BaseMin(arr, ssi_size_t(events_in_window.size()));
			break;
		case MAX:
			res = BaseMax(arr, ssi_size_t(events_in_window.size()));
			break;
		default:
			ssi_wrn("EventToStream: Statistical function not supported!");
			break;
		}
		delete arr; arr = 0;
		return res;
	}

	void EventToStream::adaptToWindow()
	{
		if (_options.window == 0){
			return;
		}
		// remove elements from events_in_window outside of window
		while (!events_in_window.empty() && events_in_window[0].clock_thread_time < getClockThreadTime() - _options.window){
			events_in_window.erase(events_in_window.begin());
		}
	}

}
