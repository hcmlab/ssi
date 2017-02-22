// Event.cpp
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

#include "thread/Event.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Event::Event (bool auto_block_after_release, bool start_in_blocked_mode) {
	
	#if _WIN32|_WIN64
	// create event
	handle = (ssi_handle_t) ::CreateEvent (0, !auto_block_after_release, !start_in_blocked_mode, 0);
	#else
	m_open=true;
    this->auto_block_after_release=auto_block_after_release;
    if(start_in_blocked_mode)block();
	#endif
}

Event::~Event () {
	#if _WIN32|_WIN64
	// close handle
	::CloseHandle ((HANDLE) handle);
	#else
	#endif
}

void Event::release () {

	#if _WIN32|_WIN64
	// put into signaled state
	::SetEvent((HANDLE)handle);
    #else
    {
    Lock lock(cond_lock);
    m_open = true;
    }

    if(auto_block_after_release)m_condition.notify_one();
    else m_condition.notify_all();
    #endif
}

void Event::block () {

	#if _WIN32|_WIN64
	// put into nonsignaled state
	::ResetEvent((HANDLE)handle);
	#else
    Lock lock(cond_lock);
    m_open=false;

	#endif
}

void Event::wait () {

	#if _WIN32|_WIN64
	// Wait until event is in signaled (green) state
	::WaitForSingleObject((HANDLE)handle, INFINITE);
	#else
    cond_lock.lock();
    while( !m_open )
	{
        m_condition.wait( cond_lock, [this] () -> bool {return m_open;} );
	}

    cond_lock.unlock();
    
    if(auto_block_after_release)m_open=false;
    

	
	#endif
}

}
