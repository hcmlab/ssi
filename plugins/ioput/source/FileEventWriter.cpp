// FileEventWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/11/16
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

#include "FileEventWriter.h"
#include "ioput/file/FileTools.h"
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

ssi_char_t *FileEventWriter::ssi_log_name = "efilewrite";

FileEventWriter::FileEventWriter (const ssi_char_t *file)
	: _file (0),
	_first_call (false),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

FileEventWriter::~FileEventWriter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void FileEventWriter::listen_enter () {

	_eout.open (_options.path, File::BINARY);
	_first_call = true;
}

bool FileEventWriter::update (ssi_event_t &e) {

	if (_first_call) {
		ITheFramework *frame = Factory::GetFramework ();
		ssi_size_t lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond;
		frame->GetStartTimeLocal (lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond);
		ssi_size_t syear, smonth, sday, shour, sminute, ssecond, smsecond;
		frame->GetStartTimeSystem (syear, smonth, sday, shour, sminute, ssecond, smsecond);
		ssi_size_t time = frame->GetStartTimeMs ();			
		/*ssi_char_t time_s[100];			
		ssi_time_sprint (time, time_s);*/
		ssi_sprint (_string, "\t<time ms=\"%u\" local=\"%02u/%02u/%02u %02u:%02u:%02u:%u\" system=\"%02u/%02u/%02u %02u:%02u:%02u:%u\"/>", time, lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond, syear, smonth, sday, shour, sminute, ssecond, smsecond);
		_eout.getFile ()->writeLine (_string);
		_first_call = false;
	}

	_eout.write (e);

	return true;
}

bool FileEventWriter::update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

	if (_first_call) {
		ITheFramework *frame = Factory::GetFramework ();
		ssi_size_t lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond;
		frame->GetStartTimeLocal (lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond);
		ssi_size_t syear, smonth, sday, shour, sminute, ssecond, smsecond;
		frame->GetStartTimeSystem (syear, smonth, sday, shour, sminute, ssecond, smsecond);
		ssi_size_t time = frame->GetStartTimeMs ();			
		/*ssi_char_t time_s[100];			
		ssi_time_sprint (time, time_s);*/
		ssi_sprint (_string, "\t<time ms=\"%u\" local=\"%02u/%02u/%02u %02u:%02u:%02u:%u\" system=\"%02u/%02u/%02u %02u:%02u:%02u:%u\"/>", time, lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond, syear, smonth, sday, shour, sminute, ssecond, smsecond);
		_eout.getFile ()->writeLine (_string);
		_first_call = false;
	}

	if (n_new_events > 0) {
		ssi_event_t **es = new ssi_event_t *[n_new_events];
		for (ssi_size_t i = 0; i < n_new_events; i++) {
			es[i] = events.next ();
		}
		for (ssi_size_t i = n_new_events; i > 0; i--) {
			_eout.write (*es[i-1]);
		}		
		delete[] es;
	}

	return true;
}

void FileEventWriter::listen_flush () {

	_eout.close ();
}

}
