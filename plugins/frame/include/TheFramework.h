// TheFramework.h
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

#pragma once

#ifndef SSI_FRAME_THEFRAMEWORK_H
#define SSI_FRAME_THEFRAMEWORK_H

#include "base/ITheFramework.h"
#include "FrameLibCons.h"
#include "buffer/TimeBuffer.h"
#include "base/ITransformable.h"
#include "Sensor.h"
#include "Consumer.h"
#include "EventConsumer.h"
#include "Provider.h"
#include "Transformer.h"
#include "base/IRunnable.h"
#include "ioput/socket/Socket.h"
#include "ioput/file/File.h"
#include "base/String.h"

#if _WIN32|_WIN64
	#include <conio.h>
#else

	#include <termios.h>

	int _kbhit(void);

	int _getch();

#endif

namespace ssi {

class TimeServer;
class Decorator;

class TheFramework : public ITheFramework {

friend class Buffer;
friend class TimeBuffer;
friend class Sensor;
friend class Provider;
friend class ConsumerBase;
friend class Consumer;
friend class Transformer;
friend class Factory;

public:

	static const ssi_char_t *SYNC_MSG_HEAD_STR;
	static const ssi_size_t SYNC_MSG_HEAD_SIZE;
	struct SYNC_MSG_TYPE
	{
		enum List
		{
			START = 0,
			STOP,
			NUM
		};
	};

	static const ssi_size_t SYNC_MSG_TYPE_SIZE;
	static const ssi_size_t SYNC_MSG_TYPE_POS;
	static const ssi_char_t *SYNC_MSG_TYPE_STR[SYNC_MSG_TYPE::NUM];
	static const ssi_char_t *SYNC_MSG_TYPE_STRLONG[SYNC_MSG_TYPE::NUM];	
	struct SYNC_MSG_ID
	{
		enum List
		{
			QUIT = 0,
			RUN_AND_QUIT,
			RUN_AND_RESTART,
			NUM
		};
	};

	static const ssi_size_t SYNC_MSG_ID_SIZE;
	static const ssi_size_t SYNC_MSG_ID_POS;
	static const ssi_char_t *SYNC_MSG_ID_STR[SYNC_MSG_ID::NUM];
	static const ssi_char_t *SYNC_MSG_ID_STRLONG[SYNC_MSG_ID::NUM];

	static const ssi_size_t SYNC_MSG_TOKENS;
	static const ssi_char_t SYNC_MSG_DELIM;
	static const ssi_size_t SYNC_MSG_SIZE;

public: 

	class Options : public OptionList {

	public:
		Options () 
			: monitor(false), mupd(100), console(false), sync(false), sport(1111), stype(Socket::TYPE::UDP), slisten(false), sdialog(false), countdown(3), runtime(0), tserver(false), tport(2222), info(false) {

			shost[0] = '\0';

			loglevel = SSI_LOG_LEVEL_DEFAULT;
			mpos[0] = 0;
			mpos[1] = 0;
			mpos[2] = 400;
			mpos[3] = 400;

			cpos[0] = 0;
			cpos[1] = 0;
			cpos[2] = 400;
			cpos[3] = 400;	

			waitid[0] = '\0';

			addOption ("countdown", &countdown, 1, SSI_SIZE, "countdown in seconds before pipeline is started");		
			addOption ("runtime", &runtime, 1, SSI_DOUBLE, "runtime in seconds until pipeline is stopped (if <= 0, pipeline will wait for key input)");
			addOption ("waitid", &waitid, SSI_MAX_CHAR, SSI_CHAR, "call wait function of object with this id (has to implement IWaitable) [overrides 'runtime']");
			
			addOption ("console", &console, 1, SSI_BOOL, "move console window");	
			addOption ("cpos", &cpos, 4, SSI_INT, "position of console window on screen [posx,posy,width,height]");
			addOption ("monitor", &monitor, 1, SSI_BOOL, "show framework monitor");		
			addOption ("mpos", &mpos, 4, SSI_INT, "position of monitor on screen [posx,posy,width,height]");
			addOption ("mupd", &mupd, 1, SSI_SIZE, "monitor update frequency in milliseconds");		
			
			addOption ("sync", &sync, 1, SSI_BOOL, "turn on sync mode: if not in listen mode send sync signal otherwise wait for such a signal");
			addOption ("slisten", &slisten, 1, SSI_BOOL, "serve as client, i.e. wait for sync signal by server (only if 'sync' option is on) [overrides 'waitid' and 'runtime']");
			addOption ("sport", &sport, 1, SSI_INT, "sync port number (-1 for any)");
			addOption ("shost", &shost, SSI_MAX_CHAR, SSI_CHAR, "sync host (empty for any)");
			addOption ("stype", &stype, 1, SSI_UCHAR, "sync protocol type (0=UDP, 1=TCP)");	
			addOption ("sdialog", &sdialog, 1, SSI_BOOL, "show sync dialog (server only)");
			
			addOption ("tserver", &tserver, 1, SSI_BOOL, "start time server (waits for a TCP connection and returns current framework UTC time in a in a SYSTEMTIME struct)");
			addOption ("tport", &tport, 1, SSI_INT, "time server listening port");					
			addOption ("info", &info, 1, SSI_BOOL, "create framework info file");
			addOption ("loglevel", &loglevel, 1, SSI_INT, "log level (0=error only, 1=warnings, 2=basic, 3=detailed, 4=debug, 5=verbose");
		}		

