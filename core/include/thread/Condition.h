// Condition.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/13
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
//
// for a discussion on the following implementation of a condition object using
// WIN32 have a look at http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
// and in particular 3.2. The SetEvent Solution

#pragma once

#ifndef SSI_THREAD_CONDITION_H
#define SSI_THREAD_CONDITION_H

#include "thread/ThreadLibCons.h"
#include "thread/Mutex.h"

#if hasCXX11threads
    #include<condition_variable>
    #include<chrono>
#endif // hasCXX11threads

namespace ssi {

class Condition {

// we maintain two different events
// events[SIGNAL]: an auto-reset event to wake up a single waiting thread
// events[BROADCAST]: a manual-reset event to wake up all waiting thread
enum EVENTS {
	SIGNAL = 0,
    BROADCAST,
    MAX_EVENTS
};

public:

	// constructor
	Condition ();

	// deconstructor
	~Condition ();

	// wait for condition
	void wait (Mutex *mutex);

	// wakes up a single waiting thread
	void wakeSingle ();

	// wakes up all waiting threads
	void wakeAll ();

private:



	// Counts the number of waiters
	int waiters_count;
	// Serialize access to waiter count
	Mutex waiters_count_mutex;

	#if hasCXX11threads

    std::condition_variable_any cond_var;
    std::mutex* my_mutex=NULL;

    #else

	// Signal and broadcast event HANDLEs
	ssi_handle_t events[Condition::MAX_EVENTS];

	#endif // hasCXX11threads
};

}

#endif // _CONDITION_H
