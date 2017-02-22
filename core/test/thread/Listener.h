// Listener.h
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

#include "Trigger.h"

#include "thread/Event.h"
#include "thread/Lock.h"
#include "thread/Thread.h"
using namespace ssi;

class Listener : public Thread, public ListenerInterface {

public:

	Listener (Trigger *trigger_, unsigned int sleep_before_listen_) 
		: trigger (trigger_),
		sleep_before_listen (sleep_before_listen_),
		event (true, true) {
	}

	~Listener () {
	}

	void update (unsigned int message_) {

		{
			Lock lock (message_mutex);
			message = message_;		
		}
		event.release ();
	}

	void run () {
		
		if (listening) {
			event.wait ();
			unsigned int local_message;
			{
				Lock lock (message_mutex);
				local_message = message;		
			}
			std::cout << this << ": " << local_message << std::endl;
		} else {
			std::cout << this << ": unexpected error, not listening" << std::endl;
		}

		sleep_ms (sleep_before_listen);
	}

	void enter () {
		listening = trigger->attach (this);
	}

	void flush () {
		trigger->detach (this);
	}

private:

	bool listening;
	unsigned int sleep_before_listen;
	unsigned int message;
	Trigger *trigger;
	Event event;
	Mutex message_mutex;
};
