// Store.h
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

#ifndef _STORE_CPP
#define _STORE_CPP

#include "thread/Condition.h"
#include "thread/Mutex.h"
#include "thread/Lock.h"
using namespace ssi;

#include <iostream>
using namespace std;

#define MAX_STORAGE 5

class Store {

public:

	Store () : level (-1), store_open (true) {
	}

	~Store () {
	}

	void close () {

		// set signal to close store
		mutex.acquire ();
		store_open = false;
		mutex.release ();

		// awake all waiting threads
		empty.wakeAll ();
		full.wakeAll ();
	}

	void put (int goods) {

		// wait until store is ready to take the goods
		mutex.acquire ();
		while (level == MAX_STORAGE - 1 && store_open)
			full.wait (&mutex);

		if (!store_open) {
			// release mutex
			mutex.release ();
			// leave store
			return;
		}

		// now hand goods to store
		storage[++level] = goods;
                cout << "store received: " << goods << endl;
		
		// wake up waiting users
		empty.wakeSingle ();

		// release mutex
		mutex.release ();
	}

	int get () {

		// wait until store owns some goods
		mutex.acquire ();
                while (level == -1 && store_open)
                {

                        empty.wait (&mutex);
                        //printf("call me some\n");
                }
		
		if (!store_open) {
			// release mutex
			mutex.release ();
			// leave store
			return -1;
		}

		// now get the goods
		int goods = storage[level--];
                cout << "store supplied: " << goods << endl;

		// wake up waiting suppliers
		full.wakeSingle ();

		// release mutex
		mutex.release ();

		return goods;
	}

private:

	bool store_open;
	int level;
	int storage[MAX_STORAGE];
	Condition full;
	Condition empty;
	Mutex mutex;
};

#endif // _STORE_CPP
