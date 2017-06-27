// FileAnnotationOut.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/10/26
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

#include "ioput/file/FileAnnotationOut.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"

namespace ssi {

int FileAnnotationOut::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileAnnotationOut::ssi_log_name[] = "fannoout__";
File::VERSION FileAnnotationOut::DEFAULT_VERSION = File::V3;

FileAnnotationOut::FileAnnotationOut ()
	: _is_open(false),
	_file_data (0),
	_file_info (0),
	_version (DEFAULT_VERSION),	
	_console (false),
	_n_labels(0),
	_path (0) {				
}

FileAnnotationOut::~FileAnnotationOut () {

	if (_file_data) {
		close ();
	}
}

bool FileAnnotationOut::open (const ssi_char_t *path,
	ssi_scheme_t &scheme,
	File::TYPE type, 
	File::VERSION version) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "open files '%s'", path);	

	_version = version;
	_scheme = scheme;
	_scheme.name = ssi_strcpy(scheme.name);
	if (_scheme.type == SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_size_t n = _scheme.discrete.n;
		_scheme.discrete.names = ssi_strcpy(n, _scheme.discrete.names);
		_scheme.discrete.ids = new ssi_size_t[n];
		memcpy(_scheme.discrete.ids, scheme.discrete.ids, n * sizeof(ssi_size_t));
	}

	if (_version < File::V3) {
		ssi_wrn ("version < V3 not supported");
		return false;
	}

	if (_file_info || _file_data) {
		ssi_wrn ("annotation already open");
		return false;
	}

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
		if (strcmp (fp.getExtension (), SSI_FILE_TYPE_ANNOTATION) != 0) {
			path_info = ssi_strcat (path, SSI_FILE_TYPE_ANNOTATION);
		} else {
			path_info = ssi_strcpy (path);
		}	
		_path = ssi_strcpy (path_info);

		_file_info = File::CreateAndOpen (File::ASCII, File::WRITE, path_info);
		if (!_file_info) {
			ssi_wrn ("could not open info file '%s'", path_info);
			return false;
		}

		ssi_sprint (_string, "<?xml version=\"1.0\" ?>\n<annotation ssi-v=\"%d\">", version);
		_file_info->writeLine (_string);
	
		ssi_char_t *path_data = ssi_strcat (path_info, "~");			
		_file_data = File::CreateAndOpen (type, File::WRITE, path_data);
		if (!_file_data) {
			ssi_wrn ("could not open data file '%s'", path_data);
			return false;
		}

		delete[] path_info;
		delete[] path_data;

	}

	_is_open = true;

	return true;
};

bool FileAnnotationOut::write (IAnnotation &data) {

	if (!_is_open)
	{
		ssi_wrn("file not open");
		return false;
	}

	ssi_size_t n = data.getSize();	
	for (ssi_size_t i = 0; i < n; i++)
    {	ssi_label_t tmp=data.getLabel(i);
        write(tmp);
	}

	return true;
}

bool FileAnnotationOut::write (ssi_label_t &data) {

	if (!_is_open)
	{
		ssi_wrn("file not open");
		return false;
	}

	if (_console) {

		if (_scheme.type == SSI_SCHEME_TYPE::DISCRETE)
		{
			ssi_print("from=%g to=%g label=%u conf=%g\n", data.discrete.from, data.discrete.to, data.discrete.id, data.confidence);
		}
		else if (_scheme.type == SSI_SCHEME_TYPE::CONTINUOUS)
		{
			ssi_print("score=%g conf=%g\n", data.continuous.score, data.confidence);
		}
		else if (_scheme.type == SSI_SCHEME_TYPE::FREE)
		{
			ssi_print("from=%g to=%g name=%s conf=%g\n", data.discrete.from, data.discrete.to, data.free.name, data.confidence);
		}

	} else {

		_n_labels++;

		switch (_version) {
			
			case File::V3: {

				if (_scheme.type == SSI_SCHEME_TYPE::DISCRETE)
				{
					if (_file_data->getType() == File::ASCII) {
						sprintf(_string, "%g;%g;%d;%g", data.discrete.from, data.discrete.to, data.discrete.id, data.confidence);
						_file_data->writeLine(_string);
					}
					else {
						_file_data->write(&data.discrete.from, sizeof(data.discrete.from), 1);
						_file_data->write(&data.discrete.to, sizeof(data.discrete.to), 1);
						_file_data->write(&data.discrete.id, sizeof(data.discrete.id), 1);
						_file_data->write(&data.confidence, sizeof(data.confidence), 1);
					}
				}
				else if (_scheme.type == SSI_SCHEME_TYPE::CONTINUOUS)
				{
					if (_file_data->getType() == File::ASCII) {
						sprintf(_string, "%g;%g", data.continuous.score, data.confidence);
						_file_data->writeLine(_string);
					}
					else {
						_file_data->write(&data.continuous.score, sizeof(data.continuous.score), 1);
						_file_data->write(&data.confidence, sizeof(data.confidence), 1);
					}
				}
				else if (_scheme.type == SSI_SCHEME_TYPE::FREE)
				{				
					ssi_char_t *tmp = ssi_strrepl(data.free.name, "\n", "");
					ssi_char_t *name = ssi_strrepl(tmp, "\r", "");
					delete[] tmp;

					if (_file_data->getType() == File::ASCII) {
						ssi_fprint(_file_data->getFile(), "%g;%g;%s;%g\n", data.free.from, data.free.to, name, data.confidence);						
					}
					else {
						_file_data->write(&data.free.from, sizeof(data.free.from), 1);
						_file_data->write(&data.free.to, sizeof(data.free.to), 1);
						ssi_size_t n_name = ssi_strlen(name);
						_file_data->write(&n_name, sizeof(n_name), 1);
						_file_data->write(name, sizeof(ssi_char_t), n_name);
						_file_data->write(&data.confidence, sizeof(data.confidence), 1);
					}

					delete[] name;
				}
			}
			break;
		}

	}

	return true;
}

