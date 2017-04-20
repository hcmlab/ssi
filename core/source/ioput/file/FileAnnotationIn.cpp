// FileAnnotationIn.cpp
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

#include "ioput/file/FileAnnotationIn.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"

namespace ssi {

File::VERSION FileAnnotationIn::DEFAULT_VERSION = File::V3;
int FileAnnotationIn::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileAnnotationIn::ssi_log_name[] = "fannoin___";

FileAnnotationIn::FileAnnotationIn ()
	: _file_data (0),
	_file_info (0),
	_path (0),	
	_label_count(0),
	_scheme(0),
	_version (File::V2) {
	
	_label.free.name = 0;
}

FileAnnotationIn::~FileAnnotationIn () {

	if (_file_data) {
		close ();
	}
}

bool FileAnnotationIn::open (const ssi_char_t *path) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "open annotation file '%s'", path);	

	if (_file_info || _file_data) {
		ssi_wrn ("annotation file already open");
		return false;
	}

	if (path == 0 || path[0] == '\0') {
		ssi_wrn ("'%s' is not a valid path", path);
		return false;
	}

	FilePath fp (path);
	ssi_char_t *path_info = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_ANNOTATION) != 0) {
		path_info = ssi_strcat (path, SSI_FILE_TYPE_ANNOTATION);
	} else {
		path_info = ssi_strcpy (path);
	}	
	_path = ssi_strcpy (path_info);

	_file_info = File::CreateAndOpen (File::ASCII, File::READ, path_info);
	if (!_file_info) {
		ssi_wrn ("could not open info file '%s'", path_info);
		return false;
	}

	TiXmlDocument doc;
	if (!doc.LoadFile (_file_info->getFile (), false)) {
		ssi_wrn ("failed loading annotation from file '%s'", path_info);
		return false;
	}

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "annotation") != 0) {
		ssi_wrn ("tag <annotation> missing");
		return false;	
	}

	int v = 0;
	_version = DEFAULT_VERSION;
	if (body->QueryIntAttribute ("ssi-v", &v) == TIXML_SUCCESS) {
		_version = ssi_cast (File::VERSION, v);
		if (_version < File::V3) {
			ssi_wrn("version < V3 not supported");
			return false;

		}
	}

	File::TYPE ftype = File::ASCII;
	int n_labels = 0;
	
	TiXmlElement *info = body->FirstChildElement ("info");
	if (info) {

		const char *ftype_name = info->Attribute("ftype");
		if (!ftype_name) {
			ssi_wrn("attribute <ftype> missing in tag <info>");
			return false;
		}

		if (strcmp(ftype_name, File::TYPE_NAMES[0]) == 0) {
			ftype = File::BINARY;
		}
		else if (strcmp(ftype_name, File::TYPE_NAMES[1]) == 0) {
			ftype = File::ASCII;
		}
		else {
			ssi_wrn("attribute <ftype> has invalid value '%s' in tag <info>", ftype_name);
			return false;
		}

		if (info->QueryIntAttribute("size", &n_labels) != TIXML_SUCCESS) {
			ssi_wrn("attribute <size> in tag <info> missing");
			return false;
		}

	}

	_n_labels = ssi_cast (ssi_size_t, n_labels);
	_label_count = 0;

	// read in meta
	
	TiXmlElement *meta = body->FirstChildElement("meta");
	if (meta)
	{
		TiXmlAttribute *meta_attribute = meta->FirstAttribute();
		do {
			_meta[meta_attribute->Name()] = meta_attribute->Value();
		} while (meta_attribute = meta_attribute->Next());
	}

	// read in scheme

	_scheme = new ssi_scheme_t;

	TiXmlElement *scheme = body->FirstChildElement ("scheme");
	if (!scheme) {
		ssi_wrn ("tag <scheme> missing");
		return false;
	}

	const ssi_char_t *scheme_name = scheme->Attribute("name");
	if (!scheme_name)
	{
		ssi_wrn("attribute <name> missing in <scheme>");
		return false;
	}
	_scheme->name = ssi_strcpy(scheme_name);

	const char *scheme_type = scheme->Attribute("type");
	if (!scheme_type)
	{
		ssi_wrn("attribute <type> missing in <scheme>");
		return false;
	}
	bool found = false;	
	for (ssi_size_t i = 0; i < SSI_SCHEME_TYPE::NUM; i++)
	{
		if (ssi_strcmp(SSI_SCHEME_NAMES[i], scheme_type, false))
		{
			found = true;
			_scheme->type = (SSI_SCHEME_TYPE::List) i;
			break;
		}
	}
	if (!found)
	{
		ssi_wrn("unknown scheme type '%s'", scheme_type);
		return false;
	}	

	switch (_scheme->type)
	{
	case SSI_SCHEME_TYPE::DISCRETE:
	{
		TiXmlElement *element = 0;
		
		element = scheme->FirstChildElement("item");
		_scheme->discrete.n = 0;
		if (element) {
			do {
				_scheme->discrete.n++;
				element = element->NextSiblingElement("item");
			} while (element);
		}
		else {
			ssi_wrn("tag <item> missing in <classes>");
			return false;
		}

		element = scheme->FirstChildElement("item");
		_scheme->discrete.names = new ssi_char_t *[_scheme->discrete.n];
		_scheme->discrete.ids = new ssi_size_t[_scheme->discrete.n];
		for (ssi_size_t i = 0; i < _scheme->discrete.n; i++) {
			const char *name = element->Attribute("name");
			if (!name) {
				ssi_wrn("attribute <name> missing in tag <item>");
				return false;
			}
			_scheme->discrete.names[i] = ssi_strcpy(name);
			int class_id = 0;
			if (element->QueryIntAttribute("id", &class_id) != TIXML_SUCCESS) {
				ssi_wrn("attribute <id> missing in tag <item>");
				return false;
			}
			_scheme->discrete.ids[i] = class_id;
			element = element->NextSiblingElement("item");
		}

		break;
	}
	case SSI_SCHEME_TYPE::CONTINUOUS:
	{		
		if (!scheme->Attribute("sr", &_scheme->continuous.sr))
		{
			ssi_wrn("attribute <sr> missing in <scheme>");
			return false;
		}
		
		double min_score;
		if (!scheme->Attribute("min", &min_score))
		{
			ssi_wrn("attribute <min> missing in <scheme>");
			return false;
		}
		_scheme->continuous.min = (ssi_real_t)min_score;

		double max_score;
		if (!scheme->Attribute("max", &max_score))
		{
			ssi_wrn("attribute <max> missing in <scheme>");
			return false;
		}
		_scheme->continuous.max = (ssi_real_t)max_score;

		break;
	}
	case SSI_SCHEME_TYPE::FREE:
	{
		break;
	}
	}
	
	_file_data = 0;
	if (_n_labels > 0)
	{
		ssi_char_t *path_data = ssi_strcat(path_info, "~");
		_file_data = File::CreateAndOpen(ftype, File::READ, path_data);
		if (!_file_data) {
			ssi_wrn("could not open data file '%s'", path_data);
			return false;
		}
		delete[] path_data;
	}

	delete[] path_info;	

	return true;
};

