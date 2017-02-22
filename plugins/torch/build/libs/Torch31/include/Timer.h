// Copyright (C) 2003--2004 Ronan Collobert (collober@idiap.ch)
//                
// This file is part of Torch 3.1.
//
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef TIMER_INC
#define TIMER_INC

#include "Object.h"

namespace Torch {

/** Timer... to measure time.
    It accumulates the time measured in several #resume()# (or
    constructor definition) and #stop()# calls.

    Use #getTime()# to know this accumulated time.

    @author Ronan Collobert (collober@idiap.ch)
*/
class Timer : public Object
{
#ifdef _MSC_VER
  private:
    static time_t base_time;
#endif
  public:
    bool is_running;
    real total_time;
    real start_time;

    /// Create the timer and start it now!
    Timer();

    /** Reset the timer. The timer will count time starting
        from now, and the accumulated time is erased.
    */
    void reset();

    /// Stop the timer. Updates accumulated time.
    void stop();

    /// Resume the timer. It will count time starting from now.
    void resume();

    /** Get the total accumulated time. (Until now, if the
        timer is still running.
    */
    real getTime();

    /** System dependent function which returns time elapsed
        since an arbitrary point reference in the past.
    */
    static real getRunTime();

    ~Timer();
};

}

#endif
