// TriggerEventSender.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/26
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "TriggerEventSender.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


namespace ssi {

	ssi_char_t *TriggerEventSender::ssi_log_name = "trigger___";

	TriggerEventSender::TriggerEventSender(const ssi_char_t *file)
		: _trigger(TRIGGER::GREATER),
		_n_map_ids(0),
		_map_ids(0),
		_eager(false),
		_send_incomplete(false),
		_skip_on_max_dur(false),
		_hard_threshold(false),
		_hangover_in(0),
		_hangover_out(0),
		_samples_inc_dur(0),
		_counter_inc_dur(0),
		_max_dur(0),
		_inc_dur(0),
		_min_dur(0),
		_loffset(0),
		_uoffset(0),
		_file(0),
		_elistener(0),
		_sample_rate(1.0),
		_sample_type(SSI_FLOAT),
		_sample_dim(0),
		_sample_num(0),
		_sample_sum(0),
		_thres_in(0),
		_thres_out(0),
		_thres_in_end(0),
		_thres_out_end(0),
		ssi_log_level(SSI_LOG_LEVEL_DEFAULT)
	{

		if (file)
		{
			if (!OptionList::LoadXML(file, &_options))
			{
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event);
	}

	TriggerEventSender::~TriggerEventSender()
	{
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
		delete[] _map_ids;
	}

	bool TriggerEventSender::setEventListener(IEventListener *listener)
	{
		ssi_event_init(_event, _options.eventType);

		if (_options.eventType == SSI_ETYPE_STRING)
		{
			ssi_event_adjust(_event, ssi_strlen(_options.eventString) + 1);
			ssi_strcpy(_event.ptr, _options.eventString);
		}
		else if (_options.eventType == SSI_ETYPE_MAP)
		{
			delete[] _map_ids; _map_ids = 0;

			_n_map_ids = ssi_split_string_count(_options.eventString, ';');
			if (_n_map_ids > 0)
			{
				ssi_char_t **ids = new ssi_char_t *[_n_map_ids];
				_map_ids = new ssi_size_t[_n_map_ids];
				ssi_split_string(_n_map_ids, ids, _options.eventString, ';');
				for (ssi_size_t i = 0; i < _n_map_ids; i++)
				{
					_map_ids[i] = Factory::AddString(ids[i]);
					delete[] ids[i];
				}
				delete[] ids;
			}
		}

		_elistener = listener;

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

		return true;
	}

	void TriggerEventSender::readOptions()
	{
		_options.lock();

		_hangover_in = _options.hangInSamples > 0 ? _options.hangInSamples : ssi_size_t(_options.hangInDuration * _sample_rate + 0.5);
		_hangover_out = _options.hangOutSamples > 0 ? _options.hangOutSamples : ssi_size_t(_options.hangOutDuration * _sample_rate + 0.5);
		_counter_in = _hangover_in;
		_counter_out = _hangover_out;

		_min_dur = _options.minDuration;
		_inc_dur = _options.incDuration;
		_max_dur = _options.maxDuration;
		_samples_inc_dur = ssi_cast(ssi_size_t, _inc_dur * _sample_rate + 0.5);

		_loffset = _options.offsetInDuration;
		_uoffset = _options.offsetOutDuration;

		_eager = _options.sendStartEvent;
		_skip_on_max_dur = _options.skipMaxDuration;

		_trigger = _options.triggerType;
		_hard_threshold = _options.hardThreshold;
		setThresholds(_options.thresholdIn, _options.thresholdOut, _options.thresholdInEnd, _options.thresholdOutEnd, _sample_type);

		_options.unlock();
	}

	void TriggerEventSender::setThresholds(double thres_in, double thres_out, double thres_in_end, double thres_out_end, ssi_type_t type)
	{
		delete[] _thres_in;
		delete[] _thres_out;
		delete[] _thres_in_end;
		delete[] _thres_out_end;

		switch (type)
		{
		case SSI_CHAR:
		{
			_thres_in = new char;
			_thres_out = new char;
			_thres_in_end = new char;
			_thres_out_end = new char;
			*ssi_pcast(char, _thres_in) = ssi_cast(char, thres_in);
			*ssi_pcast(char, _thres_out) = ssi_cast(char, thres_out);
			*ssi_pcast(char, _thres_in_end) = ssi_cast(char, thres_in_end);
			*ssi_pcast(char, _thres_out_end) = ssi_cast(char, thres_out_end);
			break;
		}
		case SSI_UCHAR:
		{
			_thres_in = new unsigned char;
			_thres_out = new unsigned char;
			_thres_in_end = new unsigned char;
			_thres_out_end = new unsigned char;
			*ssi_pcast(unsigned char, _thres_in) = ssi_cast(unsigned char, thres_in);
			*ssi_pcast(unsigned char, _thres_out) = ssi_cast(unsigned char, thres_out);
			*ssi_pcast(unsigned char, _thres_in_end) = ssi_cast(unsigned char, thres_in_end);
			*ssi_pcast(unsigned char, _thres_out_end) = ssi_cast(unsigned char, thres_out_end);
			break;
		}
		case SSI_SHORT:
		{
			_thres_in = new int16_t;
			_thres_out = new int16_t;
			_thres_in_end = new int16_t;
			_thres_out_end = new int16_t;
			*ssi_pcast(int16_t, _thres_in) = ssi_cast(int16_t, thres_in);
			*ssi_pcast(int16_t, _thres_out) = ssi_cast(int16_t, thres_out);
			*ssi_pcast(int16_t, _thres_in_end) = ssi_cast(int16_t, thres_in_end);
			*ssi_pcast(int16_t, _thres_out_end) = ssi_cast(int16_t, thres_out_end);
			break;
		}
		case SSI_USHORT:
		{
			_thres_in = new uint16_t;
			_thres_out = new uint16_t;
			_thres_in_end = new uint16_t;
			_thres_out_end = new uint16_t;
			*ssi_pcast(uint16_t, _thres_in) = ssi_cast(uint16_t, thres_in);
			*ssi_pcast(uint16_t, _thres_out) = ssi_cast(uint16_t, thres_out);
			*ssi_pcast(uint16_t, _thres_in_end) = ssi_cast(uint16_t, thres_in_end);
			*ssi_pcast(uint16_t, _thres_out_end) = ssi_cast(uint16_t, thres_out_end);
			break;
		}
		case SSI_INT:
		{
			_thres_in = new int32_t;
			_thres_out = new int32_t;
			_thres_in_end = new int32_t;
			_thres_out_end = new int32_t;
			*ssi_pcast(int32_t, _thres_in) = ssi_cast(int32_t, thres_in);
			*ssi_pcast(int32_t, _thres_out) = ssi_cast(int32_t, thres_out);
			*ssi_pcast(int32_t, _thres_in_end) = ssi_cast(int32_t, thres_in_end);
			*ssi_pcast(int32_t, _thres_out_end) = ssi_cast(int32_t, thres_out_end);
			break;
		}
		case SSI_UINT:
		{
			_thres_in = new uint32_t;
			_thres_out = new uint32_t;
			_thres_in_end = new uint32_t;
			_thres_out_end = new uint32_t;
			*ssi_pcast(uint32_t, _thres_in) = ssi_cast(uint32_t, thres_in);
			*ssi_pcast(uint32_t, _thres_out) = ssi_cast(uint32_t, thres_out);
			*ssi_pcast(uint32_t, _thres_in_end) = ssi_cast(uint32_t, thres_in_end);
			*ssi_pcast(uint32_t, _thres_out_end) = ssi_cast(uint32_t, thres_out_end);
			break;
		}
		case SSI_LONG:
		{
			_thres_in = new int64_t;
			_thres_out = new int64_t;
			_thres_in_end = new int64_t;
			_thres_out_end = new int64_t;
			*ssi_pcast(int64_t, _thres_in) = ssi_cast(int64_t, thres_in);
			*ssi_pcast(int64_t, _thres_out) = ssi_cast(int64_t, thres_out);
			*ssi_pcast(int64_t, _thres_in_end) = ssi_cast(int64_t, thres_in_end);
			*ssi_pcast(int64_t, _thres_out_end) = ssi_cast(int64_t, thres_out_end);
			break;
		}
		case SSI_ULONG:
		{
			_thres_in = new uint64_t;
			_thres_out = new uint64_t;
			_thres_in_end = new uint64_t;
			_thres_out_end = new uint64_t;
			*ssi_pcast(uint64_t, _thres_in) = ssi_cast(uint64_t, thres_in);
			*ssi_pcast(uint64_t, _thres_out) = ssi_cast(uint64_t, thres_out);
			*ssi_pcast(uint64_t, _thres_in_end) = ssi_cast(uint64_t, thres_in_end);
			*ssi_pcast(uint64_t, _thres_out_end) = ssi_cast(uint64_t, thres_out_end);
			break;
		}
		case SSI_FLOAT:
		{
			_thres_in = new float;
			_thres_out = new float;
			_thres_in_end = new float;
			_thres_out_end = new float;
			*ssi_pcast(float, _thres_in) = ssi_cast(float, thres_in);
			*ssi_pcast(float, _thres_out) = ssi_cast(float, thres_out);
			*ssi_pcast(float, _thres_in_end) = ssi_cast(float, thres_in_end);
			*ssi_pcast(float, _thres_out_end) = ssi_cast(float, thres_out_end);
			break;
		}
		case SSI_DOUBLE:
		{
			_thres_in = new double;
			_thres_out = new double;
			_thres_in_end = new double;
			_thres_out_end = new double;
			*ssi_pcast(double, _thres_in) = ssi_cast(double, thres_in);
			*ssi_pcast(double, _thres_out) = ssi_cast(double, thres_out);
			*ssi_pcast(double, _thres_in_end) = ssi_cast(double, thres_in_end);
			*ssi_pcast(double, _thres_out_end) = ssi_cast(double, thres_out_end);
			break;
		}
		default:
			ssi_err("unsupported sample type (%s)", SSI_TYPE_NAMES[type]);
		}
	}

	void TriggerEventSender::consume_enter(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{
		_trigger_on = false;
		_sample_type = stream_in[0].type;
		_sample_rate = stream_in[0].sr;

		_sample_dim = stream_in[0].dim;
		_sample_num = 0;
		_sample_sum = new ssi_real_t[_sample_dim];
		for (ssi_size_t i = 0; i < _sample_dim; i++)
		{
			_sample_sum[i] = 0;
		}

		switch (_event.type)
		{
		case SSI_ETYPE_MAP:
			ssi_event_adjust(_event, _sample_dim * sizeof(ssi_event_map_t));
			break;
		case SSI_ETYPE_TUPLE:
			ssi_event_adjust(_event, _sample_dim * sizeof(ssi_event_tuple_t));
			break;
		}

		readOptions();
	}

	void TriggerEventSender::consume(IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{

		ssi_byte_t *dataptr = stream_in[0].ptr;
		ssi_size_t dim = stream_in[0].dim;

		bool found_event;
		for (ssi_size_t i = 0; i < stream_in[0].num; i++)
		{

			//choose between onset and offset threshold
			void* threshold = !_trigger_on ? _thres_in : _thres_out;
			void* thresholdEnd = !_trigger_on ? _thres_in_end : _thres_out_end;

			found_event = _hard_threshold;
			for (ssi_size_t j = 0; j < stream_in[0].dim; j++)
			{

				bool check_threshold = false;
				switch (_sample_type) {
				case SSI_CHAR:
					check_threshold = check_thres<char>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_UCHAR:
					check_threshold = check_thres<unsigned char>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_SHORT:
					check_threshold = check_thres<int16_t>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_USHORT:
					check_threshold = check_thres<uint16_t>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_INT:
					check_threshold = check_thres<int32_t>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_UINT:
					check_threshold = check_thres<uint32_t>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_LONG:
					check_threshold = check_thres<int64_t>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_ULONG:
					check_threshold = check_thres<int64_t>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_FLOAT:
					check_threshold = check_thres<float>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				case SSI_DOUBLE:
					check_threshold = check_thres<double>(dataptr + j * stream_in[0].byte, threshold, thresholdEnd, _trigger);
					break;
				}

				if (_hard_threshold)
				{
					found_event = found_event && check_threshold;
				}
				else
				{
					found_event = found_event || check_threshold;
				}

			}

			if (!_trigger_on)
			{
				if (found_event)
				{
					// possible start of a new event
					if (_counter_in == _hangover_in)
					{
						// store start time and init max dur counter
						_trigger_start = consume_info.time + i / stream_in[0].sr;
						// reset sample sum
						_sample_num = 0;
						for (ssi_size_t i = 0; i < _sample_dim; i++)
						{
							_sample_sum[i] = 0;
						}
					}

					// check if event start is proved
					if (_counter_in-- == 0)
					{
						// signal that event start is now proved and init hangout and max counter
						_trigger_on = true;
						_counter_out = _hangover_out;
						_counter_inc_dur = _samples_inc_dur - _hangover_in;

						if (_elistener && _eager) {
							_event.time = ssi_cast(ssi_size_t, 1000 * _trigger_start + 0.5);
							_event.dur = 0;
							_event.state = SSI_ESTATE_CONTINUED;
							_event.glue_id = Factory::GetUniqueId();
							sendEvent();
						}

						SSI_DBG(SSI_LOG_LEVEL_DEBUG, "event started at %.2lfs", _trigger_start);
					}

				}
				else
				{
					// re-init hangin counter
					_counter_in = _hangover_in;
				}

				// update sample sum
				switch (_sample_type) {
				case SSI_CHAR:
					update_sum<char>(dataptr);
					break;
				case SSI_UCHAR:
					update_sum<unsigned char>(dataptr);
					break;
				case SSI_SHORT:
					update_sum<int16_t>(dataptr);
					break;
				case SSI_USHORT:
					update_sum<uint16_t>(dataptr);
					break;
				case SSI_INT:
					update_sum<int32_t>(dataptr);
					break;
				case SSI_UINT:
					update_sum<uint32_t>(dataptr);
					break;
				case SSI_LONG:
					update_sum<int64_t>(dataptr);
					break;
				case SSI_ULONG:
					update_sum<uint64_t>(dataptr);
					break;
				case SSI_FLOAT:
					update_sum<float>(dataptr);
					break;
				case SSI_DOUBLE:
					update_sum<double>(dataptr);
					break;
				}
			}
			else if (_trigger_on)
			{
				// check if incremental duration is reached
				if (_samples_inc_dur > 0 && --_counter_inc_dur == 0)
				{

					IConsumer::info info;
					ssi_time_t now = consume_info.time + i / stream_in[0].sr;
					ssi_time_t dur = now - _trigger_start;
					if (_max_dur > 0 && dur > _max_dur) {
						info.dur = _max_dur;
						info.time = now - _max_dur;
					}
					else {
						info.dur = dur;
						info.time = _trigger_start;
					}
					info.status = IConsumer::CONTINUED;

					update_h(info);

					_counter_inc_dur = _samples_inc_dur;
				}

				if (!found_event)
				{
					// possible end of a new event
					if (_counter_out == _hangover_out)
					{

						// store end time
						_trigger_stop = consume_info.time + i / stream_in[0].sr;

					}
					// check if event start is proved
					if (_counter_out-- == 0) {

						// event end is now proved and event is sent
						IConsumer::info info;
						ssi_time_t now = _trigger_stop;
						ssi_time_t dur = now - _trigger_start;
						if (_max_dur > 0 && dur > _max_dur) {
							info.dur = _max_dur;
							info.time = now - _max_dur;
						}
						else {
							info.dur = dur;
							info.time = _trigger_start;
						}
						info.status = IConsumer::COMPLETED;

						update_h(info);

						// signal end of event and init hangin counter
						_trigger_on = false;
						_counter_in = _hangover_in;

						SSI_DBG(SSI_LOG_LEVEL_DEBUG, "event stopped at %.2lfs", _trigger_stop);
					}
				}
				else
				{
					// re-init hangin counter
					_counter_out = _hangover_out;
				}
			}

			dataptr += stream_in[0].dim * stream_in[0].byte;
		}
	}

	bool TriggerEventSender::update_h(IConsumer::info info) {

		if (info.dur < _min_dur || info.dur <= 0.0)
		{
			SSI_DBG(SSI_LOG_LEVEL_DEBUG, "skip event because duration too short (%.2lf@%.2lf)", info.dur, info.time);
			return false;
		}

		if (_elistener)
		{
			_event.time = max(0, (int)ssi_cast(ssi_size_t, 1000 * (info.time - _loffset) + 0.5));
			_event.dur = max(0, (int)ssi_cast(ssi_size_t, 1000 * (info.dur + _uoffset) + 0.5));

			if (info.status == IConsumer::COMPLETED)
			{
				_event.state = SSI_ESTATE_COMPLETED;
			}
			else
			{
				_event.state = SSI_ESTATE_CONTINUED;
				if (_event.glue_id == SSI_FACTORY_UNIQUE_INVALID_ID)
				{
					_event.glue_id = Factory::GetUniqueId();
				}
			}

			sendEvent();			

			if (_event.state = SSI_ESTATE_COMPLETED)
			{
				_event.glue_id = SSI_FACTORY_UNIQUE_INVALID_ID;
			}
		}

		// maybe options have changed, read in again
		readOptions();

		return true;
	}

	void TriggerEventSender::consume_flush(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{


		delete[] _thres_in; _thres_in = 0;
		delete[] _thres_out; _thres_out = 0;
		delete[] _thres_in_end; _thres_in_end = 0;
		delete[] _thres_out_end; _thres_out_end = 0;
		delete[] _sample_sum; _sample_sum = 0;
	}

	template <class T>
	bool TriggerEventSender::check_thres(void *ptr, void *thres, void *thres_end, TRIGGER::List trigger)
	{
		T v = *ssi_pcast(T, ptr);
		T t1 = *ssi_pcast(T, thres);
		T t2 = *ssi_pcast(T, thres_end);

		switch (trigger)
		{
		case TRIGGER::GREATER:
			return v > t1;
		case TRIGGER::GREATER_EQUAL:
			return v >= t1;
		case TRIGGER::LESSER:
			return v < t1;
		case TRIGGER::LESSER_EQUAL:
			return v < t1;
		case TRIGGER::EQUAL:
			return v == t1;
		case TRIGGER::NOT_EQUAL:
			return v != t1;
		case TRIGGER::IN_RANGE:
			return v > t1 && v < t2;
		case TRIGGER::IN_RANGE_EQUAL:
			return v >= t1 && v <= t2;
		case TRIGGER::NOT_IN_RANGE:
			return v < t1 || v > t2;
		case TRIGGER::NOT_IN_RANGE_EQUAL:
			return v <= t1 || v >= t2;
		}

		return false;
	}

	template <class T>
	void TriggerEventSender::update_sum(void *ptr)
	{
		T *values = ssi_pcast(T, ptr);
		for (ssi_size_t i = 0; i < _sample_dim; i++)
		{
			_sample_sum[i] += (ssi_real_t)values[i];
		}
		_sample_num++;
	}

	void TriggerEventSender::sendEvent()
	{
		switch (_event.type)
		{
		case SSI_ETYPE_MAP:
		{
			ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _event.ptr);
			for (ssi_size_t i = 0; i < _sample_dim; i++)
			{
				ptr[i].value = _sample_num == 0 ? 0 : _sample_sum[i] / _sample_num;
				ptr[i].id = _map_ids ? _map_ids[i % _n_map_ids] : SSI_SAMPLE_GARBAGE_CLASS_ID;
			}
			break;
		}
		case SSI_ETYPE_TUPLE:
		{
			ssi_event_tuple_t *ptr = ssi_pcast(ssi_event_tuple_t, _event.ptr);
			for (ssi_size_t i = 0; i < _sample_dim; i++)
			{
				*ptr++ = _sample_num == 0 ? 0 : _sample_sum[i] / _sample_num;
			}
			break;
		}
		}

		_elistener->update(_event);
	}

}
