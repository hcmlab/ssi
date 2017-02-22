// FileTools.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/07/21
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

#ifndef SSI_IOPUT_FILETOOLS_H
#define SSI_IOPUT_FILETOOLS_H

#include "ioput/file/File.h"
#include "ioput/file/StringList.h"

/*

signal header:
	<sample_rate> <sample_dimension> <sample_bytes>

data header:
    <time> <sample_number>
 
signal files may consist of:
either: one signal header followed by one ore more data header
or:     one ore more signal header each followed by exactly one data header
a combination of both types is not supported!!!
 
*/

namespace ssi {

class FileTools {                                                                

public:
	
	// returns whole file as a string or 0 if an error occurs
	// len = length of the string without null byte
	static char *ReadAsciiFile (const ssi_char_t *path, ssi_size_t &len);
	static bool WriteAsciiFile (const ssi_char_t *path, const ssi_char_t *str);

	static bool CountLines (File &file, ssi_size_t &n_lines);
	static bool ReadRawFile (File::TYPE type,
		const ssi_char_t *path,
		ssi_stream_t &data);
	static bool ReadRawFile (File &file,	
		ssi_stream_t &data);

	// tries to read a signal header
	// returns false if end of file is reached
	static bool ReadStreamHeader (File &file,
		ssi_stream_t &data); // deprecated
	static bool ReadStreamHeader (File &file,
		ssi_stream_t &data,
		File::VERSION  &version);
	// counts the number of stream header in the file
	// each stream header must be followed by exactly one data header
	// otherwise the function call will fail
	static ssi_size_t CountStreamHeader (File &file);
	// counts the number of data header in the file
	// the file must consist of one data header followed
	// by an abitrary number of data headers	
	static ssi_size_t CountDataHeader (File &file, ssi_size_t &tot_sample_number);
	// tries to read a data header and reads in the data
	// returns false if end of file is reached
	static bool ReadStreamData (File &file,
		ssi_time_t *time,
		ssi_stream_t &data); // deprecated since V1
	static bool ReadStreamData (File &file,		
		ssi_stream_t &data,
		File::VERSION version); // deprecated since V2
	static void ReadStreamFile (File &file,
		ssi_stream_t &data); // deprecated since V2
	static void ReadStreamFile (File::TYPE type,
		const ssi_char_t *path,
		ssi_stream_t &data); // deprecated since V2
	static bool ReadStreamFile (const ssi_char_t *path,
		ssi_stream_t &data);
	static bool RepairStreamFile (File::TYPE type,
		const ssi_char_t *path,
		ssi_stream_t &data);
	static bool RepairStreamFileV2 (File::TYPE type,
		const ssi_char_t *path,
		ssi_stream_t &data);

	static void WriteStreamHeader (File &file,
		ssi_stream_t &data); // deprecated since V1
	static void WriteStreamHeader (File &file,
		ssi_stream_t &data,
		File::VERSION  version); // deprecated since V2
	static void WriteStreamData (File &file,
		ssi_time_t time,
		ssi_stream_t &data,
		bool add_header = true);  // deprecated since V2
	static void WriteStreamData (File &file,
		ssi_stream_t &data,
		File::VERSION  version,		
		bool add_header = true); // deprecated since V2 
	static void WriteStreamFile (File &file,
		ssi_stream_t &data,
		File::VERSION  version = File::DEFAULT_VERSION);
	static bool WriteStreamFile (File::TYPE type,
		const ssi_char_t *path,
		ssi_stream_t &data,
		File::VERSION version = File::DEFAULT_VERSION);

	static void ConvertStreamV0toV1Binary (const ssi_char_t *filename,
		ssi_type_t type,
		bool backup = true);
	static void ConvertStreamV1toV2Binary (const ssi_char_t *filename,
		bool backup = true);

	static bool ReadSampleHeader (File &file,
		ssi_sample_t &data);
	static bool ReadSampleData (File &file,
		ssi_sample_t &data);
	static void WriteSampleHeader (File &file,
		ssi_sample_t &data,
		File::VERSION  version); 
	static void WriteSampleData (File &file,
		ssi_sample_t &data,
		File::VERSION  version);

	static void ReadFilesFromDir (StringList &files, 
		const ssi_char_t *dir,
		const ssi_char_t *filter = "*");
	static void ReadDirsFromDir (StringList &files, 
		const ssi_char_t *dir,
		const ssi_char_t *filter = "*",
		bool ignore_hidden = true);

	static void ReadLinesFromFile(StringList &files,
		const ssi_char_t *path);
	static void ReadFilesFromFile (StringList &files, 
		const ssi_char_t *path);
	static ssi_byte_t *ReadBytesFromFile(ssi_size_t &n_bytes,
		const ssi_char_t *path);

	// Opens a file dialog which allows user to select a file
	static bool SelectFile (char *file);
	// Opens a file dialog which allows user to save a file
	static bool SelectSaveFile (char *file);
	// Opens a file dialog which allows user to select a directory
	static bool SelectDirectory (char *file);

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;
};

}

#endif
