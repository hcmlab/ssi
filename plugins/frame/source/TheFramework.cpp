// TheFramework.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2007/11/23 
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

#include "TheFramework.h"
#include "FrameMonitor.h"
#include "TimeServer.h"
#include "thread/Mutex.h"
#include "Decorator.h"

#include <fstream>
#include <iomanip>
//#define FRAMEWORK_LOG 1

#ifdef FRAMEWORK_LOG
static std::ofstream logfile;
static ssi::Mutex logmutex;
#endif
#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *TheFramework::ssi_log_name = "pipeline__";
ssi_char_t *TheFramework::info_file_name = "pipeline.info";

const ssi_char_t *TheFramework::SYNC_MSG_HEAD_STR = "SSI";
const ssi_size_t TheFramework::SYNC_MSG_HEAD_SIZE = 3;

const ssi_size_t TheFramework::SYNC_MSG_TYPE_SIZE = 4;
const ssi_size_t TheFramework::SYNC_MSG_TYPE_POS = SYNC_MSG_HEAD_SIZE+1;
const ssi_char_t *TheFramework::SYNC_MSG_TYPE_STR[TheFramework::SYNC_MSG_TYPE::NUM] = {
	"STRT",
	"STOP",
};
const ssi_char_t *TheFramework::SYNC_MSG_TYPE_STRLONG[TheFramework::SYNC_MSG_TYPE::NUM] = {
	"START",
	"STOP",
};

const ssi_size_t TheFramework::SYNC_MSG_ID_SIZE = 4;
const ssi_size_t TheFramework::SYNC_MSG_ID_POS = SYNC_MSG_TYPE_POS + SYNC_MSG_TYPE_SIZE + 1;
const ssi_char_t *TheFramework::SYNC_MSG_ID_STR[TheFramework::SYNC_MSG_ID::NUM] = {
	"QUIT",
	"RUN1",
	"RUNN",
};
const ssi_char_t *TheFramework::SYNC_MSG_ID_STRLONG[TheFramework::SYNC_MSG_ID::NUM] = {
	"QUIT         ",
	"RUN & QUIT   ",
	"RUN & RESTART",
};

const ssi_size_t TheFramework::SYNC_MSG_TOKENS = 3;
const ssi_char_t TheFramework::SYNC_MSG_DELIM = ':';
const ssi_size_t TheFramework::SYNC_MSG_SIZE = SYNC_MSG_ID_POS + SYNC_MSG_ID_SIZE + 1; // ssi + delim + type + delim + id + \0

// constructor
TheFramework::TheFramework (const ssi_char_t *file)
: _monitor (0),
	 _file (0),
	 _last_run_time (0),
	 _start_run_time (0),
	 _info (0),
	 _is_running (false),
	 _is_auto_run (true),
	 _tserver (0),
	 _sync_socket (0),
	 _sync_msg_id(SYNC_MSG_ID::RUN_AND_QUIT),
	 _cancel_wait(false),
	 ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {
#if _WIN32|_WIN64
	memset (&_system_time, 0, sizeof (_system_time));
	memset (&_local_time, 0, sizeof (_system_time));
#else
	_system_time=0;
	_local_time=0;
#endif

#ifdef FRAMEWORK_LOG 
{
	Lock lock (logmutex);
	logfile.open (THEFRAMEWORK_LOGFILENAME);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Initialize framework.." << std::endl;
}
#endif

	// mark all buffers as unused
    for (int i = 0; i < THEFRAMEWORK_BUFFER_NUM; i++)
        bufferInUse[i] = false;

	// set all threads to 0
	for (int i = 0; i < THEFRAMEWORK_THREAD_NUM; i++)
		runnable[i] = 0;

	// set all component to 0
	for (int i = 0; i < THEFRAMEWORK_COMPONENT_NUM; i++)
		component[i] = 0;
	component_counter = 0;
	
	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Framework has been initialized !" << std::endl;
}
#endif
}

// deconstructor
TheFramework::~TheFramework () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void TheFramework::SetLogLevel (int level) {
	Consumer::SetLogLevel (level);
	ConsumerBase::SetLogLevel (level);
	Transformer::SetLogLevel (level);
	Provider::SetLogLevel (level);
}

bool TheFramework::recv_sync_msg(Socket *socket, TheFramework::SYNC_MSG_TYPE::List &type, TheFramework::SYNC_MSG_ID::List &id)
{
	ssi_byte_t message[SYNC_MSG_SIZE];
	message[0] = '\0';

	int result = socket->recv(message, SYNC_MSG_SIZE);
	if (result <= 0)
	{
		ssi_wrn("could not receive message");
		return false;
	}
	else if (result != SYNC_MSG_SIZE)
	{
		ssi_wrn("received message of unexpected size");
		return false;
	}
	else
	{
		if (!ssi_strcmp(message, SYNC_MSG_HEAD_STR, false, SYNC_MSG_HEAD_SIZE))
		{
			ssi_wrn("received message with unexpected header");
			return false;
		}

		bool found_type = false;
		bool found_id = false;
		for (ssi_size_t i = 0; i < SYNC_MSG_TYPE::NUM; i++)
		{
			if (ssi_strcmp(message + SYNC_MSG_TYPE_POS, SYNC_MSG_TYPE_STR[i], false, SYNC_MSG_TYPE_SIZE))
			{
				found_type = true;
				type = (SYNC_MSG_TYPE::List) i;
				break;
			}
		}
		for (ssi_size_t i = 0; i < SYNC_MSG_ID::NUM; i++)
		{
			if (ssi_strcmp(message + SYNC_MSG_ID_POS, SYNC_MSG_ID_STR[i], false, SYNC_MSG_ID_SIZE))
			{
				found_id = true;
				id = (SYNC_MSG_ID::List) i;
				break;
			}
		}
		if (!found_type || !found_id)
		{
			ssi_wrn("received message with unexpected parameters");
			return false;
		}

		ssi_msg(SSI_LOG_LEVEL_BASIC, "received %s message %s", SYNC_MSG_TYPE_STRLONG[type], SYNC_MSG_ID_STRLONG[id]);		
	}

	return true;
}

