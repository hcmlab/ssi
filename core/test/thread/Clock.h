// Clock.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/05/21
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

#include "thread/ClockThread.h"
using namespace ssi;

class Clock : public ClockThread {

public:

	void enter() {
		setClockS(ssi_random(0.5, 1.0));
		_offset = ssi_time_ms();
	}

	virtual void clock () {
		printf("%u\n", ssi_time_ms() - _offset);
	}

protected:

	ssi_size_t _offset;

};
