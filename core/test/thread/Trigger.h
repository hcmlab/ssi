// Trigger.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/05/21
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

#pragma once

#include "thread/Thread.h"
using namespace ssi;

#include "ListenerInterface.h"

#define LISTENER_MAX 10

class Trigger : public Thread {

public:

	Trigger () {

		counter = 0;
		for (unsigned int i = 0; i < LISTENER_MAX; i++) {
			listener[i] = 0;
		}
	}

	~Trigger () {
	}

	bool attach (ListenerInterface *new_listener) {

		for (unsigned int i = 0; i < LISTENER_MAX; i++) {
			if (!listener[i]) {
				listener[i] = new_listener;
				return true;
			}
		}
		return false;
	}

	void detach (ListenerInterface *old_listener) {
		for (unsigned int i = 0; i < LISTENER_MAX; i++) {
			if (listener[i] && listener[i] == old_listener) {
				listener[i] = 0;		
			}
		}
	}

	void run () {
		::Sleep (rand () % 500);
		std::cout << "send trigger " << ++counter << std::endl;
		unsigned int sleep_between_update = 10;
		for (unsigned int i = 0; i < LISTENER_MAX; i++) {
			if (listener[i]) {
				listener[i]->update (counter);	
				sleep_ms (sleep_between_update);
			}
		}
	}

	void enter () {	
	}

	void flush () {
	}

private:

	unsigned int counter;
	ListenerInterface *listener[LISTENER_MAX];
};
