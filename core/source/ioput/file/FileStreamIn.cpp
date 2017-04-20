// FileStreamIn.cpp
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

#include "ioput/file/FileStreamIn.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"

namespace ssi {

int FileStreamIn::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;
ssi_char_t FileStreamIn::ssi_log_name[] = "fstrmin___";
ssi_size_t FileStreamIn::NEXT_CHUNK = -1;
ssi_size_t FileStreamIn::READ_ERROR = -1;

FileStreamIn::FileStreamIn ()
	: _file_data (0),
	_file_info (0),
	_n_chunks (0),
	_n_samples (0),
	_next_chunk (0),
	_samples (0),
	_time (0),
	_bytes (0),
	_path (0),
	_version (File::V2) {			

	_stream.ptr = 0;
}

FileStreamIn::~FileStreamIn () {

	if (_file_data) {
		close ();
	}
}

bool FileStreamIn::open (ssi_stream_t &data,
	const ssi_char_t *path) {

	ssi_size_t n_meta;
	ssi_byte_t *meta;

	return open (data, path, n_meta, (void **) &meta);
}

bool FileStreamIn::open (ssi_stream_t &data,
	const ssi_char_t *path,
	ssi_size_t &n_meta,
	void **meta) {

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "open stream file '%s'", path);	

	if (_file_info || _file_data) {
		ssi_wrn ("stream already open");
		return false;
	}

	if (path == 0 || path[0] == '\0') {
		ssi_wrn ("'%s' is not a valid path", path);
		return false;
	}

	FilePath fp (path);
	ssi_char_t *path_info = 0;
	if (strcmp (fp.getExtension (), SSI_FILE_TYPE_STREAM) != 0) {
		path_info = ssi_strcat (path, SSI_FILE_TYPE_STREAM);
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
		ssi_wrn ("failed loading stream from file '%s'", path_info);
		return false;
	}

	TiXmlElement *body = doc.FirstChildElement();	
	if (!body || strcmp (body->Value (), "stream") != 0) {
		ssi_wrn ("tag <stream> missing");
		return false;	
	}

	int v = 0;
	if (body->QueryIntAttribute ("ssi-v", &v) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <ssi-v> in tag <stream> missing");
		return false;	
	}
	_version = ssi_cast (File::VERSION, v);

	if (_version < File::V2) {
		ssi_wrn ("version < V2 not supported");
		return false;
	}
	
	TiXmlElement *element = body->FirstChildElement ("info");
	if (!element || strcmp (element->Value (), "info") != 0) {
		ssi_wrn ("tag <info> missing");
		return false;
	}

	ssi_time_t sample_rate = 0;
	if (element->QueryDoubleAttribute ("sr", &sample_rate) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <sr> missing in tag <info>");
		return false;
	}

	int sample_dimension = 0;
	if (element->QueryIntAttribute ("dim", &sample_dimension) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <dim> missing in tag <info>");
		return false;
	}

	int sample_byte = 0;
	if (element->QueryIntAttribute ("byte", &sample_byte) != TIXML_SUCCESS) {
		ssi_wrn ("attribute <byte> missing in tag <info>");
		return false;
	}

	const char *type = element->Attribute ("type");
	if (!type) {
		ssi_wrn ("attribute <type> missing in tag <info>");
		return false;
	}
	ssi_type_t sample_type;
	if (!ssi_name2type (type, sample_type)) {
		ssi_wrn ("could not parse type from '%s'", type);
		return false;
	}

	const char *ftype_name = element->Attribute ("ftype");
	if (!ftype_name) {
		ssi_wrn ("attribute <ftype> missing in tag <info>");
		return false;
	}
	File::TYPE ftype;
	if (strcmp (ftype_name, File::TYPE_NAMES[0]) == 0) {
		ftype = File::BINARY;
	} else if (strcmp (ftype_name, File::TYPE_NAMES[1]) == 0) {
		ftype = File::ASCII;
	} else {
		ssi_wrn ("attribute <ftype> has invalid value '%s' in tag <info>", ftype_name);
		return false;
	}

	const char *delim = 0;
	if (ftype == File::ASCII)
	{
		delim = element->Attribute("delim");
	}

	*meta = 0;
	n_meta = 0;

	element = body->FirstChildElement ("meta");
	if (element && strcmp (element->Value (), "meta") == 0) {

		switch (sample_type) {
			case SSI_IMAGE: {
				n_meta = sizeof (ssi_video_params_t);
				ssi_video_params_t *params = new ssi_video_params_t ();
				*meta = params;
				if (element->QueryIntAttribute ("width", &params->widthInPixels) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <width> missing in tag <meta>");
					return false;
				}
				if (element->QueryIntAttribute ("height", &params->heightInPixels) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <height> missing in tag <meta>");
					return false;
				}
				if (element->QueryIntAttribute ("depth", &params->depthInBitsPerChannel) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <depth> missing in tag <meta>");
					return false;
				}
				if (element->QueryIntAttribute ("channels", &params->numOfChannels) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <channels> missing in tag <meta>");
					return false;
				}
				int flip = 0;
				if (element->QueryIntAttribute ("channels", &flip) != TIXML_SUCCESS) {
					ssi_wrn ("attribute <flip> missing in tag <meta>");
				}
				params->flipImage = flip != 0;
		
				break;
			}
			default:
				break;
		}
	}

