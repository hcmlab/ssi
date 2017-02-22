// Enqueuer.h
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

#ifndef _ENQUEUER_H
#define _ENQUEUER_H

#include "struct/Queue.h"
#include "thread/Thread.h"
using namespace ssi;

class Enqueuer : public Thread {

public:

	Enqueuer (Queue *queue) :
		_queue (queue) {
	};

	~Enqueuer () {
	};

	static void ToString (ssi_size_t size, ssi_char_t *string, void *xptr) {
		int x = *ssi_pcast (int, xptr);
		ssi_sprint (string, "%2d", x);
	}

	void run () {
		int *xptr = new int;
		*xptr = x++;
		ssi_print ("enqueue %2d\n", *xptr);
		_queue->enqueue (xptr);
		_queue->print (ToString);
		sleep_s (0.2 + static_cast<ssi_time_t> (rand ()) / RAND_MAX);		
	}

private:

	int static x;
	Queue *_queue;

};

int Enqueuer::x = 0;

#endif // _PRINTER_H
