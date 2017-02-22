// ControlGrid.cpp
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

#include "ControlGrid.h"
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

ssi_char_t *ControlGrid::ssi_log_name = "ctrlgrid__";

ControlGrid::ControlGrid (const ssi_char_t *file)
: _file (0),
	_tab(0),
	_grid(0),
	_window(0),
	_n_targets(0),
	_targets(0),
	_target_ids(0),
	_selected(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

ControlGrid::~ControlGrid () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

bool ControlGrid::start () {

	bool value = _options.value;

	if (!_options.id || _options.id[0] == '\0') {
		ssi_char_t **names;
		ssi_size_t n_names = Factory::GetObjectIds(&names);
		_n_targets = 0;
		for (ssi_size_t i = 0; i < n_names; i++) {
			if (!ssi_strcmp(names[i], SSI_FACTORY_DEFAULT_ID, false, ssi_strlen(SSI_FACTORY_DEFAULT_ID))) {
				_n_targets++;
			}
		}
		if (_n_targets > 0) {
			_target_ids = new ssi_char_t *[_n_targets];
			ssi_size_t count = 0;
			for (ssi_size_t i = 0; i < n_names; i++) {
				if (!ssi_strcmp(names[i], SSI_FACTORY_DEFAULT_ID, false, ssi_strlen(SSI_FACTORY_DEFAULT_ID))) {
					_target_ids[count++] = names[i];
				} else {
					delete[] names[i];
				}
			}
		} else {
			for (ssi_size_t i = 0; i < n_names; i++) {
				delete[] names[i];
			}
		}

		delete[] names;

	} else {
		_n_targets = Factory::GetObjectIds(&_target_ids, _options.id);
	}

	if (_n_targets > 0) {
		_targets = new IObject *[_n_targets];
		for (ssi_size_t i = 0; i < _n_targets; i++) {
			ssi_strtrim(_target_ids[i]);
			ssi_msg(SSI_LOG_LEVEL_BASIC, "try to access object '%s'", _target_ids[i]);
			_targets[i] = Factory::GetObjectFromId(_target_ids[i]);
			IObject *_target = _targets[i];
		}
	}

	_tab = new Tab("Control");
	_tab->setCallback(this);

	_grid = new Grid *[_n_targets];
	for (ssi_size_t i = 0; i < _n_targets; i++) {
		_grid[i] = new Grid(_target_ids[i]);
		_grid[i]->setCallback(this);
		_tab->addClient(_target_ids[i], _grid[i]);
	}

	_window = new Window();
	_window->setClient(_tab);
	_window->setPosition(ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]));
	_window->setLimits(200, 0, 200, 0);
	_window->setTitle(_options.title);
	_window->create();
	_window->show();	

	for (ssi_size_t i = 0; i < _n_targets; i++) {
		_grid[i]->setHeaderRowHeight(0);
		_grid[i]->setRowsNumbered(false);
		_grid[i]->setEditable(true);
		_grid[i]->setProtectColor(ssi_rgb(128, 128, 128));
		_grid[i]->setShowHilight(false);
		_grid[i]->setColumnWidth(0, 75);
		_grid[i]->setExtendLastColumn(true);
		fillGrid(_grid[i], _targets[i]);
	}

	_selected = 0;

	return true;
}

void ControlGrid::fillGrid(Grid *grid, IObject *object) {

	if (!object) {
		return;
	}

	IOptions *options = object->getOptions();
	ssi_size_t n_editable = 0;
	ssi_size_t n_options = 0;
	
	if (options) {
		n_options = options->getSize();
		for (ssi_size_t i = 0; i < n_options; i++) {
			if (!options->getOption(i)->lock) {
				n_editable++;
			}
		}
	}

	if (_options.showEnabled) {
		grid->setGridDim(1 + (n_editable > 0 ? n_editable + 1 : 0), 1);
		grid->setText(1, 0, "Enabled");
		grid->setCheckBox(1, 1, object->isEnabled());
	} else {
		grid->setGridDim(n_editable, 1);
	}

	if (n_editable > 0) {

		ssi_size_t count = 0;
		if (_options.showEnabled) {
			grid->setText(2, 1, "", false);
			count = 2;
		}

		for (ssi_size_t i = 0; i < n_options; i++) {
			ssi_option_t *option = options->getOption(i);
			if (option->lock) {
				continue;
			}
			++count;
			grid->setText(count, 0, option->name, false);						
			ssi_char_t *value = 0;
			if (options->getOptionValueAsString(option->name, &value, _options.precision)) {				
				grid->setText(count, 1, value, true);
			}
			delete[] value;			
		}
	}
}

bool ControlGrid::isNumber(const ssi_char_t *s)
{	
	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;
	char * p;
	strtod(s, &p);
	return *p == '\0';
}

void ControlGrid::update(ssi_size_t row, ssi_size_t column, const ssi_char_t *text) {

	if (_n_targets == 0) {
		return;
	}

	IObject *object = _targets[_selected];
	IOptions *options = object->getOptions();

	if (!options) {
		return;
	}

	ssi_char_t name[SSI_MAX_CHAR];	
	_grid[_selected]->getText(row, 0, SSI_MAX_CHAR, name);

	if (name) {

		ssi_option_t *option = options->getOption(name);

		if (!option) {
			return;
		}

		bool result = false;
		options->lock();
		result = options->setOptionValueFromString(option->name, text);
		options->unlock();
		if (result) {
			object->notify(INotify::COMMAND::OPTIONS_CHANGE, option->name);
		} else {
			ssi_char_t *value = 0;
			if (options->getOptionValueAsString(option->name, &value, _options.precision)) {
				_grid[_selected]->setText(row, column, value);
			}
			delete[] value;
		}
	}
}

void ControlGrid::update(ssi_size_t row, ssi_size_t column, bool checked) {

	if (_n_targets == 0) {
		return;
	}

	IObject *object = _targets[_selected];

	if (_options.showEnabled && row == 1) {

		if (checked) {
			object->notify(INotify::COMMAND::WAKE_PRE);
			object->setEnabled(checked);
			object->notify(INotify::COMMAND::WAKE_POST);
		} else {
			object->notify(INotify::COMMAND::SLEEP_PRE);
			object->setEnabled(checked);
			object->notify(INotify::COMMAND::SLEEP_POST);
		}

	} else {

		IOptions *options = object->getOptions();

		if (!options) {
			return;
		}

		ssi_char_t name[SSI_MAX_CHAR];
		_grid[_selected]->getText(row, 0, SSI_MAX_CHAR, name);

		if (name) {

			ssi_option_t *option = options->getOption(name);

			if (!option) {
				return;
			}

			options->lock();
			options->setOptionValue(option->name, &checked);
			options->unlock();
			object->notify(INotify::COMMAND::OPTIONS_CHANGE, option->name);
		}
	}
}

void ControlGrid::update(ssi_size_t index) {
	_selected = index;
}

bool ControlGrid::stop() {

	_window->close();

	delete[] _targets; _targets = 0;
	for (ssi_size_t i = 0; i < _n_targets; i++) {
		delete[] _grid[i];
		delete[] _target_ids[i];
	}
	delete[] _target_ids;
	_target_ids = 0;
	_n_targets = 0;

	delete _window; _window = 0;
	delete _grid; _grid = 0;
	delete _tab; _tab = 0;

	return true;
}

bool ControlGrid::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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