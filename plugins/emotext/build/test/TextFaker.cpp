#include "TextFaker.h"
#include "base/Factory.h"
#include <stdio.h>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	TextFaker::TextFaker(const ssi_char_t *file) : _elistener(0), _file(0)
	{
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		ssi_event_init(_event, SSI_ETYPE_STRING,0,0,0,0,256);
		exit = false;
	}


	TextFaker::~TextFaker()
	{
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		ssi_event_destroy(_event);
	}


	void TextFaker::run() {
		scanf("%s", inputText);
		if (!exit)
		{
			if (std::strcmp(inputText, "exit") == 0) {
				exit = true;
				stop();
				ssi_print("Enter drücken zum beenden");
			}
			else if (std::strcmp(inputText, "") != 0) {
				ssi_print("Input erhalten:\n");
				ssi_char_t string[SSI_MAX_CHAR];
				ssi_sprint(string, inputText);
				ssi_print(string);
				strcpy(_event.ptr, inputText);
				_elistener->update(_event);
				exit = true;
			}
		}
	}

	void TextFaker::listen_enter() {
	
	}


	bool TextFaker::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {
		return false;
	}

	void TextFaker::listen_flush()
	{
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}

		if (_elistener) {
			ssi_event_reset(_event);
		}

		delete[] inputText;
	}

	bool TextFaker::setEventListener(IEventListener * listener) {
		_elistener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_eaddress.setSender(_options.sname);
		_eaddress.setEvents(_options.ename);

		_iml = _options.inputMaxLength;
		if (_elistener) {
			inputText = new char[_iml];
		}

		return true;
	}

}