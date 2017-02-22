#include "BVPBeatEventRMSSDListener.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {


	bool BVPBeatEventRMSSDListener::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
	{
		if (!events.getSize() || !n_new_events){
			return false;
		}

		Lock lock(_mutex);

		/// Convert events to bvp_event_t and add them to fifo vector.
		events.reset();
		unsigned long ct_time = getClockThreadTime();

		for (ssi_size_t i = 0; i < n_new_events; i++){
			ssi_event_t * event_source = events.next();
			bvp_event_t event_target;

			event_target.ct_time = ct_time;
			event_target.event_duration = event_source->dur;
			event_target.event_start_time = event_source->time;

			ssi_real_t interbeat_interval = 0.0;

			if (fifo.size() > 0){
				interbeat_interval = ssi_cast(ssi_real_t, (event_target.event_start_time + event_target.event_duration) - (fifo[fifo.size() - 1].event_start_time + fifo[fifo.size() - 1].event_duration));
			}
			

			event_target.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX] = interbeat_interval;

			
				fifo.push_back(event_target);
			
		}
		return true;
	}

	BVPBeatEventRMSSDListener::BVPBeatEventRMSSDListener(const ssi_char_t *file /*= 0*/)
	{
		for (ssi_size_t i = 0; i < SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS; i++){
			_channels[i] = 0;
			_providerRMSSD[i] = 0;
		}

		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}
	}

	ssi_char_t * BVPBeatEventRMSSDListener::ssi_log_name = "bvpeventrmssdlistener__";


	ssi_real_t BVPBeatEventRMSSDListener::calculateResult(ssi_size_t channel_idx)
	{
		Lock lock(_mutex);
		ssi_real_t res = 0;

		if (fifo.size() < 2){
			return 0;
		}

		bvp_event_t last_beat = fifo[0];
		bvp_event_t current_beat;

		//RMSSD = sqrt( ( (RR1-RR2)^2 + (RR2-RR3)^2 + ... + (RRn-1 - RRn)^2 ) / n)
		ssi_size_t used_beat_ct = 0;
		for (ssi_size_t i = 1; i < fifo.size(); i++){
			current_beat = fifo[i];
			// the very first found beat has a ibi of 0, because there was no previous beat to calculate it. This 0 ibi should be avoided.
			if (last_beat.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX] == 0){
				last_beat = current_beat;
				continue;
			}

			ssi_real_t diff_RR_RR = (last_beat.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX] - current_beat.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX]);

			res += diff_RR_RR * diff_RR_RR;

			last_beat = current_beat;
			used_beat_ct++;
		}

		if (used_beat_ct == 0){
			return 0;
		}
		res = sqrt(res / used_beat_ct);

	
		return res;
	}

	void BVPBeatEventRMSSDListener::clock()
	{
		{
			Lock lock(_mutex);
			_clock_thread_time = ::timeGetTime();
			clockUpdate();
		}
		ssi_real_t tmp[1];
		for (ssi_size_t i = 0; i < SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS; i++)
		{
			if (_providerRMSSD[i])
			{
				tmp[0] = calculateResult(i);
				_providerRMSSD[i]->provide(ssi_pcast(ssi_byte_t, tmp), 1);
			}
		}
	}


	bool BVPBeatEventRMSSDListener::setProvider(const ssi_char_t *name, IProvider *provider)
	{
		ssi_size_t index = 0;

		if (strcmp(name, SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_PROVIDER_NAME) == 0) {
			index = SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_INDEX;
		}
		else {
			ssi_wrn("channel with name '%s' does not exist", name);
			return false;
		}

		if (_providerRMSSD[index]) {
			ssi_wrn("provider with name '%s' was already set", name);
			return false;
		}

		IChannel *channel = getChannel(index);
		_providerRMSSD[index] = provider;
		_providerRMSSD[index]->init(channel);
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "provider '%s' set", channel->getName());

		return true;
	}

	void BVPBeatEventRMSSDListener::adaptToWindow()
	{
		if (_options.window == SSI_BVP_RMSSDEVENTLISTENER_NO_WINDOW){
			return;
		}
		// remove elements from fifo outside of window
		while (!fifo.empty() && fifo[0].ct_time < getClockThreadTime() - _options.window){
			fifo.erase(fifo.begin());
		}
	}

	void BVPBeatEventRMSSDListener::clockUpdate()
	{
		adaptToWindow();
	}

	IChannel * BVPBeatEventRMSSDListener::getChannel(ssi_size_t index)
	{
		if (index >= SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS) {
			ssi_wrn("requested index '%u' exceeds maximum number of channels '%u'", index, SSI_BVP_RMSSDEVENTLISTENER_N_CHANNELS);
			return 0;
		}

		if (!_channels[index]) {

			switch (index) {
			case SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_INDEX:
				_channels[SSI_BVP_RMSSDEVENTLISTENER_CHANNEL_INDEX] = new RMSSDChannel(_options.sr);
				break;
			}
		}

		return _channels[index];
	}

	ssi_time_t BVPBeatEventRMSSDListener::getSampleRate()
	{
		return _options.sr;
	}

}