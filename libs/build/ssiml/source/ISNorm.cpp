// ISNorm.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/08
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

#include "ISNorm.h"
#include "ModelTools.h"
#include "ioput/xml/tinyxml.h"
#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *ISNorm::ssi_log_name = "normalize_";

const ssi_char_t *ISNorm::METHOD_NAMES[] = { "None", "Scale", "ZScore" };

ISNorm::ISNorm (ISamples *samples)
	: _samples (*samples),
	_params (0) {

	ssi_sample_clone (*_samples.get (0), _sample);
	_n_streams = _samples.getStreamSize ();
	_n_samples = _samples.getSize ();
	_n_features = new ssi_size_t[_n_streams];
	_params = new Params *[_n_streams];
	for (ssi_size_t i = 0; i < _n_streams; i++) {		
		_n_features [i] = _samples.getStream(i).dim;
		_params[i] = 0;
	}
}

ISNorm::~ISNorm () {
	
	if (_params)
	{
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			if (_params[i])
			{
				//ReleaseParams(*_params[i]);
			}
		}
	}

	delete[] _params;
	delete[] _n_features;
	ssi_sample_destroy (_sample);
	_n_streams = 0;
}	

void ISNorm::ZeroParams(Params &params, METHOD::List method) {

	params.method = method;
	params.inialized = false;
	params.n_features = 0;
	params.maxs = 0;
	params.mins = 0;
	params.mean = 0;
	params.stdv = 0;
	params.limits[0] = 0;
	params.limits[1] = 1;
}

void ISNorm::InitParams(Params &params, ssi_size_t n_features) {

	params.n_features = n_features;

	switch (params.method) {
	case METHOD::SCALE:
		params.mins = new ssi_real_t[n_features];
		params.maxs = new ssi_real_t[n_features];		
		break;
	case METHOD::ZSCORE:
		params.mean = new ssi_real_t[n_features];
		params.stdv = new ssi_real_t[n_features];
		break;
	}
}

void ISNorm::ReleaseParams(Params &params) {

	delete[] params.maxs;
	delete[] params.mins;
	delete[] params.mean;
	delete[] params.stdv;

	ZeroParams(params);
}

bool ISNorm::SaveParams(const ssi_char_t *path, Params &params, File::TYPE type) {

	ssi_char_t string[SSI_MAX_CHAR];

	FilePath fp(path);
	ssi_char_t *path_xml;
	ssi_char_t *path_data;
	if (ssi_strcmp(fp.getExtension(), "norm", false)) {
		path_xml = ssi_strcpy(path);
	} else {
		path_xml = ssi_strcat(path, ".norm");
	}
	path_data = ssi_strcat(path_xml, "~");

	TiXmlElement norm("norm");
	
	TiXmlElement norm_method("method");
	norm_method.InsertEndChild(TiXmlText(METHOD_NAMES[params.method]));
	norm.InsertEndChild(norm_method);

	if (params.method == METHOD::SCALE) {
		TiXmlElement norm_limits("limits");
		ssi_sprint(string, "%f %f", params.limits[0], params.limits[1]);
		norm_limits.InsertEndChild(TiXmlText(string));
		norm.InsertEndChild(norm_limits);
	}

	TiXmlElement norm_dim("dim");
	ssi_sprint(string, "%u", params.n_features);
	norm_dim.InsertEndChild(TiXmlText(string));
	norm.InsertEndChild(norm_dim);

	TiXmlElement norm_type("type");	
	norm_type.InsertEndChild(TiXmlText(File::TYPE_NAMES[type]));
	norm.InsertEndChild(norm_type);
	
	TiXmlDocument doc;
	TiXmlDeclaration head("1.0", "", "");
	doc.InsertEndChild(head);
	doc.InsertEndChild(norm);	
	doc.SaveFile(path_xml);

	if (params.method != METHOD::NONE) {
		FILE *fp = fopen(path_data, type == File::BINARY ? "wb" : "w");
		if (fp) {
			switch (params.method) {			
			case METHOD::SCALE:
				if (type == File::BINARY) {
					fwrite(params.mins, params.n_features, sizeof(ssi_real_t), fp);
					fwrite(params.maxs, params.n_features, sizeof(ssi_real_t), fp);
				} else {
					ssi_real_t *mins = params.mins;
					ssi_real_t *maxs = params.maxs;
					for (ssi_size_t i = 0; i < params.n_features; i++) {
						ssi_fprint(fp, "%f %f\n", *mins++, *maxs++);
					}
				}
				break;
			case METHOD::ZSCORE:
				if (type == File::BINARY) {
					fwrite(params.mean, params.n_features, sizeof(ssi_real_t), fp);
					fwrite(params.stdv, params.n_features, sizeof(ssi_real_t), fp);
				}
				else {
					ssi_real_t *mean = params.mean;
					ssi_real_t *stdv = params.stdv;
					for (ssi_size_t i = 0; i < params.n_features; i++) {
						ssi_fprint(fp, "%f %f\n", *mean++, *stdv++);
					}
				}
				
				break;
			}
		}
		else {
			ssi_wrn("could not open file '%s'", path_data);
			return false;
		}
		fclose(fp);
	}

	delete[] path_data;
	delete[] path_xml;

	return true;
}

