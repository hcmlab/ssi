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
#include "ioput/file/FilePath.h"

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
	: _filepath(0),
	_file (0),
	_fileptr (0),
	_total_sample_number (0),
	_position (0),
	_n_meta (0),
	_meta (0),
	_first_call(true),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}

	_frame = Factory::GetFramework ();
}

FileWriter::~FileWriter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}

	delete[] _meta; _meta = 0;	
	_n_meta = 0;
}

void FileWriter::open()
{
	if (_out.ready())
	{
		return;
	}

	if (_options.overwrite)
	{
		ssi_mkdir_r(FilePath(_options.path).getDir());
		_filepath = ssi_strcpy(_options.path);
	}
	else
	{
		_filepath = FilePath(_options.path, SSI_FILE_TYPE_STREAM).getUnique(true);
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "open '%s'", _filepath);

	if (_options.type == File::ASCII)
	{
		_out.setDelim(_options.delim);
	}

	if (_options.meta[0] != '\0')
	{
		parse_meta(_options.meta, ';');
	}

	ssi_size_t n_keys = 0;
	ssi_char_t **keys = 0;
	ssi_char_t **values = 0;
	if (_keys_values.size() > 0)
	{
		n_keys = ssi_size_t(_keys_values.size());
		keys = new ssi_char_t *[n_keys];
		values = new ssi_char_t *[n_keys];
		for (ssi_size_t i = 0; i < n_keys; i++)
		{
			keys[i] = _keys_values[i].key.str();
			values[i] = _keys_values[i].value.str();
		}
	}

	_out.open(_stream, _filepath, _n_meta, _meta, n_keys, keys, values, _options.type);

	delete[] keys;
	delete[] values;

	_first_call = true;
}

void FileWriter::parse_meta(const ssi_char_t *string, char delim)
{
	ssi_size_t n_split = ssi_split_string_count(string, delim);
	if (n_split > 0)
	{
		ssi_char_t **split = new ssi_char_t *[n_split];
		ssi_split_string(n_split, split, string, delim);
		for (ssi_size_t n = 0; n < n_split; n++)
		{
			ssi_char_t *key = 0;
			ssi_char_t *value = 0;
			if (ssi_parse_option(split[n], &key, &value))
			{
				key_value_t key_value;
				key_value.key = key;
				key_value.value = value;
				_keys_values.push_back(key_value);
			}
			delete[] key;
			delete[] value;
			delete[] split[n];
		}
		delete[] split;
	}
}

void FileWriter::consume_enter (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	_stream = stream_in[0];		
	open();
}

void FileWriter::consume (IConsumer::info consume_info,
	ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) {

	if (!_out.ready())
	{
		return;
	}

	if (_first_call && _out.getInfoFile ())
	{
		ssi_size_t lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond;
		_frame->GetStartTimeLocal (lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond);
		ssi_size_t syear, smonth, sday, shour, sminute, ssecond, smsecond;
		_frame->GetStartTimeSystem (syear, smonth, sday, shour, sminute, ssecond, smsecond);
		ssi_size_t time = _frame->GetStartTimeMs ();				
		ssi_sprint (_string, "\t<time ms=\"%u\" local=\"%02u/%02u/%02u %02u:%02u:%02u:%u\" system=\"%02u/%02u/%02u %02u:%02u:%02u:%u\"/>", time, lyear, lmonth, lday, lhour, lminute, lsecond, lmsecond, syear, smonth, sday, shour, sminute, ssecond, smsecond);
		_out.getInfoFile ()->writeLine (_string);		
	}

	if (_out.write(stream_in[0], _options.stream))
	{
		_first_call = false;
	}	
	
	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "%u samples written", stream_in[0].num);
}

void FileWriter::close()
{
	if (!_out.ready())
	{
		return;
	}

	ssi_msg(SSI_LOG_LEVEL_BASIC, "close '%s'", _filepath);

	_out.close();

	if (_first_call && !_options.keepEmpty)
	{
		ssi_msg(SSI_LOG_LEVEL_BASIC, "remove '%s'", _filepath);
		ssi_remove(_filepath);
		ssi_remove((String(_filepath) + "~").str());
	}

	delete[] _filepath; _filepath = 0;
	_keys_values.clear();
}

void FileWriter::consume_flush (ssi_size_t stream_in_num,
	ssi_stream_t stream_in[]) 
{
	close();
}

bool FileWriter::notify(INotify::COMMAND::List command, const ssi_char_t *message) {

	switch (command) 
	{
	case INotify::COMMAND::SLEEP_POST:
	{	
		close();					
		return true;
	}
	case INotify::COMMAND::WAKE_PRE:
	{		
		open();
		return true;
	}	
	}

	return false;
}

}
