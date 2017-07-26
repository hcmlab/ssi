// ThreadSafe.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2017/07/26
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

#ifndef SSI_THREAD_THREADSAFE_H
#define SSI_THREAD_THREADSAFE_H

#include "thread/Lock.h"

// thread safe versions of ssi functions

namespace ssi
{
	class ThreadSafe
	{

	public:

		static void getcwd(ssi_size_t n, ssi_char_t *buffer);
		static void setcwd(const ssi_char_t *buffer);

	protected:

		static Mutex _workDirMutex;

	};
}

#endif