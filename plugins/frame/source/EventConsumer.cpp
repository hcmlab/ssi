// EventConsumer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/14
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

#include "EventConsumer.h"
#include "TheFramework.h"
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

ssi_char_t *EventConsumer::ssi_log_name = "econsumer_";
const ssi_size_t EventConsumer::MAX_CONSUMER = 50;

EventConsumer::EventConsumer (const ssi_char_t *file)
	: _file (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT),
	_terminate (false),
	_consumer_count (0),
	_async (false) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
		
	_consumer = new ConsumerBase *[MAX_CONSUMER];
	for (ssi_size_t i = 0; i < SSI_TRIGGER_MAX_CONSUMER; i++) {
		_consumer[i] = 0;
	}

	_frame = ssi_pcast (TheFramework, Factory::GetFramework ());

	_update_info.event = new ssi_event_t;
	ssi_event_init(*_update_info.event);
}

EventConsumer::~EventConsumer () {

	clear ();
	delete[] _consumer; _consumer = 0;

	ssi_event_destroy(*_update_info.event);
	delete _update_info.event;

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}


bool EventConsumer::AddConsumer (ITransformable *source, IConsumer *consumer, ITransformer *transformer) {

	if (transformer) {
		return AddConsumer (1, &source, consumer, &transformer);
	} else {
		return AddConsumer (1, &source, consumer);
	}

}

bool EventConsumer::AddConsumer (ssi_size_t n_sources, ITransformable **sources, IConsumer *consumer, ITransformer **transformers) {

	if (_consumer_count < SSI_TRIGGER_MAX_CONSUMER) {
		int *buffer_ids = new int[n_sources];
		for (ssi_size_t i = 0; i < n_sources; i++) {
			buffer_ids[i] = sources[i]->getBufferId ();
		}
		_consumer[_consumer_count] = new ConsumerBase (n_sources, buffer_ids, consumer, 0, 0, transformers);
		delete[] buffer_ids;
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "attached consumer '%s'", _consumer[_consumer_count]->_consumer->getName ());
		++_consumer_count;
		return true;
	}

	ssi_wrn ("max #consumer exceeded");

	return false;
}

void EventConsumer::clear () {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "detach all consumer");

	for (ssi_size_t i = 0; i < _consumer_count; i++) {
		delete _consumer[i];
		_consumer[i] = 0;
	}
	_consumer_count = 0;	
}

void EventConsumer::listen_enter () {

	_async = _options.async;

	for (ssi_size_t i = 0; i < _consumer_count; i++) {
		_consumer[i]->enter ();
	}

	_terminate = false;

	if (_async) {
		this->setName (getName());
		start ();
	}
}

bool EventConsumer::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {	

	if (n_new_events == 0) {
		return true;
	}

	if (n_new_events > 1) {
		ssi_wrn ("skip %u events", n_new_events - 1);
	}

	ssi_event_t *e = events.next (); 
	IConsumer::info info;
	info.time = e->time / 1000.0;
	info.dur = e->dur / 1000.0;
	info.status = e->state == SSI_ESTATE_COMPLETED ? IConsumer::COMPLETED : IConsumer::CONTINUED;	
	info.event = e;

	if (_async) {

		{
			Lock lock (_update_mutex);
			_update_info.time = info.time;
			_update_info.dur = info.dur;
			_update_info.status = info.status;	
			ssi_event_destroy(*_update_info.event);
			ssi_event_clone(*info.event, *_update_info.event);
		}

		_update_event.release ();

	} else {

		consume (info);		
	}	

	return true;
}

void EventConsumer::consume (IConsumer::info info) {
	
	if (info.dur <= 0) {
		return;
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "update (%.2lf@%.2lf, status: %d)", info.dur, info.time, info.status);

	for (ssi_size_t i = 0; i < _consumer_count; i++) {

        SSI_DBG (SSI_LOG_LEVEL_DEBUG, "update '%s'", _consumer[i]->_consumer->getName ());

		int status = _consumer[i]->consume (info);	
#if __gnu_linux__
        if(_terminate)return;
#endif
		// check if operation was successful
		// otherwise try to handle the error
		switch (status) {
			case TimeBuffer::SUCCESS:
				// operation was successful!							
				break;
			case TimeBuffer::DATA_NOT_IN_BUFFER_YET:
				// not all _data is yet available
				// we return and hope that it will be available at next call..
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "data not in buffer yet '%s'", _consumer[i]->_consumer->getName ());
				break;
			case THEFRAMEWORK_ERROR:
				// framework error, probably framework is in idle mode
				// there is not much to do for us but wait..
				//SSI_DBG (SSI_LOG_LEVEL_DEBUG, "framework not running '%s'", _consumer[i]->getName ());
				break;
			default:
				// well, something critical happend, probably the requested _data is not available anymore				
				ssi_wrn ("requested data not available (%s) '%s'", TimeBuffer::STATUS_NAMES[status], _consumer[i]->_consumer->getName ());
				ssi_time_t frame_time = _frame->GetElapsedTime ();
				_consumer[i]->_consumer->consume_fail (info.time, frame_time - info.time, _consumer[i]->_stream_number, _consumer[i]->_streams);
				info.time = frame_time;
				break; 
		}
	}
}

void EventConsumer::listen_flush () {

	if (_async) {
		stop ();
	}

	for (ssi_size_t i = 0; i < _consumer_count; i++) {
		_consumer[i]->flush ();
	}
}

void EventConsumer::terminate () {

	_terminate = true;

	_update_event.release ();

}

void EventConsumer::run () {

	_update_event.wait ();

	if (_terminate) {
		return;
	}

	IConsumer::info info;
	info.event = new ssi_event_t;
    {
		Lock lock (_update_mutex);

		info.dur = _update_info.dur;
		info.time = _update_info.time;
		info.status = _update_info.status;
        ssi_event_clone(*_update_info.event, *info.event);
	}

	consume (info);

	ssi_event_destroy(*info.event);
    delete info.event;

}

}
