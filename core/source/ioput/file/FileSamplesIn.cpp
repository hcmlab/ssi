// FileSamplesIn.cpp
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

#include "ioput/file/FileSamplesIn.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"

namespace ssi {

File::VERSION FileSamplesIn::DEFAULT_VERSION = File::V3;
int FileSamplesIn::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileSamplesIn::ssi_log_name[] = "fsampin___";

FileSamplesIn::FileSamplesIn ()
	: _file_data (0),
	_file_info (0),
	_file_streams (0),
	_n_samples (0),
	_sample_count (0),
	_n_streams (0),
	_streams (0),
	_n_classes (0),
	_classes (0),
	_n_users (0),
	_users (0),
	_has_missing_data (false),
	_path (0),	
	_version (File::V2) {			

	ssi_sample_create (_sample, 0, 0, 0, 0, 0);
}

FileSamplesIn::~FileSamplesIn () {

	if (_file_data) {
		close ();
	}
}

bool FileSamplesIn::open (const ssi_char_t *path) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "open samples file '%s'", path);	

	if (_file_info || _file_data) {
		ssi_wrn ("samples file already open");
		return 0;
	}

	if (path == 0 || path[0] == '\0') {
		ssi_wrn ("'%s' is not a valid path", path);
		return 0;
	}

	FilePath fp (path);
	ssi_char_t *path_info = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_SAMPLES) != 0) {
		path_info = ssi_strcat (path, SSI_FILE_TYPE_SAMPLES);
	} else {
		path_info = ssi_strcpy (path);
	}	
	_path = ssi_strcpy (path_info);

	_file_info = File::CreateAndOpen (File::ASCII, File::READ, path_info);
	if (!_file_info) {
		ssi_wrn ("could not open info file '%s'", path_info);
		return 0;
	}

	TiXmlDocument doc;
	if (!doc.LoadFile (_file_info->getFile (), false)) {
		ssi_wrn ("failed loading samples from file '%s'", path_info);
		return 0;
	}

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "samples") != 0) {
		ssi_wrn ("tag <samples> missing");
		return 0;	
	}

	int v = 0;
	if (body->QueryIntAttribute ("ssi-v", &v) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <ssi-v> in tag <samples> missing");
		return 0;	
	}
	_version = ssi_cast (File::VERSION, v);

	if (_version < File::V2) {
		ssi_wrn ("version < V2 not supported");
		return 0;
	}
	
	TiXmlElement *info = body->FirstChildElement ("info");
	if (!info || strcmp (info->Value (), "info") != 0) {
		ssi_wrn ("tag <info> missing");
		return 0;
	}

	const char *ftype_name = info->Attribute ("ftype");
	if (!ftype_name) {
		ssi_wrn ("attribute <ftype> missing in tag <info>");
		return 0;
	}
	File::TYPE ftype;
	if (strcmp (ftype_name, File::TYPE_NAMES[0]) == 0) {
		ftype = File::BINARY;
	} else if (strcmp (ftype_name, File::TYPE_NAMES[1]) == 0) {
		ftype = File::ASCII;
	} else {
		ssi_wrn ("attribute <ftype> has invalid value '%s' in tag <info>", ftype_name);
		return 0;
	}

	int n_samples = 0;
	if (info->QueryIntAttribute ("size", &n_samples) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <size> in tag <info> missing");
		return 0;	
	}
	_n_samples = ssi_cast (ssi_size_t, n_samples);
	
	switch (_version) {

		case File::V2: {

			int missing = 0;
			if (info->QueryIntAttribute ("missing", &missing) != TIXML_SUCCESS) {
				ssi_wrn ("attribute <missing> in tag <info> missing");
				return 0;	
			}
			_has_missing_data = missing != 0;

			break;
		}

		case File::V3: {

			const char *missing = info->Attribute ("missing");
			if (!missing) {
				ssi_wrn ("attribute <missing> in tag <info> missing");
				return 0;	
			}
			if (ssi_strcmp (missing, "true", false)) {
				_has_missing_data = true;
			} else {
				_has_missing_data = false;
			}
			
			break;
		}
	}


	TiXmlElement *element = 0;
	if (_version == File::V3) {
		info = body;
	}

	// read in streams
	switch (_version) {

		case File::V2: {

			TiXmlElement *streams = info->FirstChildElement ("streams");
			if (!streams || strcmp (streams->Value (), "streams") != 0) {
				ssi_wrn ("tag <streams> missing in <info>");
				return 0;
			}

			element = streams->FirstChildElement ("item");
			_n_streams = 0;
			if (element) {
				do {
					_n_streams++;
					element = element->NextSiblingElement ("item");
				} while (element);
			} else {
				ssi_wrn ("tag <item> missing in <streams>");
				return 0;
			}

			_streams = new ssi_stream_t[_n_streams];
			element = streams->FirstChildElement ("item");
			for (ssi_size_t i = 0; i < _n_streams; i++) {	

				ssi_time_t sample_rate = 0;
				if (element->QueryDoubleAttribute ("sr", &sample_rate) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <sr> missing in tag <info>");
					return 0;
				}

				int sample_dimension = 0;
				if (element->QueryIntAttribute ("dim", &sample_dimension) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <dim> missing in tag <info>");
					return 0;
				}

				int sample_byte = 0;
				if (element->QueryIntAttribute ("byte", &sample_byte) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <byte> missing in tag <info>");
					return 0;
				}

				const char *type = element->Attribute ("type");
				if (!type) {
					ssi_wrn ("attribute <type> missing in tag <info>");
					return 0;
				}
				ssi_type_t sample_type;
				if (!ssi_name2type (type, sample_type)) {
					ssi_wrn ("could not parse type from '%s'", type);
					return 0;
				}

				ssi_stream_init (_streams[i], 0, ssi_cast (int, sample_dimension), ssi_cast (int, sample_byte), sample_type, sample_rate, 0);

				element = element->NextSiblingElement ("item");
			}

			break;
		}
		
		case File::V3: {
		
			TiXmlElement *streams = info->FirstChildElement ("streams");
			if (!streams || strcmp (streams->Value (), "streams") != 0) {
				ssi_wrn ("tag <streams> missing in <info>");
				return 0;
			}

			TiXmlElement *element = streams->FirstChildElement ("item");
			_n_streams = 0;
			if (element) {
				do {
					_n_streams++;
					element = element->NextSiblingElement ("item");
				} while (element);
			} else {
				ssi_wrn ("tag <item> missing in <streams>");
				return 0;
			}

			_streams = new ssi_stream_t[_n_streams];
			_file_streams = new FileStreamIn[_n_streams];

			element = streams->FirstChildElement ("item");
			for (ssi_size_t i = 0; i < _n_streams; i++) {	
				
				const char *path = element->Attribute ("path");
				if (!path) {
					ssi_wrn ("attribute <path> missing in tag <item>");
					return 0;
				}

				ssi_char_t string[SSI_MAX_CHAR];
				if (strcmp (fp.getDir (), "") == 0) {
					ssi_sprint (string, "%s", path);
				} else {
#if _WIN32|_WIN64
					ssi_sprint (string, "%s\\%s", fp.getDir (), path);
#else
                    ssi_sprint (string, "%s/%s", fp.getDir (), path);

#endif
				}

				_file_streams[i].open (_streams[i],	string);			
				element = element->NextSiblingElement ("item");
			}

			break;	
		} 
	}

	ssi_sample_create (_sample, _n_streams, 0, 0, 0, 0);
	for (ssi_size_t i = 0; i < _n_streams; i++) {
		_sample.streams[i] = new ssi_stream_t;
		ssi_stream_init (*_sample.streams[i], 0, _streams[i].dim, _streams[i].byte, _streams[i].type, _streams[i].sr, 0);
	}

	// read in classes

	TiXmlElement *classes = info->FirstChildElement ("classes");
	if (!classes || strcmp (classes->Value (), "classes") != 0) {
		ssi_wrn ("tag <classes> missing in <info>");
		return 0;
	}

	element = classes->FirstChildElement ("item");
	_n_classes = 0;
	if (element) {
		do {
			_n_classes++;			
			element = element->NextSiblingElement ("item");
		} while (element);
	} else {
		ssi_wrn ("tag <item> missing in <classes>");
		return 0;
	}

	element = classes->FirstChildElement ("item");
	_classes = new ssi_char_t *[_n_classes];
	_n_per_class = new ssi_size_t[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		const char *name = element->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in tag <item>");
			return 0;
		}
		_classes[i] = ssi_strcpy (name);
		int n_classes = 0;
		if (element->QueryIntAttribute ("size", &n_classes) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <size> missing in tag <item>");
			return 0;
		}
		_n_per_class[i] = ssi_cast (ssi_size_t, n_classes);
		element = element->NextSiblingElement ("item");
	}

	// read in users

	TiXmlElement *users = info->FirstChildElement ("users");
	if (!users || strcmp (users->Value (), "users") != 0) {
		ssi_wrn ("tag <users> missing in <info>");
		return 0;
	}

	element = users->FirstChildElement ("item");
	_n_users = 0;
	if (element) {
		do {
			_n_users++;			
			element = element->NextSiblingElement ("item");
		} while (element);
	} else {
		ssi_wrn ("tag <item> missing in <users>");
		return 0;
	}

	element = users->FirstChildElement ("item");
	_users = new ssi_char_t *[_n_users];
	_n_per_user = new ssi_size_t[_n_users];
	for (ssi_size_t i = 0; i < _n_users; i++) {
		const char *name = element->Attribute ("name");
		if (!name) {
			ssi_wrn ("attribute <name> missing in tag <item>");
			return 0;
		}
		_users[i] = ssi_strcpy (name);
		int n_users = 0;
		if (element->QueryIntAttribute ("size", &n_users) != TIXML_SUCCESS) {
			ssi_wrn ("attribute <size> missing in tag <item>");
			return 0;
		}
		_n_per_user[i] = ssi_cast (ssi_size_t, n_users);
		element = element->NextSiblingElement ("item");
	}

	ssi_char_t *path_data = ssi_strcat (path_info, "~");			
	_file_data = File::CreateAndOpen (ftype, File::READ, path_data);
	if (!_file_data) {
		ssi_wrn ("could not open data file '%s'", path_data);
		return false;
	}

	_sample_count = 0;

	delete[] path_info;
	delete[] path_data;

	return true;
};