void TheFramework::send_sync_msg(Socket *socket, SYNC_MSG_TYPE::List type, SYNC_MSG_ID::List id)
{
	ssi_char_t message[SYNC_MSG_SIZE];
	ssi_sprint(message, "%s%c%s%c%s", SYNC_MSG_HEAD_STR, SYNC_MSG_DELIM, SYNC_MSG_TYPE_STR[type], SYNC_MSG_DELIM, SYNC_MSG_ID_STR[id]);	
	int result = socket->send(message, SYNC_MSG_SIZE);
	if (result <= 0)
	{
		ssi_wrn("could not send %s message", SYNC_MSG_TYPE_STRLONG[type]);
	}
}

void TheFramework::countdown(ssi_size_t n_seconds)
{
	// countdown
	if (n_seconds > 0) {
		ssi_print("\n");
		ssi_print_off("seconds to start:   ");
		for (ssi_size_t i = n_seconds; i > 0; i--) {
			ssi_print("\b\b%02u", i);
			ssi_sleep(1000);
		}
	ssi_print("\b\bok\n\n");
	}
}

void TheFramework::Start () {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Starting framework.." << std::endl;
}
#endif

	SetLogLevel (_options.loglevel);

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start %d threads", (int)_jobs.size());

	// start pre jobs
	std::vector<job_s>::iterator iter;
	for (iter = _jobs.begin (); iter != _jobs.end (); iter++) {
		if (iter->type == EXECUTE::PRE) {
			if (!ssi_execute (iter->exe.str (), iter->args.str (), iter->wait)) {
				ssi_wrn ("failed executing '%s' with arguments '%s'", iter->exe.str (), iter->args.str ());
			}
		}
	}

	// reset all buffers
	for (int i = 0; i < THEFRAMEWORK_BUFFER_NUM; i++) {
		if (bufferInUse[i]) {
			ssi_pcast (TimeBuffer, buffer[i])->reset (0);
		}
	}

	// start monitor
	if (_options.monitor) {
		ssi_rect_t rect;
		rect.left = _options.mpos[0];
		rect.top = _options.mpos[1];
		rect.width = _options.mpos[2];
		rect.height = _options.mpos[3];
		#if (defined(ANDROID))
		#else
		_monitor = new FrameMonitor (_options.mupd, rect);
		#endif
		_monitor->start ();
	}

	// move console
	if (_options.console) {
        #if _WIN32|_WIN64
		HWND hWnd = GetConsoleWindow();
		::MoveWindow(hWnd, _options.cpos[0], _options.cpos[1], _options.cpos[2], _options.cpos[3], true);
        #endif
	}

	// start all threads
	for (int i = 0; i < THEFRAMEWORK_THREAD_NUM; i++) {
		if (runnable[i]) {
			runnable[i]->start ();
		}
		
	}

	_sync_msg_id = SYNC_MSG_ID::RUN_AND_QUIT;

	// send sync signal
	if (_options.sync)
	{
		ssi_print("\n");
		ssi_print_off("creating sync socket [%s]\n\n\n", _options.slisten ? "client" : "server");

		// client
		if (_options.slisten)
		{
			_sync_socket = Socket::CreateAndConnect(_options.stype, Socket::SERVER, _options.sport, _options.shost);
			ssi_print("\n");
			ssi_print_off("waiting for sync message to start\n\n\n");
			SYNC_MSG_TYPE::List type;
			SYNC_MSG_ID::List id;
			do
			{
				recv_sync_msg(_sync_socket, type, id);
			}
			while (type != SYNC_MSG_TYPE::START);
			_sync_msg_id = id;
		}
		else
		{
			// server dialog
			if (_options.sdialog)
			{
				int result = 0;
				int selection = 0;
				do
				{
					ssi_print("\t%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", 201, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 187);
					for (int i = 0; i < SYNC_MSG_ID::NUM; i++)
					{
						ssi_print("\t%c [%d] %s %c\n", 186, i, SYNC_MSG_ID_STRLONG[i], 186);
					}
					ssi_print("\t%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", 200, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 205, 188);
					ssi_print("\n\t> ");
					result = scanf("%d", &selection);
					getchar();
				} while (result != 1 || selection < 0 || selection >= SYNC_MSG_ID::NUM);
				_sync_msg_id = (SYNC_MSG_ID::List) selection;
				ssi_print("\n");
			}
			else
			{
				countdown(_options.countdown);
			}
			_sync_socket = Socket::CreateAndConnect(_options.stype, Socket::CLIENT, _options.sport, _options.shost);
		}
	}
	else
	{
		countdown(_options.countdown);
	}

	// signal that framework is running
	_is_running = true;

	// set frame start time
	#if _WIN32|_WIN64
	_start_run_time = ::timeGetTime ();		
	::GetSystemTime (&_system_time);
	::SystemTimeToTzSpecificLocalTime (NULL, &_system_time, &_local_time);
	#else
	uint64_t ms=0;
	timespec ts;
	clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
	ms= ts.tv_sec*1000+ (uint64_t)(ts.tv_nsec/1000000);
	_start_run_time=ms;
	_system_time=ms;
	#endif

	// server
	if (_options.sync && !_options.slisten) 
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "sending %s message %s", SYNC_MSG_TYPE_STRLONG[SYNC_MSG_TYPE::START], SYNC_MSG_ID_STRLONG[_sync_msg_id]);
		send_sync_msg(_sync_socket, SYNC_MSG_TYPE::START, _sync_msg_id);
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "start");	

	if (_options.info) {
		_info = File::CreateAndOpen (File::ASCII, File::WRITE, info_file_name);
		if (_info) {
			FILE *fp = _info->getFile ();
			#if _WIN32|_WIN64
			ssi_fprint (fp, "start system %02d/%02d/%02d %02d:%02d:%02d:%d\n",  (int) _system_time.wYear, (int) _system_time.wMonth, (int) _system_time.wDay, (int) _system_time.wHour, (int) _system_time.wMinute, (int) _system_time.wSecond, (int) _system_time.wMilliseconds);
			ssi_fprint (fp, "start local %02d/%02d/%02d %02d:%02d:%02d:%d\n",  (int) _local_time.wYear, (int) _local_time.wMonth, (int) _local_time.wDay, (int) _local_time.wHour, (int) _local_time.wMinute, (int) _local_time.wSecond, (int) _local_time.wMilliseconds);					
			#endif		
			_info->flush ();
		}
	}

	// start time server
	if (_options.tserver) {
		_tserver = new ssi::TimeServer (_options.tport);
		_tserver->start ();
	}

	// update windows
	std::vector<Decorator*>::iterator it;
	for (it = _decorators.begin(); it != _decorators.end(); it++) {
		(*it)->update();
	}
}

