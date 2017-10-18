#include "BVPBeatEventListenerBase.h"

namespace ssi{

	BVPBeatEventListenerBase::~BVPBeatEventListenerBase()
	{
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		for (ssi_size_t i = 0; i < SSI_BVP_EVENTLISTENER_N_CHANNELS; i++){
			delete _channels[i]; _channels[i] = 0;
		}
	}

	ssi_size_t BVPBeatEventListenerBase::getChannelSize()
	{
		return SSI_BVP_EVENTLISTENER_N_CHANNELS;
	}


	bool BVPBeatEventListenerBase::setProvider(const ssi_char_t *name, IProvider *provider)
	{
		ssi_size_t index = 0;

		if (strcmp(name, SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_PROVIDER_NAME) == 0) {
			index = SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX;
		}
		else if (strcmp(name, SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_PROVIDER_NAME) == 0) {
			index = SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_INDEX;
		}
		else if (strcmp(name, SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_PROVIDER_NAME) == 0) {
			index = SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX;
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

	bool BVPBeatEventListenerBase::connect()
	{
		setClockHz(getSampleRate());
		return true;
	}

	bool BVPBeatEventListenerBase::disconnect()
	{
		return true;
	}

	bool BVPBeatEventListenerBase::start()
	{
		return ClockThread::start();
	}

	bool BVPBeatEventListenerBase::stop()
	{
		return ClockThread::stop();
	}

	ssi_real_t BVPBeatEventListenerBase::BaseSum(ssi_real_t * arr, ssi_size_t arr_len)
	{
		ssi_real_t res = 0;
		for (ssi_size_t i = 0; i < arr_len; i++){
			res += arr[i];
		}
		return res;
	}

	ssi_real_t BVPBeatEventListenerBase::BaseMean(ssi_real_t * arr, ssi_size_t arr_len)
	{
		if (arr_len == 0){
			return 0.0;
		}
		return BaseSum(arr, arr_len) / arr_len;
	}

	ssi_real_t BVPBeatEventListenerBase::BaseStdD(ssi_real_t * arr, ssi_size_t arr_len)
	{
		return sqrt(BaseVar(arr, arr_len));
	}

	ssi_real_t BVPBeatEventListenerBase::BaseVar(ssi_real_t * arr, ssi_size_t arr_len)
	{
		ssi_real_t res = 0;
		ssi_var(arr_len, 1, arr, &res);
		return res;
	}

	BVPBeatEventListenerBase::BVPBeatEventListenerBase(const ssi_char_t *file) : _file(0), _clock_thread_time(0l)
	{
		for (ssi_size_t i = 0; i < SSI_BVP_EVENTLISTENER_N_CHANNELS; i++){
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

	ssi::Mutex BVPBeatEventListenerBase::_mutex;

	void BVPBeatEventListenerBase::clock()
	{
		{
			Lock lock(_mutex);
#if _WIN32||_WIN64
			_clock_thread_time = ::timeGetTime();
#else

            timespec ts;
            clock_gettime (CLOCK_MONOTONIC_RAW, &ts);



            _clock_thread_time= ts.tv_sec*1000+ (uint64_t)(ts.tv_nsec/1000000L);
#endif
			clockUpdate();
		}
		ssi_real_t tmp[1];
		for (ssi_size_t i = 0; i < SSI_BVP_EVENTLISTENER_N_CHANNELS; i++)
		{
			if (_provider[i])
			{
				tmp[0] = calculateResult(i);
				_provider[i]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
			}
		}
	}

	unsigned long BVPBeatEventListenerBase::getClockThreadTime()
	{
		Lock Lock(_mutex);
		return _clock_thread_time;
	}


	bool BVPBeatEventListenerBase::hasChannel(ssi_size_t index)
	{
		if (index < 0 || index >= SSI_BVP_EVENTLISTENER_N_CHANNELS){
			return false;
		}
		return _channels[index] != 0;
	}
}
