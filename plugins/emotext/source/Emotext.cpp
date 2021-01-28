// MyEmotext.cpp
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

#include "Emotext.h"
#include <iostream>
#include <sstream>
#include "base/Factory.h"

namespace ssi {

	char Emotext::ssi_log_name[] = "emoText";

	Emotext::Emotext(const ssi_char_t *file)
		: _file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}

		helper = new Helper();
		helper->readDic("..\\resources\\angst.csv",  &_dictionary);
		helper->readNegWords("..\\resources\\NegatingWordList.txt", &_negWords);
		ssi_event_init(_event, SSI_ETYPE_STRING, 0, 0, 0, 0, 256);
	}

	Emotext::~Emotext() {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void Emotext::print() {

		ssi_print("%s says: Hello!\n", "MyEmotext");		
	}

	bool Emotext::setEventListener(IEventListener *listener) {

		_listener = listener;
		_event.sender_id = Factory::AddString(_options.sname);
		if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}
		_event.event_id = Factory::AddString(_options.ename);
		if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
			return false;
		}

		_event_address.setSender(_options.sname);
		_event_address.setEvents(_options.ename);

		return true;

	}

	void Emotext::listen_enter(){
	}

	bool Emotext::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {
		ssi_event_t *e = 0;
		events.reset();

		ssi_event_t **es = new ssi_event_t *[n_new_events];
		for (ssi_size_t i = 0; i < n_new_events; i++) {
			es[i] = events.next();
		}
 		for (ssi_size_t i = n_new_events; i > 0; i--)  {
			e = es[i - 1];
			
			if (_listener) {
				if (e->type == SSI_ETYPE_STRING){
					ssi_char_t *estr = ssi_pcast(ssi_char_t, e->ptr);
					ssi_print("\nDEBUG:\t%s", estr);
					char* stemmedWords = new char[sizeof(estr)* sizeof(char)];
					helper->stem(estr, stemmedWords, "en");
					double val = getValence(stemmedWords, &_dictionary);

					std::stringstream ss;
					ss << "Valence: " << val << std::endl;
					strcpy(_event.ptr, ss.str().c_str());
					_listener->update(_event);
				}
			}
		}

		return true;
	}
	void Emotext::listen_flush() {
	}


	double Emotext::getValence(const char* inputWords, std::list<word> *dictionary) {

		char* preprocessedInput = new char[strlen(inputWords) * sizeof(char) * 3];
		bool neg = preprocessNegWords(inputWords, preprocessedInput);

		char* tmpArray = new char[strlen(inputWords) * sizeof(char)];
		strcpy(tmpArray, inputWords);
		char* currentWord;
		currentWord = strtok(tmpArray, " ");
		double val = 0;
		while (currentWord != NULL)
		{
			std::string wordTmp = std::string(currentWord);
			for (std::list<word>::iterator it = dictionary->begin(); it != dictionary->end(); it++) {
				if (strcmp((it->englishStem).c_str(), currentWord) == 0) {
					val += neg ? -it->val : it->val;
					break;
				}
			}
			currentWord = strtok(NULL, " ");
		}
		return val;
	}

	bool Emotext::preprocessNegWords(const char* inputWords, char* outputWords) {
		std::string input = std::string(inputWords);
		std::string output;
		bool negFound = false;

		std::stringstream s(input);
		std::string word;
		// vector<string> c = ... ;

		bool retval = false;

		for (int i = 0; s >> word; i++) {

			if (negFound) {
				output.append("NOT");
				retval = true;
			}
			output.append(word + " ");

			for each (std::string neg in _negWords) {
				//std::cout << neg << std::endl;
				if (word.compare(neg) == 0) {
					negFound = !negFound;
					break;
				}
			}
		}
		strcpy(outputWords, output.c_str());
		//std::cout << "processed: " << output << std::endl;

		return retval;
	}


}
