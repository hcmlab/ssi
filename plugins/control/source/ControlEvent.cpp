// ControlEvent.cpp
// author: Dominik Schiller <wagner@hcm-lab.de>
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



#include "ControlEvent.h"
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

	ssi_char_t *ControlEvent::ssi_log_name = "ctrlevent";

	ControlEvent::ControlEvent(const ssi_char_t *file)
	: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	ControlEvent::~ControlEvent() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void ControlEvent::listen_enter() {
	}

	bool ControlEvent::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {
		ssi_event_t *e = 0;
		events.reset();
		for (ssi_size_t nevent = 0; nevent < n_new_events; nevent++){
			e = events.next();
			if (e->type == SSI_ETYPE_STRING){
				ssi_char_t *estr = ssi_pcast(ssi_char_t, e->ptr);

				ssi_msg(SSI_LOG_LEVEL_BASIC, "try to access option '%s'", estr);

				ssi_char_t *id = 0;
				ssi_char_t *arg = 0;
				IObject *object = parseCommand(estr, &id, &arg);

				bool result = false;

				if (object && arg) {
					result = sendCommand(object, id, arg);
				}

				if (!result) {
					ssi_wrn("could not parse command '%s'", estr);

				}
			}			
		}
		return true;
	}

	void ControlEvent::listen_flush() {
	}


	IObject *ControlEvent::parseCommand(const ssi_char_t *line, ssi_char_t **id, ssi_char_t **arg) {

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

	bool ControlEvent::sendCommand(IObject *object, const ssi_char_t *id, const ssi_char_t *arg) {

		if (ssi_strcmp(arg, "off", false)) {

			ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> go to sleep", id);

			object->notify(INotify::COMMAND::SLEEP_PRE);
			object->setEnabled(false);
			object->notify(INotify::COMMAND::SLEEP_POST);

		}
		else if (ssi_strcmp(arg, "on", false)) {

			ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> wake up", id);

			object->notify(INotify::COMMAND::WAKE_PRE);
			object->setEnabled(true);
			object->notify(INotify::COMMAND::WAKE_POST);

		}
		else if (ssi_strcmp(arg, "reset", false)) {

			ssi_msg(SSI_LOG_LEVEL_BASIC, "'%s' -> reset", id);

			object->notify(INotify::COMMAND::RESET);

		}
		else { // change options

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
					}
					else {
						ssi_wrn("option '%s' is locked", name);
					}
				}
				else {
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

			}
			else {

				return false;

			}
		}
		return true;
	}
}
