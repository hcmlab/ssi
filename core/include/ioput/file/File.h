// File.h
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

#ifndef SSI_IOPUT_FILE_H
#define SSI_IOPUT_FILE_H

#include "SSI_Cons.h"

namespace ssi {

#define SSI_FILE_DEFAULT_DELIM " "
#define SSI_FILE_DEFAULT_FLAGS ""

class File {

public:

	enum MODE : unsigned char  {
		READ = 0,  // "r"
		WRITE,     // "w"
		APPEND,    // "a"
		READPLUS,  // "r+"
		WRITEPLUS, // "w+"
		APPENDPLUS // "a+"
	};
	static ssi_char_t *MODE_NAMES[6];

	enum TYPE : unsigned char  {
		BINARY = 0,
		ASCII,
		BIN_LZ4
	};
	static ssi_char_t *TYPE_NAMES[3];

	enum ORIGIN : unsigned char  {
		BEGIN = 0,  // SEEK_SET
		CURRENT,	// SEEK_CUR
		END			// SEEK_END
	};

	enum VERSION : unsigned char {
		V0 = 0,		// original format
		V1 = 1,		// id + version + type
		V2 = 2,		// xml
		V3 = 3      // xml ex
	};
	static ssi_char_t *VERSION_NAMES[4];
	static File::VERSION  DEFAULT_VERSION;
	static ssi_char_t FILE_ID[];

public:

	static File *Create (TYPE type,
		MODE mode,
		const char *path,
		FILE *file = 0);
	static File *CreateAndOpen (TYPE type,
		MODE mode,
		const char *path); // note: if path is empty stdout is used
	virtual ~File ();

	virtual bool open ();
	virtual bool close ();

	// read/write operations should return the number of read/written bytes 
	virtual ssi_size_t read (void *ptr, ssi_size_t size, ssi_size_t count) = 0;
	virtual ssi_size_t write (const void *ptr, ssi_size_t size, ssi_size_t count) = 0;
	virtual ssi_size_t readLine (ssi_size_t num, ssi_char_t *string) = 0;
	virtual ssi_size_t writeLine (const ssi_char_t *string) = 0;

	virtual void setFormat (const char *delim, const char *flags) {};
	virtual void setType (ssi_type_t type) {};

	bool seek (int32_t offset, File::ORIGIN origin = File::BEGIN);
	ssi_size_t tell ();
	bool ready ();
	bool flush ();

	bool isOpen () { return _is_open; };
	File::TYPE getType () { return _type; };
	File::MODE getMode () { return _mode; };
	const char *getPath () { return _path; };
	FILE *getFile () { return _file; };

	static void SetLogLevel (int level) { ssi_log_level = level; };

protected:

	File ();
	void init ();

	static int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	File::TYPE _type;
	File::MODE _mode;
	bool _read_mode, _write_mode;
	ssi_char_t *_path;
	FILE *_file;
	ssi_handle_t _winfile;
	bool _close_file;
	bool _is_open;
};

}

#endif