	ssi_char_t *path_data = ssi_strcat (path_info, "~");			
	_file_data = File::CreateAndOpen (ftype, File::READ, path_data);
	if (!_file_data) {
		ssi_wrn ("could not open data file '%s'", path_data);
		return false;
	}
	if (ftype == File::ASCII) {
		_file_data->setFormat(delim, SSI_FILE_DEFAULT_FLAGS);
	}

	ssi_stream_init (data, 0, sample_dimension, sample_byte, sample_type, sample_rate, 0);
	ssi_stream_init (_stream, 0, sample_dimension, sample_byte, sample_type, sample_rate, 0);

	// count chunks
	_n_chunks = 0;	
	_next_chunk = 0;
	element = body->FirstChildElement ("chunk");
	if (element) {
		do {
			_n_chunks++;
			element = element->NextSiblingElement ("chunk");
		} while (element);
	}

	// read in chunks
	_n_samples = 0;
	_samples = new ssi_size_t[_n_chunks];
	_time = new ssi_time_t[_n_chunks];
	_bytes = new ssi_size_t[_n_chunks];	
	element = body->FirstChildElement ("chunk");
	ssi_size_t n = 0;
	if (element) {
		do {
			ssi_time_t time = 0;
			if (element->QueryDoubleAttribute ("from", &time) != TIXML_SUCCESS) {
				ssi_wrn ("attribute <time> missing in tag <chunk>");
				return false;
			}
			_time[n] = time;
			int bytes = 0;
			if (element->QueryIntAttribute ("byte", &bytes) != TIXML_SUCCESS) {
				ssi_wrn ("attribute <bytes> missing in tag <chunk>");
				return false;
			}
			_bytes[n] = ssi_cast (ssi_size_t, bytes);
			int samples = 0;
			if (element->QueryIntAttribute ("num", &samples) != TIXML_SUCCESS) {
				ssi_wrn ("attribute <time> missing in tag <chunk>");
				return false;
			}
			_n_samples += ssi_cast (ssi_size_t, samples);
			_samples[n++] = ssi_cast (ssi_size_t, samples);
			element = element->NextSiblingElement ("chunk");
		} while (element);
	}

	delete[] path_info;
	delete[] path_data;

	return true;
};

ssi_size_t FileStreamIn::read (ssi_stream_t &data) {

	if (data.sr != _stream.sr || data.byte != _stream.byte || data.dim != _stream.dim || data.type != _stream.type) {
		ssi_wrn ("stream is not compatible");
		return READ_ERROR;
	}

	if (data.num > 0) {
		if (_file_data->getType () == File::ASCII) {
			_file_data->setType (data.type);		
			if (!_file_data->read (data.ptr, data.dim, data.dim * data.num)) {
				ssi_wrn ("could not read <data>");
				return READ_ERROR;
			}
		} else {
			if (!_file_data->read (data.ptr, 1, data.tot)) {
				ssi_wrn ("could not read <data>");
				return READ_ERROR;
			}
		}
	}

	data.time += data.num / data.sr;

	return data.num;
}

ssi_size_t FileStreamIn::read (ssi_stream_t &data, 
	ssi_size_t chunk_id) {

	if (data.sr != _stream.sr || data.byte != _stream.byte || data.dim != _stream.dim || data.type != _stream.type) {
		ssi_wrn ("stream is not compatible");
		return READ_ERROR;
	}

	if (chunk_id != NEXT_CHUNK && chunk_id >= _n_chunks) {
		ssi_wrn ("requested chunk '%u' exceeds #chunks '%u'", chunk_id, _n_chunks);
		return READ_ERROR;
	} else if (chunk_id == NEXT_CHUNK && _next_chunk >= _n_chunks) {
		return 0;
	} else if (chunk_id == NEXT_CHUNK) {
		chunk_id = _next_chunk++;
	}

	data.time = _time[chunk_id];
	ssi_size_t num = _samples[chunk_id];	
	if (num == 0) {
		data.num = 0;
		data.tot = 0;
		return 0;
	}

	if (data.num_real < num) {
		ssi_stream_adjust (data, num);
	}

	if (_file_data->tell () != _bytes[chunk_id]) {
		_file_data->seek (_bytes[chunk_id], File::BEGIN);
	}

	_file_data->setType (data.type);	
	if (!_file_data->read (data.ptr, data.byte, num * data.dim)) {
		ssi_wrn ("could not read <data>");
		return READ_ERROR;
	}	
	data.num = num;
	data.tot = num * data.dim * data.byte;

	return num;
}

bool FileStreamIn::close () { 

	ssi_msg (SSI_LOG_LEVEL_DETAIL, "close stream file '%s'", _path);

	if (!_file_data || !_file_data->close ()) {
		ssi_wrn ("could not close data file '%s'", _path);
		return false;
	}

	if (!_file_info || !_file_info->close ()) {
		ssi_wrn ("could not close info file '%s'", _file_info->getPath ());
		return false;
	}

	delete _file_data; _file_data = 0;
	delete _file_info; _file_info = 0;
	delete[] _path; _path = 0;
	delete[] _samples; _samples = 0;
	delete[] _time; _time = 0;
	delete[] _bytes; _bytes = 0;
	_n_chunks = 0;
	_n_samples = 0;
	_next_chunk = 0;

	return true;
};

}
