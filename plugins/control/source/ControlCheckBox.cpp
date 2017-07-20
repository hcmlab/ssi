// ControlCheckBox.cpp
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

#include "ControlCheckBox.h"
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

ssi_char_t *ControlCheckBox::ssi_log_name = "ctrlcheck_";

ControlCheckBox::ControlCheckBox (const ssi_char_t *file)
: _file (0),
	_checkbox (0),
	_window(0),
	_n_targets(0),
	_targets(0),
	_target_ids(0),
	_ready(false),
	_is_option(false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

ControlCheckBox::~ControlCheckBox () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool ControlCheckBox::start () {

	bool value = _options.value;
	_is_option = _options.option[0] != '\0';	
	
	_n_targets = Factory::GetObjectIds(&_target_ids, _options.id);
	if (_n_targets > 0) {
		
		_targets = new IObject *[_n_targets];
		_ready = new bool[_n_targets];
		for (ssi_size_t i = 0; i < _n_targets; i++) {
			ssi_strtrim(_target_ids[i]);
			if (_is_option) {
				ssi_msg(SSI_LOG_LEVEL_BASIC, "try to access option '%s:%s'", _target_ids[i], _options.option);
			} else {
				ssi_msg(SSI_LOG_LEVEL_BASIC, "try to access object '%s'", _target_ids[i]);
			}
			_targets[i] = Factory::GetObjectFromId(_target_ids[i]);
			IObject *_target = _targets[i];
			if (_target && _is_option) {
				_ready[i] = false;
				IOptions *options = _target->getOptions();
				if (options) {
					ssi_option_t *o = options->getOption(_options.option);
					if (o) {
						if (o->lock) {
							ssi_wrn("option '%s:%s' is locked", _target_ids[i], _options.option);
						}
						else {
							if (o->type == SSI_BOOL && o->num == 1) {
								_ready[i] = true;
								value = *(ssi_pcast(bool, o->ptr));
							}
							else {
								ssi_wrn("option ''%s:%s'' is not a single boolean value", _target_ids[i], _options.option);
							}
						}
					}
				}
			}
		}
	}
	ssi_char_t name[SSI_MAX_CHAR];
	if (_is_option) {
		ssi_sprint(name, "%s", _options.option);
	} else {
		ssi_sprint(name, "enabled");
	}

	_checkbox = new CheckBox(_options.label, value);
	_checkbox->setCallback(this);

	ssi_sprint(name, "%s", _options.id);

	_window = new Window();
	_window->setClient(_checkbox);
	_window->setPosition(ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]));
	_window->setTitle(_options.title);
	_window->create();
	_window->show();	

	return true;
}

void ControlCheckBox::update(bool value) {
	
		if (_is_option) {			
			for (ssi_size_t i = 0; i < _n_targets; i++) {
				if (_targets[i] && _ready[i]) {					
					ssi_msg(SSI_LOG_LEVEL_DETAIL, "change option(s) '%s:%s' to %s", _target_ids[i], _options.option, value ? "true" : "false");
					_targets[i]->getOptions()->lock();
					_targets[i]->getOptions()->setOptionValue(_options.option, &value);
					_targets[i]->getOptions()->unlock();
					_targets[i]->notify(INotify::COMMAND::OPTIONS_CHANGE, _options.option);
				}
			}
		} else {			
			for (ssi_size_t i = 0; i < _n_targets; i++) {					
				if (_targets[i]) {
					ssi_msg(SSI_LOG_LEVEL_DETAIL, "turn object '%s' %s", _target_ids[i], value ? "on" : "off");
					_targets[i]->notify(value ? INotify::COMMAND::WAKE_PRE : INotify::COMMAND::SLEEP_PRE);
					_targets[i]->setEnabled(value);
					_targets[i]->notify(value ? INotify::COMMAND::WAKE_POST : INotify::COMMAND::SLEEP_POST);
				}
			}
		}
}

bool ControlCheckBox::stop() {

	_window->close();

	delete[] _targets; _targets = 0;
	delete[] _ready; _ready = 0;
	for (ssi_size_t i = 0; i < _n_targets; i++) {
		delete[] _target_ids[i];
	}
	delete[] _target_ids;
	_target_ids = 0;
	_n_targets = 0;
	_is_option = false;

	delete _window; _window = 0;
	delete _checkbox; _checkbox = 0;

	return true;
}

bool ControlCheckBox::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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