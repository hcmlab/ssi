// ControlSlider.cpp
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

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "ControlSlider.h"
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

ssi_char_t *ControlSlider::ssi_log_name = "ctrslider_";

ControlSlider::ControlSlider (const ssi_char_t *file)
: _file (0),
	_slider (0),
	_window(0),
	_target(0),
	_ready(false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

ControlSlider::~ControlSlider () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool ControlSlider::start () {

	ssi_real_t value = _options.defval;

	ssi_msg(SSI_LOG_LEVEL_BASIC, "try to access option '%s:%s'", _options.id, _options.option);

	_target = Factory::GetObjectFromId(_options.id);
	if (_target) {
		IOptions *options = _target->getOptions();
		if (options) {
			ssi_option_t *o = options->getOption(_options.option);
			if (o) {
				if (o->lock) {
					ssi_wrn("option '%s:%s' is locked", _options.id, _options.option);
				} else {
					if (o->type == SSI_REAL && o->num == 1) {
						_ready = true;
						value = *(ssi_pcast(ssi_real_t, o->ptr));
					} else {
						ssi_wrn("option ''%s:%s'' is not a single real value", _options.id, _options.option);
					}
				}
			}
		}
	}

	ssi_char_t name[SSI_MAX_CHAR];
	ssi_sprint(name, "'%s:%s'", _options.id, _options.option);

	_slider = new Slider(_options.title[0] == '\0' ? name : _options.title, value, _options.minval, _options.maxval, _options.steps, _options.orientation);
	_slider->setCallback(this);

	ssi_sprint(name, "'%s:%s' (%f)", _options.id, _options.option, value);

	_window = new Window();
	_window->setClient(_slider);	
	_window->setPosition(ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]));
	_window->setTitle(_options.title[0] == '\0' ? name : _options.title);
	_window->create();
	_window->show();	

	return true;
}

void ControlSlider::update(ssi_real_t value) {
	
	if (_ready) {
		ssi_msg(SSI_LOG_LEVEL_DETAIL, "change option '%s:%s' to %.2f", _options.id, _options.option, value);
		_target->getOptions()->lock();
		_target->getOptions()->setOptionValue(_options.option, &value);
		_target->getOptions()->unlock();
		_target->notify(INotify::COMMAND::OPTIONS_CHANGE, _options.option);
	}
}

bool ControlSlider::stop() {

	_window->close();

	_target = 0;
	_ready = false;

	delete _window; _window = 0;
	delete _slider; _slider = 0;	

	return true;
}

bool ControlSlider::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) {
	case INotify::COMMAND::WINDOW_HIDE:
	{
		if (_window) {
			_window->hide();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_SHOW:
	{
		if (_window) {
			_window->show();
			return true;
		}
		break;
	}
	case INotify::COMMAND::WINDOW_MOVE:
	{
		if (_window) {
			return _window->setPosition(message);
		}
		break;
	}

	}

	return false;
}

}

#endif