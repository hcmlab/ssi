// MyThread.cpp
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

#include "MyThread.h"

int MyThread::_counter = 0;
Mutex MyThread::_mutex;

MyThread::MyThread (const ssi_char_t *msg, 
	ssi_size_t sleep_in_ms, 
	bool single_execution)
	: Thread (single_execution), 
	_sleep_in_ms (sleep_in_ms) {
	_msg = ssi_strcpy (msg);

	setName (msg);
}

MyThread::~MyThread () {

	delete[] _msg;
}

void MyThread::run () {

	sleep_ms (_sleep_in_ms);
	{
		Lock lock (_mutex);
		ssi_print ("%d: %s\n", ++_counter, _msg); 
	}
}

void MyThread::enter () {

	ssi_print ("enter %s\n", _msg); 	
}

void MyThread::flush () {

	ssi_print ("flush %s\n", _msg); 	
}

