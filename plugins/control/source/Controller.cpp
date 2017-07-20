// Controller.cpp
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

#include "Controller.h"
#include "base/Factory.h"
#include "graphic/Window.h"
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

ssi_char_t *Controller::ssi_log_name = "controller";

Controller::Controller (const ssi_char_t *file)
: _file (0),
	_console (0),
	_window(0),
	_object_ids(0),
	_n_object_ids(0),
	_n_buffer(0),
	_buffer(0),
	_n_history(0),
	_history(0),
	_history_insert(0),
	_history_last(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

Controller::~Controller () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool Controller::start () {

	_n_buffer = _options.buffer;
	_buffer = new ssi_char_t[_n_buffer + 1];
	_buffer[0] = '\0';

	_console = new TextBox("", true, _n_buffer);
	_console->setCallback(this);

	_n_history = _options.history;
	_history = new ssi_char_t *[_n_history];
	_history_insert = 0;
	_history_last = 0;
	for (ssi_size_t i = 0; i < _n_history; i++) {
		_history[i] = 0;
	}

	_window = new Window();
	_window->setClient(_console);	
	_window->setPosition(ssi_rect(_options.pos[0], _options.pos[1], _options.pos[2], _options.pos[3]));
	_window->setTitle(_options.title);
	_window->create();
	_window->show();	

	_n_object_ids = Factory::GetObjectIds(&_object_ids);

	return true;
}

bool Controller::stop() {

	_window->close();

	delete _window; _window = 0;
	delete _console; _console = 0;
	
	delete[] _buffer; _buffer = 0;
	_n_buffer = 0;

	for (ssi_size_t i = 0; i < _n_history; i++) {
		delete[] _history[i];
	}
	delete[] _history; _history = 0;
	_n_history = 0;
	_history_insert = 0;
	_history_last = 0;

	for (ssi_size_t i = 0; i < _n_object_ids; i++) {
		delete[] _object_ids[i];
	}
	delete[] _object_ids;
	_object_ids = 0;

	return true;
}

void Controller::mouseUp(ssi_size_t position, IWindow::KEY vkey) {
	_console->set(_console->get());
}

void Controller::showHistory(int move) {

	const ssi_char_t *entry = 0;

	for (ssi_size_t i = 0; i < _n_history; i++) {
		_history_last += move;
		if (_history_last < 0) {
			_history_last = _n_history - 1;
		} else if (_history_last >= (int)_n_history) {
			_history_last = 0;
		}
		if (entry = _history[_history_last]) {
			break;
		}
	}

	if (entry) {
		_console->set(entry);
	}
}

bool Controller::keyDown(IWindow::KEY key, IWindow::KEY vkey) {

	switch (key) {

	case VK_TAB:
		autoComplete();
		return false;

	case VK_RETURN:
		return false;

	case VK_UP:
		showHistory(-1);
		return false;	

	case VK_DOWN:
		showHistory(1);
		return false;
	}	

	return true;
}

ssi_char_t *Controller::lastLine(const ssi_char_t *text) {

	if (text[0] == '\0') {
		return ssi_strcpy(text);
	}

	if (text[ssi_strlen(text)] == '\n') {
		return ssi_strcpy("");
	}
	
	ssi_size_t n_lines = ssi_split_string_count(text, '\n');
	ssi_char_t **lines = new ssi_char_t *[n_lines];
	ssi_split_string(n_lines, lines, text, '\n');
	ssi_char_t *line = lines[n_lines - 1];

	for (ssi_size_t i = 0; i < n_lines - 1; i++) {
		delete[] lines[i];
	}
	delete[] lines;

	return line;
}

void Controller::autoCompleteObject(const ssi_char_t *id) {

	ssi_size_t n_id = ssi_strlen(id);

	_buffer[0] = '\0';
	ssi_size_t n_used = 0;

	ssi_size_t n_possible = 0;
	ssi_size_t last_id = 0;
	for (ssi_size_t i = 0; i < _n_object_ids; i++) {
		if (ssi_strcmp(SSI_FACTORY_DEFAULT_ID, _object_ids[i], true, ssi_strlen(SSI_FACTORY_DEFAULT_ID))) {
			continue;
		}
		if (n_id == 0 || ssi_strcmp(id, _object_ids[i], true, n_id)) {
			ssi_sprint(_buffer + n_used, "%s [%s]\r\n", _object_ids[i], Factory::GetObjectFromId(_object_ids[i])->isEnabled() ? "on" : "off");
			n_used = ssi_strlen(_buffer);
			n_possible++;
			last_id = i;
		}
	}

	if (n_possible == 0) {
		_console->set(id);
	}
	else if (n_possible == 1) {
		_console->set(_object_ids[last_id]);
	}
	else {
		ssi_sprint(_buffer + n_used, "\r\n%s", id);
		n_used = ssi_strlen(_buffer);
		_console->set(_buffer);
	}
}

void Controller::autoCompleteOption(const ssi_char_t *id, const ssi_char_t *arg) {

	IObject *object = Factory::GetObjectFromId(id);

	_buffer[0] = '\0';
	ssi_size_t n_used = 0;

	if (object) {

		IOptions *options = object->getOptions();

		ssi_size_t n_options = options->getSize();
		ssi_size_t n_possible = 0;
		for (ssi_size_t i = 0; i < n_options; i++) {
			ssi_option_t *option = options->getOption(i);
			if (option->lock) {
				continue;
			}
			if (arg == 0 || ssi_strcmp(arg, option->name, true, ssi_strlen(arg))) {
				ssi_char_t *value = 0;
				if (options->getOptionValueAsString(option->name, &value)) {
					ssi_sprint(_buffer + n_used, "%s=%s\r\n", option->name, value);
					n_used = ssi_strlen(_buffer);
					n_possible++;
				}
				delete[] value;
			}
		}

		if (n_possible == 0) {
			_console->set(id);
			if (arg) {
				_console->add(" ");
				_console->add(arg);
			}
		}
		else if (n_possible == 1) {
			_console->set(id);
			_console->add(" ");
			_console->add(_buffer);
		}
		else {
			ssi_sprint(_buffer + n_used, "\r\n%s %s", id, arg == 0 ? "" : arg);
			n_used = ssi_strlen(_buffer);
			_console->set(_buffer);
		}
	}
}

void Controller::autoComplete() {

	ssi_char_t *line = lastLine(_console->get());
	ssi_strtrim(line);
	
	ssi_size_t n_tokens = ssi_split_string_count(line, ' ');
	
	if (n_tokens == 0) {
		autoCompleteObject(line);
	} else {

		ssi_char_t **tokens = new ssi_char_t *[n_tokens];
		ssi_split_string(n_tokens, tokens, line, ' ');

		bool found = false;
		for (ssi_size_t i = 0; i < _n_object_ids; i++) {
			if (ssi_strcmp(tokens[0], _object_ids[i])) {
				if (n_tokens == 1) {
					autoCompleteOption(tokens[0], 0);
				}
				else {
					autoCompleteOption(tokens[0], tokens[1]);
				}
				found = true;
			}
		}

		if (!found) {
			autoCompleteObject(tokens[0]);
		}

		for (ssi_size_t i = 0; i < n_tokens; i++) {
			delete[] tokens[i];
		}
		delete[] tokens;
	}

	
	delete[] line;
}

IObject *Controller::parseCommand(const ssi_char_t *line, ssi_char_t **id, ssi_char_t **arg) {

	*id = 0;
	*arg = 0;

	if (!line || line[0] == '\0') {
		return 0;
	}

	ssi_size_t n_tokens = ssi_split_string_count(line, ' ');	
	ssi_char_t **tokens = new ssi_char_t *[n_tokens];
	ssi_split_string(n_tokens, tokens, line, ' ');

	*id = tokens[0];
	ssi_strtrim(*id);

	IObject *object = Factory::GetObjectFromId(*id);

	if (n_tokens > 1) {
		*arg = tokens[1];
		ssi_strtrim(*arg);
	}
	
	for (ssi_size_t i = 2; i < n_tokens; i++) {
		delete[] tokens[i];
	}
	delete[] tokens;

	return object;
}

bool Controller::sendCommand(IObject *object, const ssi_char_t *id, const ssi_char_t *arg) {

	if (ssi_strcmp(arg, "off", false)) {

		ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> go to sleep", id);

		object->notify(INotify::COMMAND::SLEEP_PRE);
		object->setEnabled(false);
		object->notify(INotify::COMMAND::SLEEP_POST);

	} else if (ssi_strcmp(arg, "on", false)) {

		ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> wake up", id);

		object->notify(INotify::COMMAND::WAKE_PRE);
		object->setEnabled(true);
		object->notify(INotify::COMMAND::WAKE_POST);

	} else if (ssi_strcmp(arg, "reset", false)) {

		ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> reset", id);
		
		object->notify(INotify::COMMAND::RESET);

	} else { // change options

		ssi_size_t n_tokens = ssi_split_string_count(arg, '=');

		if (n_tokens == 2) {

			ssi_char_t **tokens = new ssi_char_t *[n_tokens];
			ssi_split_string(n_tokens, tokens, arg, '=');

			ssi_char_t *name = tokens[0];
			ssi_char_t *value = tokens[1];

			ssi_strtrim(name);
			ssi_strtrim(value);

			IOptions *options = object->getOptions();
			ssi_option_t *option = options->getOption(name);

			if (option) {
				if (!option->lock) {

					ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> change option '%s=%s'", id, name, value);

					options->lock();
					options->setOptionValueFromString(name, value);
					options->unlock();

					object->notify(INotify::COMMAND::OPTIONS_CHANGE, name);
				} else {
					ssi_wrn("option '%s' is locked", name);
				}
			} else {
				ssi_wrn("an option '%s' does not exist", name);
			}

			for (ssi_size_t i = 0; i < n_tokens; i++) {
				delete[] tokens[i];
			}
			delete[] tokens;

		}
		else if (n_tokens == 1) { // message

			ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> send message '%s'", id, arg);

			object->notify(INotify::COMMAND::MESSAGE, arg);

		} else {

			return false;

		}
	}

	return true;
}

void Controller::update(const ssi_char_t *text) {

	ssi_char_t *line = lastLine(text);
	ssi_strtrim(line);

	if (!line || line[0] == '\0') {
		return;
	}

	ssi_char_t *id = 0;
	ssi_char_t *arg = 0;
	IObject *object = parseCommand(line, &id, &arg);

	bool result = false;

	if (object && arg) {
		result = sendCommand(object, id, arg);		
	}

	if (!result) {	
		ssi_wrn("could not parse command '%s'", line);
		delete[] line;
	} else {
		delete[] _history[_history_insert];
		_history[_history_insert++] = line;
		if (_history_insert >= (int)_n_history) {
			_history_insert = 0;
		}
	}

	delete[] id;
	delete[] arg;

	if (result) {
		_console->set("");
	}
}

bool Controller::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

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