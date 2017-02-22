// Mutex.cpp
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

#include "thread/Mutex.h"
#if hasCXX11threads
    #include <mutex>
#endif // hasCXX11threads

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {
#if hasCXX11threads

Mutex::Mutex () {

}

Mutex::~Mutex () {

}

void Mutex::acquire () {
//printf("acquire: %x t: %x\n", &this->critSec, std::this_thread::get_id());

    critSec.lock();
    closed=true;
    tID=std::this_thread::get_id();
}

/* not compatible to vista
int Mutex::acquire_try () {
	return critSec.try_lock();
}*/

void Mutex::release () {
   // printf("release: %x t: %x\n", &this->critSec, std::this_thread::get_id());
    if(tID==std::this_thread::get_id()&&closed)
    {
        closed=false;
        critSec.unlock();
    }
    else
    {
        //ssi_wrn("you would crash");
    }
}
#else

Mutex::Mutex () {
	 ::InitializeCriticalSection (&critSec);
}

Mutex::~Mutex () {
	::DeleteCriticalSection (&critSec);
}

void Mutex::acquire () {
	::EnterCriticalSection (&critSec);
}

/* not compatible to vista
int Mutex::acquire_try () {
	return ::TryEnterCriticalSection (&critSec);
}*/

void Mutex::release () {
	::LeaveCriticalSection (&critSec);
}
#endif // hasCXX11threads
}
