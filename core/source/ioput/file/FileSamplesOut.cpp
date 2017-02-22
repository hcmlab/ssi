// FileSamplesOut.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/27
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

#include "ioput/file/FileSamplesOut.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"

namespace ssi {

int FileSamplesOut::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileSamplesOut::ssi_log_name[] = "fsampout__";
File::VERSION FileSamplesOut::DEFAULT_VERSION = File::V3;

FileSamplesOut::FileSamplesOut ()
	: _file_data (0),
	_file_info (0),
	_file_streams (0),
	_version (DEFAULT_VERSION),
	_n_samples (0),
	_n_classes (0),
	_classes (0),
	_n_per_class (0),
	_n_garbage_class (0),
	_n_users (0),
	_n_per_user (0),	
	_n_streams (0),
	_streams (0),
	_console (false),
	_has_missing_data (false),
	_path (0) {				
}

FileSamplesOut::~FileSamplesOut () {

	if (_file_data) {
		close ();
	}
}

bool FileSamplesOut::open (ISamples &data,
	const ssi_char_t *path,
	File::TYPE type, 
	File::VERSION version) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "open files '%s'", path);	

	_version = version;

	if (_version < File::V2) {
		ssi_wrn ("version < V2 not supported");
		return false;
	}

	if (_file_info || _file_data) {
		ssi_wrn ("samples already open");
		return false;
	}

	_n_users = data.getUserSize ();
	_users = new ssi_char_t *[_n_users];
	_n_per_user = new ssi_size_t[_n_users];
	for (ssi_size_t i = 0; i < _n_users; i++) {
		_users[i] = ssi_strcpy (data.getUserName (i));
		_n_per_user[i] = 0;
	}
	_n_classes = data.getClassSize ();
	_classes = new ssi_char_t *[_n_classes];
	_n_per_class = new ssi_size_t[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_classes[i] = ssi_strcpy (data.getClassName (i));
		_n_per_class[i] = 0;
	}
	
	_n_streams = data.getStreamSize ();
	_streams = new ssi_stream_t[_n_streams];
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		ssi_stream_t s = data.getStream (i);
		ssi_stream_init (_streams[i], 0, s.dim, s.byte, s.type, s.sr, 0);
	}

	_has_missing_data = false;

	if (path == 0 || path[0] == '\0') {
		_console = true;
	}

	if (_console) {
		
		_file_data = File::CreateAndOpen (type, File::WRITE, "");
		if (!_file_data) {
			ssi_wrn ("could not open console");
			return false;
		}

	} else {

		FilePath fp (path);
		ssi_char_t *path_info = 0;
		if (strcmp (fp.getExtension (), SSI_FILE_TYPE_SAMPLES) != 0) {
			path_info = ssi_strcat (path, SSI_FILE_TYPE_SAMPLES);
		} else {
			path_info = ssi_strcpy (path);
		}	
		_path = ssi_strcpy (path_info);

		_file_info = File::CreateAndOpen (File::ASCII, File::WRITE, path_info);
		if (!_file_info) {
			ssi_wrn ("could not open info file '%s'", path_info);
			return false;
		}

		ssi_sprint (_string, "<?xml version=\"1.0\" ?>\n<samples ssi-v=\"%d\">", version);
		_file_info->writeLine (_string);
	
		ssi_char_t *path_data = ssi_strcat (path_info, "~");			
		_file_data = File::CreateAndOpen (type, File::WRITE, path_data);
		if (!_file_data) {
			ssi_wrn ("could not open data file '%s'", path_data);
			return false;
		}

		if (_version == File::V3) {

			_file_streams = new FileStreamOut[_n_streams];
			ssi_char_t string[SSI_MAX_CHAR];
			for (ssi_size_t i = 0; i < _n_streams; i++) {
				ssi_sprint (string, "%s.#%u", path_info, i);
				_file_streams[i].open (_streams[i], string, type);				
			}

		}

		delete[] path_info;
		delete[] path_data;

	}

	return true;
};

bool FileSamplesOut::write (ISamples &data) {

	data.reset ();
	ssi_sample_t *sample = 0;
	while (sample = data.next ()) {
		write (*sample);
	}

	return true;
}

