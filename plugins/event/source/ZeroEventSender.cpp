// ZeroEventSender.cpp
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

#include "ZeroEventSender.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


#if __gnu_linux__
using std::min;
using std::max;
#endif


namespace ssi {

ssi_char_t *ZeroEventSender::ssi_log_name = "zeroesend_";

ZeroEventSender::ZeroEventSender (const ssi_char_t *file)
: _hangover_in (0),
	_hangover_out (0),
	_samples_inc_dur(0),
	_counter_inc_dur(0),
	_max_dur (0),
	_min_dur (0),
	_loffset (0),
	_uoffset (0),
	_skip_on_max_dur (false),
	_hard_threshold(false),
	_eager(false),
	_file (0),
	_check (0),
	_elistener (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init(_event);
}

ZeroEventSender::~ZeroEventSender () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	ssi_event_destroy(_event);
}

void ZeroEventSender::readOptions() {

	_options.lock();

	_eager = _options.eager;
	_hard_threshold = _options.hard;
	_hangover_in = _options.hangin;
	_hangover_out = _options.hangout;
	_counter_in = _hangover_in;
	_counter_out = _hangover_out;
	_min_dur = _options.mindur;
	_max_dur = _options.maxdur;
	_inc_dur = _options.incdur;
	_loffset = _options.loffset;
	_uoffset = _options.uoffset;
	_skip_on_max_dur = _options.skip;
	_samples_inc_dur = ssi_cast(ssi_size_t, _inc_dur * _sample_rate);

	_options.unlock();
}

void ZeroEventSender::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_wrn("deprecated, use 'TriggerEventSender' instead");

	_sample_rate = stream_in[0].sr;
	_trigger_on = false;

	readOptions();

	switch (stream_in[0].type) {
		case SSI_CHAR:
			_check = &check_char;
			break;
		case SSI_UCHAR:
			_check = &check_uchar;
			break;
		case SSI_SHORT:
			_check = &check_short;
			break;
		case SSI_USHORT:
			_check = &check_ushort;
			break;
		case SSI_INT:
			_check = &check_int;
			break;
		case SSI_UINT:
			_check = &check_uint;
			break;
		case SSI_LONG:
			_check = &check_long;
			break;
		case SSI_ULONG:
			_check = &check_ulong;
			break;
		case SSI_FLOAT:
			_check = &check_float;
			break;
		case SSI_DOUBLE:
			_check = &check_double;
			break;
		default:
			ssi_err ("unsupported sample type");
	}
}

bool ZeroEventSender::setEventListener (IEventListener *listener) {

	if (_options.empty) {
		ssi_event_init(_event, SSI_ETYPE_EMPTY);
	}
	else {
		ssi_event_init(_event, SSI_ETYPE_STRING);
		ssi_event_adjust(_event, ssi_strlen(_options.string) + 1);
		ssi_strcpy(_event.ptr, _options.string);
	}

	_elistener = listener;

	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

	} else {

		ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);
	}

	return true;
}

void ZeroEventSender::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_byte_t *dataptr = stream_in[0].ptr;
	ssi_size_t dim = stream_in[0].dim;

	bool found_event;
	for (ssi_size_t i = 0; i < stream_in[0].num; i++) {

		found_event = _hard_threshold;;
		for (ssi_size_t j = 0; j < stream_in[0].dim; j++) {
			if (_hard_threshold) {
				found_event = found_event && _check (dataptr);
			} else {
				found_event = found_event || _check (dataptr);
			}
			dataptr += stream_in[0].byte;
		}

		if (!_trigger_on) {

			if (found_event) {

				// possible start of a new event
				if (_counter_in == _hangover_in) {

					// store start time and init max dur counter
					_trigger_start = consume_info.time + i / stream_in[0].sr;
				}
				// check if event start is proved
				if (_counter_in-- == 0) {

					// signal that event start is now proved and init hangout and max counter
					_trigger_on = true;
					_counter_out = _hangover_out;
					_counter_inc_dur = _samples_inc_dur - _hangover_in;

					if (_elistener && _eager) {
						_event.time = ssi_cast (ssi_size_t, 1000 * _trigger_start + 0.5);
						_event.dur = 0;
						_event.state = SSI_ESTATE_CONTINUED;
						_event.glue_id = Factory::GetUniqueId ();
						_elistener->update (_event);
					}

					SSI_DBG (SSI_LOG_LEVEL_DEBUG, "event started at %.2lfs", _trigger_start);
				}

			} else {

				// re-init hangin counter
				_counter_in = _hangover_in;

			}
		} else if (_trigger_on) {

			// check if incremental duration is reached
			if (_samples_inc_dur > 0 && --_counter_inc_dur == 0) {

				IConsumer::info info;
				ssi_time_t now = consume_info.time + i / stream_in[0].sr;
				ssi_time_t dur = now - _trigger_start;
				if (_max_dur > 0 && dur > _max_dur) {
					info.dur = _max_dur;
					info.time = now - _max_dur;
				} else {
					info.dur = dur;
					info.time = _trigger_start;
				}
				info.status = IConsumer::CONTINUED;

				update_h(info);

				_counter_inc_dur = _samples_inc_dur;
			}

			if (!found_event) {

				// possible end of a new event
				if (_counter_out == _hangover_out) {

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
					} else {
						info.dur = dur;
						info.time = _trigger_start;
					}
					info.status = IConsumer::COMPLETED;

					update_h (info);

					// signal end of event and init hangin counter
					_trigger_on = false;
					_counter_in = _hangover_in;

					SSI_DBG (SSI_LOG_LEVEL_DEBUG, "event stopped at %.2lfs", _trigger_stop);
				}
			} else {
				// re-init hangin counter
				_counter_out = _hangover_out;
			}
		}
	}
}

bool ZeroEventSender::update_h (IConsumer::info info) {

	if (info.dur < _min_dur || info.dur <= 0.0) {
		SSI_DBG (SSI_LOG_LEVEL_DEBUG, "skip event because duration too short (%.2lf@%.2lf)", info.dur, info.time);
		return false;
	}

	if (_elistener) {

		_event.time = max (0, (int)ssi_cast (ssi_size_t, 1000 * (info.time - _loffset) + 0.5));
		_event.dur = max (0, (int)ssi_cast (ssi_size_t, 1000 * (info.dur + _uoffset) + 0.5));

		if (info.status == IConsumer::COMPLETED) {
			_event.state = SSI_ESTATE_COMPLETED;
		} else {
			_event.state = SSI_ESTATE_CONTINUED;
			if (_event.glue_id == SSI_FACTORY_UNIQUE_INVALID_ID) {
				_event.glue_id = Factory::GetUniqueId ();
			}
		}

		_elistener->update (_event);

		if (_event.state = SSI_ESTATE_COMPLETED) {
			_event.glue_id = SSI_FACTORY_UNIQUE_INVALID_ID;
		}
	}

	// maybe options have changed, read in again
	readOptions();

	return true;
}

void ZeroEventSender::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
}

}