void TheFramework::Wait () {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Wait to stop framework.." << std::endl;
}
#endif

	if (_sync_msg_id == SYNC_MSG_ID::QUIT)
	{
		return;
	}

	if (_options.sync && _options.slisten) 
	{
		ssi_print("\n");
		ssi_print_off("waiting for sync message to stop\n\n\n");
		SYNC_MSG_TYPE::List type;
		SYNC_MSG_ID::List id;
		do
		{
			recv_sync_msg(_sync_socket, type, id);
		} while (type != SYNC_MSG_TYPE::STOP);
		_sync_msg_id = id;
		delete _sync_socket; _sync_socket = 0;
	} 
	else 
	{
		if (_options.runtime > 0)
		{
			int32_t runtime_ms = (int32_t) (_options.runtime * 1000);

			ssi_print("\n");
			ssi_print_off("pipeline stops after %d ms\n\n", runtime_ms);

			ssi_sleep(runtime_ms);
		} 
		else
		{
			ssi_print("\n");
			ssi_print_off("press enter to stop\n\n");			

			while (true) {

				if (_cancel_wait)
					break;
#if __ANDROID__
#else
				if (_kbhit() != 0)
					if (_getch() == '\r' || _getch() == '\n')	//Win: \r
						break;
#endif

				ssi_sleep(10);
			}
		}
	}
}

void TheFramework::CancelWait () {
	_cancel_wait = true;
}

void TheFramework::Stop () {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Stopping framework.." << std::endl;
}
#endif

	// signal that we try to stop framework
	_last_run_time = TheFramework::GetElapsedTimeMs ();
	_is_running = false;

	// send sync signal to stop clients
	if (_options.sync && ! _options.slisten) {
		ssi_msg(SSI_LOG_LEVEL_BASIC, "sending %s message %s", SYNC_MSG_TYPE_STRLONG[SYNC_MSG_TYPE::STOP], SYNC_MSG_ID_STRLONG[_sync_msg_id]);
		send_sync_msg(_sync_socket, SYNC_MSG_TYPE::STOP, _sync_msg_id);
		delete _sync_socket; _sync_socket = 0;
	}

	ssi_msg (SSI_LOG_LEVEL_BASIC, "stop (runtime=%.2lfs)", _last_run_time / 1000.0);
	if (_info) {
		
		#if _WIN32|_WIN64
		SYSTEMTIME stop_sytem_time, stop_local_time;
		::GetSystemTime (&stop_sytem_time);
		::SystemTimeToTzSpecificLocalTime (NULL, &stop_sytem_time, &stop_local_time);
		FILE *fp = _info->getFile ();
		ssi_fprint (fp, "stop system %02d/%02d/%02d %02d:%02d:%02d:%d\n",  (int) stop_sytem_time.wYear, (int) stop_sytem_time.wMonth, (int) stop_sytem_time.wDay, (int) stop_sytem_time.wHour, (int) stop_sytem_time.wMinute, (int) stop_sytem_time.wSecond, (int) stop_sytem_time.wMilliseconds);
		ssi_fprint (fp, "stop local %02d/%02d/%02d %02d:%02d:%02d:%d\n",  (int) stop_local_time.wYear, (int) stop_local_time.wMonth, (int) stop_local_time.wDay, (int) stop_local_time.wHour, (int) stop_local_time.wMinute, (int) stop_local_time.wSecond, (int) stop_local_time.wMilliseconds);		
		
		#else
		uint64_t stop_sytem_time;
		uint64_t ms=0;
		timespec ts;
		clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
		ms= ts.tv_sec*1000+ (uint64_t)(ts.tv_nsec/1000000);
		stop_sytem_time=ms;

		#endif
    _info->flush ();
	}
	delete _info; _info = 0;

    // wake up threads
    for (int i = 0; i < THEFRAMEWORK_BUFFER_NUM; i++) {
        if (!bufferInUse[i]) continue;
		// wake up waiting threads
		bufferCondFull[i].wakeAll ();
		bufferCondEmpty[i].wakeAll ();
    }

	// stop all threads
	for (int i = 0; i < THEFRAMEWORK_THREAD_NUM; i++) {
		if (runnable[i]) {
			runnable[i]->stop ();
		}
	}

	// stop monitor
	if (_options.monitor) {
		_monitor->stop ();
		delete _monitor;
		_monitor = 0;
	}

	// stop time server
	if (_options.tserver) {
		_tserver->close ();				
		_tserver->stop ();
		delete _tserver;
		_tserver = 0;
	}

	// start post jobs
	std::vector<job_s>::iterator iter;
	for (iter = _jobs.begin (); iter != _jobs.end (); iter++) {
		if (iter->type == EXECUTE::POST) {
			if (!ssi_execute (iter->exe.str (), iter->args.str (), iter->wait)) {
				ssi_wrn ("failed executing '%s' with arguments '%s'", iter->exe.str (), iter->args.str ());
			}
		}
	}

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Framework has been stopped !" << std::endl;
	logfile.close ();
}
#endif
}

void TheFramework::Clear () {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Shutting down signal to framework.." << std::endl;
}
#endif

	ssi_msg (SSI_LOG_LEVEL_BASIC, "framework shutdown");

	// remove all buffer
    for (int i = 0; i < THEFRAMEWORK_BUFFER_NUM; i++) {
        if (!bufferInUse[i]) continue;
		// wake up waiting threads
		bufferCondFull[i].wakeAll ();
		bufferCondEmpty[i].wakeAll ();
		// remove buffer
        this->RemBuffer (i);
    }

	// removes threads
	for (int i = 0; i < THEFRAMEWORK_THREAD_NUM; i++) {
		if (runnable[i]) {
			runnable[i] = 0;
		}
	}

	// delete component
	for (int i = 0; i < THEFRAMEWORK_COMPONENT_NUM; i++) {
		if (component[i]) {
			delete component[i];
			component[i] = 0;
		}
	}
	component_counter = 0;

	// clear decorators
	_decorators.clear();

	// delete jobs
	_jobs.clear ();

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Good bye!" << std::endl;
}
#endif
}