bool FileAnnotationOut::writeMeta(const ssi_char_t *key, const ssi_char_t *value) {

	if (!_is_open)
	{
		ssi_wrn("file not open");
		return false;
	}

	_meta[key] = value;

	return true;
}

bool FileAnnotationOut::close () {

	if (!_is_open)
	{
		ssi_wrn("file not open");
		return false;
	}

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "close files '%s'", _path);

	if (!_console) {

		if (!_file_data->close ()) {
			ssi_wrn ("could not close data file '%s'", _path);
			return false;
		}

		switch (_version) {
			
			case File::V3: {

				TiXmlElement info ("info" );	
				info.SetAttribute ("ftype", File::TYPE_NAMES[_file_data->getType()]);
				info.SetAttribute ("size", _n_labels);
				info.Print(_file_info->getFile(), 1);
				
				if (_meta.size() > 0)
				{
					TiXmlElement meta("meta");
					for (std::map<String, String>::iterator it = _meta.begin(); it != _meta.end(); it++) {
						meta.SetAttribute(it->first.str(), it->second.str());
					}
					_file_info->writeLine("");
					meta.Print(_file_info->getFile(), 1);
				}

				TiXmlElement scheme ("scheme");
				scheme.SetAttribute("name", _scheme.name);
				scheme.SetAttribute("type", SSI_SCHEME_NAMES[_scheme.type]);
				if (_scheme.type == SSI_SCHEME_TYPE::DISCRETE)
				{
					for (ssi_size_t i = 0; i < _scheme.discrete.n; i++) {
						TiXmlElement item("item");
						item.SetAttribute("name", _scheme.discrete.names[i]);
						item.SetAttribute("id", _scheme.discrete.ids[i]);
						scheme.InsertEndChild(item);
					}
				}
				else if (_scheme.type == SSI_SCHEME_TYPE::CONTINUOUS)
				{
					scheme.SetDoubleAttribute("sr",_scheme.continuous.sr);
					scheme.SetAttribute("min", (int) _scheme.continuous.min);
					scheme.SetAttribute("max", (int) _scheme.continuous.max);
				}				
				_file_info->writeLine("");
				scheme.Print(_file_info->getFile(), 1);

				_file_info->writeLine ("\n</annotation>");

				if (!_file_info->close ()) {
					ssi_wrn ("could not close info file '%s'", _file_info->getPath ());
					return false;
				}

			}
			break;
		}
	}

	delete _file_data; _file_data = 0;
	delete _file_info; _file_info = 0;
	delete _path; _path = 0;

	delete[] _scheme.name;
	if (_scheme.type == SSI_SCHEME_TYPE::DISCRETE)
	{
		for (ssi_size_t i = 0; i < _scheme.discrete.n; i++)
		{
			delete[] _scheme.discrete.names[i];
		}
		delete[] _scheme.discrete.names;
		_scheme.discrete.names = 0;
		delete[] _scheme.discrete.ids;
		_scheme.discrete.ids = 0;
	}

	_meta.clear();
	_n_labels = 0;

	return true;
};

}
 
