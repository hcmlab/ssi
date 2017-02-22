#include "BVPBeatEventRawListener.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


namespace ssi{

	ssi_char_t * BVPBeatEventRawListener::ssi_log_name = "bvpeventrawlistener__";

	BVPBeatEventRawListener::BVPBeatEventRawListener(const ssi_char_t *file /*= 0*/) : BVPBeatEventListenerBase(file)
	{
			
		_last_event.ct_time = _current_event.ct_time = 0l;
		_last_event.event_duration = _current_event.event_duration = 0;
		_last_event.event_start_time = _current_event.event_start_time = 0;

		for (ssi_size_t i = 0; i < SSI_BVP_EVENTLISTENER_N_CHANNELS; i++)
		{
			_last_event.event_features[i] = _current_event.event_features[i] = 0.0;
		}
	}

	ssi_time_t BVPBeatEventRawListener::getSampleRate()
	{
		return _options.sr;
	}

	bool BVPBeatEventRawListener::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
	{
		if (!events.getSize() || !n_new_events){
			return false;
		}
		/// Convert events to bvp_event_t and add them to fifo vector.
		events.reset();
		ssi_event_t * event_source = 0;

		for (ssi_size_t i = 0; i < n_new_events; i++){

			Lock lock(_mutex);

			event_source = events.next();

			_current_event.ct_time = 0l; //unused
			_current_event.event_duration = event_source->dur;
			_current_event.event_start_time = event_source->time;

			ssi_real_t interbeat_interval = 0.0;
			ssi_real_t heart_rate = 0.0;

			//time between the peeks of the latest two beats
			interbeat_interval = ssi_cast(ssi_real_t, (_current_event.event_start_time + _current_event.event_duration) - (_last_event.event_start_time + _last_event.event_duration));
			// 60 seconds / time between two beats = beats per minute
			heart_rate = 60000.0f / interbeat_interval;
			
			_current_event.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX] = interbeat_interval;
			_current_event.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_INDEX] = heart_rate;

			if (event_source->type == SSI_ETYPE_TUPLE){
				_current_event.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX] = ssi_pcast(ssi_real_t, event_source->ptr)[SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX];
			}
			else if (event_source->type == SSI_ETYPE_MAP){
				_current_event.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX] = ssi_pcast(ssi_event_map_t, event_source->ptr)[SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX].value;
			}

			_last_event = _current_event;
		}
		return true;
	}

	ssi_real_t BVPBeatEventRawListener::calculateResult(ssi_size_t channel_idx)
	{
		Lock lock(_mutex);
		return _current_event.event_features[channel_idx];
	}


	IChannel * BVPBeatEventRawListener::getChannel(ssi_size_t index)
	{
	
		if (index >= SSI_BVP_EVENTLISTENER_N_CHANNELS) {
			ssi_wrn("requested index '%u' exceeds maximum number of channels '%u'", index, SSI_BVP_EVENTLISTENER_N_CHANNELS);
			return 0;
		}

		if (!_channels[index]) {

			switch (index) {
			case SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX:
				_channels[SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX] = new AmplitudeChannel(_options.sr);
				break;
			case SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_INDEX:
				_channels[SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_INDEX] = new HeartRateChannel(_options.sr);
				break;
			case SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX:
				_channels[SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX] = new InterbeatIntervalChannel(_options.sr);
				break;
			}
		}

		return _channels[index];
	}

}
