// FileTools.cpp
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

#include "ioput/file/FileTools.h"
#include "ioput/file/FileStreamOut.h"
#include "ioput/file/FileStreamIn.h"
#include "ioput/wav/WavTools.h"
#include "ioput/file/FilePath.h"

#if _WIN32|_WIN64
#include <commdlg.h> // for file dialog
#include <shlobj.h> // for directory dialog
#else
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
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

ssi_char_t *FileTools::ssi_log_name = "filetools_";
int FileTools::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

ssi_char_t *FileTools::ReadAsciiFile (const ssi_char_t *path, ssi_size_t &len) {

	len = 0;
	ssi_char_t *buffer = 0;

	FILE *fp = fopen (path, "r");
	if (fp) {
		
		int c;

		while (c = getc (fp) != EOF) {
			len++;
		}

		fseek (fp, 0, 0);
		buffer = new ssi_char_t[len+1];

		for (ssi_size_t i = 0; i < len; i++) {
			buffer[i] = getc (fp);
		}
		buffer[len] = '\0';		

		fclose (fp);

	} else {
		ssi_wrn ("could not open file '%s'", path);
	}

	return buffer;
}


bool FileTools::WriteAsciiFile (const ssi_char_t *path, const ssi_char_t *str) {
	
	FILE *fp = fopen (path, "w");
	if (fp) {		
		fprintf (fp, "%s", str);
		fclose (fp);
		return true;
	} 
	return false;
}

#if _WIN32|_WIN64
static int CALLBACK BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {

	if (uMsg == BFFM_INITIALIZED) {
		LPCTSTR startFolder = reinterpret_cast<LPCTSTR> (lpData);
		::SendMessage (hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(startFolder));
	}

	return 0;
}
#endif

bool FileTools::CountLines (File &file, ssi_size_t &n_lines) {

	int64_t pos = file.tell ();

	FILE *fp = file.getFile ();
	char c = getc (fp);
	n_lines = 0;
	while (c != EOF) {
		c = getc (fp);
		if (c == '\n') {
			n_lines++;
		}
	}

	file.seek (pos);

	return true;
}

bool FileTools::ReadRawFile (File::TYPE type,
	const ssi_char_t *path,		
	ssi_stream_t &data) {

	File *file = File::CreateAndOpen (type, File::READ, path);		
	bool result = FileTools::ReadRawFile (*file, data);
	delete file;
	
	return result;
}

bool FileTools::ReadRawFile (File &file,	
	ssi_stream_t &data) {

	switch (file.getType ()) {
		case File::ASCII: {

			ssi_size_t n_samples;
			if (!FileTools::CountLines (file, n_samples)) {
				return false;
			}
			ssi_stream_adjust (data, n_samples);

			file.setType (data.type);
			if (!file.read (data.ptr, data.dim, data.num * data.dim)) {
				return false;
			}

			break;
		}
		case File::BINARY: {

			int64_t pos = file.tell ();
			
			ssi_size_t n_samples = ssi_size_t(pos / (data.dim * data.byte));
			ssi_stream_adjust (data, n_samples);

			fread (data.ptr, data.tot, 1, file.getFile ());

			break;
		}
	}

	return false;
}

bool FileTools::ReadStreamHeader (File &file,
	ssi_stream_t &data) {

	File::VERSION  version;
	return ReadStreamHeader (file, data, version);
}

