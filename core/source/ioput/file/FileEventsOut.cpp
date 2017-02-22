// FileEventsOut.cpp
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

#include "ioput/file/FileEventsOut.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"
#include "base/Factory.h"

namespace ssi {

int FileEventsOut::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileEventsOut::ssi_log_name[] = "fevntout__";
File::VERSION FileEventsOut::DEFAULT_VERSION = File::V2;

FileEventsOut::FileEventsOut ()
	: //_file_data (0),
	_file (0),
	_n_events (0),
	_console (false),
	_path (0) {				

	_board = Factory::GetEventBoard ();
}

FileEventsOut::~FileEventsOut () {

	/*if (_file_data) {
		close ();
	}*/
	if (_file) {
		close ();
	}
}

bool FileEventsOut::open (const ssi_char_t *path,
	File::TYPE type, 
	File::VERSION version) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "open files '%s'", path);	

	if (version < File::V2) {
		ssi_wrn ("version < V2 not supported");
		return false;
	}

	if (_file) {
		ssi_wrn ("events already open");
		return false;
	}

	_n_events = 0;

	if (path == 0 || path[0] == '\0') {
		_console = true;
	}

	if (_console) {
		
		/*_file_data = File::CreateAndOpen (type, File::WRITE, "");
		if (!_file_data) {
			ssi_wrn ("could not open console");
			return false;
		}*/
		_file = File::CreateAndOpen (type, File::WRITE, "");
		if (!_file) {
			ssi_wrn ("could not open console");
			return false;
		}

	} else {

		FilePath fp (path);
		ssi_char_t *path_info = 0;
		if (strcmp (fp.getExtension (), SSI_FILE_TYPE_EVENTS) != 0) {
			path_info = ssi_strcat (path, SSI_FILE_TYPE_EVENTS);
		} else {
			path_info = ssi_strcpy (path);
		}	
		_path = ssi_strcpy (path_info);

		_file = File::CreateAndOpen (File::ASCII, File::WRITE, path_info);
		if (!_file) {
			ssi_wrn ("could not open info file '%s'", path_info);
			return false;
		}

		ssi_sprint (_string, "<?xml version=\"1.0\" ?>\n<events ssi-v=\"%d\">", version);
		_file->writeLine (_string);
	
		/*ssi_char_t *path_data = ssi_strcat (path_info, "~");			
		_file_data = File::CreateAndOpen (type, File::WRITE, path_data);
		if (!_file_data) {
			ssi_wrn ("could not open data file '%s'", path_data);
			return false;
		}*/

		delete[] path_info;
		//delete[] path_data;

	}

	return true;
};

bool FileEventsOut::write (IEvents &data) {

	data.reset ();
	ssi_event_t *e = 0;
	while (e = data.next ()) {
		write (*e);
	}

	return true;
}

bool FileEventsOut::write (ssi_event_t &data) {

	if (_console) {

		ssi_print ("sender=%s event=%s from=%.2lf dur=%.2f\n", Factory::GetString (data.sender_id), Factory::GetString (data.event_id), data.time/1000.0, data.dur/1000.0);

	} else {

		ssi_event_t *e = &data;

		TiXmlElement sample ("event"); 
		sample.SetAttribute ("sender", Factory::GetString (e->sender_id));
		sample.SetAttribute ("event", Factory::GetString (e->event_id));
		sample.SetAttribute ("from", e->time);
		sample.SetAttribute ("dur", e->dur);
		sample.SetDoubleAttribute ("prob", e->prob);
		sample.SetAttribute ("type", SSI_ETYPE_NAMES[e->type]);
		sample.SetAttribute ("state", SSI_ESTATE_NAMES[e->state]);
		sample.SetAttribute ("glue", e->glue_id);		

		switch (e->type) {
			case SSI_ETYPE_UNDEF:
			case SSI_ETYPE_EMPTY:
				break;
			case SSI_ETYPE_TUPLE: {
				ssi_size_t num = e->tot / sizeof (ssi_event_tuple_t);
				ssi_char_t string[SSI_MAX_CHAR];
				if (!ssi_array2string (num, ssi_pcast (ssi_event_tuple_t, e->ptr), SSI_MAX_CHAR, string)) {
					return false;
				}
				TiXmlText text (string);
				sample.InsertEndChild (text);
				break;
			}
			case SSI_ETYPE_STRING: {	
				TiXmlText text (ssi_pcast (ssi_char_t, e->ptr));
				sample.InsertEndChild (text);				
				break;
			}
			case SSI_ETYPE_MAP: {
				ssi_size_t num = e->tot / sizeof (ssi_event_map_t);
				ssi_event_map_t *ts = ssi_pcast (ssi_event_map_t, e->ptr);
				for (ssi_size_t n = 0; n < num; n++) {
					TiXmlElement tuple ("tuple");
					tuple.SetAttribute ("string", Factory::GetString (ts[n].id));
					tuple.SetDoubleAttribute ("value", ts[n].value);
					sample.InsertEndChild (tuple);
				}
				break;
			}
		}

		sample.Print (_file->getFile (), 1);
		ssi_fprint (_file->getFile (), "\n");

	}

	return true;
}

bool FileEventsOut::close () { 

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "close files '%s'", _path);

	if (!_console) {

		_file->writeLine ("</events>");
		if (!_file->close ()) {
			ssi_wrn ("could not close info file '%s'", _file->getPath ());
			return false;
		}
	}
	
	delete _file; _file = 0;
	delete _path; _path = 0;

	_n_events = 0;
	
	return true;
};

}
 
