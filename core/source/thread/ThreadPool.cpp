// ThreadPool.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/07/17
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

#include "thread/ThreadPool.h"
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

ssi_char_t *ThreadPool::ssi_log_name = "threadpool";
int ThreadPool::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

// constructor
ThreadPool::ThreadPool (const ssi_char_t *name,
	ssi_size_t n_worker)
	: _start_time (0),
	_stop_time (0),
	_n_worker (n_worker) {

	_name = ssi_strcpy (name);
	_worker = new Worker[_n_worker];
	ssi_char_t string[SSI_MAX_CHAR];
	for (ssi_size_t i = 0; i < _n_worker; i++) {
		ssi_sprint (string, "%s#%02u", _name, i);
		_worker[i].setName (string);
	}
}

// deconstructor
ThreadPool::~ThreadPool () {

	delete[] _worker;
	delete[] _name;
}

void ThreadPool::add (job_s job) {

	job.success = false;
	_jobs.push_back (job);
}

void ThreadPool::clear () {
	_jobs.clear ();
}

bool ThreadPool::work () {

	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "start %d jobs '%s'", (int) _jobs.size (), _name);
	_start_time = ssi_time_ms ();

	std::vector<job_s>::iterator it;
	bool found = false;
	for (it = _jobs.begin (); it < _jobs.end (); it++) {
		found = false;
		while (!found) {
			for (ssi_size_t i = 0; i < _n_worker; i++) {
				if (_worker[i].ready ()) {
					_worker[i].set (&(*it));
					_worker[i].start ();
					found = true;
					break;
				}
			}
			#if __gnu_linux__
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			#else
			Sleep (10);
			#endif
		}
	}

	ssi_size_t count = 0;
	while (count < _n_worker) {
		count = 0;
		for (ssi_size_t i = 0; i < _n_worker; i++) {
			if (_worker[i].ready ()) {
				count++;
			}
		}
			#if __gnu_linux__
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			#else
			Sleep (10);
			#endif
	}

	bool success = true;
	ssi_size_t i = 0;
	for (it = _jobs.begin (); it < _jobs.end (); it++, i++) {
		if (!it->success) {
			success = false;
			ssi_wrn ("job #%u failed", i);
		}
	}

	ssi_char_t string[SSI_MAX_CHAR];
	_stop_time = ssi_time_ms ();
	ssi_time_sprint (_stop_time - _start_time, string);
	ssi_msg (SSI_LOG_LEVEL_DEFAULT, "finished %d jobs after %s '%s'", (int) _jobs.size (), string, _name);

	return success;
}

ThreadPool::Worker::Worker ()
	: Thread (true), _ready (true) {
}

void ThreadPool::Worker::set (job_s *job) {
	_job = job;
}

void ThreadPool::Worker::enter () {
	Lock lock (_mutex);
	_ready = false;
}

void ThreadPool::Worker::run () {

	_job->success = _job->job (_job->n_in, _job->in, _job->n_out, _job->out);
}

void ThreadPool::Worker::flush () {
	Lock lock (_mutex);
	_ready = true;
}

bool ThreadPool::Worker::ready () {
	bool ready = false;
	{
		Lock lock (_mutex);
		ready = _ready;
	}
	return ready;
}

}