bool FileTools::ReadStreamHeader (File &file,
	ssi_stream_t &data,
	File::VERSION  &version) {

	if (!file.ready ()) {
		return false;
	}

	// determine version
		
	ssi_char_t id[3];
	version = File::V0;

	switch (file.getType ()) {

		case File::ASCII: {
			ssi_char_t string[256];
			int64_t pos = file.tell ();
			if (!file.readLine (256, string)) {
				ssi_err ("could not read <id>");
			}
			if (! (string[0] == 'S' && string[1] == 'S' && string[2] == 'I')) {		
				file.seek (pos, File::BEGIN);
			} else {
				int value = 0;
				sscanf (string+4, "%i", &value);
				version = ssi_cast (File::VERSION, value);
			}		
			break;
		}

		case File::BINARY: {
			if (!file.read (id, 1, 3)) {
				ssi_err ("could not read <id>");
			}
			if (! (id[0] == 'S' && id[1] == 'S' && id[2] == 'I')) {		
				file.seek (-3, File::CURRENT);
			} else {
				if (!file.read (&version, sizeof (version), 1)) {
					ssi_err ("could not read <version>");
				}
			}
			break;
		}

		default:
			ssi_err ("type not supported");
	}

	// read remaining header

	ssi_size_t sample_dimension;
	ssi_size_t sample_bytes;
	ssi_time_t sample_rate;
	ssi_type_t sample_type = SSI_UNDEF;
	
	switch (file.getType ()) {

		case File::ASCII: {

			switch (version) {

				case File::V0: {

					ssi_char_t string[1024];
					if (!file.readLine (1024, string)) {
						ssi_err ("could not read <sample_rate> <sample_dimension> <sample_bytes>");
					}
					sscanf (string, "%lf %u %u", &sample_rate, &sample_dimension, &sample_bytes);

					break;
				}

				case File::V1: {
					
					ssi_char_t string[1024];
					if (!file.readLine (1024, string)) {
						ssi_err ("could not read <sample_rate> <sample_dimension> <sample_bytes> <sample_type>");
					}
					int value = 0;
					sscanf (string, "%lf %u %u %d", &sample_rate, &sample_dimension, &sample_bytes, &value);
					sample_type = ssi_cast (ssi_type_t, value);

					break;
				}
			}

			break;
		}

		case File::BINARY: {

				switch (version) {

				case File::V0: {

					if (!file.read (&sample_rate, sizeof (sample_rate), 1)) {
						ssi_err ("could not read <sample_rate>");
					}

					if (!file.read (&sample_dimension, sizeof (sample_dimension), 1)) {
						ssi_err ("could not read <sample_dimension>");
					}

					if (!file.read (&sample_bytes, sizeof (sample_bytes), 1)) {
						ssi_err ("could not read <sample_bytes>");
					}

					break;
				}

				case File::V1: {

					if (!file.read (&sample_rate, sizeof (sample_rate), 1)) {
						ssi_err ("could not read <sample_rate>");
					}

					if (!file.read (&sample_dimension, sizeof (sample_dimension), 1)) {
						ssi_err ("could not read <sample_dimension>");
					}

					if (!file.read (&sample_bytes, sizeof (sample_bytes), 1)) {
						ssi_err ("could not read <sample_bytes>");
					}

					if (!file.read (&sample_type, sizeof (sample_type), 1)) {
						ssi_err ("could not read <sample_type>");
					}

					break;
				}
			}

			break;
		}

		default:
			ssi_err ("type not supported");
	}

	ssi_stream_init (data, 0, sample_dimension, sample_bytes, sample_type, sample_rate, 0);

	return true;
}

ssi_size_t FileTools::CountStreamHeader (File &file) {

	// store position
	int64_t offset = file.tell ();

	// read until end of file is reached
	ssi_stream_t data;
	ssi_size_t counter = 0;
	ssi_time_t time = 0.0;
	File::VERSION version;

	while (FileTools::ReadStreamHeader (file, data, version)) {
		
		// skip data
		ssi_size_t sample_number;
			
		switch (file.getType ()) {

			case File::ASCII: {
				
				ssi_char_t string[1024];
				if (!file.readLine (1024, string)) {
					ssi_err ("could not read <time> <sample_number>");
				}
				sscanf (string, "%lf %u", &time, &sample_number);

				for (ssi_size_t i = 0; i < sample_number; i++) {
					file.readLine (1024, string);
				}

				break;
			} 
			
			case File::BINARY: {	

				if (!file.read (&time, sizeof (ssi_time_t), 1)) {
					ssi_err ("could not read <time>");
				}

				if (!file.read (&sample_number, sizeof (ssi_size_t), 1)) {
					ssi_err ("could not read <sample_number>");
				}

				file.seek (sample_number * data.byte * data.dim, File::CURRENT);

				break;
			}

			default:
				ssi_err ("type not supported");
		}
		
		// increment counter
		++counter;
	}

	// go back to original position
	file.seek (offset, File::BEGIN);

	return counter;
}

