// FileSamplesIn.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/10/21
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

#ifndef SSI_IOPUT_FILEEVENTSIN_H
#define SSI_IOPUT_FILEEVENTSIN_H

#include "base/IEvents.h"
#include "ioput/file/File.h"
#include "event/EventAddress.h"

namespace ssi {

class TiXmlElement;
class TiXmlDocument;

class FileEventsIn : public IEvents {

public:

	static File::VERSION DEFAULT_VERSION;

public:

	FileEventsIn();
	virtual ~FileEventsIn();

	bool open (const ssi_char_t *path);	
	bool close ();

	void reset () {
		if (_file) {
			_current = _first;
			_event_count = 0;
		}
	}
	ssi_event_t *get(ssi_size_t index);
	ssi_event_t *next();
	ssi_size_t getSize () {
		return _n_events;
	}

	File::VERSION getVersion () {
		return _version;
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	static int ssi_log_level;
	static ssi_char_t ssi_log_name[];

	File *_file;
	File::VERSION _version;
		
	TiXmlDocument *_doc;
	TiXmlElement *_first;
	TiXmlElement *_current;	
	ssi_event_t _event;
	ssi_size_t _n_events;
	ssi_size_t _event_count;
	
	ssi_char_t _string[1024];
	ssi_char_t *_path;	
};

}

#endif