const ssi_label_t *FileAnnotationIn::get (ssi_size_t index) {

	reset ();
	const ssi_label_t *s = 0; 
	for (ssi_size_t i = 0; i <= index; i++) {
		s = next ();
	}
	return s;
}

const ssi_label_t *FileAnnotationIn::next () {

	if (!_file_data || !_file_info) {
		ssi_wrn ("files not open");
        return NULL;
	}
	if (_label_count++ >= _n_labels) {
        return NULL;
	}

	switch (_version) {

		case File::V3: {

			switch (_file_data->getType ()) 
			{
				case File::ASCII: 
				{
					switch (_scheme->type)
					{
						case SSI_SCHEME_TYPE::DISCRETE:
						{
							if (!_file_data->readLine(SSI_MAX_CHAR, _string)) {
								ssi_wrn("could not read <from> <to> <id> <conf>");
                                return NULL;
							}
							sscanf(_string, "%lf;%lf;%d;%f", &_label.discrete.from, &_label.discrete.to, &_label.discrete.id, &_label.confidence);
						}
						break;

						case SSI_SCHEME_TYPE::CONTINUOUS:
						{
							if (!_file_data->readLine(SSI_MAX_CHAR, _string)) {
								ssi_wrn("could not read <score> <conf>");
                                return NULL;
							}
							sscanf(_string, "%f;%f", &_label.continuous.score, &_label.confidence);
						}
						break;

						case SSI_SCHEME_TYPE::FREE:
						{
							if (!_file_data->readLine(SSI_MAX_CHAR, _string)) {
								ssi_wrn("could not read <from> <to> <name> <conf>");
                                return NULL;
							}					
							ssi_size_t n_tokens = ssi_split_string_count(_string, ';', true);
							if (n_tokens != 4)
							{
								ssi_wrn("could not read <from> <to> <name> <conf>");
                                return NULL;
							}
							ssi_char_t **tokens = new ssi_char_t *[n_tokens];
							ssi_split_string(n_tokens, tokens, _string, ';', true);
							sscanf(tokens[0], "%lf", &_label.free.from);
							sscanf(tokens[1], "%lf", &_label.free.to);
							delete[] _label.free.name;
							_label.free.name = ssi_strcpy(tokens[2]);
							sscanf(tokens[3], "%f", &_label.confidence);
							for (ssi_size_t i = 0; i < n_tokens; i++)
							{
								delete[] tokens[i];
							}
							delete[] tokens;
						}
						break;
					}
				}
				break;


				case File::BINARY: 
				{
	
					switch (_scheme->type)
					{
						case SSI_SCHEME_TYPE::DISCRETE:
						{
							if (!_file_data->read(&_label.discrete.from, sizeof(_label.discrete.from), 1)) {
								ssi_wrn("could not read <from>");
                                return NULL;
							}
							if (!_file_data->read(&_label.discrete.to, sizeof(_label.discrete.to), 1)) {
								ssi_wrn("could not read <to>");
                                return NULL;
							}
							if (!_file_data->read(&_label.discrete.id, sizeof(_label.discrete.id), 1)) {
								ssi_wrn("could not read <id>");
                                return NULL;
							}
							if (!_file_data->read(&_label.confidence, sizeof(_label.confidence), 1)) {
								ssi_wrn("could not read <confidence>");
                                return NULL;
							}
						}
						break;
						case SSI_SCHEME_TYPE::CONTINUOUS:
						{
							if (!_file_data->read(&_label.continuous.score, sizeof(_label.continuous.score), 1)) {
								ssi_wrn("could not read <from>");
                                return NULL;
							}
							if (!_file_data->read(&_label.confidence, sizeof(_label.confidence), 1)) {
								ssi_wrn("could not read <confidence>");
                                return NULL;
							}
						}
						break;
						case SSI_SCHEME_TYPE::FREE:
						{
							if (!_file_data->read(&_label.free.from, sizeof(_label.free.from), 1)) {
								ssi_wrn("could not read <from>");
                                return NULL;
							}
							if (!_file_data->read(&_label.free.to, sizeof(_label.free.to), 1)) {
								ssi_wrn("could not read <to>");
                                return NULL;
							}
							ssi_size_t n_name;
							if (!_file_data->read(&n_name, sizeof(n_name), 1)) {
								ssi_wrn("could not read <n_name>");
                                return NULL;
							}
 							if (n_name > 0 && !_file_data->read(_string, n_name, 1)) {
								ssi_wrn("could not read <name>");
                                return NULL;
							}
							_string[n_name] = '\0';
							delete[] _label.free.name;
							_label.free.name = ssi_strcpy(_string);
							if (!_file_data->read(&_label.confidence, sizeof(_label.confidence), 1)) {
								ssi_wrn("could not read <confidence>");
                                return NULL;
							}
						}
						break;
					}
				}
				break;
			}
		}
	}

	return &_label;
}

