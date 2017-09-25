// DecisionSmoother.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/06/24
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

#include "DecisionSmoother.h"
#include "base/Factory.h"
#include "base/ITheFramework.h"
#include "thread/RunAsThread.h"
#include "thread/Timer.h"

#if __gnu_linux__
using std::min;
using std::max;
#endif

namespace ssi {

char DecisionSmoother::ssi_log_name[] = "decsmooth_";

DecisionSmoother::DecisionSmoother (const ssi_char_t *file)
:	_file (0),
	_listener (0),
	_dim(0),
	_target(0),
	_decision(0),
	_speed(0),
	_decay(0),
	_average(false),
	_window(0),
	_last_decision_time(0),
	_counter(0),
	_thread(0),
	_timer(0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_MAP);
	_frame = Factory::GetFramework();
}

DecisionSmoother::~DecisionSmoother () {
	
	ssi_event_destroy (_event);

	delete[] _target;
	delete[] _decision;
}

bool DecisionSmoother::setEventListener (IEventListener *listener) {

	_listener = listener;
	
	if (_options.address[0] != '\0') {

		SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

	}
	else {

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

void DecisionSmoother::init(ssi_event_t e) {

	if (e.type == SSI_ETYPE_MAP) {

		_dim = e.tot / sizeof(ssi_event_map_t);

		if (_dim > 0) {

			_target = new ssi_real_t[_dim];
			_decision = new ssi_real_t[_dim];
			for (ssi_size_t i = 0; i < _dim; i++) {
				_target[i] = _decision[i] = 0;
			}

			ssi_event_adjust(_event, _dim * sizeof(ssi_event_map_t));
			ssi_event_map_t *src = ssi_pcast(ssi_event_map_t, e.ptr);
			ssi_event_map_t *dst = ssi_pcast(ssi_event_map_t, _event.ptr);
			for (ssi_size_t i = 0; i < _dim; i++) {
				dst[i].value = 0;
				dst[i].id = src[i].id;
			}			
		}

		_counter = 0;
	}
}

bool DecisionSmoother::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {

	case INotify::COMMAND::RESET:

		Lock lock(_mutex);

		for (ssi_size_t i = 0; i < _dim; i++) {
			_target[i] = 0;
		}
		_counter = 0;

		return true;
	}

	return false;
}

void DecisionSmoother::readOptions() {

	_options.lock();

	_decay = _options.decay;
	_speed = _options.speed;
	_window = _options.window;
	_average = _options.average;

	_options.unlock();
}

void DecisionSmoother::listen_enter (){
	
	_dim = 0;
	_counter = 0;

	readOptions();	
	
	_event.dur = _options.update_ms;
	_event.time = 0;
	_last_decision_time = 0;

	if (_options.update_ms > 0) {
		_timer = new Timer(_options.update_ms);
		_thread = new RunAsThread(&send, this);
		_thread->start();
	}
}

void DecisionSmoother::send(void *ptr) {

	DecisionSmoother *me = ssi_pcast(DecisionSmoother, ptr);

	if (me->_listener) {

		me->readOptions();

		Lock lock(me->_mutex);

		if (me->_dim > 0) {

			ssi_event_map_t *dst = ssi_pcast(ssi_event_map_t, me->_event.ptr);
			for (ssi_size_t i = 0; i < me->_dim; i++) {

				if (me->_speed <= 0) {
					me->_decision[i] = me->_target[i];
				} else {

					if (me->_decision[i] < me->_target[i]) {
						me->_decision[i] += me->_speed;
						if (me->_decision[i] > me->_target[i]) {
							me->_decision[i] = me->_target[i];
						}
					}

					if (me->_decision[i] > me->_target[i]) {
						me->_decision[i] -= me->_speed;
						if (me->_decision[i] < me->_target[i]) {
							me->_decision[i] = me->_target[i];
						}
					}
				}

				dst[i].value = me->_decision[i];

				me->_target[i] -= me->_decay;
                me->_target[i] = max(me->_target[i], 0.0f);
			}
			me->_event.time = me->_frame->GetElapsedTimeMs() - me->_event.dur;

			me->_listener->update(me->_event);			
		}
	}

	if (me->_thread) {
		me->_timer->wait();
	}
}

bool DecisionSmoother::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	ssi_event_t *e = 0;
	events.reset ();

	if (n_new_events > 0) {

		{
			Lock lock(_mutex);
			e = events.next();

			if (_dim == 0) {
				init(*e);
			}

			if (e->type == SSI_ETYPE_MAP) {

				ssi_event_map_t *src = ssi_pcast(ssi_event_map_t, e->ptr);
				ssi_event_map_t *dst = ssi_pcast(ssi_event_map_t, _event.ptr);
				ssi_size_t dim = e->tot / sizeof(ssi_event_map_t);

				if (dim == _dim) {

					if (_average) {

						if (_window > 0) {

							ssi_time_t delta = (time_ms - _last_decision_time) / 1000.0;

							ssi_real_t alpha = 0;
							if (delta >= 0 && delta <= _window) {
								alpha = ssi_cast (ssi_real_t, (_window - delta) / _window);
							}
							
							for (ssi_size_t i = 0; i < _dim; i++) {
								//printf("delta=%.2lf alpha=%.2f: %.2f + ->", delta, alpha, _target[i]);
								_target[i] = alpha * _target[i] + (1.0f - alpha) * src[i].value;								
								//printf(" %.2f\n", _target[i]);
							}

						} else {

							++_counter;
							for (ssi_size_t i = 0; i < _dim; i++) {
								_target[i] += (src[i].value - _target[i]) / _counter;
							}
						}

					}
					else {

						for (ssi_size_t i = 0; i < _dim; i++) {
							_target[i] = src[i].value;
						}

					}

				} else {
					ssi_wrn("dimension mismatch '%u' != '%u'", dim, _dim);
				}				
			}
		}

		if (!_thread) {
			_event.dur = time_ms - _last_decision_time;
		}

		_last_decision_time = time_ms;

		if (!_thread) {			
			send(this);
		}
	}

	return true;
		
}

void DecisionSmoother::listen_flush (){

	if (_thread) {
		_thread->stop();
	}
	delete _thread; _thread = 0;
	delete _timer; _timer = 0;

	delete[] _target; _target = 0;
	delete[] _decision; _decision  = 0;

	ssi_event_reset(_event);

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

}

}