		void setMonitorPos (int x, int y, int width, int height) {
			monitor = true;
			mpos[0] = x;
			mpos[1] = y;
			mpos[2] = width;
			mpos[3] = height;
		}

		void setConsolePos (int x, int y, int width, int height) {
			console = true;
			cpos[0] = x;
			cpos[1] = y;
			cpos[2] = width;
			cpos[3] = height;
		}

		ssi_size_t countdown;
		double runtime;
		ssi_char_t waitid[SSI_MAX_CHAR];

		bool console;
		int cpos[4];
		bool monitor;
		int mpos[4];		
		ssi_size_t mupd;
		bool sync;

		int sport;
		ssi_char_t shost[SSI_MAX_CHAR];
		Socket::TYPE::List stype;
		bool slisten;
		bool sdialog;

		bool info;

		bool tserver;
		int tport;	

		int loglevel;
	};

public:

	Options *getOptions () { return &_options; }
	static const ssi_char_t *GetCreateName () { return "TheFramework"; }
	const ssi_char_t *getName () { return GetCreateName(); }
	const ssi_char_t *getInfo () { return "Handles data buffering and communication between components of a pipeline."; }
	static IObject *Create (const char *file) { return new TheFramework (file); }

	void SetLogLevel (int level);

    void Start ();
    void Stop ();
    void Clear ();
	void Wait ();
	void CancelWait ();
	bool DoRestart();

	ssi_time_t GetStartTime ();
	ssi_size_t GetStartTimeMs ();
	void GetStartTimeLocal (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond);
	void GetStartTimeSystem (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond);
	ssi_time_t GetElapsedTime ();
	ssi_size_t GetElapsedTimeMs ();
	ssi_time_t GetRunTime ();
	ssi_size_t GetRunTimeMs ();	
	
	bool IsInIdleMode ();

	void SetAutoRun (bool flag) {
		_is_auto_run = flag;
	}
	bool IsAutoRun () {
		return _is_auto_run;
	}

	void AddConsumer (ITransformable *source,
		IConsumer *iconsumer, 
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		ITransformer *transformer = 0,
		ITransformable *trigger = 0);
	void AddConsumer (ssi_size_t n_sources, 
		ITransformable **sources,
		IConsumer *iconsumer, 
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		ITransformer **itransformer = 0,
		ITransformable *trigger = 0);

	void AddEventConsumer (ITransformable *source,
		IConsumer *iconsumer, 
		ITheEventBoard *event_board,
		const ssi_char_t *address,
		IEvents::EVENT_STATE_FILTER::List state_filter = IEvents::EVENT_STATE_FILTER::ALL,
		ITransformer *transformer = 0);
	void AddEventConsumer(ssi_size_t n_sources,
		ITransformable **sources,
		IConsumer *iconsumer,
		ITheEventBoard *event_board,
		const ssi_char_t *address,
		IEvents::EVENT_STATE_FILTER::List state_filter = IEvents::EVENT_STATE_FILTER::ALL,
		ITransformer **itransformer = 0);

	ITransformable *AddProvider (ISensor *sensor,
		const ssi_char_t *channel,
		IFilter *filter = 0,
		const ssi_char_t *buffer_capacity = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		const ssi_char_t *watch_interval = THEFRAMEWORK_DEFAULT_WATCH_DUR,
		const ssi_char_t *sync_interval = THEFRAMEWORK_DEFAULT_SYNC_DUR);
	void AddSensor (ISensor *isensor);

	ITransformable *AddTransformer (ITransformable *source, 
		ITransformer *itransformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		const ssi_char_t *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		ITransformable *trigger = 0);
	ITransformable *AddTransformer (ITransformable *source, 
		ssi_size_t n_xtra_sources,
		ITransformable **xtra_sources,
		ITransformer *itransformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		const ssi_char_t *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		ITransformable *trigger = 0);

	void AddDecorator(IObject *decorator);
	int AddRunnable (IRunnable *runnable);
	void AddExeJob (const ssi_char_t *exe, const ssi_char_t *args, EXECUTE::list type, int wait);
	void SetStartMessage(const ssi_char_t *text);
	void SetWaitable(IWaitable *waitable);

