// TheEventBoard.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/07
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

#include "TheEventBoard.h"
#include "event/EventList.h"
#include "EventQueue.h"
#include "event/IESelect.h"
#include "EventBoardWorker.h"
#include "event/EventAddress.h"
#include "thread/Lock.h"
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

ssi_char_t *ssi_log_name = "eboard____";

TheEventBoard::TheEventBoard (const ssi_char_t *file) 
: _events (0),
	_queue (0),
	_worker (0),
	_mutex (0),
	_is_running (false),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_events = new EventList (_options.n_events);
	_queue = new EventQueue (_options.n_events);
	_mutex = new Mutex ();

	_listener.init (_options.n_listener);
	_sender.init (_options.n_sender);
	_ieselect.init (_options.n_listener);
	_concerns_listener.init (_options.n_listener);
}

TheEventBoard::~TheEventBoard () {

	release ();
	
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void TheEventBoard::release () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "release event board");

	if (_is_running) {
		Stop ();
	}
	
	Clear ();

	delete _events;
	_events = 0;
	delete _queue;
	_queue = 0;
	delete _mutex;
	_mutex = 0;


}

void TheEventBoard::Start () {
	
	ssi_msg (SSI_LOG_LEVEL_BASIC, "start event board worker");

	if (!_is_running) {

		_is_running = true;

		for (ssi_size_t i = 0; i < _sender.count (); i++) {
			_sender[i]->send_enter ();
		}

		for (ssi_size_t i = 0; i < _listener.count (); i++) {
			_listener[i]->listen_enter ();
		}		

		_worker = new EventBoardWorker (this);
		_worker->start ();
		
	} else {
		ssi_wrn ("already running");
	
	}
}

void TheEventBoard::Stop () {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop event board worker");

	if (_is_running) {		
		_worker->stop ();
		delete _worker; _worker = 0;

		for (ssi_size_t i = 0; i < _listener.count (); i++) {
			_listener[i]->listen_flush ();			
		}		

		for (ssi_size_t i = 0; i < _sender.count (); i++) {
			_sender[i]->send_flush ();
		}

		_events->clear();
		_queue->clear();

		_is_running = false;
	} else {
		ssi_wrn ("not running");
	}
}

bool TheEventBoard::update (ssi_event_t &e) {

	if (!_is_running) {
		return false;
	}

	return _queue->push (e);
}

bool TheEventBoard::process (ssi_size_t n) {

	{
		Lock lock (*_mutex);

		_events->reset ();

		// determine listener
		for (ssi_size_t i = 0; i < _listener.count (); i++) {
			_concerns_listener[i] = 0;
		}

		ssi_event_t *e = 0;
		for (ssi_size_t i = 0; i < n; i++) {			
			if (e = _events->next ()) {
				for (ssi_size_t j = 0; j < _listener.count (); j++) {
					if (_ieselect[j]->check (*e, false)) {
						_concerns_listener[j]++;
					}
				}
			}	
		}
		
		ssi_size_t time_ms = 0;
		for (ssi_size_t i = 0; i < _listener.count (); i++) {
			if (_listener[i]->isEnabled()) {
				time_ms = Factory::GetFramework()->GetElapsedTimeMs();
				_ieselect[i]->setTime(Factory::GetFramework()->GetElapsedTimeMs());
				_ieselect[i]->reset();
				_listener[i]->update(*_ieselect[i], _concerns_listener[i], time_ms);
			}
		}
	}

	return true;
}

bool TheEventBoard::RegisterSender(IObject &sender) {
	
	bool result = false;

	ssi_size_t count = 0;
	{
		Lock lock (*_mutex);
		count = _sender.count ();
		if (count >= _sender.size ()) {
			ssi_wrn ("#sender exceeds available space '%u'", _sender.size ());			
		} else {

			result = sender.setEventListener (this);
			if (result) {
				*_sender.next () = &sender;			
			}
		}
	}

	if (result) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' sends '%s'", Factory::GetObjectId(&sender), sender.getEventAddress());		
	}
	
	return result;	
}

bool TheEventBoard::RegisterListener (IObject &listener,		
	const ssi_char_t *address, 
	ssi_size_t time_span_ms,
	IEvents::EVENT_STATE_FILTER::List state_filter) {

	EventAddress ea;
	ea.setAddress (address);

	ssi_size_t n_sender = ea.getSenderSize ();
	ssi_size_t n_events = ea.getEventsSize ();

	ssi_size_t *sender_ids = n_sender > 0 ? new ssi_size_t[n_sender] : 0;
	ssi_size_t *events_ids = n_events > 0 ? new ssi_size_t[n_events] : 0;

	for (ssi_size_t i = 0; i < n_sender; i++) {
		const ssi_char_t *s = ea.getSender (i);
		if ((sender_ids[i] = Factory::GetStringId (s)) == SSI_FACTORY_STRINGS_INVALID_ID) {
			ssi_wrn ("unkown sender '%s'", s);		
			sender_ids[i] = Factory::AddString(s);
			//return false;
		}
	}

	for (ssi_size_t i = 0; i < n_events; i++) {
		const ssi_char_t *e = ea.getEvent (i);
		if ((events_ids[i] = Factory::GetStringId (e)) == SSI_FACTORY_STRINGS_INVALID_ID) {
			ssi_wrn ("unknown event '%s'", e);	
			events_ids[i] = Factory::AddString(e);
			//return false;
		}
	}
		
	ssi_size_t count = 0;
	{
		Lock lock (*_mutex);
		count = _listener.count ();
		if (count >= _listener.size ()) {
			ssi_wrn ("#listener exceeds available space '%u'", _listener.size ());
			return false;
		} else {
			*_listener.next () = &listener;
			IESelect *ieselect = new IESelect (_events);
			ieselect->set (n_sender, sender_ids, n_events, events_ids, time_span_ms, state_filter);
			*_ieselect.next () = ieselect;
		}
	}

	delete[] sender_ids;
	delete[] events_ids;

	if (ssi_strcmp("EventConsumer", listener.getName()))
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "'EventConsumer' receives '%s'", address ? address : "*");
	}
	else
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' receives '%s'", Factory::GetObjectId(&listener), address ? address : "*");
	}

	return true;

}

void TheEventBoard::Clear () {

	for (IESelect **ptr =  _ieselect.ptr (); ptr < _ieselect.end (); ptr++) {
		delete *ptr; *ptr = 0;
	}
	_ieselect.clear ();
	_listener.clear ();
	_sender.clear ();
}

void TheEventBoard::Print (FILE *file) {
	
	{
		Lock lock (*_mutex);
		Print (*_events, file);
	}
}

void TheEventBoard::Print (IEvents &events, FILE *file) {

	events.reset ();
	ssi_event_t *e = 0;
	ssi_size_t i = 0;
	ssi_fprint (file, "#\ttype\tsender\tevent\ttime\tdur\tsize\n--------------------------------------------------------\n");
	while (e = events.next ()) {
		if (e->sender_id == SSI_FACTORY_STRINGS_INVALID_ID || e->event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			ssi_wrn ("abort printing because of an invalid sender/event id");
			return;
		}
		ssi_fprint (file, "%03u\t%s\t%s\t%s\t%u\t%u\t%u\n", i++, SSI_ETYPE_NAMES[e->type], e->sender_id == SSI_FACTORY_STRINGS_INVALID_ID ? "" : Factory::GetString (e->sender_id), e->event_id == SSI_FACTORY_STRINGS_INVALID_ID ? "" : Factory::GetString (e->event_id), e->time, e->dur, e->tot);
	}
}

}