ssi_size_t FileTools::CountDataHeader (File &file, ssi_size_t &tot_sample_number) {

	// store position
	int64_t offset = file.tell ();

	// read until end of file is reached
	ssi_stream_t data;
	tot_sample_number = 0;
	ssi_size_t counter = 0;
	ssi_time_t time = 0.0;
	File::VERSION version;

	// read stream header
	FileTools::ReadStreamHeader (file, data, version);

	// read data headers
	while (file.ready ()) {
		
		// skip data
		ssi_size_t sample_number;
			
		switch (file.getType ()) {

			case File::ASCII: {
			
				ssi_char_t string[1024];
				if (!file.readLine (1024, string)) {
					ssi_err ("could not read <time> <sample_number>");
				}
				sscanf (string, "%lf %u", &time, &sample_number);

				for (ssi_size_t i = 0; i < sample_number; i++) {
					file.readLine (1024, string);
				}

				break;

			} 

			case File::BINARY: {

				if (!file.read (&time, sizeof (ssi_time_t), 1)) {
					ssi_err ("could not read <time>");
				}

				if (!file.read (&sample_number, sizeof (ssi_size_t), 1)) {
					ssi_err ("could not read <sample_number>");
				}

				file.seek (sample_number * data.byte * data.dim, File::CURRENT);

				break;
			}
			
			default:
				ssi_err ("type not supported");
			
		}
		
		// increment counter
		++counter;
		tot_sample_number += sample_number;
	}

	// go back to original position
	file.seek (offset, File::BEGIN);

	return counter;
}

bool FileTools::ReadStreamData (File &file,
	ssi_time_t *time,
	ssi_stream_t &data) {

	if (!ReadStreamData (file, data, File::V0)) {
		return false;
	}

	*time = data.time;

	return true;
}

bool FileTools::ReadStreamData (File &file,
	ssi_stream_t &data,
	File::VERSION  version) {

	if (!file.ready ()) {
		return false;
	}

	ssi_size_t sample_number;
			
	switch (file.getType ()) {

		case File::ASCII: {
		
			ssi_char_t string[1024];
			if (!file.readLine (1024, string)) {
				ssi_err ("could not read <time> <sample_number>");
			}
			sscanf (string, "%lf %u", &data.time, &sample_number);

			break;
		}

		case File::BINARY: {

			if (!file.read (&data.time, sizeof (data.time), 1)) {
				ssi_err ("could not read <time>");
			}

			if (!file.read (&sample_number, sizeof (sample_number), 1)) {
				ssi_err ("could not read <sample_number>");
			}

			break;
		}
	
		default: {
			ssi_wrn ("type not supported");
			return false;
		}
	}

	ssi_stream_adjust (data, sample_number);

	file.setType (data.type);	
	if (sample_number > 0) {
		if (!file.read (data.ptr, data.byte, sample_number * data.dim)) {
			ssi_wrn ("could not read <data>");
			return false;
		}
	}

	return true;
}

void FileTools::ReadStreamFile (File &file,
	ssi_stream_t &data) {
	
	File::VERSION version;

	// allocate data
	ssi_size_t tot_sample_number;
	FileTools::CountDataHeader (file, tot_sample_number);

	// read data
	ssi_stream_t tmp;
	
	if(FileTools::ReadStreamHeader (file, tmp, version))
	{
		ssi_stream_init (data, tot_sample_number, tmp.dim, tmp.byte, tmp.type, tmp.sr, tmp.time);
		data.num = 0;
		data.tot = 0;
		while (file.ready ()) {					
			FileTools::ReadStreamData (file, tmp, version);
			ssi_stream_cat (tmp, data);
		}
		ssi_stream_destroy (tmp);
	}
}

bool FileTools::RepairStreamFile (File::TYPE type,
	const ssi_char_t *path,
	ssi_stream_t &data) {	

	// open file
	File *file = File::CreateAndOpen (type, File::READ, path);	
	
	// try to read header
	if (!FileTools::ReadStreamHeader (*file, data)) {
		ssi_wrn ("repair failed: could not parse header");
		delete file;
		return false;
	}
	
	// check header
	if (data.num == 0) {
		ssi_msg_static (SSI_LOG_LEVEL_DEFAULT, "field 'sample number' is empty");

		ssi_size_t sample_number = 0;
		ssi_stream_adjust (data, 1);

		int64_t pos = file->tell ();

		file->setType (data.type);
		while (file->read (data.ptr, data.byte, 1 * data.dim)) {
			sample_number++;
		}		
			
		if (sample_number > 0) {			
			file->seek (pos, File::BEGIN);
			ssi_stream_adjust (data, sample_number);
			if (!file->read (data.ptr, data.byte, sample_number * data.dim)) {
				ssi_wrn ("could not read <data>");
				delete file;
				return false;
			}
		}
		
	} else {
		ssi_wrn ("repair failed: header seems to be correct");
		delete file;
		return false;
	}

	return true;
}

