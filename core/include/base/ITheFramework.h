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

#ifndef SSI_BASE_ITHEFRAMEWORK_H
#define SSI_BASE_ITHEFRAMEWORK_H

#include "base/IObject.h"
#include "base/IWaitable.h"
#include "base/ITransformable.h"
#include "base/ISensor.h"
#include "base/IConsumer.h"
#include "base/IProvider.h"
#include "base/ITransformer.h"
#include "base/IFeature.h"
#include "base/IFilter.h"
#include "base/ITheEventBoard.h"

namespace ssi {

class ITheFramework : public IObject {

public:

	struct EXECUTE {
		enum list {
			NOW = 0, // will be exeuted immediately
			PRE = 1, // will be executed before the pipeline is started 
			POST = 2 // will be executed after the pipeline is stopped
		};
	};

public: 	

	virtual ~ITheFramework () {};

	virtual void SetLogLevel (int level) = 0;

    virtual void Start () = 0;
    virtual void Stop () = 0;
	virtual void Clear () = 0;
	virtual void Wait () = 0;
	virtual void CancelWait() = 0;
	virtual bool DoRestart() = 0;

	virtual ssi_time_t GetStartTime () = 0;
	virtual ssi_size_t GetStartTimeMs () = 0;
	virtual void GetStartTimeLocal (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond) = 0;
	virtual void GetStartTimeSystem (ssi_size_t &year, ssi_size_t &month, ssi_size_t &day, ssi_size_t &hour, ssi_size_t &minute, ssi_size_t &second, ssi_size_t &msecond) = 0;
	virtual ssi_time_t GetElapsedTime () = 0;
	virtual ssi_size_t GetElapsedTimeMs () = 0;
	virtual ssi_time_t GetRunTime () = 0;
	virtual ssi_size_t GetRunTimeMs () = 0;
	virtual bool IsInIdleMode () = 0;

	virtual ITransformable *AddProvider (ISensor *isensor,
		const ssi_char_t *channel,
		IFilter *ifilter = 0,
		const ssi_char_t *buffer_size = "10.0s",
		const ssi_char_t *watch_interval = "1.0s",
		const ssi_char_t *sync_interval ="5.0s") = 0;
	virtual void AddSensor (ISensor *isensor) = 0;

	virtual ITransformable *AddTransformer (ITransformable *source, 
		ITransformer *itransformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		const ssi_char_t *buffer_size = "10.0s",
		ITransformable *trigger = 0) = 0;
	virtual ITransformable *AddTransformer (ITransformable *source, 
		ssi_size_t n_xtra_sources,
		ITransformable **xtra_sources,
		ITransformer *itransformer,
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		const ssi_char_t *buffer_size = "10.0s",
		ITransformable *trigger = 0) = 0;

	virtual void AddConsumer (ITransformable *source,
		IConsumer *iconsumer, 
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		ITransformer *transformer = 0,
		ITransformable *trigger = 0) = 0;
	virtual void AddConsumer (ssi_size_t n_sources, 
		ITransformable **sources,
		IConsumer *iconsumer, 
		const ssi_char_t *frame_size,
		const ssi_char_t *delta_size = 0,
		ITransformer **itransformer = 0,
		ITransformable *trigger = 0) = 0;

	virtual void AddEventConsumer (ITransformable *source,
		IConsumer *iconsumer, 
		ITheEventBoard *event_board,
		const ssi_char_t *address,
		IEvents::EVENT_STATE_FILTER::List state_filter = IEvents::EVENT_STATE_FILTER::ALL,
		ITransformer *transformer = 0) = 0;
	virtual void AddEventConsumer (ssi_size_t n_sources, 
		ITransformable **sources,
		IConsumer *iconsumer, 
		ITheEventBoard *event_board,
		const ssi_char_t *address,
		IEvents::EVENT_STATE_FILTER::List state_filter = IEvents::EVENT_STATE_FILTER::ALL,
		ITransformer **itransformer = 0) = 0;

	virtual void AddDecorator(IObject *decorator) = 0;
	virtual int AddRunnable (IRunnable *runnable = 0) = 0;
	// last paramter defines milliseconds to wait, if < 0 until job is finished [-1] 
	virtual void AddExeJob (const ssi_char_t *exe, const ssi_char_t *args, EXECUTE::list type, int wait) = 0;
	virtual void SetStartMessage(const ssi_char_t *text) = 0;	
	virtual void SetWaitable(IWaitable *waitable) = 0;

	virtual bool IsBufferInUse (int buffer_id) = 0;
	virtual int GetData (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration) = 0;
	virtual int GetData (int buffer_id, ssi_stream_t &stream, ssi_time_t start_time, ssi_time_t duration) = 0;
	virtual int GetData (int buffer_id, ssi_byte_t *data, ssi_size_t samples, ssi_lsize_t position) = 0;
	virtual int GetDataTry (int buffer_id, ssi_byte_t **data, ssi_size_t &samples_in, ssi_size_t &samples_out, ssi_time_t start_time, ssi_time_t duration) = 0;
	virtual bool GetCurrentSampleTime (int buffer_id, ssi_time_t &time) = 0;
	virtual bool GetCurrentWritePos (int buffer_id, ssi_lsize_t &position) = 0;
	virtual bool SetCurrentSampleTime (int buffer_id, ssi_time_t time) = 0;
	virtual bool GetLastAccessedSampleTime (int buffer_id, ssi_time_t &time) = 0;
	virtual bool GetOffsetTime (int buffer_id, ssi_time_t &time) = 0;
	virtual bool GetSampleRate (int buffer_id, ssi_time_t &sample_rate) = 0;
	virtual bool GetTotalSampleBytes (int buffer_id, ssi_size_t &sample_bytes) = 0;
	virtual bool GetSampleBytes (int buffer_id, ssi_size_t &sample_bytes) = 0;
	virtual bool GetSampleType (int buffer_id, ssi_type_t &sample_type) = 0;
	virtual bool GetSampleDimension (int buffer_id, ssi_size_t &sample_dimension) = 0;
	virtual bool GetCapacity (int buffer_id, ssi_time_t &capacity) = 0;

};

}

#endif // THEFRAMEWORK_H
