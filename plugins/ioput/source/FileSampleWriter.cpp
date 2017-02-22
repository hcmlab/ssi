// FileSampleWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/11/20
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

#include "FileSampleWriter.h"
#include "ioput/file/FileTools.h"
#include "model/SampleList.h"
#include "base/Factory.h"
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

ssi_char_t *FileSampleWriter::ssi_log_name = "filesampwr";

FileSampleWriter::FileSampleWriter (const ssi_char_t *file)
	: _file (0),
	_sample(0),
	_mutex(0),
	_class_id(SSI_ISAMPLES_GARBAGE_CLASS_ID),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	_mutex = new Mutex();
	_frame = Factory::GetFramework ();
}

FileSampleWriter::~FileSampleWriter () {

	delete _mutex; _mutex = 0;

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void FileSampleWriter::consume_enter(ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_msg(SSI_LOG_LEVEL_BASIC, "open sample output > '%s'", _options.path);

	if (_options.classes[0] == '\0') {
		ssi_err("classes not set");
	}
	if (_options.user[0] == '\0') {
		ssi_err("user not set");
	}

	ssi_size_t n_classes = ssi_split_string_count(_options.classes, ';');

	ssi_msg(SSI_LOG_LEVEL_BASIC, "found %u class names", n_classes);
	ssi_char_t **classes = new ssi_char_t *[n_classes];
	ssi_split_string(n_classes, classes, _options.classes, ';');

	SampleList samples;
	samples.addUserName(_options.user);
	for (ssi_size_t i = 0; i < n_classes; i++) {
		samples.addClassName (classes[i]);
		delete[] classes[i];
	}
	delete[] classes;	

	_sample = new ssi_sample_t;
	ssi_sample_create(*_sample, stream_in_num, 0, 0, 0, 1.0f);
	for (ssi_size_t i = 0; i < stream_in_num; i++) {
		_sample->streams[i] = &stream_in[i];
	}
	// add dummy to set up streams
	samples.addSample(_sample, true);
	_out.open(samples, _options.path, _options.type, _options.version);			

	_first_call = true;	
}

bool FileSampleWriter::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (n_new_events > 0) {

		ssi_event_t **es = new ssi_event_t *[n_new_events];
		for (ssi_size_t i = 0; i < n_new_events; i++) {
			es[i] = events.next();
		}

		for (ssi_size_t i = 0; i < n_new_events; i++) {
			classFromEvent(es[i]);
		}

		delete[] es;
	}	

	return true;
}

bool FileSampleWriter::classFromEvent(ssi_event_t *e) {

	if (e->type == SSI_ETYPE_STRING && e->ptr) {

		Lock lock(*_mutex);

		if (e->ptr[0] == '\0') {
			_class_id = SSI_ISAMPLES_GARBAGE_CLASS_ID;
		} else {
			_class_id = _out.getClassId(e->ptr);
		}
		return true;
	}
	return false;
}

void FileSampleWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_first_call && _out.getInfoFile ()) {
		ssi_size_t lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond;
		_frame->GetStartTimeLocal (lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond);
		ssi_size_t syear, smonth, sday, shour, sminute, ssecond, smsecond;
		_frame->GetStartTimeSystem (syear, smonth, sday, shour, sminute, ssecond, smsecond);
		ssi_size_t time = _frame->GetStartTimeMs ();			
		ssi_sprint (_string, "\t<time ms=\"%u\" local=\"%02u/%02u/%02u %02u:%02u:%02u:%u\" system=\"%02u/%02u/%02u %02u:%02u:%02u:%u\"/>", time, lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond, syear, smonth, sday, shour, sminute, ssecond, smsecond);
		_out.getInfoFile ()->writeLine (_string);
		_first_call = false;
	}

	if (consume_info.event) {		
		classFromEvent(consume_info.event);
	}

	{
		Lock lock(*_mutex);
		_sample->class_id = _class_id;
	}
	
	_sample->time = consume_info.time;
	for (unsigned int i = 0; i < stream_in_num; i++) {		
		_sample->streams[i] = &stream_in[i];
	}
	
	_out.write(*_sample);

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "added a sample");
}

void FileSampleWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {
	
	_out.close ();	

	_class_id = SSI_ISAMPLES_GARBAGE_CLASS_ID;
	for (ssi_size_t i = 0; i < stream_in_num; i++) {
		_sample->streams[i] = 0;
	}
	ssi_sample_destroy(*_sample);
	delete _sample; _sample = 0;
	

	ssi_msg(SSI_LOG_LEVEL_BASIC, "close sample output > '%s'", _options.path);
}

}
