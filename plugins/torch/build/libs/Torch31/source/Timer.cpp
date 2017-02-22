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

#include "Timer.h"
#ifndef _MSC_VER
#include <sys/times.h>
#include <unistd.h>
#endif

namespace Torch {

#ifdef _MSC_VER
time_t Timer::base_time = 0;
#endif

real Timer::getRunTime()
{
#ifdef _MSC_VER
  time_t truc_foireux;
  time(&truc_foireux);
  return(difftime(truc_foireux, base_time));
#else
  struct tms current;
  times(&current);
  
  real norm = (real)sysconf(_SC_CLK_TCK);
  return(((real)current.tms_utime)/norm);
#endif
}

Timer::Timer()
{
#ifdef _MSC_VER
	while(!base_time)
		time(&base_time);
#endif
  total_time = 0;
  is_running = true;
  start_time = getRunTime();
}

void Timer::reset()
{
  total_time = 0;
  start_time = getRunTime();
}

void Timer::stop()
{
  if(!is_running)
    return;
  
  real current_time = getRunTime() - start_time;
  total_time += current_time;
  is_running = false;
}

void Timer::resume()
{
  if(is_running)
    return;

  start_time = getRunTime();
  is_running = true;
}

real Timer::getTime()
{
  if(is_running)
  {
    real current_time = getRunTime() - start_time;
    return(total_time+current_time);
  }
  else
    return total_time;
}

Timer::~Timer()
{
}

}
