// Timer.h
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

#ifndef SSI_THREAD_TIMER_H
#define SSI_THREAD_TIMER_H

#include "ThreadLibCons.h"

namespace ssi {

class Timer {

public:

	Timer(bool high_precision = false);
	Timer (ssi_size_t interval_in_ms, bool high_precision = false);
	Timer (ssi_time_t interval_in_s, bool high_precision = false);
	~Timer ();

	void setClockS(ssi_time_t seconds);
	void setClockMs(ssi_size_t milliseconds);
	void setClockHz(ssi_time_t hz);

	void reset ();
	void wait ();

protected:

	bool _highprec;
	ssi_time_t _delta;
	ssi_time_t _next;
	#if _WIN32|_WIN64
	DWORD _now;
	DWORD _init;
	#else
	uint32_t _now;
	uint32_t _init;
	#endif
};

}

#endif // _Timer_H
