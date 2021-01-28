// MyEmotext.h
// author: Dominik <dominik.schiller@student.uni-augsburg.de>
// created: 11/3/2015
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_EMOTEXT_MYEMOTEXT_H
#define SSI_EMOTEXT_MYEMOTEXT_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "Helper.h"


namespace ssi {

	class Emotext : public IObject {

	public:

		class Options : public OptionList {

		public:

			Options() {

				setSenderName("nsender");
				setEventName("nevent");
				setDicPath ("npath");

				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");
				addOption("dicpath", dicpath, SSI_MAX_CHAR, SSI_CHAR, "dictionary path");
			}

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
			void setDicPath(const ssi_char_t *dirNameGesticon) {
				if (dirNameGesticon) {
					ssi_strcpy(this->dicpath, dirNameGesticon);
				}
			}
			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];
			ssi_char_t dicpath[SSI_MAX_CHAR];
		};

	public:

		static const ssi_char_t *GetCreateName() { return "Emotext"; };
		static IObject *Create(const ssi_char_t *file) { return new Emotext(file); };
		~Emotext();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "just a sample object"; };

		virtual void print();

		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

		double getValence(const char* inputWords, std::list<word> *dictionary);
		bool Emotext::preprocessNegWords(const char* inputWords, char* outputWords);

		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}


	protected:

		Emotext(const ssi_char_t *file = 0);

		std::list<word> _dictionary;
		std::list<std::string> _negWords;

		ssi_char_t *_file;
		Options _options;
		static char ssi_log_name[];

		EventAddress _event_address;
		IEventListener *_listener;
		ssi_event_t _event;

		Helper *helper;

	};

}

#endif