bool TheFramework::DoRestart()
{
	return _sync_msg_id == SYNC_MSG_ID::RUN_AND_RESTART;
}

// Adds a buffer to the framework
//int TheFramework::AddBuffer (ssi_time_t sample_rate, ssi_size_t sample_dimension, ssi_size_t sample_bytes, ssi_time_t buffer_duration, ssi_time_t sync_duration) {
int TheFramework::AddBuffer(ssi_time_t sample_rate, ssi_size_t sample_dimension, ssi_size_t sample_bytes, ssi_type_t sample_type, const char *buffer_size) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to add buffer.." << std::endl;
}
#endif

	// find a free buffer
	int buffer_id = -1;
	for (int i = 0; i < THEFRAMEWORK_BUFFER_NUM; i++) {
		// get mutex for the buffer
		Lock lock (bufferMutex[i]);
		// check if buffer is still available
		if (!bufferInUse[i]) {
			buffer_id = i;
			break;
		}
	}
	if (buffer_id == -1) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [all buffer in use]" << std::endl;
}
#endif

		return THEFRAMEWORK_ERROR;
	}

	// get mutex for the buffer
	Lock lock (bufferMutex[buffer_id]);

    // create the new buffer
	ssi_size_t buffer_size_in_samples = 0;
	if (!ssi_parse_samples(buffer_size, buffer_size_in_samples, sample_rate)) {
		ssi_err("could not parse buffer size '%s'", buffer_size);
	}
	buffer[buffer_id] = (Buffer *) new TimeBuffer(buffer_size_in_samples, sample_rate, sample_dimension, sample_bytes, sample_type);
	// store sync time
//	syncDur[buffer_id] = ssi_cast (ssi_size_t, sync_duration * sample_rate);
//	syncDurCounter[buffer_id] = 0;
    // and mark it as active
    bufferInUse[buffer_id] = true;

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Added buffer " << buffer_id << " !" << std::endl;
}
#endif

    return buffer_id;
}

bool TheFramework::SetMetaData (int buffer_id, ssi_size_t size, const void *meta) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to set meta data to buffer " << buffer_id << ".." << std::endl;
}
#endif

	// check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
		return false;
	}

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return false;
    }

    // now set meta data
	buffer[buffer_id]->setMetaData (size, meta);

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Set meta data to buffer " << buffer_id << " !" << std::endl;
}
#endif

    return true;
}

const void *TheFramework::GetMetaData (int buffer_id, ssi_size_t &size) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to get meta data of buffer " << buffer_id << ".." << std::endl;
}
#endif

	// check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
		return 0;
	}

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return 0;
    }

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Get meta data of buffer " << buffer_id << " !" << std::endl;
}
#endif

	return buffer[buffer_id]->getMetaData (size);
}

// Removes a buffer from the framework
bool TheFramework::RemBuffer (int buffer_id) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to remove buffer " << buffer_id << ".." << std::endl;
}
#endif

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return false;
    }

    // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return false;
    }

    // now remove the buffer
	delete buffer[buffer_id];
    // and mark it as inactive
    bufferInUse[buffer_id] = false;

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Removed buffer " << buffer_id << " !" << std::endl;
}
#endif

    return true;
}

// Resets the buffer
bool TheFramework::ResetBuffer (int buffer_id, ssi_time_t offset) {

	#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to reset buffer " << buffer_id << ".." << std::endl;
}
#endif

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return false;
    }

    // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return false;
    }

    // now reset the buffer
	static_cast<TimeBuffer*>(buffer[buffer_id])->reset (offset);
	

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Reseted buffer " << buffer_id << " !" << std::endl;
}
#endif

    return true;

}

// Appends data to a buffer
int TheFramework::PushData (int buffer_id, const ssi_byte_t *data, ssi_size_t samples) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to supply data to buffer " << buffer_id << ".." << std::endl;
}
#endif

	// check if framework is in running
    if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [framework in is not running]" << std::endl;
}
#endif
		Sleep (THEFRAMEWORK_SLEEPTIME_IF_IDLE);
        return THEFRAMEWORK_ERROR;
    }

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

	// add data to buffer
	TimeBuffer::STATUS status;
	status =  static_cast<TimeBuffer*>(buffer[buffer_id])->push (data, samples);

	// check if it is time to synchronize buffer
//	if (status == TimeBuffer::SUCCESS) {
//		syncDurCounter[buffer_id] += samples;
//		if (syncDurCounter[buffer_id] > syncDur[buffer_id]) {
//			static_cast<TimeBuffer*>(buffer[buffer_id])->sync (GetElapsedTime ());
//			syncDurCounter[buffer_id] = 0;
//		}
//	}

	if (status == TimeBuffer::SUCCESS) {
		// wake up one waiting runnable (in case one or more are waiting)
		bufferCondEmpty[buffer_id].wakeAll ();
		//bufferCondEmpty[buffer_id].wakeSingle ();
	}

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	ssi_time_t duration = samples / static_cast<TimeBuffer*>(buffer[buffer_id])->sample_rate;
	ssi_time_t current_time = static_cast<TimeBuffer*>(buffer[buffer_id])->getCurrentSampleTime ();
	if (status == TimeBuffer::SUCCESS) {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Supplied data to buffer " << buffer_id << " (" << current_time << "," << duration << ")" << " !" << std::endl;	
	} else {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Failed to supply data to buffer " << buffer_id << " (" << current_time << "," << duration << ")" << " !" << std::endl;
	}
}
#endif

    return status;
}

