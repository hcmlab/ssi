// FrameLibCons.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/04
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

#ifndef SSI_FRAME_FRAMELIBCONS_H
#define	SSI_FRAME_FRAMELIBCONS_H

#include "SSI_Cons.h"
#include "thread/Mutex.h"
#include "thread/Thread.h"
#include "thread/Condition.h"
#include "thread/Lock.h"
#include "thread/Timer.h"

namespace ssi {

// 0 = no messages
// 1 = basic messages
// > 1 detailed messages
#define FRAME_DEBUG_LEVEL     0

// define for debug information
//#define FRAMEWORK_LOG
#define THEFRAMEWORK_LOGFILENAME "framework.log"

// in case that framework is in idle modus 
// requesting threads are put to sleep for a while
#define THEFRAMEWORK_SLEEPTIME_IF_IDLE 1

//! Max number of buffers managed by the framework
#define THEFRAMEWORK_BUFFER_NUM 128

//! default number of threads which can be handled by the framework
#define THEFRAMEWORK_THREAD_NUM 128

//! default number of components which can be handled by the framework
#define THEFRAMEWORK_COMPONENT_NUM 128

//! Default capacity of a buffer
#define THEFRAMEWORK_DEFAULT_BUFFER_CAP "10.0s"

//! Default duration until next watch check
#define THEFRAMEWORK_DEFAULT_WATCH_DUR "1.0s"

//! Default duration until next synchronized push call
#define THEFRAMEWORK_DEFAULT_SYNC_DUR "5.0s"

//! returned by a funciton if an error occured internal to the framework
#define THEFRAMEWORK_ERROR -1

//! default number of listener which can be attached to a trigger
#define SSI_TRIGGER_MAX_CONSUMER 50
#define SSI_TRIGGER_DEFAULT_MAX_DURATION 5



}

#endif

									