	bool IsBufferInUse (int buffer_id);

	int GetData (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration);
	int GetData (int buffer_id, ssi_stream_t &stream, ssi_time_t start_time, ssi_time_t duration);
	int GetData (int buffer_id, ssi_byte_t *data, ssi_size_t samples, ssi_lsize_t position);
	int GetDataTry (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration);
	
	bool GetCurrentSampleTime (int buffer_id, ssi_time_t &time);
	bool GetCurrentWritePos (int buffer_id, ssi_lsize_t &position);
	bool SetCurrentSampleTime (int buffer_id, ssi_time_t time);
	bool GetLastAccessedSampleTime (int buffer_id, ssi_time_t &time);
	bool GetOffsetTime (int buffer_id, ssi_time_t &time);
	bool GetSampleRate (int buffer_id, ssi_time_t &sample_rate);
	bool GetTotalSampleBytes (int buffer_id, ssi_size_t &sample_bytes);
	bool GetSampleBytes (int buffer_id, ssi_size_t &sample_bytes);
	bool GetSampleType (int buffer_id, ssi_type_t &sample_type);
	bool GetSampleDimension (int buffer_id, ssi_size_t &sample_dimension);
	bool GetCapacity (int buffer_id, ssi_time_t &capacity);

protected:
    
 	int AddBuffer (ssi_time_t sample_rate, ssi_size_t sample_dimension, ssi_size_t sample_bytes, ssi_type_t sample_type, const char *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP);
	bool SetMetaData (int buffer_id, ssi_size_t size, const void *meta);
	const void *GetMetaData (int buffer_id, ssi_size_t &size);
    bool RemBuffer (int buffer_id);
	bool ResetBuffer (int buffer_id, ssi_time_t offset);

    int PushData  (int buffer_id, const ssi_byte_t *data, ssi_size_t sample_number);
    int PushZeros  (int buffer_id);
	
	bool Synchronize (int buffer_id);

protected:

    TheFramework (const ssi_char_t *_file);
	~TheFramework ();
	ssi_char_t *_file;
	Options _options;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
	static ssi_char_t *info_file_name;

	Mutex _mutex;
	IRunnable *_monitor;

	void countdown(ssi_size_t n_seconds);
	void message();

	Socket *_sync_socket;
	SYNC_MSG_ID::List _sync_msg_id;
	bool recv_sync_msg(Socket *socket, SYNC_MSG_TYPE::List &type, SYNC_MSG_ID::List &id);
	void send_sync_msg(Socket *socket, SYNC_MSG_TYPE::List type, SYNC_MSG_ID::List id);
#if _WIN32|_WIN64
	SYSTEMTIME _system_time;
	SYSTEMTIME _local_time;
#else
    /*
        typedef struct _SYSTEMTIME {
          uint32_t wYear;
          uint32_t wMonth;
          uint32_t wDayOfWeek;
          uint32_t wDay;
          uint32_t wHour;
          uint32_t wMinute;
          uint32_t wSecond;
          uint32_t wMilliseconds;
        } SYSTEMTIME;

        SYSTEMTIME _system_time;
        SYSTEMTIME _local_time;*/

        uint64_t _system_time;
        uint64_t _local_time;
#endif
	ssi_size_t _start_run_time;
	ssi_size_t _last_run_time;

	volatile bool _is_running;
	bool _is_auto_run;

	bool _cancel_wait;

    // buffer array
    Buffer *buffer[THEFRAMEWORK_BUFFER_NUM];
	// runnable array
	IRunnable *runnable[THEFRAMEWORK_THREAD_NUM];
	// component array
	IRunnable *component[THEFRAMEWORK_COMPONENT_NUM];
	ssi_size_t component_counter;
	// mutex to lock buffer
	Mutex bufferMutex[THEFRAMEWORK_BUFFER_NUM];
	// condition variables for full buffers
	Condition bufferCondFull[THEFRAMEWORK_BUFFER_NUM];
	// condition variables for empty buffers
	Condition bufferCondEmpty[THEFRAMEWORK_BUFFER_NUM];
	// is buffer in use?
    bool bufferInUse[THEFRAMEWORK_BUFFER_NUM];
	// time between two synchronized push calls
	//ssi_size_t syncDur[THEFRAMEWORK_BUFFER_NUM];
	//ssi_size_t syncDurCounter[THEFRAMEWORK_BUFFER_NUM];

	// decorator
	std::vector<Decorator *> _decorators;

	// waitable
	IWaitable *_waitable;

	// executable jobs 
	struct job_s {
		String exe; 
		String args;
		EXECUTE::list type;
		int wait; 
	};
	std::vector<job_s> _jobs;
	ssi_char_t *_start_message;

	File *_info;
	TimeServer *_tserver;
};

}

#endif // THEFRAMEWORK_H
