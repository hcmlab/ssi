#include "BVPBeatEventSender.h"
#include <fstream>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi{

	BVPBeatEventSender::~BVPBeatEventSender()
	{
		if (_elistener){
			ssi_event_destroy(_event);
		}

		if (_file) {
			OptionList::SaveXML(_file, _options);
			delete[] _file; _file = 0;
		}
	}

	BVPBeatEventSender::BVPBeatEventSender(const ssi_char_t *file /*= 0*/):
		_file(0),
		_elistener(0),
		_send_as_tuple(false),
		_etuple_minimum_id(0),
		_etuple_maximum_id(0),
		_etuple_amplitude_id(0),
		_etuple_energy_id(0),
		_etuple_power_id(0)

	{
		if (file) {
			if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);
		}
	}

	void BVPBeatEventSender::handleBeat(bvp_beat_t beat)
	{
		if (_options.beep)
		{
			Beep(1000, 250);
		}

		if (!_elistener) {
			return;
		}

		_event.dur = ssi_cast(ssi_size_t, beat.duration * 1000);
		_event.time = ssi_cast(ssi_size_t, beat.start_time * 1000);
		_event.state = SSI_ESTATE_COMPLETED;

		if (_send_as_tuple){
			ssi_event_map_t * ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
			ptr[SSI_BVP_EVENT_FEATURE_MINIMUM_INDEX].value = beat.minimum;
			ptr[SSI_BVP_EVENT_FEATURE_MAXIMUM_INDEX].value = beat.maximum;
			ptr[SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX].value = beat.maximum - beat.minimum;
			ptr[SSI_BVP_EVENT_FEATURE_ENERGY_INDEX].value = beat.energy;
			ptr[SSI_BVP_EVENT_FEATURE_POWER_INDEX].value = beat.power;

#ifdef SSI_BVP_DETECTION_DEBUG_THRESHOLDS
			ptr[SSI_BVP_EVENT_FEATURE_DEBUG_MEAN_INDEX].value = beat.mean;
			ptr[SSI_BVP_EVENT_FEATURE_DEBUG_LOWER_INDEX].value = beat.lower;
			ptr[SSI_BVP_EVENT_FEATURE_DEBUG_HIGHER_INDEX].value = beat.higher;
#endif // SSI_BVP_DETECTION_DEBUG_THRESHOLDS
		}
		else{
			ssi_real_t * ptr = ssi_pcast(ssi_real_t, _event.ptr);
			ptr[SSI_BVP_EVENT_FEATURE_MINIMUM_INDEX] = beat.minimum;
			ptr[SSI_BVP_EVENT_FEATURE_MAXIMUM_INDEX] = beat.maximum;
			ptr[SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX] = beat.maximum - beat.minimum;
			ptr[SSI_BVP_EVENT_FEATURE_ENERGY_INDEX] = beat.energy;
			ptr[SSI_BVP_EVENT_FEATURE_POWER_INDEX] = beat.power;
#ifdef SSI_BVP_DETECTION_DEBUG_THRESHOLDS
			ptr[SSI_BVP_EVENT_FEATURE_DEBUG_MEAN_INDEX] = beat.mean;
			ptr[SSI_BVP_EVENT_FEATURE_DEBUG_LOWER_INDEX] = beat.lower;
			ptr[SSI_BVP_EVENT_FEATURE_DEBUG_HIGHER_INDEX] = beat.higher;
#endif // SSI_BVP_DETECTION_DEBUG_THRESHOLDS
		}
		_elistener->update(_event);
	}

	void BVPBeatEventSender::consume_enter(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{
		_bvp_beat_detection = new BVPBeatDetection(_options.mean_window, _options.print);
	}

	void BVPBeatEventSender::consume(IConsumer::info consume_info, ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{
		for (ssi_size_t i = 0; i < stream_in[0].num; i++){
			_bvp_beat_detection->findBeat(stream_in[0].ptr, stream_in[0].sr, i, consume_info.time, this);
		}
	}

	void BVPBeatEventSender::consume_flush(ssi_size_t stream_in_num, ssi_stream_t stream_in[])
	{
		delete _bvp_beat_detection; _bvp_beat_detection = 0;
	}

	bool BVPBeatEventSender::setEventListener(IEventListener *listener)
	{
		_elistener = listener;

		_send_as_tuple = _options.tuple;
		ssi_size_t data_ct = SSI_BVP_EVENT_FEATURE_N_VALUES;

		if (_send_as_tuple){
			_etuple_minimum_id = Factory::AddString("minimum");
			_etuple_maximum_id = Factory::AddString("maximum");
			_etuple_amplitude_id = Factory::AddString("amplitude");
			_etuple_energy_id = Factory::AddString("energy");
			_etuple_power_id = Factory::AddString("power");

			ssi_event_init(_event, SSI_ETYPE_MAP);

			ssi_event_adjust(_event, data_ct * sizeof(ssi_event_map_t));
			ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
			ptr[SSI_BVP_EVENT_FEATURE_MINIMUM_INDEX].id = _etuple_minimum_id;
			ptr[SSI_BVP_EVENT_FEATURE_MAXIMUM_INDEX].id = _etuple_maximum_id;
			ptr[SSI_BVP_EVENT_FEATURE_AMPLITUDE_INDEX].id = _etuple_amplitude_id;
			ptr[SSI_BVP_EVENT_FEATURE_ENERGY_INDEX].id = _etuple_energy_id;
			ptr[SSI_BVP_EVENT_FEATURE_POWER_INDEX].id = _etuple_power_id;
		}
		else{
			ssi_event_init(_event, SSI_ETYPE_TUPLE);
			ssi_event_adjust(_event, data_ct * sizeof(ssi_real_t));
		}

		_event.sender_id = Factory::AddString(_options.s_name);
		_event.event_id = Factory::AddString(_options.e_name);

		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.s_name);
		_event_address.setEvents(_options.e_name);

		return true;
	}

	char BVPBeatEventSender::ssi_log_name[] = "bvpeventsender__";

}