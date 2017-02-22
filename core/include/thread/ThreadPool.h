// ThreadPool.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/07/12
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

#ifndef SSI_THREAD_THREADPOOL_H
#define SSI_THREAD_THREADPOOL_H

#include "thread/Thread.h"
#include "thread/ThreadLibCons.h"
#include "thread/Mutex.h"

namespace ssi {
 
class ThreadPool {

public:

	typedef bool (thread_pool_job)(ssi_size_t n_in, void *in, ssi_size_t n_out, void *out);

	struct job_s {
		thread_pool_job *job;
		ssi_size_t n_in;
		void *in;
		ssi_size_t n_out;
		void *out;
		bool success;
	};

protected:

	class Worker : public Thread {

	public:

		Worker ();
		void set (job_s *job);
		void enter ();
		void run ();
		void flush ();
		bool ready ();

	protected:

		job_s *_job;	
		Mutex _mutex;
		bool _ready;
	};

public:

	ThreadPool (const ssi_char_t *name,
		ssi_size_t n_threads);
	virtual ~ThreadPool ();

	void add (job_s job);	
	ssi_size_t size () {
		return ssi_cast (ssi_size_t, _jobs.size ());
	}
	job_s get (ssi_size_t i) {
		return _jobs[i];
	}
	bool work ();		
	void clear ();

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	ssi_char_t *_name;
	ssi_size_t _start_time;
	ssi_size_t _stop_time;

	std::vector<job_s> _jobs;
	ssi_size_t _n_worker;
	Worker *_worker;

};


}

#endif