bool ISNorm::LoadParams(const ssi_char_t *path, Params &params) {

	FilePath fp(path);
	ssi_char_t *path_xml;
	ssi_char_t *path_data;
	if (ssi_strcmp(fp.getExtension(), ".norm", false)) {
		path_xml = ssi_strcpy(path);
	}
	else {
		path_xml = ssi_strcat(path, ".norm");
	}
	path_data = ssi_strcat(path_xml, "~");

	TiXmlDocument doc;
	if (!doc.LoadFile(path_xml)) {
		ssi_wrn("failed loading trainer from file '%s' (r:%d,c:%d)", path_xml, doc.ErrorRow(), doc.ErrorCol());
		return false;
	}

	TiXmlElement *body = doc.FirstChildElement();
	if (!body || strcmp(body->Value(), "norm") != 0) {
		ssi_wrn("tag <norm> missing");
		return false;
	}

	TiXmlElement *element = 0;
	
	element = body->FirstChildElement("method");
	const ssi_char_t *method_s = element->GetText();
	bool found = false;
	METHOD::List method;
	for (ssi_size_t i = 0; i < METHOD::NUM; i++) {
		if (ssi_strcmp(method_s, METHOD_NAMES[i], false)) {
			method = (METHOD::List) i;
			found = true;
		}
	}
	if (!found) {
		ssi_wrn("unkown method '%s'", method_s);
		return false;
	}

	ISNorm::ZeroParams(params, method);

	element = body->FirstChildElement("dim");
	const ssi_char_t *dim_s = element->GetText();	
	ssi_size_t dim;
	if (sscanf(dim_s, "%u", &dim) != 1) {
		ssi_wrn("could not parse dimension '%s'", dim_s);
		return false;
	}
	ISNorm::InitParams(params, dim);

	if (method == METHOD::SCALE) {
		element = body->FirstChildElement("limits");
		const ssi_char_t *limits_s = element->GetText();
		if (sscanf(limits_s, "%f %f", params.limits, params.limits + 1) != 2) {
			ssi_wrn("could not parse limits '%s'", limits_s);
			return false;
		}
	}

	element = body->FirstChildElement("type");
	const ssi_char_t *type_s = element->GetText();
	found = false;
	File::TYPE type;
	if (ssi_strcmp(type_s, File::TYPE_NAMES[File::ASCII], false)) {
		type = File::ASCII;
		found = true;
	} else if (ssi_strcmp(type_s, File::TYPE_NAMES[File::BINARY], false)) {
		type = File::BINARY;
		found = true;
	}
 	
	if (!found) {
		ssi_wrn("unkown type '%s'", type_s);
		return false;
	}

	if (method != METHOD::NONE) {
		FILE *fp = fopen(path_data, type == File::BINARY ? "rb" : "r");
		if (fp) {
			switch (params.method) {
			case METHOD::NONE:
				break;
			case METHOD::SCALE:
				if (type == File::BINARY) {				
					fread(params.mins, params.n_features, sizeof(ssi_real_t), fp);
					fread(params.maxs, params.n_features, sizeof(ssi_real_t), fp);
				} else {
					ssi_real_t *mins = params.mins;
					ssi_real_t *maxs = params.maxs;
					for (ssi_size_t i = 0; i < params.n_features; i++) {
						fscanf(fp, "%f %f\n", mins++, maxs++);
					}
				}
				break;
			case METHOD::ZSCORE:
				if (type == File::BINARY) {
					fread(params.mean, params.n_features, sizeof(ssi_real_t), fp);
					fread(params.stdv, params.n_features, sizeof(ssi_real_t), fp);
				} else {
					ssi_real_t *mean = params.mean;
					ssi_real_t *stdv = params.stdv;
					for (ssi_size_t i = 0; i < params.n_features; i++) {
						fscanf(fp, "%f %f\n", mean++, stdv++);
					}
				}
				break;
			}
			fclose(fp);
		}
		else {
			ssi_wrn("could not open file '%s'", path_data);
			return false;
		}
	}

	delete[] path_xml;
	delete[] path_data;

	return true;
}