// Appends zeros to a buffer
int TheFramework::PushZeros (int buffer_id) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to supply zeros to buffer " << buffer_id << ".." << std::endl;
}
#endif

	// check if framework is running
    if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [framework is not running]" << std::endl;
}
#endif
		Sleep (THEFRAMEWORK_SLEEPTIME_IF_IDLE);
        return THEFRAMEWORK_ERROR;
    }

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

	// add zeros to buffer
	TimeBuffer::STATUS status = TimeBuffer::SUCCESS;
	ssi_time_t frame_time = GetElapsedTime ();
	ssi_time_t buffer_time = static_cast<TimeBuffer*>(buffer[buffer_id])->getLastAccessedSampleTime ();
	if (buffer_time < frame_time) {
		ssi_size_t samples = ssi_cast (ssi_size_t, (frame_time - buffer_time) * static_cast<TimeBuffer*>(buffer[buffer_id])->sample_rate);
		status =  static_cast<TimeBuffer*>(buffer[buffer_id])->pushZeros (samples);
	}
	
	if (status == TimeBuffer::SUCCESS) {
		// wake up one waiting runnable (in case one or more are waiting)
		bufferCondEmpty[buffer_id].wakeAll ();
		//bufferCondEmpty[buffer_id].wakeSingle ();
	}

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	ssi_time_t frame_time = GetElapsedTime ();
	ssi_time_t buffer_time = static_cast<TimeBuffer*>(buffer[buffer_id])->getLastAccessedSampleTime ();
	if (buffer_time < frame_time) {
		ssi_size_t samples = ssi_cast (ssi_size_t, (frame_time - buffer_time) * static_cast<TimeBuffer*>(buffer[buffer_id])->sample_rate);
		ssi_time_t duration = samples / static_cast<TimeBuffer*>(buffer[buffer_id])->sample_rate;
		ssi_time_t current_time = static_cast<TimeBuffer*>(buffer[buffer_id])->getCurrentSampleTime ();
		if (status == TimeBuffer::SUCCESS) {
			logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Supplied data to buffer " << buffer_id << " (" << current_time << "," << duration << ")" << " !" << std::endl;	
		} else {
			logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Failed to supply data to buffer " << buffer_id << " (" << current_time << "," << duration << ")" << " !" << std::endl;
		}
	}
}
#endif

    return status;
}

// Gets data from the buffer
// waits if buffer is not ready
int TheFramework::GetData (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to receive data from buffer " << buffer_id << ".." << std::endl;
}
#endif

	// check if the framework is is running
    if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [framework is not running]" << std::endl;
}
#endif
		Sleep (THEFRAMEWORK_SLEEPTIME_IF_IDLE);
        return THEFRAMEWORK_ERROR;
    }

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    // get mutex for the buffer
	bufferMutex[buffer_id].acquire ();

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    TimeBuffer::STATUS status;

	for (;;) {

		// try to get data from the buffer
		status = static_cast<TimeBuffer*>(buffer[buffer_id])->get (data, samples_in, samples_out, start_time, duration);

		if (status != TimeBuffer::SUCCESS && 
			_is_running &&
			status != TimeBuffer::DATA_NOT_IN_BUFFER_ANYMORE &&
			status != TimeBuffer::INPUT_ARRAY_TOO_SMALL && 
			status != TimeBuffer::DURATION_TOO_LARGE && 
			status != TimeBuffer::DURATION_TOO_SMALL
			) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Put receiving component to sleep (buffer " << buffer_id << ")" << std::endl;
}
#endif

			// if operation was not successful and the framework is running
			// put the calling runnable to sleep
			bufferCondEmpty[buffer_id].wait (&bufferMutex[buffer_id]);

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Wake up receiving component (buffer " << buffer_id << ")" << std::endl;
}
#endif

		} else {
			// otherwise leave
			break;
		}

		// leave if framework is in not running
		if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Leave because framework is not running (buffer " << buffer_id << ")" << std::endl;
}
#endif
			bufferMutex[buffer_id].release ();
			return THEFRAMEWORK_ERROR;
		}
	}

	// wake up one waiting runnable (in case one or more are waiting)
	//bufferCondFull[buffer_id].wakeAll ();
	bufferCondFull[buffer_id].wakeSingle ();

	// release mutex for the buffer
	bufferMutex[buffer_id].release ();

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	if (status == TimeBuffer::SUCCESS) {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Received data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;
	} else {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Failed to receive data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;
	}
}
#endif

    return status;
}

// Gets data from the buffer
// waits if buffer is not ready
int TheFramework::GetData (int buffer_id, ssi_stream_t &stream, ssi_time_t start_time, ssi_time_t duration) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to receive data from buffer " << buffer_id << ".." << std::endl;
}
#endif

	// check if the framework is running
    if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [framework is not running]" << std::endl;
}
#endif
		Sleep (THEFRAMEWORK_SLEEPTIME_IF_IDLE);
        return THEFRAMEWORK_ERROR;
    }

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    // get mutex for the buffer
	bufferMutex[buffer_id].acquire ();

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    TimeBuffer::STATUS status;

	for (;;) {

		// try to get data from the buffer
		status = static_cast<TimeBuffer*>(buffer[buffer_id])->get (&stream.ptr, stream.num_real, stream.num, start_time, duration);
		stream.tot_real = stream.num_real * stream.byte * stream.dim;
		stream.tot = stream.num * stream.byte * stream.dim;

		if (status != TimeBuffer::SUCCESS && 
			_is_running &&
			status != TimeBuffer::DATA_NOT_IN_BUFFER_ANYMORE &&
			status != TimeBuffer::INPUT_ARRAY_TOO_SMALL && 
			status != TimeBuffer::DURATION_TOO_LARGE && 
			status != TimeBuffer::DURATION_TOO_SMALL
			) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Put receiving runnable to sleep (buffer " << buffer_id << ")" << std::endl;
}
#endif

			// if operation was not successful and the framework is running
			// put the calling runnable to sleep
			bufferCondEmpty[buffer_id].wait (&bufferMutex[buffer_id]);

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Wake up receiving runnable (buffer " << buffer_id << ")" << std::endl;
}
#endif

		} else {
			// otherwise leave
			break;
		}

		// leave if framework is not running
		if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Leave because is not running (buffer " << buffer_id << ")" << std::endl;
}
#endif
			bufferMutex[buffer_id].release ();
			return THEFRAMEWORK_ERROR;
		}
	}

	// wake up one waiting runnable (in case one or more are waiting)
	//bufferCondFull[buffer_id].wakeAll ();
	bufferCondFull[buffer_id].wakeSingle ();

	// release mutex for the buffer
	bufferMutex[buffer_id].release ();

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	if (status == TimeBuffer::SUCCESS) {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Received data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;
	} else {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Failed to receive data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;
	}
}
#endif

    return status;
}


