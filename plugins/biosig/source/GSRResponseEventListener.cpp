#include "GSRResponseEventListener.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi{

	Mutex GSREventListener::_mutex;

	ssi_char_t * GSREventListener::ssi_log_name = "GSREventListener__";

	GSREventListener::GSREventListener(const ssi_char_t *file) : _file(0), _clock_thread_time(0l){

		for (ssi_size_t i = 0; i < SSI_GSR_EVENTLISTENER_N_CHANNELS; i++){
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

	GSREventListener::~GSREventListener() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		for (ssi_size_t i = 0; i < SSI_GSR_EVENTLISTENER_N_CHANNELS; i++){
			delete _channels[i]; _channels[i] = 0;
		}
	}

	ssi_size_t GSREventListener::getChannelSize(){
		return SSI_GSR_EVENTLISTENER_N_CHANNELS;
	}

	IChannel *GSREventListener::getChannel(ssi_size_t index){
		if (index >= SSI_GSR_EVENTLISTENER_N_CHANNELS) {
			ssi_wrn("requested index '%u' exceeds maximum number of channels '%u'", index, SSI_GSR_EVENTLISTENER_N_CHANNELS);
			return 0;
		}

		if (!_channels[index]) {

			switch (index) {
			case SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX:
				_channels[SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX] = new NumberOfResponsesChannel(_options.sr);
				break;
			case SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX:
				_channels[SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX] = new AmplitudeChannel(_options.sr);
				break;
			case SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX:
				_channels[SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX] = new RisingTimeChannel(_options.sr);
				break;
			case SSI_GSR_EVENTLISTENER_ENERGY_INDEX:
				_channels[SSI_GSR_EVENTLISTENER_ENERGY_INDEX] = new EnergyChannel(_options.sr);
				break;
			case SSI_GSR_EVENTLISTENER_POWER_INDEX:
				_channels[SSI_GSR_EVENTLISTENER_POWER_INDEX] = new PowerChannel(_options.sr);
				break;
			}
		}

		return _channels[index];
	}

	bool GSREventListener::setProvider(const ssi_char_t *name, IProvider *provider){
		ssi_size_t index = 0;

		if (strcmp(name, SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_PROVIDER_NAME) == 0) {
			index = SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX;
		}
		else if (strcmp(name, SSI_GSR_EVENTLISTENER_AMPLITUDE_PROVIDER_NAME) == 0) {
			index = SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX;
		}
		else if (strcmp(name, SSI_GSR_EVENTLISTENER_RISING_TIME_PROVIDER_NAME) == 0) {
			index = SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX;
		}
		else if (strcmp(name, SSI_GSR_EVENTLISTENER_POWER_PROVIDER_NAME) == 0) {
			index = SSI_GSR_EVENTLISTENER_POWER_INDEX;
		}
		else if (strcmp(name, SSI_GSR_EVENTLISTENER_ENERGY_PROVIDER_NAME) == 0) {
			index = SSI_GSR_EVENTLISTENER_ENERGY_INDEX;
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

	bool GSREventListener::connect(){
		setClockHz(_options.sr);
		return true;
	}
	bool GSREventListener::disconnect(){
		return true;
	}

	bool GSREventListener::start(){
		return ClockThread::start();
	}

	bool GSREventListener::stop(){
		return ClockThread::stop();
	}

	bool GSREventListener::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms){
		if (!events.getSize() || !n_new_events){
			return false;
		}

		/// Convert events to gsr_event_t and add them to fifo vector.
		events.reset();
		unsigned long ct_time;
		{
			Lock lock(_mutex);
			ct_time = _clock_thread_time;
		}

		for (ssi_size_t i = 0; i < n_new_events; i++){
			ssi_event_t * event_source = events.next();
			gsr_event_t event_target;
			
			event_target.ct_time = ct_time;

			if (event_source->type == SSI_ETYPE_TUPLE){
				event_target.event_features[SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX] = ssi_pcast(ssi_real_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_AMP_INDEX];
				event_target.event_features[SSI_GSR_EVENTLISTENER_ENERGY_INDEX] = ssi_pcast(ssi_real_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_ENERGY_INDEX];
				event_target.event_features[SSI_GSR_EVENTLISTENER_POWER_INDEX] = ssi_pcast(ssi_real_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_POWER_INDEX];
				event_target.event_features[SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX] = ssi_pcast(ssi_real_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_RISING_TIME_INDEX];
			}
			else if (event_source->type == SSI_ETYPE_MAP){
				event_target.event_features[SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX] = ssi_pcast(ssi_event_map_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_AMP_INDEX].value;
				event_target.event_features[SSI_GSR_EVENTLISTENER_ENERGY_INDEX] = ssi_pcast(ssi_event_map_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_ENERGY_INDEX].value;
				event_target.event_features[SSI_GSR_EVENTLISTENER_POWER_INDEX] = ssi_pcast(ssi_event_map_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_POWER_INDEX].value;
				event_target.event_features[SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX] = ssi_pcast(ssi_event_map_t, event_source->ptr)[SSI_GSR_EVENT_FEATURE_RISING_TIME_INDEX].value;
			}

			{
				Lock lock(_mutex);
				fifo.push_back(event_target);
			}
		}
		return true;
	}

	ssi_real_t GSREventListener::calculateResult(gsr_response_feature_statistical_function_t s_fn_type, ssi_size_t channel_idx){
		Lock lock(_mutex);

		if (s_fn_type == GSR_SUM && channel_idx == SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX){
			// = number of events
			return ssi_cast(ssi_real_t, ssi_size_t(fifo.size()));
		}

		if (s_fn_type == GSR_MEAN && channel_idx == SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX){
			// Mean of number of responses = number of events / number of events = 1
			return 1;
		}

		// copy regarded channel/feature into an array
		ssi_real_t * arr = new ssi_real_t[ssi_size_t(fifo.size())];

		for (ssi_size_t i = 0; i < ssi_size_t(fifo.size()); i++){
			arr[i] = fifo[i].event_features[channel_idx];
		}

		ssi_real_t res = 0;

		// call statistical function
		switch (s_fn_type)
		{
		case GSR_SUM:
			res = BaseSum(arr, ssi_size_t(fifo.size()));
			break;
		case GSR_MEAN:
			res = BaseMean(arr, ssi_size_t(fifo.size()));
			break;
		case GSR_VARIANCE:
			res = BaseVar(arr, ssi_size_t(fifo.size()));
			break;
		case GSR_STANDARD_DEVIATION:
			res = BaseStdD(arr, ssi_size_t(fifo.size()));
			break;
		default:
			break;
		}
		delete arr; arr = 0;
		return res;
	}

	ssi_real_t GSREventListener::BaseSum(ssi_real_t * arr, ssi_size_t arr_len){

		ssi_real_t res = 0;
		for (ssi_size_t i = 0; i < arr_len; i++){
			res += arr[i];
		}
		return res;
	}

	ssi_real_t GSREventListener::BaseMean(ssi_real_t * arr, ssi_size_t arr_len){
		if (arr_len == 0){
			return 0.0;
		}
		return BaseSum(arr, arr_len) / arr_len;
	}

	ssi_real_t GSREventListener::BaseStdD(ssi_real_t * arr, ssi_size_t arr_len){
		return sqrt(BaseVar(arr, arr_len));
	}

	ssi_real_t GSREventListener::BaseVar(ssi_real_t * arr, ssi_size_t arr_len){
		ssi_real_t res = 0;
		ssi_var(arr_len, 1, arr, &res);
		return res;
	}

	bool GSREventListener::hasChannel(ssi_size_t index){
		if (index < 0 || index >= SSI_GSR_EVENTLISTENER_N_CHANNELS){
			return false;
		}
		return _channels[index] != 0;
	}

	void GSREventListener::clock(){

		// Update the window
		{
			Lock lock(_mutex);
#if _WIN32||_WIN64
            _clock_thread_time = ::timeGetTime();
#else

            timespec ts;
            clock_gettime (CLOCK_MONOTONIC_RAW, &ts);



            _clock_thread_time= ts.tv_sec*1000+ (uint64_t)(ts.tv_nsec/1000000L);
#endif
			adaptToWindow();
		}

		// Write results to output stream
		if (_provider[SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX]){
			ssi_real_t tmp[1] = { calculateResult(_options.statisticalFn, SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX) };
			_provider[SSI_GSR_EVENTLISTENER_NUMBER_OF_RESPONSES_INDEX]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
		}
		if (_provider[SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX]){
			ssi_real_t tmp[1] = { calculateResult(_options.statisticalFn, SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX) };
			_provider[SSI_GSR_EVENTLISTENER_AMPLITUDE_INDEX]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
		}
		if (_provider[SSI_GSR_EVENTLISTENER_ENERGY_INDEX]){
			ssi_real_t tmp[1] = { calculateResult(_options.statisticalFn, SSI_GSR_EVENTLISTENER_ENERGY_INDEX) };
			_provider[SSI_GSR_EVENTLISTENER_ENERGY_INDEX]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
		}
		if (_provider[SSI_GSR_EVENTLISTENER_POWER_INDEX]){
			ssi_real_t tmp[1] = { calculateResult(_options.statisticalFn, SSI_GSR_EVENTLISTENER_POWER_INDEX) };
			_provider[SSI_GSR_EVENTLISTENER_POWER_INDEX]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
		}
		if (_provider[SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX]){
			ssi_real_t tmp[1] = { calculateResult(_options.statisticalFn, SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX) };
			_provider[SSI_GSR_EVENTLISTENER_RISING_TIME_INDEX]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
		}
	}

	void GSREventListener::adaptToWindow(){
		if (_options.window == SSI_GSR_EVENTLISTENER_NO_WINDOW){
			return;
		}
		// remove elements from fifo outside of window
		while (!fifo.empty() && fifo[0].ct_time < _clock_thread_time - _options.window){
			fifo.erase(fifo.begin());
		}
	}
}