bool ISNorm::setNorm (ssi_size_t index,
	Params &params) {

	if (index >= _n_streams) {
		ssi_wrn("index exceeds #streams (%u != %u)", index, _n_streams);
		return false;
	}

	if (params.inialized && params.n_features != _n_features[index]) {
		ssi_wrn("precomputed parameters are bad #features (%u != %u)", params.n_features, _n_features[index]);
		return false;
	}

	_params[index] = &params;	
	if (!params.inialized) {

		InitParams(params, _n_features[index]);

		if (params.method != METHOD::NONE) {

			ssi_real_t **matrix;
			ssi_size_t *classes;

			ModelTools::CreateSampleMatrix(_samples, index, _n_samples, params.n_features, &classes, &matrix);

			switch (params.method) {
			case METHOD::SCALE: {
				ssi_minmax(_n_samples, params.n_features, matrix[0], params.mins, params.maxs);
				ssi_real_t *aptr = params.mins;
				ssi_real_t *bptr = params.maxs;
				for (ssi_size_t j = 0; j < params.n_features; j++) {
					*bptr -= *aptr++;
					if (*bptr == 0.0f) {
						ssi_wrn("zero interval '%u'", j);
						*bptr = 1.0f;
					}
					bptr++;
				}
				break;
			}

			case METHOD::ZSCORE: {
				ssi_mean(_n_samples, params.n_features, matrix[0], params.mean);
				ssi_stdv(_n_samples, params.n_features, matrix[0], params.stdv);
				ssi_real_t *bptr = params.stdv;
				for (ssi_size_t j = 0; j < params.n_features; j++) {
					*bptr = sqrt(*bptr);
					if (*bptr == 0.0f) {
						ssi_wrn("zero stdv '%u'", j);
						*bptr = 1.0f;
					}
					bptr++;
				}
				break;
			}
			}

			ModelTools::ReleaseSampleMatrix(_n_samples, classes, matrix);
		}
		params.inialized = true;
	}

	return true;
}

ssi_sample_t *ISNorm::get (ssi_size_t index) {

	ssi_sample_t *tmp = _samples.get (index);
	if (tmp) {
		_sample.class_id = tmp->class_id;
		_sample.time = tmp->time;
		_sample.user_id = tmp->user_id;
		_sample.score = tmp->score;
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			memcpy (_sample.streams[i]->ptr, tmp->streams[i]->ptr, _n_features[i] * sizeof (ssi_real_t));
			norm (*_sample.streams[i], i);	
		}
		return &_sample;
	}	
	return 0;	
}

ssi_sample_t *ISNorm::next () {

	ssi_sample_t *tmp = _samples.next ();
	if (tmp) {
		_sample.class_id = tmp->class_id;
		_sample.time = tmp->time;
		_sample.user_id = tmp->user_id;
		_sample.score = tmp->score;
		for (ssi_size_t i = 0; i < _n_streams; i++) {
			memcpy (_sample.streams[i]->ptr, tmp->streams[i]->ptr, _n_features[i] * sizeof (ssi_real_t));
			norm (*_sample.streams[i], i);	
		}
		return &_sample;
	}	
	return 0;
}

bool ISNorm::Norm(ssi_stream_t &stream, Params params) {

	if (stream.dim != params.n_features) {
		ssi_wrn("wrong stream dimension '%u' != '%u'", stream.dim, params.n_features);
		return false;
	}

	if (params.method == METHOD::NONE) {
		return true;
	}

	switch (params.method) {

	case METHOD::SCALE: {

		ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);
		ssi_real_t *aptr = params.mins;
		ssi_real_t *bptr = params.maxs;
		for (ssi_size_t j = 0; j < params.n_features; j++) {
			*ptr -= *aptr++;
			*ptr++ /= *bptr++;
		}
		if (params.limits[0] != 0.0f || params.limits[1] != 1.0f) {
			ptr = ssi_pcast(ssi_real_t, stream.ptr);
			ssi_real_t a = params.limits[1] - params.limits[0];
			ssi_real_t b = params.limits[0];
			for (ssi_size_t j = 0; j < params.n_features; j++) {
				*ptr *= a;
				*ptr++ += b;
			}
		}

		break;
	}

	case METHOD::ZSCORE: {

		ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);
		ssi_real_t *aptr = params.mean;
		ssi_real_t *bptr = params.stdv;
		for (ssi_size_t j = 0; j < params.n_features; j++) {
			*ptr -= *aptr++;
			*ptr++ /= *bptr++;
		}

		break;
	}

	}

	return true;
}

void ISNorm::norm (ssi_stream_t &stream, ssi_size_t stream_index) {

	if (_params[stream_index]) {
		Norm(stream, *_params[stream_index]);
	}
}

void ISNorm::norm (ssi_sample_t &sample) {

	for (ssi_size_t i = 0; i < sample.num; i++) {		
		norm (*sample.streams[i], i);
	}
}

void ISNorm::norm (ISamples &samples) {

	samples.reset ();
	ssi_sample_t *sample = 0;
	while (sample = samples.next ()) {
		norm (*sample);
	}
}




}