// Gets data from the buffer
// waits if buffer is not ready
int TheFramework::GetData (int buffer_id, ssi_byte_t *data, ssi_size_t samples, ssi_lsize_t position) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to receive data from buffer " << buffer_id << ".." << std::endl;
}
#endif

	// check if the framework is running
    if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [framework is not running]" << std::endl;
}
#endif
		Sleep (THEFRAMEWORK_SLEEPTIME_IF_IDLE);
        return THEFRAMEWORK_ERROR;
    }

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    // get mutex for the buffer
	bufferMutex[buffer_id].acquire ();

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    TimeBuffer::STATUS status;

	for (;;) {

		// try to get data from the buffer
		status = static_cast<TimeBuffer*>(buffer[buffer_id])->get (data, samples, position);

		if (status != TimeBuffer::SUCCESS && 
			_is_running &&
			status != TimeBuffer::DATA_NOT_IN_BUFFER_ANYMORE &&
			status != TimeBuffer::INPUT_ARRAY_TOO_SMALL && 
			status != TimeBuffer::DURATION_TOO_LARGE && 
			status != TimeBuffer::DURATION_TOO_SMALL
			) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Put receiving runnable to sleep (buffer " << buffer_id << ")" << std::endl;
}
#endif

			// if operation was not successful and the framework is running
			// put the calling runnable to sleep
			bufferCondEmpty[buffer_id].wait (&bufferMutex[buffer_id]);

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Wake up receiving runnable (buffer " << buffer_id << ")" << std::endl;
}
#endif

		} else {
			// otherwise leave
			break;
		}

		// leave if framework is not running
		if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Leave because framework is not running (buffer " << buffer_id << ")" << std::endl;
}
#endif
			bufferMutex[buffer_id].release ();
			return THEFRAMEWORK_ERROR;
		}
	}

	// wake up one waiting runnable (in case one or more are waiting)
	//bufferCondFull[buffer_id].wakeAll ();
	bufferCondFull[buffer_id].wakeSingle ();

	// release mutex for the buffer
	bufferMutex[buffer_id].release ();

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	ssi_time_t start_time = (position / static_cast<TimeBuffer*>(buffer[buffer_id])->sample_rate) - static_cast<TimeBuffer*>(buffer[buffer_id])->getOffsetTime ();
	ssi_time_t duration = samples / static_cast<TimeBuffer*>(buffer[buffer_id])->sample_rate;
	if (status == TimeBuffer::SUCCESS) {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Received data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;
	} else {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Failed to receive data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;
	}
}
#endif

    return status;
}

// Gets data from buffer
int TheFramework::GetDataTry (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration) {

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to receive data from buffer " << buffer_id << std::endl;
}
#endif

	// check if the framework is running
    if (!_is_running) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [framework is not running]" << std::endl;
}
#endif
		Sleep (THEFRAMEWORK_SLEEPTIME_IF_IDLE);
        return THEFRAMEWORK_ERROR;
    }

    // check if buffer id is valid
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [invalid buffer id " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

    // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is in use
    if (!bufferInUse[buffer_id]) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [buffer not in use: " << buffer_id << "]" << std::endl;
}
#endif
        return THEFRAMEWORK_ERROR;
    }

	// try to get data from the buffer
    TimeBuffer::STATUS status = static_cast<TimeBuffer*>(buffer[buffer_id])->get (data, samples_out, samples_in, start_time, duration);
	// wake up one waiting runnable (in case one or more are waiting)
	//bufferCondFull[buffer_id].wakeAll ();
	bufferCondFull[buffer_id].wakeSingle ();

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	if (status == TimeBuffer::SUCCESS) {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Received data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;	
	} else {
		logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Failed to receive data from buffer " << buffer_id << " (" << start_time << " to " << start_time + duration << ") !" << std::endl;
	}
}
#endif

    return status;
}

// returns buffer time
bool TheFramework::GetCurrentSampleTime (int buffer_id, ssi_time_t &time) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	 // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now return buffer time
	time = static_cast<TimeBuffer*>(buffer[buffer_id])->getCurrentSampleTime ();

	return true;
}

// returns current write position
bool TheFramework::GetCurrentWritePos (int buffer_id, ssi_lsize_t &position) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	 // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now return buffer time
	position = static_cast<TimeBuffer*>(buffer[buffer_id])->getCurrentWritePos ();

	return true;
}

// returns buffer time
bool TheFramework::SetCurrentSampleTime (int buffer_id, ssi_time_t time) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	 // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now return buffer time
	static_cast<TimeBuffer*>(buffer[buffer_id])->setCurrentSampleTime (time);

	return true;
}

// returns buffer time
bool TheFramework::GetLastAccessedSampleTime (int buffer_id, ssi_time_t &time) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now return buffer time
	time = static_cast<TimeBuffer*>(buffer[buffer_id])->getLastAccessedSampleTime ();

	return true;
}

// returns offset time of buffer
bool TheFramework::GetOffsetTime (int buffer_id, ssi_time_t &offset) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	 // get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now get requested value
	offset = static_cast<TimeBuffer*>(buffer[buffer_id])->getOffsetTime ();

	return true;
}

// returns sample rate of buffer
bool TheFramework::GetSampleRate (int buffer_id, ssi_time_t &sample_rate) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now get requested value
	sample_rate = static_cast<TimeBuffer*>(buffer[buffer_id])->sample_rate;

	return true;
}

// returns total sample bytes of buffer
bool TheFramework::GetTotalSampleBytes (int buffer_id, ssi_size_t &sample_bytes) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now get requested value
	sample_bytes = static_cast<TimeBuffer*>(buffer[buffer_id])->sample_total_bytes;

	return false;
}

// returns sample bytes of buffer
bool TheFramework::GetSampleBytes (int buffer_id, ssi_size_t &sample_bytes) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now get requested value
	sample_bytes = static_cast<TimeBuffer*>(buffer[buffer_id])->sample_bytes;

	return true;
}