bool FileSamplesOut::write (ssi_sample_t &data) {

	if (data.num != _n_streams) {
		ssi_wrn ("#streams differs ('%u' != '%u')", data.num, _n_streams);
		return false;
	}
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		if (data.streams[i]->byte != _streams[i].byte || data.streams[i]->dim != _streams[i].dim || data.streams[i]->type != _streams[i].type) {
			ssi_wrn ("stream is not compatible");
			return false;
		}
	}
	if (data.class_id >= _n_classes && data.class_id != SSI_ISAMPLES_GARBAGE_CLASS_ID) {
		ssi_wrn ("class id '%u' exceeds #classes '%u')", data.class_id, _n_classes);
		return false;
	}
	if (data.user_id >= _n_users) {
		ssi_wrn ("user id '%u' exceeds #users '%u')", data.user_id, _n_users);
		return false;
	}

	if (_console) {

		ssi_print ("class=%u user=%u time=%.2lf score=%.2f\n", data.class_id, data.user_id, data.time, data.score);

	} else {

		_n_samples++;
		if (data.class_id == SSI_ISAMPLES_GARBAGE_CLASS_ID) {
			_n_garbage_class++;
		} else {
			_n_per_class[data.class_id]++;
		}
		_n_per_user[data.user_id]++;

		switch (_version) {
			
			case File::V2: {

				if (_file_data->getType () == File::ASCII) {
					sprintf (_string, "%u %u %lf %.2f %u", data.user_id, data.class_id, data.time, data.score, data.num);
					_file_data->writeLine (_string);				
				} else {
					_file_data->write (&data.user_id, sizeof (data.user_id), 1);
					_file_data->write (&data.class_id, sizeof (data.class_id), 1);
					_file_data->write (&data.time, sizeof (data.time), 1);
					_file_data->write (&data.score, sizeof (data.score), 1);
					_file_data->write (&data.num, sizeof (data.num), 1);				
				}

				for (ssi_size_t i = 0; i < _n_streams; i++) {

					ssi_stream_t &stream = *data.streams[i];			
			
					if (_file_data->getType () == File::ASCII) {
						sprintf (_string, "%u", stream.num);
						_file_data->writeLine (_string);
						if (stream.num > 0) {
							_file_data->setType (stream.type);
							_file_data->write (stream.ptr, stream.dim, stream.num * stream.dim);
						} else {
							_has_missing_data = true;
						}
					} else {
						_file_data->write (&stream.num, sizeof (stream.num), 1);
						if (stream.num > 0) {
							_file_data->write (stream.ptr, sizeof (ssi_byte_t), stream.tot);
						} else {
							_has_missing_data = true;
						}
					}
				}
			}
			break;

			case File::V3: {

				if (_file_data->getType () == File::ASCII) {
					sprintf (_string, "%u %d %.2f %lf", data.user_id, data.class_id, data.score, data.time);
					_file_data->writeLine (_string);				
				} else {
					_file_data->write (&data.user_id, sizeof (data.user_id), 1);
					_file_data->write (&data.class_id, sizeof (data.class_id), 1);
					_file_data->write (&data.score, sizeof (data.score), 1);					
					_file_data->write (&data.time, sizeof (data.time), 1);					
				}

				for (ssi_size_t i = 0; i < _n_streams; i++) {

					ssi_stream_t &stream = *data.streams[i];
					stream.time = data.time;
					_file_streams[i].write (stream, false);
					if (stream.num == 0) {
						_has_missing_data = true;
					}
				}
			}
			break;
		}

	}

	return true;
}

