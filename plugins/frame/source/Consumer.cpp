// Consumer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/28
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

#include "Consumer.h"
#include "TheFramework.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

int Consumer::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t *Consumer::ssi_log_name = "consume___";

Consumer::Consumer (int buffer_id,
		IConsumer *consumer, 
		ssi_size_t frame_size,
		ssi_size_t delta_size,
		ITransformer *transformer,
		int trigger_id)
	: ConsumerBase (buffer_id,
		consumer, 
		frame_size,
		delta_size,
		transformer,
		trigger_id) {

	// set thread name
	Thread::setName(_consumer->getName());

	// add consumer to framework
	_frame = ssi_pcast (TheFramework, Factory::GetFramework ());
	if (_frame->IsAutoRun ()) {
		_frame->AddRunnable (this);
	}
}

Consumer::Consumer (ssi_size_t stream_number, 
		int *_buffer_id,
		IConsumer *consumer, 
		ssi_size_t frame_size,
		ssi_size_t delta_size,		
		ITransformer **transformer,
		int trigger_id)
	: ConsumerBase (stream_number, 
		_buffer_id,
		consumer, 
		frame_size,
		delta_size,
		transformer,
		trigger_id) {

	// set thread name
	Thread::setName(_consumer->getName());

	// add consumer to framework
	_frame = ssi_pcast (TheFramework, Factory::GetFramework ());
	if (_frame->IsAutoRun ()) {
		_frame->AddRunnable (this);
	}
}

Consumer::~Consumer () {
}

void Consumer::enter () {

	ConsumerBase::enter ();

	_consume_info.status = IConsumer::NO_TRIGGER;
	_consume_info.time = _frame->GetElapsedTime ();
	_consume_info.dur = _frame_size_in_sec + _delta_size_in_sec;
	_consume_info.event = 0;
}

void Consumer::run () {

	// try to consume _data
	// if an error occurs during receive operation of one stream
	// operation fails and error code is returned
	int status = consume (_consume_info);
	
	// check if operation was successful
	// otherwise try to handle the error
	switch (status) {
		case TimeBuffer::SUCCESS:
			// operation was successful!
			// we can move our clock a bit forward
			_consume_info.time += _frame_size_in_sec;			
			break;
		case TimeBuffer::DATA_NOT_IN_BUFFER_YET:
			// not all _data is yet available
			// we return and hope that it will be available at next call..			
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "data not in buffer yet '%s:%s'", _consumer->getName (), Factory::GetObjectId(_consumer));
			return;
		case THEFRAMEWORK_ERROR:
			// framework error, probably framework is in idle mode
			// there is not much to do for us but wait..
			//SSI_DBG (SSI_LOG_LEVEL_DEBUG, "framework not running '%s'", _consumer->getName ());
			return;
		default:
			// well, something critical happend, probably the requested _data is not available anymore
			// all we can do is to reset the timer and hope that we will succeed next _time..
			ssi_wrn ("requested data not available (%s) '%s:%s'", TimeBuffer::STATUS_NAMES[status], _consumer->getName (), Factory::GetObjectId(_consumer));
			ssi_time_t frame_time = _frame->GetElapsedTime ();
			_consumer->consume_fail (_consume_info.time, frame_time - _consume_info.time, _stream_number, _streams);
			_consume_info.time = frame_time;
			return; 
	}
}

void Consumer::flush () {
	ConsumerBase::flush ();
}

}

