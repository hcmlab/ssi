// EventSlider.cpp
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

#include "EventSlider.h"
#include "base/Factory.h"
#include "graphic/Window.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif


namespace ssi {

ssi_char_t *EventSlider::ssi_log_name = "evntslider";

EventSlider::EventSlider (const ssi_char_t *file)
: _file (0),
	_elistener (0),
	_slider (0),
	_window(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	ssi_event_init (_event, SSI_ETYPE_NTUPLE);
}

EventSlider::~EventSlider () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	ssi_event_destroy (_event);	
}

bool EventSlider::setEventListener (IEventListener *listener) {

	delete _slider; _slider = 0;

	ssi_char_t name[SSI_MAX_CHAR];
	ssi_sprint (name, "%s@%s", _options.ename, _options.sname);

	_elistener = listener;
	_event.sender_id = Factory::AddString (_options.sname);
	if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}
	_event.event_id = Factory::AddString (_options.ename);
	if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
		return false;
	}

	_event_address.setSender (_options.sname);
	_event_address.setEvents (_options.ename);

	ssi_event_adjust (_event, sizeof (ssi_event_tuple_t));
	ssi_event_tuple_t *t = ssi_pcast (ssi_event_tuple_t, _event.ptr);
	t->id = Factory::AddString (_options.vname);

	return true;
}

void EventSlider::send_enter() {

	ssi_char_t name[SSI_MAX_CHAR];
	ssi_sprint(name, "%s@%s", _options.ename, _options.sname);

	_slider = new Slider(name, _options.defval, _options.minval, _options.maxval, _options.steps);
	_slider->setCallback(this);

	_window = new Window();
	_window->setClient(_slider);
	ssi_rect_t rect = ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]);
	_window->setPosition(rect);
	_window->setStyle(IWindow::STYLES::NO_CLOSE | IWindow::STYLES::NO_MAXIMIZE);
	_window->create();
	_window->show();
}

void EventSlider::update (ssi_real_t value) {

	ssi_event_tuple_t *t = (ssi_event_tuple_t *) _event.ptr;
	t->value = value;

	if (_elistener) {
		_elistener->update (_event);
	}
}

void EventSlider::send_flush() {

	_window->close();
	delete _window; _window = 0;
	delete _slider; _slider = 0;	
}

}