// returns sample bytes of buffer
bool TheFramework::GetSampleType (int buffer_id, ssi_type_t &sample_type) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now get requested value
	sample_type = static_cast<TimeBuffer*>(buffer[buffer_id])->sample_type;

	return true;
}

// returns sample dimension of buffer
bool TheFramework::GetSampleDimension (int buffer_id, ssi_size_t &sample_dimension) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now get requested value
	sample_dimension = static_cast<TimeBuffer*>(buffer[buffer_id])->sample_dimension;

	return true;
}

// returns capacity of buffer
bool TheFramework::GetCapacity (int buffer_id, ssi_time_t &capacity) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	if (!bufferInUse[buffer_id]) {
		return false;
	}

	// now get requested value
	capacity = static_cast<TimeBuffer*>(buffer[buffer_id])->getCapacity ();

	return true;
}

// returns sample dimension of buffer
bool TheFramework::IsBufferInUse (int buffer_id) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

    // now check if buffer is not in use yet
	// in this case return false
	return bufferInUse[buffer_id];
}

// synchronize buffer buffer
bool TheFramework::Synchronize (int buffer_id) {

	// check if buffer id is valid
	// otherwise return false
    if (buffer_id < 0 || buffer_id >= THEFRAMEWORK_BUFFER_NUM) {
		return false;
    }

	// get mutex for the buffer
    Lock lock (bufferMutex[buffer_id]);

	// synchronize buffer with current framwork clock
    static_cast<TimeBuffer*>(buffer[buffer_id])->sync (GetElapsedTime ());

	return true;
}

// returns sample dimension of buffer
bool TheFramework::IsInIdleMode () {

	return !_is_running;
}

ssi_size_t TheFramework::GetStartTimeMs () {

	return !_is_running ? 0 : _start_run_time;
}

ssi_time_t TheFramework::GetStartTime () {

	return !_is_running ? 0 : _start_run_time / 1000.0;
}

void TheFramework::GetStartTimeLocal (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond) {
	#if _WIN32|_WIN64
	year = _local_time.wYear;
	month = _local_time.wMonth;
	day = _local_time.wDay;
	hour = _local_time.wHour;
	minute = _local_time.wMinute;
	second = _local_time.wSecond;
	msecond = _local_time.wMilliseconds;
	#else
	year = _local_time;
	month = _local_time;
	day = _local_time;
	hour = _local_time;
	minute = _local_time;
	second = _local_time;
	msecond = _local_time;

	#endif
}

void TheFramework::GetStartTimeSystem (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond) {
	#if _WIN32|_WIN64
	year = _system_time.wYear;
	month = _system_time.wMonth;
	day = _system_time.wDay;
	hour = _system_time.wHour;
	minute = _system_time.wMinute;
	second = _system_time.wSecond;
	msecond = _system_time.wMilliseconds;
	#else
	year = _system_time;
	month = _system_time;
	day = _system_time;
	hour = _system_time;
	minute = _system_time;
	second = _system_time;
	msecond = _system_time;
	
	#endif
}

ssi_size_t TheFramework::GetElapsedTimeMs () {

	return !_is_running ? 0 : ssi_time_ms () - _start_run_time;
}

ssi_time_t TheFramework::GetElapsedTime () {

	return !_is_running ? 0 : GetElapsedTimeMs () / 1000.0;
}

ssi_size_t TheFramework::GetRunTimeMs () {

	return _last_run_time;
}

ssi_time_t TheFramework::GetRunTime () {

	return _last_run_time > 0 ? _last_run_time / 1000.0 : 0;
}

void TheFramework::AddDecorator(IObject *decorator) {
	_decorators.push_back(ssi_pcast(Decorator, decorator));
}

void TheFramework::AddExeJob (const ssi_char_t *exe, const ssi_char_t *args, EXECUTE::list type, int wait) {

	if (type == EXECUTE::NOW) {
		if (!ssi_execute (exe, args, wait)) {
			ssi_wrn ("failed executing '%s' with arguments '%s'", exe, args);
		}
	} else {
		job_s job;
		job.exe = String (exe);
		job.args = String (args);
		job.wait = wait;
		job.type = type;
		_jobs.push_back (job);
	}
}

// Adds a runnable to the framework
int TheFramework::AddRunnable (IRunnable *new_thread) {

	if (_is_running) {
		return THEFRAMEWORK_ERROR;
	}

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Try to add runnable.." << std::endl;
}
#endif

	// find a free runnable
	int thread_id = -1;
	for (int i = 0; i < THEFRAMEWORK_THREAD_NUM; i++) {
		if (runnable[i] == 0) {
			runnable[i] = new_thread;
			thread_id = i;
			break;
		}
	}
	if (thread_id == -1) {
#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "ERROR [all threads in use]" << std::endl;
}
#endif
		return THEFRAMEWORK_ERROR;
	}

#ifdef FRAMEWORK_LOG
{
	Lock lock (logmutex);
	logfile << std::setw (6) << std::setprecision (2) << std::fixed << GetElapsedTime () << "\t" << "Added runnable " << thread_id << " !" << std::endl;
}
#endif

    return thread_id;
}

void TheFramework::AddEventConsumer (ITransformable *source,
	IConsumer *iconsumer, 
	ITheEventBoard *event_board,
	const ssi_char_t *address,
	IEvents::EVENT_STATE_FILTER::List state_filter,
	ITransformer *transformer) {

	EventConsumer *consumer = new EventConsumer ();
	consumer->getOptions ()->async = true;
	component[component_counter++] = consumer;	
	
	consumer->AddConsumer (source, iconsumer, transformer);
	event_board->RegisterListener(*consumer, address, 0, state_filter);
}

void TheFramework::AddEventConsumer (ssi_size_t n_sources, 
	ITransformable **sources,
	IConsumer *iconsumer, 
	ITheEventBoard *event_board,
	const ssi_char_t *address,
	IEvents::EVENT_STATE_FILTER::List state_filter,
	ITransformer **itransformer) {

	EventConsumer *consumer = new EventConsumer ();
	consumer->getOptions ()->async = true;
	component[component_counter++] = consumer;	
	
	consumer->AddConsumer (n_sources, sources, iconsumer, itransformer);
	event_board->RegisterListener(*consumer, address, 0, state_filter);
}

