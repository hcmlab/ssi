// TalkingThread.h
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

#include "Counter.h"

#include "thread/Thread.h"
using namespace ssi;

#include <iostream>
using namespace std;

class TalkingThread : public Thread {

public:

	TalkingThread (const char *text, int time, Counter *counter_, bool single_execution = false)
		: Thread (single_execution), wait (time), counter (counter_) {
		voice = new char[strlen (text) + 1];
		strcpy (voice, text);
	}

	~TalkingThread () {
		delete voice;
	}

	virtual void run () {
		::Sleep (wait);
		cout << counter->increase () << ": " << voice << endl; 
	}

	virtual void enter () {
		cout << "enter " << voice << endl; 	
	}

	virtual void flush () {
		cout << "flush " << voice << endl;
	}

private:

	char *voice;
	int wait;
	Counter *counter;
};
