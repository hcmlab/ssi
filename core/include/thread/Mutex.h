// Mutex.h
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



#ifndef SSI_THREAD_MUTEX_H
#define SSI_THREAD_MUTEX_H

#include "thread/ThreadLibCons.h"

#if hasCXX11threads
    #include <thread>
    #include <mutex>
    #include <chrono>
    #include <condition_variable>
#endif // hasCXX11threads


namespace ssi {

//! \brief A mutex class.
//!
class Mutex {

public:

	//! \brief Constructor
	//
	Mutex ();

	//! \brief Deconstructor
	//
	~Mutex ();

	//! \brief Acquire mutex
	//
	void acquire ();


	#if hasCXX11threads
        void lock(){acquire();}
        void unlock(){release();}



        #endif // hasCXX11threads

	//! \brief Release mutex
	//
	void release ();

private:

	// the mutex


    #if hasCXX11threads

        std::recursive_mutex critSec;
        std::thread::id tID;
        bool closed=false;



  //      (my_mutex, std::adopt_lock);
    #else
      CRITICAL_SECTION critSec;
    #endif // hasCXX11threads

};

}

#endif // _MUTEX_H