bool FileTools::RepairStreamFileV2 (File::TYPE type,
	const ssi_char_t *path,
	ssi_stream_t &data) {	

	// open file
	File *file = File::CreateAndOpen (type, File::READ, path);	
	
	ssi_size_t sample_number = 0;
	ssi_stream_adjust (data, 1);

	int64_t pos = file->tell ();

	file->setType (data.type);
	while (file->read (data.ptr, data.byte, 1 * data.dim)) {
		sample_number++;
	}		
			
	if (sample_number > 0) {			
		file->seek (pos, File::BEGIN);
		ssi_stream_adjust (data, sample_number);
		if (!file->read (data.ptr, data.byte, sample_number * data.dim)) {
			ssi_wrn ("could not read <data>");
			delete file;
			return false;
		}
	}
		
	return true;
}

void FileTools::ReadStreamFile (File::TYPE type,
	const ssi_char_t *path,
	ssi_stream_t &data) {

	File *file = File::CreateAndOpen (type, File::READ, path);		
	ReadStreamFile (*file, data);
	delete file;
}

bool FileTools::ReadStreamFile (const ssi_char_t *path,
	ssi_stream_t &data) {

	FilePath fp(path);

	if (ssi_strcmp(fp.getExtension(), SSI_FILE_TYPE_WAV, false))
	{
		return WavTools::ReadWavFile(path, data, true);
	}
	else
	{
		FileStreamIn file_in;
		if (!file_in.open(data, path)) {
			return false;
		}
		ssi_stream_adjust(data, file_in.getTotalSampleSize());
		ssi_size_t num = 0;
		ssi_byte_t *ptr = data.ptr;
		while (num = file_in.read(data, FileStreamIn::NEXT_CHUNK)) {
			if (num == FileStreamIn::READ_ERROR) {
				return false;
			}
			data.ptr += num * data.byte * data.dim;
		}
		data.ptr = ptr;
	}

	return true;
}

void FileTools::WriteStreamHeader (File &file,
	ssi_stream_t &data) {

	WriteStreamHeader (file, data, File::V0);
}

void FileTools::WriteStreamHeader (File &file,
	ssi_stream_t &data,
	File::VERSION  version) {

	switch (version) {

		case File::V0: {

			switch (file.getType ()) {

				case File::ASCII: {
					ssi_char_t string[1024];
					sprintf (string, "%Lf %u %u", data.sr, data.dim, data.byte);
					file.writeLine (string);
					break;
				}

				case File::BINARY: {
					file.write (&data.sr, sizeof (ssi_time_t), 1);
					file.write (&data.dim, sizeof (ssi_size_t), 1);
					file.write (&data.byte, sizeof (ssi_size_t), 1);
					break;
				}

				default:
					ssi_err ("type not supported");
			}

			break;

		}

		case File::V1: {

			switch (file.getType ()) {

				case File::ASCII: {

					ssi_char_t string[1024];
					char buf[10];
					ssi_sprint (buf, "%d", version);
					sprintf (string, "%s@%s", File::FILE_ID, buf);
					file.writeLine (string);
					ssi_sprint (buf, "%d", data.type);
					sprintf (string, "%Lf %u %u %s", data.sr, data.dim, data.byte, buf);
					file.writeLine (string);

					break;
				} 

				case File::BINARY: {

					file.write (File::FILE_ID, sizeof (ssi_char_t), 3);
					file.write (&version, sizeof (version), 1);
					file.write (&data.sr, sizeof (data.sr), 1);
					file.write (&data.dim, sizeof (data.dim), 1);
					file.write (&data.byte, sizeof (data.byte), 1);
					file.write (&data.type, sizeof (data.type), 1);

					break;
				}

				default:
					ssi_err ("type not supported");
			}

			break;
		}

	   default:
			ssi_err ("version not supported");
	}
}

void FileTools::WriteStreamData (File &file,
	ssi_time_t time,
	ssi_stream_t &data,
	bool add_header) {

	data.time = time;
	WriteStreamData (file, data, File::V0, add_header);
}


