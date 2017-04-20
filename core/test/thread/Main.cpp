// Main.cpp
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

#include "ioput/example/Exsemble.h"
#include "thread/ThreadPool.h"
using namespace ssi;

#include "TalkingThread.h"
#include "Store.h"
#include "User.h"
#include "Supplier.h"
#include "Printer.h"
#include "PrintJob.h"
#include "Clock.h"
#include "Trigger.h"
#include "Listener.h"
#include "Enqueuer.h"
#include "Dequeuer.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

bool ex_talking (void *arg);
bool ex_printer(void *arg);
bool ex_store(void *arg);
bool ex_timer(void *arg);
bool ex_tthread(void *arg);
bool ex_trigger(void *arg);
bool ex_queue(void *arg);
bool ex_pool(void *arg);

int main () {

#ifdef USE_SSI_LEAK_DETECTOR
	{
#endif

	ssi_print ("%s\n\nbuild version: %s\n\n", SSI_COPYRIGHT, SSI_VERSION);

#if SSI_RANDOM_LEGACY_FLAG	
	ssi_random_seed();
#endif

	Exsemble ex;
	ex.add(&ex_talking, 0, "TALKING", "Multiple threads sharing a variable.");
	ex.add(&ex_printer, 0, "PRINTER", "Multiple threads communicating with each other.");
	ex.add(&ex_store, 0, "STORE", "User-supplier example.");
	ex.add(&ex_timer, 0, "CLOCK", "How to use a clocked thread.");
	ex.add(&ex_trigger, 0, "TRIGGER", "Trigger-listener example.");
	ex.add(&ex_queue, 0, "QUEUE", "How to use a thread-safe queue.");
	ex.add(&ex_pool, 0, "POOL", "How to execute a pool of parallel jobs.");
	ex.show();

#ifdef USE_SSI_LEAK_DETECTOR
	}
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}

bool ex_talking (void *arg) {

	Counter counter;
	TalkingThread tt1 ("guten tag ", 700, &counter);

	tt1.setName ("guten tag");
	TalkingThread tt2 ("moin moin ", 1100, &counter);
	tt2.setName ("moin moin");
	TalkingThread tt3 ("gruess gott ", 1300, &counter);
	tt3.setName ("gruess gott");
	TalkingThread tt4 ("single hello ", 3000, &counter, true);
	tt4.setName ("single hello");

	ssi_print ("thread monitor:\n");
	Thread::PrintInfo ();

	tt1.start ();
	tt2.start ();
	tt3.start ();
	tt4.start ();

	ssi_print ("thread monitor:\n");
	Thread::PrintInfo ();

	// waits until tt4 has finished
	tt4.stop ();

	//ssi_print ("thread monitor:\n");
	//Thread::PrintInfo ();

	ssi_print ("\n\n\tpress enter to stop threads\n\n");
	getchar ();

	ssi_print ("thread monitor:\n");
	Thread::PrintInfo ();

	tt1.stop ();
	tt2.stop ();
	tt3.stop ();

	ssi_print ("thread monitor:\n");
	Thread::PrintInfo ();

	return true;
}

bool ex_printer (void *arg) {

	int job_num = 10;

	Printer printer;
	PrintJob **jobs = new PrintJob*[job_num];

	for (int i = 0; i < job_num; i++) {
		jobs[i] = new PrintJob (&printer, 3+i);
		jobs[i]->start();
	}

	ssi_print ("\n\n\tpress enter to stop threads\n\n");
	getchar ();

	for (int i = 0; i < job_num; i++) {
		jobs[i]->stop();
		delete jobs[i];
	}

	delete[] jobs;

	return true;
}

bool ex_store (void *arg) {

    int user_num = 7;
    int supp_num = 5;

	Store store;
	User **user = new User*[user_num];
	Supplier **supp = new Supplier*[supp_num];

	for (int i = 0; i < user_num; i++) {
		user[i] = new User (&store);
		user[i]->start();
	}
	for (int i = 0; i < supp_num; i++) {
		supp[i] = new Supplier (&store);
		supp[i]->start();
	}

	ssi_print ("\n\n\tpress enter to stop threads\n\n");
	getchar ();

	store.close ();

	for (int i = 0; i < user_num; i++) {
		user[i]->stop();
		delete user[i];
	}
	for (int i = 0; i < supp_num; i++) {
		supp[i]->stop();
		delete supp[i];
	}

	delete[] user;
	delete[] supp;

	return true;
}

bool ex_timer (void *arg) {

	Clock thread;
	thread.start ();

	ssi_print ("\n\n\tpress enter to stop thread\n\n");
	getchar ();

	thread.stop ();

	return true;
}

bool ex_trigger (void *arg) {

	Trigger trigger;
	Listener *listener[5];

	for (unsigned int i = 0; i < 5; i++) {
		listener[i] = new Listener (&trigger, i*i*100);
	}

	trigger.start ();

	for (unsigned int i = 0; i < 5; i++) {
		listener[i]->start();
	}

	ssi_print ("\n\n\tpress enter to stop threads\n\n");
	getchar ();

	for (unsigned int i = 0; i < 5; i++) {
		listener[i]->stop();
	}

	trigger.stop ();

	for (unsigned int i = 0; i < 5; i++) {
		delete listener[i];
	}

	return true;
}

bool ex_queue (void *arg) {

	Queue q(10);
	Enqueuer *e[5];
	Dequeuer *d[5];

	for (int i = 0; i < 5; i++) {
		e[i] = new Enqueuer (&q);
		d[i] = new Dequeuer (&q);
	}

	for (int i = 0; i < 5; i++) {
		e[i]->start();
		d[i]->start();
	}

	getchar ();

	for (int i = 0; i < 5; i++) {
		e[i]->stop();
		d[i]->stop();
	}

	for (int i = 0; i < 5; i++) {
		delete e[i];
		delete d[i];
	}

	return true;
}

struct pool_in_s {
	ssi_size_t id;
	ssi_size_t sleep_ms;
};

bool pool_job (ssi_size_t n_in, void *in, ssi_size_t n_out, void *out) {
	pool_in_s *s = ssi_pcast (pool_in_s, in);
	printf ("id=%u, sleep for %u ms\n", s->id, s->sleep_ms);
	Sleep (s->sleep_ms);
	return true;
}

bool ex_pool (void *arg) {

	ThreadPool tp ("mypool", 5);

	Randomi random(0, 500);

	ThreadPool::job_s job[20];
	pool_in_s job_in[20];
	for (ssi_size_t i = 0; i < 20; i++) {
		job_in[i].id = i;
		job_in[i].sleep_ms = 100u + ssi_size_t(random.next());
		job[i].n_in = sizeof (job_in[i]);
		job[i].in = &job_in[i];
		job[i].n_out = 0;
		job[i].out = 0;
		job[i].job = pool_job;
		tp.add (job[i]);
	}
	tp.work ();

	return true;
}
