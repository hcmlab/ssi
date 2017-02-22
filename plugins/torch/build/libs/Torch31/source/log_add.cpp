// Copyright (C) 2003--2004 Samy Bengio (bengio@idiap.ch)
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

#include "log_add.h"
#include "general.h"

namespace Torch {

#ifdef USE_DOUBLE
#define MINUS_LOG_THRESHOLD -39.14
#else
#define MINUS_LOG_THRESHOLD -18.42
#endif

real logAdd(real log_a, real log_b)
{
  if (log_a < log_b) {
    real tmp = log_a;
    log_a = log_b;
    log_b = tmp;
  }

  real minusdif = log_b - log_a;
#ifdef DEBUG
  if (isnan(minusdif))
    error("LogAdd: minusdif (%f) log_b (%f) or log_a (%f) is nan",minusdif,log_b,log_a);
#endif
  if (minusdif < MINUS_LOG_THRESHOLD)
    return log_a;
  else
    return log_a + log1p(exp(minusdif));
}

real logSub(real log_a, real log_b)
{
  if (log_a < log_b)
    error("LogSub: log_a (%f) should be greater than log_b (%f)", log_a, log_b);

  real minusdif = log_b - log_a;
#ifdef DEBUG
  if (isnan(minusdif))
    error("LogSub: minusdif (%f) log_b (%f) or log_a (%f) is nan",minusdif,log_b,log_a);
#endif
  if (log_a == log_b)
    return LOG_ZERO;
  else if (minusdif < MINUS_LOG_THRESHOLD)
    return log_a;
  else
    return log_a + log1p(-exp(minusdif));
}

}
