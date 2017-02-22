// FileEventsOut.h
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

#ifndef SSI_IOPUT_FILEEVENTSOUT_H
#define SSI_IOPUT_FILEEVENTSOUT_H

#include "ioput/file/File.h"
#include "base/IEvents.h"
#include "base/ITheEventBoard.h"

namespace ssi {

class FileEventsOut {

public:

	static File::VERSION DEFAULT_VERSION;

public:

	FileEventsOut ();
	virtual ~FileEventsOut ();

	bool open (const ssi_char_t *path,
		File::TYPE type, 
		File::VERSION version = DEFAULT_VERSION);
	bool close ();
	bool write (IEvents &data);
	bool write (ssi_event_t &data);
	
	//File *getDataFile() {
	//	return _file_data;
	//}
	File *getFile() {
		return _file;
	}

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	//File *_file_data;
	File *_file;

	ssi_size_t _n_events;
	
	ssi_char_t _string[1024];
	bool _console;
	ssi_char_t *_path;

	ITheEventBoard *_board;
};

}

#endif