void FileTools::WriteStreamData (File &file,
	ssi_stream_t &data,
	File::VERSION  version,
	bool add_header) {

	switch (file.getType ()) {

		case File::ASCII: {

			ssi_char_t string[1024];
			if (add_header) {
				sprintf (string, "%lf %u", data.time, data.num);		
				file.writeLine (string);
			}
			file.setType (data.type);
			file.write (data.ptr, data.dim, data.num * data.dim);

			break;
		}

		case File::BINARY: {

			if (add_header) {
				file.write (&data.time, sizeof (data.time), 1);		
				file.write (&data.num, sizeof (data.num), 1);
			}

			if (data.num > 0) {
				file.write (data.ptr, sizeof (ssi_byte_t), data.num * data.dim * data.byte);
			}

			break;
		}

		default:
			ssi_err ("type not supported");
	}
}

void FileTools::WriteStreamFile (File &file,
	ssi_stream_t &data,
	File::VERSION version) {
	
	FileTools::WriteStreamHeader (file, data, version);
	FileTools::WriteStreamData (file, data, version, true);
}

bool FileTools::WriteStreamFile (File::TYPE type,
	const ssi_char_t *path,
	ssi_stream_t &data,
	const ssi_char_t *delim,
	File::VERSION version) {

	if (version <= File::V1) {

		File *file = File::CreateAndOpen (type, File::WRITE, path);		
		WriteStreamFile (*file, data, version);
		delete file;
		return true;

	} else {
		FileStreamOut out;
		out.setDelim(delim);
		bool result = true;
		result = result && out.open (data, path, type, version);
		result = result &&out.write (data, false);
		result = result &&out.close ();
		return result;
	}	
}

void FileTools::ConvertStreamV0toV1Binary (const ssi_char_t *filename,
	ssi_type_t type,
	bool backup) {
	
	ssi_char_t string[1024];
	ssi_char_t *bakname = ssi_strcat (filename, ".bak");
	
	// create backup
	string[0] = '\0';
	ssi_sprint (string, "copy /B %s %s", filename, bakname);
	if (system (string)) {
		ssi_err ("could not copy file '%s' -> '%s'", filename, bakname);
	}

	// convert
	{
		File *file_in = File::CreateAndOpen (File::BINARY, File::READ, bakname);
		File *file_out  = File::CreateAndOpen (File::BINARY, File::WRITE, filename);

		ssi_stream_t data;
		ssi_stream_init (data, 0, 0, 0, SSI_UNDEF, 0);				
		File::VERSION version = File::V1;
		FileTools::ReadStreamHeader (*file_in, data);
		data.type = type;
		FileTools::WriteStreamHeader (*file_out, data, version);
		ssi_time_t time;		
		while (file_in->ready ()) {					
			FileTools::ReadStreamData (*file_in, &time, data);
			data.time = time;			
			FileTools::WriteStreamData (*file_out, data, version);
		}
		ssi_stream_destroy (data);

		delete file_in;
		delete file_out;
	}

	// delete backup
	if (!backup) {
		string[0] = '\0';
		ssi_sprint (string, "del %s", bakname);
		if (system (string)) {
			ssi_err ("could not delete file '%s'", bakname);
		}
	}

	delete[] bakname;

}

void FileTools::ConvertStreamV1toV2Binary (const ssi_char_t *filename,
	bool backup) {
	
	ssi_char_t string[1024];
	ssi_char_t *bakname = ssi_strcat (filename, ".bak");
	
	// create backup
	string[0] = '\0';
	ssi_sprint (string, "copy /B %s %s", filename, bakname);
	if (system (string)) {
		ssi_err ("could not copy file '%s' -> '%s'", filename, bakname);
	}

	// convert
	{
		File *file_in = File::CreateAndOpen (File::BINARY, File::READ, bakname);
		FileStreamOut file_out;		

		ssi_stream_t data;
		ssi_stream_init (data, 0, 0, 0, SSI_UNDEF, 0);		
		File::VERSION version;
		FileTools::ReadStreamHeader (*file_in, data, version);		
		file_out.open (data, filename, File::BINARY, File::V2);		
		while (file_in->ready ()) {					
			FileTools::ReadStreamData (*file_in, data, version);
			file_out.write (data, false);			
		}
		ssi_stream_destroy (data);

		delete file_in;		
	}

	// delete backup
	if (!backup) {
		string[0] = '\0';
		ssi_sprint (string, "del %s", bakname);
		if (system (string)) {
			ssi_err ("could not delete file '%s'", bakname);
		}
	}

	delete[] bakname;

}

