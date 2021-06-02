#pragma once
#ifndef _TextFaker_H
#define _TextFaker_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Timer.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

	class TextFaker : public IObject, public Thread
	{
		class Options : public OptionList {

		public:

			Options()
			{

				setSenderName("TextFaker");
				setEventName("NewInput");

				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");
				addOption("dimension", &inputMaxLength, 1, SSI_INT, "maximum input length in char");
			};

			void setSenderName(const ssi_char_t *sname) {
				if (sname) {
					ssi_strcpy(this->sname, sname);
				}
			}
			void setEventName(const ssi_char_t *ename) {
				if (ename) {
					ssi_strcpy(this->ename, ename);
				}
			}


			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];
			ssi_size_t inputMaxLength;
		};
	public:
		static const ssi_char_t *GetCreateName() { return "TextFaker"; };
		static IObject *Create(const ssi_char_t *file) { return new TextFaker(file); };

		TextFaker::Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Provides text input"; };

		void run();

		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

		bool setEventListener(IEventListener *listener);

		~TextFaker();

		bool exit;

	protected:
		TextFaker(const ssi_char_t *file = 0);
		TextFaker::Options _options;
		ssi_char_t *_file;
		void sendEvent();
		IEventListener *_elistener;
		ssi_event_t _event;
		EventAddress _eaddress;
		char *inputText;
		ssi_size_t _iml;
	};
}

#endif