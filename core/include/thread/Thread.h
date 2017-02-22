// Thread.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/12
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

#ifndef SSI_THREAD_THREAD_H
#define SSI_THREAD_THREAD_H

#include "base/IRunnable.h"
#include "thread/ThreadLibCons.h"
#include "thread/Event.h"
#include "thread/Mutex.h"

#if hasCXX11threads
    #include <thread>
    #include <chrono>
    #define Sleep( MSEC) std::this_thread::sleep_for( std::chrono::milliseconds( MSEC ) );
#endif // hasCXX11threads

#define SSI_THREAD_MONITOR_MAX 256

void myFunk();


namespace ssi {

//! \brief A thread class.
class Thread : public IRunnable {

private:

// struct with thread info
#ifdef _DEBUG
/*
	typedef struct tagTHREADNAME_INFO {
		  DWORD dwType; // must be 0x1000
		  LPCSTR szName; // pointer to name (in user addr space)
		  DWORD dwThreadID; // thread ID (-1=caller thread)
		  DWORD dwFlags; // reserved for future use, must be zero
	} THREADNAME_INFO;*/
#endif

public:

	Thread (bool single_execution = false,
		ssi_size_t timeout_in_ms = 10000);
	virtual ~Thread ();

	//! \brief Starts the thread
	//
	//! Executes the run method as own thread until stop () is called.
	//! Note: Does not return before enter () is finished.
	//
	bool start ();

	//! \brief Stops the thread
	//
	//! Stops execution of the run method.
	//! Note: Does not return before flush () is finished.
	//
	bool stop ();

	//! \brief Returns true if run() is called for the first time
	bool isFirstCycle();

	//! \brief Returns true if thread is active
	bool isActive ();

	ssi_size_t getElapsedTime () {
		if (_is_active) {
			return ssi_elapsed_ms (_start_time);
		} else {
			return _stop_time - _start_time;
		}
	}

	void setName (const ssi_char_t *name) {
		delete[] _name;
		_name = ssi_strcpy (name);
	}
	const ssi_char_t *getName () {
		return _name;
	}
	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

	static void PrintInfo (FILE *file = ssiout);

protected:

	static int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	//! \brief Executed before stop () waits for termination of the thread (virtual)
	//
	//! Gives user the chance to react to a stop () call,
	//! e.g. to wake up a thread if otherwise run () will never finish
	//
	virtual void terminate () {};

	//! \brief Executed before run () is invoked (virtual)
	//
	virtual void enter () {};

	//! \brief Executed after run () has terminated (virtual)
	//
	virtual void flush () {};

	//! \brief Run method (virtual)
	//
	virtual void run () = 0;

	//! \brief Put thread to sleep
	//
	//! milliseconds		sleep time in milliseconds
	//
	void sleep_ms (ssi_size_t milliseconds);

	//! \brief Put thread to sleep
	//
	//! milliseconds		sleep time in seconds
	//
	void sleep_s (ssi_time_t seconds);

	//! Set to true when run () is called for the first time and set to false afterwards
	bool _is_first_cycle;

	//! Set to true when start () is called and set to false
	//! when stop () is called. Signals threadfun () to terminate.
	bool _is_active;

	//! If set to true, run() is executed once
	//! otherwise run() is looped until stop() is called
	bool _single_execution;

	// enters the thread
	#if hasCXX11threads
	static void threadFun (void * pArg);
	#else
	static unsigned int WINAPI threadFun (void * pArg);
	#endif // hasCXX11threads

	// function to set thread name
#ifdef _DEBUG
	//void SetThreadName (DWORD dwThreadID, LPCSTR szThreadName);
#endif
    #if hasCXX11threads
    std::thread* _handle;
    std::thread::id _tid;
    #else
	ssi_handle_t _handle;  // thread handle
	unsigned int _tid;     // thread id
	#endif // hasCXX11threads
	Event _event;   // thread event
	ssi_size_t _timeout; // _timeout in milliseconds
	Mutex _mutex; // thread mutex
	ssi_char_t *_name; // thread name
	ssi_size_t _start_time; // thread start time
	ssi_size_t _stop_time; // thread stop time
	ssi_size_t _monitor_id; // monitor id

	// monitor
	static Thread *_monitor[SSI_THREAD_MONITOR_MAX];
	static ssi_size_t _monitor_counter;
	static Mutex _monitor_mutex;
};


}

#endif // _THREAD_H