bool FileAnnotationIn::close () { 

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "close annotation file '%s'", _path);

	if (_file_data && !_file_data->close ()) {
		ssi_wrn ("could not close data file '%s'", _path);
		return false;
	}

	if (_file_info && !_file_info->close ()) {
		ssi_wrn ("could not close info file '%s'", _file_info->getPath ());
		return false;
	}

	delete _file_data; _file_data = 0;
	delete _file_info; _file_info = 0;
	delete _path; _path = 0;
	
	if (_scheme)
	{
		delete[] _scheme->name;
		if (_scheme->type == SSI_SCHEME_TYPE::DISCRETE)
		{
			for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
			{
				delete[] _scheme->discrete.names[i];
			}
			delete[] _scheme->discrete.names;
			_scheme->discrete.names = 0;
			delete[] _scheme->discrete.ids;
			_scheme->discrete.ids = 0;
		} 
		else if (_scheme->type == SSI_SCHEME_TYPE::FREE)
		{
			delete[] _label.free.name; _label.free.name = 0;
		}
		delete _scheme;
	}

	return true;
};

void FileAnnotationIn::reset() {
	if (_file_data) {
		_file_data->seek(0);
		_label_count = 0;
	}
}

const ssi_char_t *FileAnnotationIn::getMeta(const ssi_char_t *key)
{
	std::map<String, String>::iterator it;
	it = _meta.find(String(key));
	if (it == _meta.end())
	{
		return 0;
	}
	else
	{
		return _meta[key].str();
	}
}

ssi_size_t FileAnnotationIn::getMetaSize()
{
	return (ssi_size_t)_meta.size();
}

const ssi_char_t *FileAnnotationIn::getMetaKey(ssi_size_t index)
{
	for (std::map<String, String>::iterator it = _meta.begin(); it != _meta.end(); it++)
	{
		if (index-- == 0)
		{
			return it->first.str();
		}
	}

	return 0;
}

const ssi_scheme_t *FileAnnotationIn::getScheme()
{
	return _scheme;
}

ssi_size_t FileAnnotationIn::getSize() {
	return _n_labels;
}

File *FileAnnotationIn::getInfoFile() {
	return _file_info;
}

File *FileAnnotationIn::getDataFile() {
	return _file_data;
}

File::VERSION FileAnnotationIn::getVersion() {
	return _version;
}

void FileAnnotationIn::SetLogLevel(int level) {
	ssi_log_level = level;
}

}
