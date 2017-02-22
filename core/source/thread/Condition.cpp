// Condition.cpp
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

#include "thread/Condition.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

// constructor
Condition::Condition ()
: waiters_count (0) {
#if hasCXX11threads

#else
	// Create an auto-reset event.
	events[Condition::SIGNAL] = ::CreateEvent (NULL,  // no security
		FALSE, // auto-reset event
		FALSE, // non-signaled initially
		NULL); // unnamed

  // Create a manual-reset event.
	events[Condition::BROADCAST] = ::CreateEvent (NULL,  // no security
		TRUE,  // manual-reset
		FALSE, // non-signaled initially
		NULL); // unnamed
#endif // hasCXX11threads

}

// deconstructor
Condition::~Condition () {
}




// wait for condition
void Condition::wait (Mutex *mutex) {


#if hasCXX11threads


	


     cond_var.wait(*mutex);



#else

// Avoid race conditions.
	waiters_count_mutex.acquire ();
	waiters_count++;
	waiters_count_mutex.release ();

	// It's ok to release the <external_mutex> here since Win32
	// manual-reset events maintain state when used with
	// <SetEvent>.  This avoids the "lost wakeup" bug...
	mutex->release ();

	int result = ::WaitForMultipleObjects (2, // Wait on both events
		(HANDLE *) events, // events
		FALSE, // Wait for either event to be signaled
		INFINITE); // Wait "forever"


	waiters_count_mutex.acquire ();
	waiters_count--;
	bool last_waiter = result == WAIT_OBJECT_0 + Condition::BROADCAST && waiters_count == 0;
	waiters_count_mutex.release ();

	// Some thread called <pthread_cond_broadcast>.
	if (last_waiter)
		// We're the last waiter to be notified or to stop waiting, so
		// reset the manual event.
		::ResetEvent ((HANDLE) events[Condition::BROADCAST]);
// hasCXX11threads
	// Reacquire the <external_mutex>.
	mutex->acquire ();
#endif
}



// wakes up a single waiting thread
void Condition::wakeSingle () {

#if hasCXX11threads

        cond_var.notify_one();

#else
    	// Avoid race conditions.
	waiters_count_mutex.acquire ();
	bool have_waiters = waiters_count > 0;
	waiters_count_mutex.release ();

	if (have_waiters)
		::SetEvent((HANDLE) events[Condition::SIGNAL]);
#endif // hasCXX11threads
}




// wakes up all waiting threads
void Condition::wakeAll () {


#if hasCXX11threads

        cond_var.notify_all();

#else
	// Avoid race conditions.
	waiters_count_mutex.acquire ();
	bool have_waiters = waiters_count > 0;
	waiters_count_mutex.release ();


	if (have_waiters)
		::SetEvent ((HANDLE) events[Condition::BROADCAST]);
#endif // hasCXX11threads
}
}
