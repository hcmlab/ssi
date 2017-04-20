// FileEventWriter.h
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

#pragma once

#ifndef SSI_FILE_FILEEVENTWRITER_H
#define SSI_FILE_FILEEVENTWRITER_H

#include "ioput/option/OptionList.h"
#include "base/IObject.h"
#include "ioput/file/FileEventsOut.h"

namespace ssi {

class FileEventWriter : public IObject {

public:

	class Options : public OptionList {

	public:

		Options () {
		
			setPath ("events");
			addOption ("path", &path, SSI_MAX_CHAR, SSI_CHAR, "path of output file");
		};

		void setPath (const ssi_char_t *path) {
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}

		ssi_char_t path[SSI_MAX_CHAR];
		bool storeSystemTime;
	};

public: 	

	static const ssi_char_t *GetCreateName () { return "FileEventWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new FileEventWriter (file); };
	~FileEventWriter ();

	FileEventWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Stores events to a file on disk."; };

	void listen_enter ();
	bool update (ssi_event_t &e);
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	FileEventWriter (const ssi_char_t *file = 0);
	FileEventWriter::Options _options;
	ssi_char_t *_file;

	FileEventsOut _eout;
	ssi_char_t _string[SSI_MAX_CHAR];

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
};

}

#endif