bool FileTools::ReadSampleHeader (File &file,
	ssi_sample_t &data) {

	if (!file.ready ()) {
		return false;
	}

	switch (file.getType ()) {

		case File::ASCII: {

			ssi_char_t string[1024];
			if (!file.readLine (1024, string)) {
				ssi_err ("could not read <user_id> <class_id> <time> <score> <num>");
			}
			sscanf (string, "%u %u %lf %f %u", &data.user_id, &data.class_id, &data.time, &data.score, &data.num);

			break;
		} 

		case File::BINARY: {
	
			if (!file.read (&data.user_id, sizeof (data.user_id), 1)) {
				ssi_err ("could not read <user_id>");
			}

			if (!file.read (&data.class_id, sizeof (data.class_id), 1)) {
				ssi_err ("could not read <class_id>");
			}

			if (!file.read (&data.time, sizeof (data.time), 1)) {
				ssi_err ("could not read <time>");
			}

			if (!file.read(&data.score, sizeof(data.score), 1)) {
				ssi_err ("could not read <score>");
			}

			if (!file.read (&data.num, sizeof (data.num), 1)) {
				ssi_err ("could not read <num>");
			}

			break;
		}

	   default:
			ssi_err ("type not supported");
	}

	data.streams = new ssi_stream_t *[data.num];
	for (ssi_size_t i = 0; i < data.num; i++) {
		data.streams[i] = 0;
	}

	return true;
}

bool FileTools::ReadSampleData (File &file,
	ssi_sample_t &data) {

	if (!file.ready ()) {
		return false;
	}

	for (ssi_size_t i = 0; i < data.num; i++) {
		data.streams[i] = new ssi_stream_t;
		File::VERSION version;
		FileTools::ReadStreamHeader (file, *data.streams[i], version);
		FileTools::ReadStreamData (file, *data.streams[i], version);
	}

	return true;
}

void FileTools::WriteSampleHeader (File &file,
	ssi_sample_t &data,
	File::VERSION version) {

	switch (file.getType ()) {

		case File::ASCII: {

			ssi_char_t string[1024];
			sprintf (string, "%u %u %lf %.2f %u", data.user_id, data.class_id, data.time, data.score, data.num);
			file.writeLine (string);

			break;

		}
		
		case File::BINARY: {

			file.write (&data.user_id, sizeof (data.user_id), 1);
			file.write (&data.class_id, sizeof (data.class_id), 1);
			file.write (&data.time, sizeof (data.time), 1);
			file.write (&data.score, sizeof (data.score), 1);
			file.write (&data.num, sizeof (data.num), 1);

			break;
		}

	   default:
			ssi_err ("type not supported");
	}
}

void FileTools::WriteSampleData (File &file,
	ssi_sample_t &data,
	File::VERSION  version) {

	for (ssi_size_t i = 0; i < data.num; i++) {
		FileTools::WriteStreamHeader (file, *data.streams[i], version);
		FileTools::WriteStreamData (file, *data.streams[i], version);
	}
}
#if __gnu_linux__
uint8_t hasExt(const char* ext, const char* filename)
{
    int a=((int)ext[0]);
    int b= ((int)'*');
    if(a==b)return 1;


	int i=0;
	uint8_t ret=1;
	int ext_len=strlen(ext);
	int filename_len=strlen(filename);
	ret=ret&&(filename[filename_len-ext_len-1]=='.');
	for(i=0; i< ext_len; i++)
		ret=ret&&(ext[i]==filename[filename_len-ext_len+i]);
	
	return ret;
}
#endif

void FileTools::ReadFilesFromDir (StringList &files, 
	const ssi_char_t *dir,
	const ssi_char_t *filter) {
		
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "read filenames '%s' [filter='%s']", dir, filter);
#if _WIN32|_WIN64
	WIN32_FIND_DATA FindFileData;
	ssi_char_t *dirspec = new ssi_char_t[strlen (dir) + 1 + strlen (filter) + 1];
	ssi_sprint (dirspec, "%s\\%s", dir, filter);

	HANDLE  hFind = FindFirstFile (dirspec, &FindFileData);

	ssi_char_t filename[SSI_MAX_CHAR];
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				ssi_sprint (filename, "%s\\%s", dir, FindFileData.cFileName);
				files.add (filename);
				SSI_DBG (SSI_LOG_LEVEL_DEBUG, "  %s", filename);
			}
		} while (FindNextFile (hFind, &FindFileData));

		FindClose(hFind);

	} else {
		ssi_wrn ("no files found in '%s' that match '%s'", dir, filter);
	}

	delete[] dirspec;