bool FileSamplesOut::close () { 

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "close files '%s'", _path);

	if (!_console) {

		if (!_file_data->close ()) {
			ssi_wrn ("could not close data file '%s'", _path);
			return false;
		}

		switch (_version) {
			
			case File::V2: {

				TiXmlElement info ("info" );	
				info.SetAttribute ("ftype", File::TYPE_NAMES[_file_data->getType ()]);
				info.SetAttribute ("size", _n_samples);
				info.SetAttribute ("missing", _has_missing_data ? 1 : 0);
				info.SetAttribute ("garbage", _n_garbage_class);

				TiXmlElement streams ("streams");
				for (ssi_size_t i = 0; i < _n_streams; i++) {
					TiXmlElement item ("item");
					item.SetDoubleAttribute ("sr", _streams[i].sr);
					item.SetAttribute ("dim", _streams[i].dim);
					item.SetAttribute ("byte", _streams[i].byte);
					item.SetAttribute ("type", SSI_TYPE_NAMES[_streams[i].type]);
					streams.InsertEndChild (item);
				}
				info.InsertEndChild (streams);

				TiXmlElement classes ("classes");
				for (ssi_size_t i = 0; i < _n_classes; i++) {
					TiXmlElement item ("item");			
					item.SetAttribute ("name", _classes[i]);			
					item.SetAttribute ("size", _n_per_class[i]);
					classes.InsertEndChild (item);
				}
				info.InsertEndChild (classes);

				TiXmlElement users ("users");
				for (ssi_size_t i = 0; i < _n_users; i++) {
					TiXmlElement item ("item");			
					item.SetAttribute ("name", _users[i]);			
					item.SetAttribute ("size", _n_per_user[i]);
					users.InsertEndChild (item);
				}
				info.InsertEndChild (users);

				info.Print (_file_info->getFile (), 1);
				_file_info->writeLine ("\n</samples>");

				if (!_file_info->close ()) {
					ssi_wrn ("could not close info file '%s'", _file_info->getPath ());
					return false;
				}

			}
			break;

			case File::V3: {

				TiXmlElement info ("info" );	
				info.SetAttribute ("ftype", File::TYPE_NAMES[_file_data->getType ()]);
				info.SetAttribute ("size", _n_samples);
				info.SetAttribute ("missing", _has_missing_data ? "true" : "false");
				info.SetAttribute ("garbage", _n_garbage_class);

				TiXmlElement streams ("streams");
				for (ssi_size_t i = 0; i < _n_streams; i++) {
					TiXmlElement item ("item");
					FilePath fp (_file_streams[i].getInfoFile ()->getPath ());
					item.SetAttribute ("path", fp.getName ());
					streams.InsertEndChild (item);
				}

				TiXmlElement classes ("classes");
				for (ssi_size_t i = 0; i < _n_classes; i++) {
					TiXmlElement item ("item");			
					item.SetAttribute ("name", _classes[i]);			
					item.SetAttribute ("size", _n_per_class[i]);
					classes.InsertEndChild (item);
				}

				TiXmlElement users ("users");
				for (ssi_size_t i = 0; i < _n_users; i++) {
					TiXmlElement item ("item");			
					item.SetAttribute ("name", _users[i]);			
					item.SetAttribute ("size", _n_per_user[i]);
					users.InsertEndChild (item);
				}

				info.Print (_file_info->getFile (), 1);
				_file_info->writeLine ("");
				streams.Print (_file_info->getFile (), 1);
				_file_info->writeLine ("");
				classes.Print (_file_info->getFile (), 1);
				_file_info->writeLine ("");
				users.Print (_file_info->getFile (), 1);
				_file_info->writeLine ("\n</samples>");

				if (!_file_info->close ()) {
					ssi_wrn ("could not close info file '%s'", _file_info->getPath ());
					return false;
				}
			}
		}
	}

	if (_file_streams) {
		for (ssi_size_t i = 0; i < _n_streams; i++) {			
			_file_streams[i].close ();
		}
	}

	delete _file_data; _file_data = 0;
	delete _file_info; _file_info = 0;
	delete[] _file_streams; _file_streams = 0;
	delete _path; _path = 0;

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		delete[] _classes[i];
	}
	delete[] _classes; _classes = 0;
	delete[] _n_per_class; _n_per_class = 0;
	_n_garbage_class = 0;
	for (ssi_size_t i = 0; i < _n_users; i++) {
		delete[] _users[i];
	}
	delete[] _users; _users = 0;
	delete[] _n_per_user; _n_per_user = 0;	
	delete[] _streams; _streams = 0;

	_n_samples = 0;
	_n_classes = 0;
	_n_users = 0;	
	_n_streams = 0;

	return true;
};

ssi_size_t FileSamplesOut::getClassId(const ssi_char_t *name) {

	ssi_size_t result = SSI_ISAMPLES_GARBAGE_CLASS_ID;
	
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		if (ssi_strcmp(name, _classes[i])) {
			return i;
		}
	}

	return result;
}

ssi_size_t FileSamplesOut::getUserId(const ssi_char_t *name) {

	ssi_size_t result = SSI_ISAMPLES_GARBAGE_USER_ID;

	for (ssi_size_t i = 0; i < _n_users; i++) {
		if (ssi_strcmp(name, _users[i])) {
			return i;
		}
	}

	return result;
}

}
 
