// Rawinput.h
// author: Andreas Seiderer <seiderer@hcm-lab.de>
// created: 2015/08/10
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

#ifndef SSI_SENSOR_RAWINPUT_H
#define	SSI_SENSOR_RAWINPUT_H

#include "base/IProvider.h"
#include "thread/Timer.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

#include <sstream>

#include <Windows.h>
//#include <Hidpi.h>
#include <Hidsdi.h>

namespace ssi {


class Rawinput : public IObject, public Thread {

public:
	class Options : public OptionList {

	public:

		Options () {
			activedevices[0] = true;
			activedevices[1] = true;
			activedevices[2] = false;

			sendId = true;


			setSender("rawinput");

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption("activedevices", activedevices, 3, SSI_BOOL, "activate / disable devices [mouse, keyboard, joystick]");

			addOption("sendId", &sendId, 1, SSI_BOOL, "send hardware id in SSI event string for joystick / gamepad devices");

                };

		void setSender(const ssi_char_t *sname) {
			if (sname) {
				ssi_strcpy(this->sname, sname);
			}
		}

		void setDevices(bool *list) {
			if (list) {
				memcpy(activedevices, list, sizeof(bool) * 3);
			}
		}

		ssi_char_t sname[SSI_MAX_CHAR];
		bool activedevices[3];

		bool sendId;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Rawinput"; };
	static IObject *Create(const ssi_char_t *file) { return new Rawinput(file); };
	~Rawinput();
	Rawinput::Options *getOptions() { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Provides mouse cursor position and button state."; };

	bool setEventListener(IEventListener *listener);
	const ssi_char_t *getEventAddress() {
		return _eaddress.getAddress();
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}


	void enter();
	void run();
	bool stop();


	static Rawinput *inst;

	void sendMouseEvent(unsigned short usFlags, unsigned short usButtonFlags, unsigned short usButtonData, unsigned long ulRawButtons, long lLastX, long lLastY, unsigned long ulExtraInformation);
	void sendKeyboardEvent(unsigned short flags, unsigned short vkey, unsigned int message);
	void sendJoystickEvent(int* buttonStates, unsigned int buttoncount, const char* axes, char* id);

	

protected:

	Rawinput(const ssi_char_t *file = 0);
	Rawinput::Options _options;
	ssi_char_t *_file;

	int ssi_log_level;



	IEventListener *_elistener;
	ssi_event_t _event;
	EventAddress _eaddress;


	IEventListener *_elistener_key;
	ssi_event_t _event_key;

	IEventListener *_elistener_joy;
	ssi_event_t _event_joy;

	bool stop_loop;
};

}

#endif

