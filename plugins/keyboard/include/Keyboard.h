// Keyboard.h
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2015/10/29
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

// Provides live recording from Alive Heart Rate Monitor via Bluetooth

#pragma once

#ifndef SSI_SENSOR_KEYBOARDSENSOR_H
#define	SSI_SENSOR_KEYBOARDSENSOR_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Timer.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class Keyboard : public IObject, public Thread {

public:
		
	enum KeyMap
	{
		VirtualKey = 0,
		ScanCode = 1,
		DirectInput = 2,
		VirtualKey_DE = 3,
	};

	class Options : public OptionList {

	public:

		Options ()
			: keymap(KeyMap::VirtualKey), eager(true), waitForEnter(false), maxBuffer(256) {

			setSender("Keyboard");
			setEvent("KeyEvent");

			addOption ("KeyMap", &keymap, 1, SSI_INT, "key map to use (0 = VirtualKey, 1 = ScanCode, 2 = DirectInput, 3 = VirtualKey_DE)");
			addOption("Eager", &eager, 1, SSI_BOOL, "send incomplete events while key is pressed down (otherwise only a completed event will be send when key is released)");
			addOption("WaitForEnter", &waitForEnter, 1, SSI_BOOL, "buffer characters until user presses enter (see MaxBuffer)");			
			addOption("MaxBuffer", &maxBuffer, 1, SSI_SIZE, "maximal number of buffered characters");

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");
        };

		void setSender(const ssi_char_t *sname) {
			if (sname) {
				ssi_strcpy(this->sname, sname);
			}
		}
		void setEvent(const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy(this->ename, ename);
			}
		}
	
		KeyMap keymap;
		bool eager;
		bool waitForEnter;
		ssi_size_t maxBuffer;

		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
	};

	struct KeyEvent
	{
		int scanCode;
		int vkCode;
		bool isUp;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Keyboard"; };
	static IObject *Create (const ssi_char_t *file) { return new Keyboard (file); };
	~Keyboard ();
	Keyboard::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Provides keyboard button presses"; };

	void enter ();
	void run ();
	void flush();

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress() {
		return _eaddress.getAddress();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	static ssi_char_t ssi_log_name[];

	std::string getHex(int x);
protected:

	Keyboard (const ssi_char_t *file = 0);
	Keyboard::Options _options;
	ssi_char_t *_file;

	int ssi_log_level;

	void sendEvent(int scanCode, int vkCode, bool isUp);
	IEventListener *_elistener;
	ssi_event_t _event;
	EventAddress _eaddress;

	bool _new_word;
	ssi_size_t _n_word;
	ssi_size_t _n_word_buffer;
	ssi_char_t *_word_buffer;

	static HANDLE s_hThread;
	static HHOOK s_hHookKeyboard;
	static bool s_stopThread;
	static bool s_hookStarted;

	static bool s_newKeyEvent;
	static KeyEvent s_keyEvent;

	static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI MessageLoopThread(LPVOID lpThreadParameter);

	typedef std::map<int, std::string> inputKeyMapType;
	static inputKeyMapType s_directInputKeyMap;
	static inputKeyMapType s_virtualKeyMap;
	static inputKeyMapType s_virtualKeyMap_DE;
	static inputKeyMapType s_scanCodeKeyMap;
};

}

#endif