#else
	ssi_char_t *dirspec = new ssi_char_t[strlen (dir) + 1 + strlen (filter) + 1];
	ssi_sprint (dirspec, "%s/%s", dir, filter);

	DIR *Dir;
  struct dirent *dent;
  struct stat stbuf;
  ssi_char_t filename[SSI_MAX_CHAR];

  if(!(Dir = opendir(dir)))
  {
    perror("opendir()");
    //exit(EXIT_FAILURE);
    return;
  }

  while((dent = readdir(Dir)))
  {
 

    snprintf(filename, PATH_MAX, "%s/%s", dir,dent->d_name);
    if((stat(filename, &stbuf)) == -1)
    {
      perror("stat()");
      continue;
    }

    if(!S_ISDIR(stbuf.st_mode)&&hasExt(filter, filename))
    {
      puts(dent->d_name);
      files.add (filename);
      }
  }

  closedir(Dir);
#endif
}

void FileTools::ReadDirsFromDir (StringList &files, 
	const ssi_char_t *dir,
	const ssi_char_t *filter,
	bool ignore_hidden) {
		
	ssi_msg (SSI_LOG_LEVEL_DETAIL, "read directories from directory %s", dir);
#if _WIN32|_WIN64
	WIN32_FIND_DATA FindFileData;
	ssi_char_t *dirspec = new ssi_char_t[strlen (dir) + 1 + strlen (filter) + 1];
	ssi_sprint (dirspec, "%s\\%s", dir, filter);

	HANDLE  hFind = FindFirstFile (dirspec, &FindFileData);

	ssi_char_t filename[SSI_MAX_CHAR];
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!(ignore_hidden && FindFileData.cFileName[0] == '.')) {
					ssi_sprint (filename, "%s\\%s", dir, FindFileData.cFileName);
					files.add (filename);
					SSI_DBG (SSI_LOG_LEVEL_DEBUG, "  %s", filename);
				}
			}
		} while (FindNextFile (hFind, &FindFileData));

		FindClose(hFind);

	} else {
		ssi_wrn ("no files found in '%s' that match '%s'", dir, filter);
	}

	delete[] dirspec;
#else
	
	ssi_char_t *dirspec = new ssi_char_t[strlen (dir) + 1 + strlen (filter) + 1];
	ssi_sprint (dirspec, "%s/%s", dir, filter);

	DIR *Dir;
  struct dirent *dent;
  struct stat stbuf;
  ssi_char_t filename[SSI_MAX_CHAR];

  if(!(Dir = opendir(dir)))
  {
    perror("opendir()");
    exit(EXIT_FAILURE);
  }

  while((dent = readdir(Dir)))
  {
 

    snprintf(filename, SSI_MAX_CHAR, "%s/%s", dir,dent->d_name);
    if((stat(filename, &stbuf)) == -1)
    {
      perror("stat()");
      continue;
    }

    if(S_ISDIR(stbuf.st_mode))
    {
      puts(dent->d_name);
      files.add (filename);
    }
  }

  closedir(Dir);
#endif
}

void FileTools::ReadLinesFromFile(StringList &files,
	const ssi_char_t *path) {

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "read filenames from file '%s'", path);

	File *file = File::CreateAndOpen(File::ASCII, File::READ, path);

	ssi_char_t line[SSI_MAX_CHAR];
	while (file->ready())
	{
		file->readLine(SSI_MAX_CHAR, line);
		ssi_strtrim(line);
		if (line[0] != '\0')
		{			
			files.add(line);
		}
	}

	delete file;
}

void FileTools::ReadFilesFromFile (StringList &files, 
	const ssi_char_t *path) {
		
	ReadLinesFromFile(files, path);
}

