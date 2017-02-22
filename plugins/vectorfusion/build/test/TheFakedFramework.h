// ITheFramework.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/20 
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_THEFAKEDFRAMEWORK_H
#define SSI_THEFAKEDFRAMEWORK_H

#include "base/ITheFramework.h"

namespace ssi {

class TheFakedFramework : public ITheFramework {

public: 	

	IOptions *getOptions () { return 0; }
	static const ssi_char_t *GetCreateName () { return "TheFramework"; }
	const ssi_char_t *getName () { return GetCreateName(); }
	const ssi_char_t *getInfo () { return "A faked framework."; }
	static IObject *Create (const char *file) { return new TheFakedFramework (); }

	TheFakedFramework () {
		_time = 0;
	};
	~TheFakedFramework () {};

	void SetLogLevel (int level) {};

    void Start () {};
    void Stop () {};
	void Clear () {};

	ssi_time_t GetStartTime () { return 0; };
	ssi_size_t GetStartTimeMs () { return 0; };
	void GetStartTimeLocal (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond) {};
	void GetStartTimeSystem (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond) {};
	ssi_time_t GetElapsedTime () { return 0; };
	ssi_size_t GetElapsedTimeMs () { return _time; };
	ssi_time_t GetRunTime () { return 0; };
	ssi_size_t GetRunTimeMs () { return 0; };
	bool IsInIdleMode () { return false; };

	ITransformable *AddProvider (ISensor *isensor,
		const ssi_char_t *channel,
		IFilter *ifilter = 0,
		const ssi_char_t *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		ssi_time_t check_interval_in_seconds = 1.0,
		ssi_time_t sync_interval_in_seconds = 5.0) { return 0; };
	void AddSensor (ISensor *isensor) {};

	ITransformable *AddTransformer (ITransformable *source, 
		ITransformer *itransformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		const ssi_char_t *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		ITransformable *trigger = 0) {
		return 0;
	};
	ITransformable *AddTransformer (ITransformable *source, 
		ssi_size_t n_xtra_sources,
		ITransformable **xtra_sources,
		ITransformer *itransformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		const ssi_char_t *buffer_size = THEFRAMEWORK_DEFAULT_BUFFER_CAP,
		ITransformable *trigger = 0) {
		return 0;
	};

	void AddConsumer (ITransformable *source,
		IConsumer *iconsumer, 
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		ITransformer *transformer = 0,
		ITransformable *trigger = 0) {};
	void AddConsumer (ssi_size_t n_sources, 
		ITransformable **sources,
		IConsumer *iconsumer, 
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		ITransformer **itransformer = 0,
		ITransformable *trigger = 0) {};

	void AddEventConsumer (ITransformable *source,
		IConsumer *iconsumer, 
		ITheEventBoard *event_board,
		const ssi_char_t *address,
		IEvents::EVENT_STATE_FILTER::List state_filter, 
		ITransformer *transformer = 0) {};
	void AddEventConsumer (ssi_size_t n_sources, 
		ITransformable **sources,
		IConsumer *iconsumer, 
		ITheEventBoard *event_board,
		const ssi_char_t *address,
		IEvents::EVENT_STATE_FILTER::List state_filter,
		ITransformer **itransformer = 0) {};

	int AddRunnable (IRunnable *runnable = 0) { return 0; };

	void AddDecorator(IObject *decorator) {};

	bool IsBufferInUse (int buffer_id) { return false; };
	int GetData (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration) { return 0; };
	int GetData (int buffer_id, ssi_stream_t &stream, ssi_time_t start_time, ssi_time_t duration) { return 0; };
	int GetData (int buffer_id, ssi_byte_t *data, ssi_size_t samples, ssi_size_t position) { return 0; };
	int GetDataTry (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration) { return 0; };
	bool GetCurrentSampleTime (int buffer_id, ssi_time_t &time) { return false; };
	bool GetCurrentWritePos (int buffer_id, ssi_size_t &position) { return false; };
	bool SetCurrentSampleTime (int buffer_id, ssi_time_t time) { return false; };
	bool GetLastAccessedSampleTime (int buffer_id, ssi_time_t &time) { return false; };
	bool GetOffsetTime (int buffer_id, ssi_time_t &time) { return false; };
	bool GetSampleRate (int buffer_id, ssi_time_t &sample_rate) { return false; };
	bool GetTotalSampleBytes (int buffer_id, ssi_size_t &sample_bytes) { return false; };
	bool GetSampleBytes (int buffer_id, ssi_size_t &sample_bytes) { return false; };
	bool GetSampleType (int buffer_id, ssi_type_t &sample_type) { return false; };
	bool GetSampleDimension (int buffer_id, ssi_size_t &sample_dimension) { return false; };
	bool GetCapacity (int buffer_id, ssi_time_t &capacity) { return false; };

	void AddExeJob(const ssi_char_t *,const ssi_char_t *,ssi::ITheFramework::EXECUTE::list,int) { };

	void CancelWait() {

	}

	void Wait () {
		ssi_print ("\n\n\tpress enter to stop pipeline\n");
		getchar ();
	}

	void SetElapsedTimeMs (ssi_size_t time) {
		_time = time;
	}

protected:

	ssi_size_t _time;
};

}

#endif // THEFRAMEWORK_H
