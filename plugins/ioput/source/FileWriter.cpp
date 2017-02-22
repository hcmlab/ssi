// FileWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/04
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

#include "FileWriter.h"
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

ssi_char_t *FileWriter::ssi_log_name = "filewriter";

FileWriter::FileWriter (const ssi_char_t *file)
	: _file (0),
	_fileptr (0),
	_total_sample_number (0),
	_position (0),
	_n_meta (0),
	_meta (0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}

	_frame = Factory::GetFramework ();
}

FileWriter::~FileWriter () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	delete[] _meta; _meta = 0;
	_n_meta = 0;
}

void FileWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "open stream output > '%s'", _options.path);

	if (_options.version <= File::V1) {

		_total_sample_number = 0;
		// open file	
		_fileptr = File::CreateAndOpen (_options.type, _options.mode, _options.path);		
		_fileptr->setFormat (_options.delim, SSI_FILE_DEFAULT_FLAGS);
		_fileptr->setType (stream_in[0].type);
		
		// write header
		FileTools::WriteStreamHeader (*_fileptr, stream_in[0], _options.version);

		// if continuous stream add zero timestamp, store position and 
		// add enough space to insert in total sample number
		if (_options.version <= File::V1) {
			if (_options.stream) {
				_position = _fileptr->tell ();
				if (_fileptr->getType () == File::ASCII) {
					sprintf (_string, "                        ");
					_fileptr->writeLine (_string);
				} else {
					ssi_time_t consume_time = 0.0;
					_fileptr->write (&consume_time, sizeof (ssi_time_t), 1);
					_fileptr->write (&_total_sample_number, sizeof (ssi_size_t), 1);
				}
			}
		}
	} else {
		if (_options.type == File::ASCII) {
			_out.setDelim(_options.delim);
		}
		_out.open (stream_in[0], _n_meta, _meta, _options.path, _options.type, _options.version);
	}

	_first_call = true;
}

void FileWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (_options.version <= File::V1) {
		FileTools::WriteStreamData (*_fileptr, stream_in[0], _options.version, !_options.stream);
		_total_sample_number += stream_in[0].num;
	} else {
		if (_first_call && _out.getInfoFile ()) {
			ssi_size_t lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond;
			_frame->GetStartTimeLocal (lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond);
			ssi_size_t syear, smonth, sday, shour, sminute, ssecond, smsecond;
			_frame->GetStartTimeSystem (syear, smonth, sday, shour, sminute, ssecond, smsecond);
			ssi_size_t time = _frame->GetStartTimeMs ();			
			/*ssi_char_t time_s[100];			
			ssi_time_sprint (time, time_s);*/
			ssi_sprint (_string, "\t<time ms=\"%u\" local=\"%02u/%02u/%02u %02u:%02u:%02u:%u\" system=\"%02u/%02u/%02u %02u:%02u:%02u:%u\"/>", time, lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond, syear, smonth, sday, shour, sminute, ssecond, smsecond);
			_out.getInfoFile ()->writeLine (_string);
			_first_call = false;
		}
		_out.write (stream_in[0], _options.stream);
	}

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%u samples written", stream_in[0].num);
}

void FileWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	ssi_msg (SSI_LOG_LEVEL_BASIC, "close stream output > '%s'", _options.path);	

	if (_options.version <= File::V1) {

		// if continuous fill in total sample number
		if (_options.stream) {	
			_fileptr->seek (_position, File::BEGIN);
			if (_fileptr->getType () == File::ASCII) {
				sprintf (_string, "%.1lf %20u", 0.0, _total_sample_number);
				_fileptr->writeLine (_string);
			} else {
				ssi_time_t consume_time = 0.0;
				_fileptr->write (&consume_time, sizeof (ssi_time_t), 1);
				_fileptr->write (&_total_sample_number, sizeof (ssi_size_t), 1);
			}
		}

		delete _fileptr;
		_fileptr = 0;

	} else {
		_out.close ();	
	}
}

}
