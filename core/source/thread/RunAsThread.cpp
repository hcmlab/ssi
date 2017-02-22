// RunAsThread.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/05/13
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

#include "thread/RunAsThread.h"
#include "thread/Lock.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *RunAsThread::ssi_log_name = "threadrun_";
int RunAsThread::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

RunAsThread::RunAsThread (fp_run_t fp, 
	void *ptr,
	bool single,
	ssi_size_t timeout_in_ms,
	const ssi_char_t *name)
	: Thread (single, timeout_in_ms) {

	setName(name);

	_fp = fp;
	_ptr = ptr;
}

void RunAsThread::run () {
	_fp (_ptr);
}

}
