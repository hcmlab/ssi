#include "BVPBeatEventStatisticalListener.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi{

	bool BVPBeatEventStatisticalListener::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
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
			ssi_real_t heart_rate = 0.0;

			if (ssi_size_t(fifo.size()) > 0){
				//time between the peeks of the latest two beats
				interbeat_interval = ssi_cast(ssi_real_t, (event_target.event_start_time + event_target.event_duration) - (fifo.back().event_start_time + fifo.back().event_duration));
				// 60 seconds / time between two beats = beats per minute
				heart_rate = 60000.0f / interbeat_interval;
			}

			event_target.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_INTERBEAT_INTERVAL_INDEX] = interbeat_interval;
			event_target.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_HEART_RATE_INDEX] = heart_rate;

			if (event_source->type == SSI_ETYPE_TUPLE){
				event_target.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX] = ssi_pcast(ssi_real_t, event_source->ptr)[SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX];
			}
			else if (event_source->type == SSI_ETYPE_MAP){
				event_target.event_features[SSI_BVP_EVENTLISTENER_CHANNEL_AMPLITUDE_INDEX] = ssi_pcast(ssi_event_map_t, event_source->ptr)[SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX].value;
			}

			fifo.push_back(event_target);
			//ssi_print("Fifo size: %i\n", ssi_size_t(fifo.size()));

		}
		return true;
	}

	ssi_char_t * BVPBeatEventStatisticalListener::ssi_log_name = "bvpeventstatisticallistener";


	ssi_real_t BVPBeatEventStatisticalListener::calculateResult(ssi_size_t channel_idx)
	{
		Lock lock(_mutex);
		// copy regarded channel/feature into an array
		ssi_real_t * arr = new ssi_real_t[ssi_size_t(fifo.size())];

		for (ssi_size_t i = 0; i < ssi_size_t(fifo.size()); i++){
			arr[i] = fifo[i].event_features[channel_idx];
		}

		ssi_real_t res = 0;

		// call statistical function
		switch (_options.statisticalFn)
		{
		case BVP_SUM:
			res = BaseSum(arr, ssi_size_t(fifo.size()));
			break;
		case BVP_MEAN:
			res = BaseMean(arr, ssi_size_t(fifo.size()));
			break;
		case BVP_VARIANCE:
			res = BaseVar(arr, ssi_size_t(fifo.size()));
			break;
		case BVP_STANDARD_DEVIATION:
			res = BaseStdD(arr, ssi_size_t(fifo.size()));
			break;
		default:
			ssi_err("Statistical function not supported!");
			break;
		}
		delete arr; arr = 0;
		return res;
	}

	void BVPBeatEventStatisticalListener::adaptToWindow()
	{
		if (_options.window == SSI_BVP_EVENTLISTENER_NO_WINDOW){
			return;
		}
		// remove elements from fifo outside of window
		while (!fifo.empty() && fifo[0].ct_time < getClockThreadTime() - _options.window){
			fifo.erase(fifo.begin());
		}
	}

	void BVPBeatEventStatisticalListener::clockUpdate()
	{
		adaptToWindow();
	}

	IChannel * BVPBeatEventStatisticalListener::getChannel(ssi_size_t index)
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

	ssi_time_t BVPBeatEventStatisticalListener::getSampleRate()
	{
		return _options.sr;
	}

}