ssi_byte_t *FileTools::ReadBytesFromFile(ssi_size_t &n_bytes,
	const ssi_char_t *path) {

	ssi_byte_t *bytes = 0;
	n_bytes = 0;

	FILE *fp = fopen(path, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		n_bytes = ssi_cast(ssi_size_t, ftell(fp));
		fseek(fp, 0, SEEK_SET);
		bytes = new ssi_byte_t[n_bytes];
		fread(bytes, n_bytes, 1, fp);
		fclose(fp);
	}

	return bytes;
}

#if _WIN32|| _WIN64
bool FileTools::SelectFile (char *file) {

	OPENFILENAME OpenFileName;
	char szFile[MAX_PATH];
	char CurrentDir[MAX_PATH];

	szFile[0] = 0;
	ssi_getcwd( MAX_PATH, CurrentDir );

	OpenFileName.lStructSize = sizeof (OPENFILENAME);
	OpenFileName.hwndOwner = NULL;
	OpenFileName.lpstrFilter = "All Files\0*.*\0\0";
	OpenFileName.lpstrCustomFilter = NULL;
	OpenFileName.nMaxCustFilter = 0;
	OpenFileName.nFilterIndex = 0;
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = sizeof( szFile );
	OpenFileName.lpstrFileTitle = NULL;
	OpenFileName.nMaxFileTitle = 0;
	OpenFileName.lpstrInitialDir = CurrentDir;
	OpenFileName.lpstrTitle = "Select file";
	OpenFileName.nFileOffset = 0;
	OpenFileName.nFileExtension = 0;
	OpenFileName.lpstrDefExt = NULL;
	OpenFileName.lCustData = 0;
	OpenFileName.lpfnHook = NULL;
	OpenFileName.lpTemplateName = NULL;
	OpenFileName.Flags = OFN_EXPLORER;

	if (::GetOpenFileName( &OpenFileName )) {
		ssi_strcpy (file, szFile );
		return true;
	} else {
		return false;
	}
}

bool FileTools::SelectSaveFile (char *file) {

	OPENFILENAME OpenFileName;
	char szFile[MAX_PATH];
	char CurrentDir[MAX_PATH];

	szFile[0] = 0;
	ssi_getcwd(MAX_PATH, CurrentDir);

	OpenFileName.lStructSize = sizeof (OPENFILENAME);
	OpenFileName.hwndOwner = NULL;
	OpenFileName.lpstrFilter = "All Files\0*.*\0\0";
	OpenFileName.lpstrCustomFilter = NULL;
	OpenFileName.nMaxCustFilter = 0;
	OpenFileName.nFilterIndex = 0;
	OpenFileName.lpstrFile = szFile;
	OpenFileName.nMaxFile = sizeof( szFile );
	OpenFileName.lpstrFileTitle = NULL;
	OpenFileName.nMaxFileTitle = 0;
	OpenFileName.lpstrInitialDir = CurrentDir;
	OpenFileName.lpstrTitle = "Select file";
	OpenFileName.nFileOffset = 0;
	OpenFileName.nFileExtension = 0;
	OpenFileName.lpstrDefExt = NULL;
	OpenFileName.lCustData = 0;
	OpenFileName.lpfnHook = NULL;
	OpenFileName.lpTemplateName = NULL;
	OpenFileName.Flags = OFN_EXPLORER;

	if (::GetSaveFileName( &OpenFileName )) {
		ssi_strcpy (file, szFile );
		return true;
	} else {
		return false;
	}
}

bool FileTools::SelectDirectory (char *file) {

	BROWSEINFO browseInfo;
	::ZeroMemory (&browseInfo, sizeof (browseInfo));

	char CurrentDir[MAX_PATH];

	ssi_getcwd(MAX_PATH, CurrentDir);
	
    browseInfo.hwndOwner = NULL ;
    browseInfo.pidlRoot = NULL; 
    browseInfo.lpszTitle = "Select directory";   
    browseInfo.ulFlags = 0; 
    browseInfo.lpfn = BrowseForFolderCallback; 
    browseInfo.lParam = reinterpret_cast<LPARAM>(static_cast<LPCTSTR>(CurrentDir));  

    LPITEMIDLIST lpItemIDList = ::SHBrowseForFolder (&browseInfo);
    if (!lpItemIDList)
		return false;

	if (!::SHGetPathFromIDList (lpItemIDList, file))
		return false;
	
	ssi_setcwd (file);

	return true;
}
#endif

}