ssi_sample_t *FileSamplesIn::get (ssi_size_t index) {

	reset ();
	ssi_sample_t *s = 0; 
	for (ssi_size_t i = 0; i <= index; i++) {
		s = next ();
	}
	return s;
}

ssi_sample_t *FileSamplesIn::next () {

	if (!_file_data || !_file_info) {
		ssi_wrn ("files not open");
		return 0;
	}
	if (_sample_count++ >= _n_samples) {
		return 0;
	}

	switch (_version) {

		case File::V2: {

			switch (_file_data->getType ()) {

				case File::ASCII: {
			
					if (!_file_data->readLine (SSI_MAX_CHAR, _string)) {
						ssi_wrn ("could not read <user_id> <class_id> <time> <prob> <num>");
						return 0;
					}
					sscanf (_string, "%u %u %lf %f %u", &_sample.user_id, &_sample.class_id, &_sample.time, &_sample.score, &_sample.num);

					break;
				} 

				case File::BINARY: {
	
					if (!_file_data->read (&_sample.user_id, sizeof (_sample.user_id), 1)) {
						ssi_wrn ("could not read <user_id>");
						return 0;
					}

					if (!_file_data->read (&_sample.class_id, sizeof (_sample.class_id), 1)) {
						ssi_wrn ("could not read <class_id>");
						return 0;
					}

					if (!_file_data->read (&_sample.time, sizeof (_sample.time), 1)) {
						ssi_wrn ("could not read <time>");
						return 0;
					}

					if (!_file_data->read (&_sample.score, sizeof (_sample.score), 1)) {
						ssi_wrn ("could not read <score>");
						return 0;
					}

					if (!_file_data->read (&_sample.num, sizeof (_sample.num), 1)) {
						ssi_wrn ("could not read <num>");
						return 0;
					}

					break;
				}
			}

			for (ssi_size_t i = 0; i < _sample.num; i++) {

				ssi_size_t num = 0;

				switch (_file_data->getType ()) {

					case File::ASCII: {

						if (!_file_data->readLine (SSI_MAX_CHAR, _string)) {
							ssi_wrn ("could not read <num>");
							return 0;
						}
						sscanf (_string, "%u", &num);

						break;
					} 

					case File::BINARY: {
		
						if (!_file_data->read (&num, sizeof (num), 1)) {
							ssi_wrn ("could not read <num>");
							return 0;
						}

						break;
					}
				}

				if (num > 0) {

					ssi_stream_adjust (*_sample.streams[i], num);
					_file_data->setType (_sample.streams[i]->type);	
					if (num > 0) {
						if (!_file_data->read (_sample.streams[i]->ptr, _sample.streams[i]->byte, num * _sample.streams[i]->dim)) {
							ssi_wrn ("could not read <data>");
							return 0;
						}
					}
				} else {
					_sample.streams[i]->num = 0;
				}
			}

			break;
		}

		case File::V3: {

			switch (_file_data->getType ()) {

				case File::ASCII: {
			
					if (!_file_data->readLine (SSI_MAX_CHAR, _string)) {
						ssi_wrn ("could not read <user_id> <class_id> <prob> <time>");
						return 0;
					}
					sscanf (_string, "%u %u %f %lf", &_sample.user_id, &_sample.class_id, &_sample.score, &_sample.time);

					break;
				} 

				case File::BINARY: {
	
					if (!_file_data->read (&_sample.user_id, sizeof (_sample.user_id), 1)) {
						ssi_wrn ("could not read <user_id>");
						return 0;
					}

					if (!_file_data->read (&_sample.class_id, sizeof (_sample.class_id), 1)) {
						ssi_wrn ("could not read <class_id>");
						return 0;
					}

					if (!_file_data->read(&_sample.score, sizeof(_sample.score), 1)) {
						ssi_wrn ("could not read <score>");
						return 0;
					}

					if (!_file_data->read (&_sample.time, sizeof (_sample.time), 1)) {
						ssi_wrn ("could not read <time>");
						return 0;
					}

					break;
				}
			}

			for (ssi_size_t i = 0; i < _sample.num; i++) {
				if (_file_streams[i].read(*_sample.streams[i], _sample_count - 1) == FileStreamIn::READ_ERROR)
				{
					ssi_wrn("could not read sample #%u", i);
					return 0;
				}
			}

			break;
		}
		
	}

	return &_sample;
}

bool FileSamplesIn::close () { 

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "close samples file '%s'", _path);

	if (!_file_data->close ()) {
		ssi_wrn ("could not close data file '%s'", _path);
		return false;
	}

	if (!_file_info->close ()) {
		ssi_wrn ("could not close info file '%s'", _file_info->getPath ());
		return false;
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
	delete[] _streams; _streams = 0;
	ssi_sample_destroy (_sample);
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		delete[] _classes[i];
	}
	delete[] _n_per_class; _n_per_class = 0;
	delete[] _classes; _classes = 0;
	for (ssi_size_t i = 0; i < _n_users; i++) {
		delete[] _users[i];
	}
	delete[] _n_per_user; _n_per_user = 0;
	delete[] _users; _users = 0;
	_has_missing_data = false;
	_n_samples = 0;
	_sample_count = 0;
	_n_streams = 0;
	_n_users = 0;

	return true;
};

}
