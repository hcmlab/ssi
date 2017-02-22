// Timer.cpp
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

#include "thread/Timer.h"

#if __gnu_linux__
#include <ctime>
#include <stdint.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#endif

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Timer::Timer(bool high_precision)
	: _highprec(high_precision),
	_delta (0),
	_next(0) {
#if _WIN32|_WIN64
	if (_highprec) {
		timeBeginPeriod(1);
	}
#endif
	reset();

}

Timer::Timer (ssi_size_t interval, bool high_precision)
: _highprec (high_precision){
#if _WIN32|_WIN64
	if (_highprec) {
		timeBeginPeriod (1);
	}
#endif
	setClockMs(interval);
	reset();
		


}

Timer::Timer (ssi_time_t timeout, bool high_precision)
: _highprec (high_precision){
#if _WIN32|_WIN64
	if (_highprec) {
		timeBeginPeriod (1);
	}
#endif
	setClockS(timeout);
	

	reset();



}

Timer::~Timer () {
#if _WIN32|_WIN64
	if (_highprec) {
		timeEndPeriod (1);
	}
#endif
}

void Timer::setClockS(ssi_time_t seconds) {

	_next = _delta = seconds;
}

void Timer::setClockMs(ssi_size_t milliseconds) {
	
	

	_next = _delta = milliseconds / 1000.0;
}

void Timer::setClockHz(ssi_time_t hz) {



	_next = _delta = 1.0 / hz;
}

void Timer::reset() {
#if _WIN32|_WIN64
	_init = ::timeGetTime();
#else
	uint32_t ms=0;
	timespec ts;
	clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
	ms= ts.tv_sec*1000+ (uint32_t)(ts.tv_nsec/1000000);
	_init =ms;

#endif
}

void Timer::wait () {
#if _WIN32|_WIN64
	_now = ::timeGetTime () - _init;
	ssi_size_t next_ms = ssi_cast (DWORD, 1000.0*_next + 0.5);
	if (_now < next_ms) {
		::Sleep (next_ms - _now);
	}
	_next += _delta;
#else
	uint64_t ms=0;
	timespec ts;
	clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
	

	
	ms= ts.tv_sec*1000+ (uint64_t)(ts.tv_nsec/1000000L);
	_now=ms - _init;
	ssi_size_t next_ms=1000.0*_next + 0.5;
	

	
	if (_now < next_ms) {
		
		std::this_thread::sleep_for(std::chrono::milliseconds(next_ms - _now));
		}
	_next += _delta;

#endif
}

}