void TheFramework::AddConsumer (ITransformable *source,
	IConsumer *iconsumer, 
	const ssi_char_t *frame_size,
	const ssi_char_t *delta_size,
	ITransformer *transformer,
	ITransformable *trigger) {

	ssi_size_t frame_size_in_samples = 0;
	ssi_parse_samples (frame_size, frame_size_in_samples, source->getSampleRate ());

	ssi_size_t delta_size_in_samples = 0;
	if (delta_size) {
		ssi_parse_samples (delta_size, delta_size_in_samples, source->getSampleRate ());
	}

	int trigger_id = -1;
	if (trigger) {
		trigger_id = trigger->getBufferId ();
	}
	Consumer *consumer = new Consumer (source->getBufferId (), iconsumer, frame_size_in_samples, delta_size_in_samples, transformer, trigger_id);
	component[component_counter++] = consumer;
};

void TheFramework::AddConsumer (ssi_size_t n_sources, 
	ITransformable **sources,
	IConsumer *iconsumer, 
	const ssi_char_t *frame_size,
	const ssi_char_t *delta_size,
	ITransformer **itransformer,
	ITransformable *trigger) {

	int *buffer_ids = new int[n_sources];
	for (ssi_size_t i = 0; i < n_sources; i++) {
		buffer_ids[i] = sources[i]->getBufferId ();
	}

	ssi_size_t frame_size_in_samples = 0;
	ssi_parse_samples (frame_size, frame_size_in_samples, sources[0]->getSampleRate ());

	ssi_size_t delta_size_in_samples = 0;
	if (delta_size) {
		ssi_parse_samples (delta_size, delta_size_in_samples, sources[0]->getSampleRate ());
	}

	int trigger_id = -1;
	if (trigger) {
		trigger_id = trigger->getBufferId ();
	}
	Consumer *consumer = new Consumer (n_sources, buffer_ids, iconsumer, frame_size_in_samples, delta_size_in_samples, itransformer, trigger_id);
	component[component_counter++] = consumer;
	delete[] buffer_ids;
};

ITransformable *TheFramework::AddProvider (ISensor *isensor,
	const ssi_char_t *channel,
	IFilter *ifilter,
	const ssi_char_t *buffer_size,
	const ssi_char_t *check_interval_in_seconds,
	const ssi_char_t *sync_interval_in_seconds) {

	Provider *provider = new Provider(ifilter, buffer_size, check_interval_in_seconds, sync_interval_in_seconds);
	if (!isensor->setProvider (channel, provider)) {
		return 0;
	}
	component[component_counter++] = provider;

	return provider;
};

void TheFramework::AddSensor (ISensor *isensor) {

	Sensor *sensor = new Sensor (isensor);
	component[component_counter++] = sensor;
};

ITransformable *TheFramework::AddTransformer (ITransformable *source, 
	ITransformer *itransformer,
	const ssi_char_t *frame_size,
	const ssi_char_t *delta_size,
	const ssi_char_t *buffer_size,
	ITransformable *trigger) {

	ssi_size_t frame_size_in_samples = 0;
	ssi_parse_samples (frame_size, frame_size_in_samples, source->getSampleRate ());

	ssi_size_t delta_size_in_samples = 0;
	if (delta_size) {
		ssi_parse_samples (delta_size, delta_size_in_samples, source->getSampleRate ());
	}

	int trigger_id = -1;
	if (trigger) {
		trigger_id = trigger->getBufferId();
	}
	Transformer *transformer = new Transformer (source->getBufferId (), itransformer, frame_size_in_samples, delta_size_in_samples, buffer_size, trigger_id);
	component[component_counter++] = transformer;

	return transformer;
};

ITransformable *TheFramework::AddTransformer (ITransformable *source, 
	ssi_size_t n_xtra_sources,
	ITransformable **xtra_sources,
	ITransformer *itransformer,
	const ssi_char_t *frame_size,
	const ssi_char_t *delta_size,
	const ssi_char_t *buffer_size,
	ITransformable *trigger) {

	int *xtra_buffer_ids = new int[n_xtra_sources];
	for (ssi_size_t i = 0; i < n_xtra_sources; i++) {
		xtra_buffer_ids[i] = xtra_sources[i]->getBufferId ();
	}

	ssi_size_t frame_size_in_samples = 0;
	ssi_parse_samples (frame_size, frame_size_in_samples, source->getSampleRate ());

	ssi_size_t delta_size_in_samples = 0;
	if (delta_size) {
		ssi_parse_samples (delta_size, delta_size_in_samples, source->getSampleRate ());
	}

	int trigger_id = -1;
	if (trigger) {
		trigger_id = trigger->getBufferId();
	}
	Transformer *transformer = new Transformer (source->getBufferId (), n_xtra_sources, xtra_buffer_ids, itransformer, frame_size_in_samples, delta_size_in_samples, buffer_size, trigger_id);
	component[component_counter++] = transformer;
	delete[] xtra_buffer_ids;

	return transformer;
};

}
#if __ANDROID__

#else

#if __gnu_linux__
	int _kbhit(void) {
	   struct termios term, oterm;
	   int fd = 0;
	   int c = 0;

	   tcgetattr(fd, &oterm);
	   memcpy(&term, &oterm, sizeof(term));
	   term.c_lflag = term.c_lflag & (!ICANON);
	   term.c_cc[VMIN] = 0;
	   term.c_cc[VTIME] = 1;
	   tcsetattr(fd, TCSANOW, &term);
	   c = getchar();
	   tcsetattr(fd, TCSANOW, &oterm);

	   if (c != -1)
       ungetc(c, stdin);

	   return ((c != -1) ? 1 : 0);
	}

	int _getch()
	{
	   static int ch = -1, fd = 0;
	   struct termios neu, alt;

	   fd = fileno(stdin);
	   tcgetattr(fd, &alt);
	   neu = alt;
	   neu.c_lflag &= ~(ICANON|ECHO);
	   tcsetattr(fd, TCSANOW, &neu);
	   ch = getchar();
	   tcsetattr(fd, TCSANOW, &alt);

	   return ch;
	}
#endif

#endif
