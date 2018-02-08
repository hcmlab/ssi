// Annotation.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/04
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

#include "Annotation.h"
#include "SampleList.h"
#include "ioput/file/FileAnnotationOut.h"
#include "ioput/file/FileAnnotationIn.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#if __gnu_linux__
using std::min;
using std::max;
#endif

namespace ssi {

ssi_char_t *Annotation::ssi_log_name = "annotation";

bool Annotation::check_type(SSI_SCHEME_TYPE::List type)
{
	if (_scheme->type != type)
	{
		ssi_wrn("type mismatch '%s != %s'", SSI_SCHEME_NAMES[type], SSI_SCHEME_NAMES[_scheme->type]);
		return false;
	}
	return true;
}

bool Annotation::check_not_type(SSI_SCHEME_TYPE::List type)
{
	if (_scheme->type == type)
	{
		ssi_wrn("type not supported '%s'", SSI_SCHEME_NAMES[type]);
		return false;
	}
	return true;
}

Annotation::Annotation()
	: _scheme(0)
{
}

Annotation::Annotation(Annotation &self)
	: _scheme(0)
{
	setScheme(*self._scheme);
	reserve(self.size());
	for (Annotation::iterator it = self.begin(); it != self.end(); it++)
	{
		push_back(*it);
	}
	_meta = self._meta;
}

Annotation::~Annotation()
{
	release();
}

void Annotation::empty()
{
	if (_scheme)
	{
		if (_scheme->type == SSI_SCHEME_TYPE::FREE)
		{
			for (std::vector<ssi_label_t>::iterator it = begin(); it != end(); it++)
			{
				delete[] it->free.name;
			}
		}
	}
	clear();
}

void Annotation::release_scheme()
{
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
		delete _scheme; _scheme = 0;
	}
}

void Annotation::release()
{
	empty();
	release_scheme();
	_meta.clear();
}

ssi_size_t *Annotation::default_ids(ssi_size_t n)
{
	if (n > 0)
	{
		ssi_size_t *ids = new ssi_size_t[n];
		for (ssi_size_t i = 0; i < n; i++)
		{
			ids[i] = i;
		}
		return ids;
	}

	return 0;
}

void Annotation::copy_ids(ssi_size_t n, const ssi_size_t *from, ssi_size_t *to)
{
	memcpy(to, from, n * sizeof(ssi_size_t));
}

bool Annotation::setSchemeName(const ssi_char_t *name)
{
	if (!_scheme || !name)
	{
		return false;
	}

	delete[] _scheme->name;
	_scheme->name = ssi_strcpy(name);

	return true;
}

bool Annotation::setScheme(const ssi_scheme_t scheme)
{
	release();

	_scheme = new ssi_scheme_t;
	_scheme->name = ssi_strcpy(scheme.name);
	_scheme->type = scheme.type;

	if (scheme.type == SSI_SCHEME_TYPE::DISCRETE)
	{
		_scheme->discrete.n = scheme.discrete.n;
		_scheme->discrete.names = ssi_strcpy(scheme.discrete.n, scheme.discrete.names);
		_scheme->discrete.ids = new ssi_size_t[_scheme->discrete.n];
		copy_ids(_scheme->discrete.n, scheme.discrete.ids, _scheme->discrete.ids);
	}
	else if (scheme.type == SSI_SCHEME_TYPE::CONTINUOUS)
	{
		_scheme->continuous.min = scheme.continuous.min;
		_scheme->continuous.max = scheme.continuous.max;
		_scheme->continuous.sr = scheme.continuous.sr;
	}	

	return true;
}

bool Annotation::setFreeScheme(const ssi_char_t *scheme_name)
{
	release();

	_scheme = new ssi_scheme_t;
	_scheme->name = ssi_strcpy(scheme_name);
	_scheme->type = SSI_SCHEME_TYPE::FREE;

	return true;
}

bool Annotation::setDiscreteScheme(const ssi_char_t *scheme_name)
{
	release();

	_scheme = new ssi_scheme_t;
	_scheme->name = ssi_strcpy(scheme_name);
	_scheme->type = SSI_SCHEME_TYPE::DISCRETE;
	_scheme->discrete.n = 0;
	_scheme->discrete.names = 0;
	_scheme->discrete.ids = 0;

	return true;
}

bool Annotation::setDiscreteScheme(const ssi_char_t *scheme_name,
	StringList &class_names,
	const ssi_size_t *class_ids)
{
	release();

	if (class_names.size() == 0)
	{
		return false;
	}

	_scheme = new ssi_scheme_t;
	_scheme->name = ssi_strcpy(scheme_name);
	_scheme->type = SSI_SCHEME_TYPE::DISCRETE;
	_scheme->discrete.n = ssi_size_t(class_names.size());
	_scheme->discrete.names = 0;
	if (_scheme->discrete.n > 0)
	{
		_scheme->discrete.names = new ssi_char_t *[_scheme->discrete.n];
		for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
		{
			_scheme->discrete.names[i] = ssi_strcpy(class_names[i].str());
		}
		_scheme->discrete.ids = default_ids(_scheme->discrete.n);
		if (class_ids)
		{
			copy_ids(_scheme->discrete.n, class_ids, _scheme->discrete.ids);
		}
	}

	return true;
}

bool Annotation::setDiscreteScheme(const ssi_char_t *scheme_name,
	ssi_size_t n_classes,
	const ssi_char_t **class_names,
	const ssi_size_t *class_ids)
{
	release();

	if (n_classes == 0)
	{
		return false;
	}

	_scheme = new ssi_scheme_t;
	_scheme->name = ssi_strcpy(scheme_name);
	_scheme->type = SSI_SCHEME_TYPE::DISCRETE;
	_scheme->discrete.n = n_classes;
	_scheme->discrete.names = ssi_strcpy(n_classes, class_names);

	_scheme->discrete.ids = default_ids(_scheme->discrete.n);
	if (class_ids)
	{
		copy_ids(_scheme->discrete.n, class_ids, _scheme->discrete.ids);
	}

	return true;
}

bool Annotation::setDiscreteScheme(const ssi_char_t *scheme_name,
	std::map<String, ssi_size_t> &classes)
{
	release();
	
	if (classes.size() < 1)
	{
		return false;
	}

	_scheme = new ssi_scheme_t;
	_scheme->name = ssi_strcpy(scheme_name);
	_scheme->type = SSI_SCHEME_TYPE::DISCRETE;
	_scheme->discrete.n = (ssi_size_t)classes.size();
	_scheme->discrete.names = new ssi_char_t *[_scheme->discrete.n];
	_scheme->discrete.ids = default_ids(_scheme->discrete.n);
	ssi_size_t i = 0;
	for (std::map<String,ssi_size_t>::iterator it = classes.begin(); it != classes.end(); it++, i++)
	{
		_scheme->discrete.names[i] = ssi_strcpy(it->first.str());
		_scheme->discrete.ids[i] = it->second;
	}

	return true;
}

bool Annotation::setContinuousScheme(const ssi_char_t *scheme_name,
	ssi_time_t sr,
	ssi_real_t min_score,
	ssi_real_t max_score)
{
	release();

	_scheme = new ssi_scheme_t;
	_scheme->name = ssi_strcpy(scheme_name);
	_scheme->type = SSI_SCHEME_TYPE::CONTINUOUS;
	_scheme->continuous.sr = sr;
	_scheme->continuous.min = min_score;
	_scheme->continuous.max = max_score;

	return true;
}

bool Annotation::hasScheme()
{
	return _scheme != 0;
}

const ssi_scheme_t *Annotation::getScheme()
{
	return _scheme;
}

ssi_size_t Annotation::getSize()
{
	return (ssi_size_t) size();
}

ssi_label_t Annotation::getLabel(ssi_size_t index)
{
	return at(index);
}

bool Annotation::compare_label(const ssi_label_t lhs, const ssi_label_t rhs)
{
	if (lhs.discrete.from == rhs.discrete.from)
	{
		return lhs.discrete.to < rhs.discrete.to;
	}
	else
	{
		return lhs.discrete.from < rhs.discrete.from;
	}
}

void Annotation::sort()
{
	if (_scheme->type != SSI_SCHEME_TYPE::CONTINUOUS)
	{
		std::sort(begin(), end(), compare_label);
	}
}

bool Annotation::hasClassId(ssi_int_t class_id)
{
	if (class_id < 0)
	{
		return true;
	}

	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		if (class_id == _scheme->discrete.ids[i])
		{
			return true;
		}
	}

	return false;
}

bool Annotation::hasClassName(const ssi_char_t *name)
{
	if (!name)
	{
		return false;
	}

	if (ssi_strcmp(name, SSI_SAMPLE_GARBAGE_CLASS_NAME))
	{
		return true;
	}

	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		if (ssi_strcmp(name, _scheme->discrete.names[i]))
		{			
			return true;
		}
	}

	return false;
}

const ssi_char_t *Annotation::getClassName(ssi_int_t class_id)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return 0;
	}

	if (class_id < 0)
	{
		return SSI_SAMPLE_GARBAGE_CLASS_NAME;
	}

	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		if (class_id == (ssi_int_t) _scheme->discrete.ids[i])
		{
			return _scheme->discrete.names[i];
		}
	}

	ssi_wrn("invalid class index '%u'", class_id);
	return 0;
}

bool Annotation::getClassId(const ssi_char_t *name, ssi_int_t &class_id)
{
	class_id = SSI_SAMPLE_GARBAGE_CLASS_ID;

	if (!check_type(SSI_SCHEME_TYPE::DISCRETE))
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	if (ssi_strcmp(name, SSI_SAMPLE_GARBAGE_CLASS_NAME))
	{		
		return true;
	}

	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		if (ssi_strcmp(name, _scheme->discrete.names[i]))
		{
			class_id = _scheme->discrete.ids[i];
			return true;
		}
	}
	
	ssi_wrn("invalid class name '%s'", name);
	return false;
}

bool Annotation::getClassIndex(ssi_int_t class_id, ssi_size_t &index)
{
	if (!check_type(SSI_SCHEME_TYPE::DISCRETE))
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	if (class_id < 0)
	{
		return false;
	}

	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		if (class_id == _scheme->discrete.ids[i])
		{
			index = i;
			return true;
		}
	}

	ssi_wrn("invalid class index '%d'", class_id);
	return false;
}

bool Annotation::getClassIndex(const ssi_char_t *name, ssi_size_t &index)
{
	ssi_int_t class_id;
	if (!getClassId(name, class_id))
	{
		return false;
	}
	return getClassIndex(class_id, index);
}

ssi_size_t Annotation::getClassSize(ssi_int_t class_id)
{
	if (!check_type(SSI_SCHEME_TYPE::DISCRETE))
	{
		ssi_wrn("not a discrete scheme");
		return 0;
	}

	ssi_size_t count = 0;

	for (iterator it = begin(); it != end(); it++)
	{
		if (it->discrete.id == class_id)
		{
			++count;
		}
	}

	return 0;
}

void Annotation::setMeta(const ssi_char_t *key, const ssi_char_t *value)
{
	std::map<String,String>::iterator it;
	it = _meta.find(String(key));
	if (it == _meta.end())
	{
		_meta.insert(std::pair<String, String>(String(key),String(value)));
	}
	else
	{
		_meta[String(key)] = String(value);
	}
}

const ssi_char_t *Annotation::getMeta(const ssi_char_t *key)
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

ssi_size_t Annotation::getMetaSize()
{
	return (ssi_size_t)_meta.size();
}

const ssi_char_t *Annotation::getMetaKey(ssi_size_t index)
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

bool Annotation::add(Annotation &annotation)
{
	if (!_scheme)
	{
		return false;
	}

	bool result = true;
	for (Annotation::iterator it = annotation.begin(); it != annotation.end(); it++)
	{
		result &= add(*it);
	}

	return result;
}


bool Annotation::add(const ssi_label_t &label)
{
	if (!_scheme)
	{
		return false;
	}

	switch (_scheme->type)
	{
	case SSI_SCHEME_TYPE::DISCRETE:
	{
		return add(label.discrete.from, label.discrete.to, label.discrete.id, label.confidence);		
	}
	case SSI_SCHEME_TYPE::CONTINUOUS:
	{
		return add(label.continuous.score, label.confidence);		
	}
	case SSI_SCHEME_TYPE::FREE:
	{
		return add(label.free.from, label.free.to, label.free.name, label.confidence);		
	}
	}
	
	return false;
}

bool Annotation::add(ssi_time_t from, ssi_time_t to, ssi_int_t class_id, ssi_real_t conf)
{
	if (!check_type(SSI_SCHEME_TYPE::DISCRETE))
	{
		return false;
	}

	if (!hasClassId(class_id))
	{
		ssi_wrn("unknown class id '%u'", class_id);
		return false;
	}

	ssi_label_t s;
	s.confidence = conf;
	s.discrete.from = from;
	s.discrete.to = to;
	s.discrete.id = class_id;

	push_back(s);

	return true;
}

bool Annotation::add(ssi_time_t from, ssi_time_t to, const ssi_char_t *name, ssi_real_t conf)
{
	if (!name)
	{
		return false;
	}

	if (_scheme->type == SSI_SCHEME_TYPE::DISCRETE)
	{
		if (name[0] == '\0')
		{
			ssi_wrn("empty class name");
			return false;
		}

		ssi_int_t id;
		if (getClassId(name, id))
		{
			return add(from, to, id, conf);
		}
	} 
	else if (_scheme->type == SSI_SCHEME_TYPE::FREE)
	{
		ssi_label_t s;
		s.confidence = conf;
		s.free.to = to;
		s.free.from = from;
		s.free.name = ssi_strcpy(name);
		push_back(s);

		return true;
	}

	return false;
}

bool Annotation::add(const ssi_char_t *from, const ssi_char_t *to, const ssi_char_t *name, const ssi_char_t *conf)
{
	if (!name || name[0] == '\0')
	{
		return false;
	}

	return add(ssi_time_t(atof(from)), ssi_time_t(atof(to)), name, ssi_real_t(atof(conf)));
}

bool Annotation::add(const ssi_char_t *from, const ssi_char_t *to, const ssi_char_t *name, ssi_real_t conf)
{
	if (!name || name[0] == '\0')
	{
		return false;
	}

	return add(ssi_time_t(atof(from)), ssi_time_t(atof(to)), name, conf);
}

bool Annotation::add(ssi_real_t score, ssi_real_t conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (score < _scheme->continuous.min)
	{
		ssi_wrn("score below min '%g < %g'", score, _scheme->continuous.max);
	}

	if (score > _scheme->continuous.max)
	{
		ssi_wrn("score above max '%g > %g'", score, _scheme->continuous.max);
	}

	ssi_label_t s;
	s.confidence = conf;
	s.continuous.score = score;
	push_back(s);

	return true;
}

bool Annotation::add(const ssi_char_t *score, const ssi_char_t *conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	add(ssi_real_t(atof(score)), ssi_real_t(atof(conf)));

	return true;
}

bool Annotation::add(const ssi_char_t *score, ssi_real_t conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	add(ssi_real_t(atof(score)), conf);

	return true;
}

bool Annotation::addStream(ssi_stream_t scores, ssi_size_t score_dim, ssi_real_t conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (_scheme->continuous.sr != scores.sr)
	{
		ssi_wrn("sample rate mismatch '%g != %g'", scores.sr, _scheme->continuous.sr);
	}

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, scores.ptr);
	for (ssi_size_t i = 0; i < scores.num; i++)
	{
		add(*(ptr+ score_dim), conf);
		ptr += scores.dim;
	}

	return true;
}

bool Annotation::addStream(ssi_stream_t scores, ssi_size_t score_dim, ssi_size_t conf_dim)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (_scheme->continuous.sr != scores.sr)
	{
		ssi_wrn("sample rate mismatch '%g != %g'", scores.sr, _scheme->continuous.sr);
	}

	ssi_real_t *ptr = ssi_pcast(ssi_real_t, scores.ptr);
	for (ssi_size_t i = 0; i < scores.num; i++)
	{
		add(*(ptr + score_dim), *(ptr + conf_dim));
		ptr += scores.dim;
	}

	return true;
}

bool Annotation::addStream(ssi_stream_t stream, ssi_size_t dim, ssi_real_t thres, ssi_int_t class_id, ssi_real_t conf, ssi_time_t min_duration)
{
	if (!check_type(SSI_SCHEME_TYPE::DISCRETE))
	{
		return false;
	}

	if (!hasClassId(class_id))
	{
		return false;
	}

	if (stream.type != SSI_FLOAT)
	{
		return false;
	}

	if (dim >= stream.dim)
	{
		return false;
	}

	ssi_time_t dt = 1.0 / stream.sr;
	ssi_time_t time = 0;
	
	ssi_time_t from = 0.0, to = 0.0;
	ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr) + dim;
	ssi_real_t x = 0;
	bool activity = *ptr > thres;

	for (ssi_size_t i = 0; i < stream.num; i++)
	{
		x = *ptr;
		if (activity && x <= thres)
		{
			to = time;
			activity = false;
			if (to - from >= min_duration)
			{
				add(from, to, class_id, conf);
			}
		}
		else if (!activity && x > thres)
		{
			from = time;
			activity = true;
		}

		time += dt;
		ptr += stream.dim;
	}

	if (activity)
	{
		to = time;
		if (to - from >= min_duration)
		{
			add(from, to, class_id, conf);
		}
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, ssi_size_t column_from, ssi_size_t column_to, ssi_size_t column_class, ssi_size_t column_conf)
{
	if (!check_not_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

    ssi_size_t column_max = max(column_from, max(column_to, max(column_class, column_conf)));
	if (column_max >= file.getColumnSize())
	{
		ssi_wrn("invalid column index '%u >= %u'", column_max, file.getColumnSize());
		return false;
	}

	std::vector<ssi_char_t*>::iterator from = file[column_from].begin();
	std::vector<ssi_char_t*>::iterator to = file[column_to].begin();
	std::vector<ssi_char_t*>::iterator id = file[column_class].begin();
	std::vector<ssi_char_t*>::iterator conf = file[column_conf].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*from++, *to++, *id++, *conf++);
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, ssi_size_t column_from, ssi_size_t column_to, ssi_size_t column_class, ssi_real_t conf)
{
	if (!check_not_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{		
		return false;
	}

    ssi_size_t column_max = max(column_from, max(column_to, column_class));
	if (column_max >= file.getColumnSize())
	{
		ssi_wrn("invalid column index '%u >= %u'", column_max, file.getColumnSize());
		return false;
	}

	std::vector<ssi_char_t*>::iterator from = file[column_from].begin();
	std::vector<ssi_char_t*>::iterator to = file[column_to].begin();
	std::vector<ssi_char_t*>::iterator id = file[column_class].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*from++, *to++, *id++, conf);
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, const ssi_char_t *column_from, const ssi_char_t *column_to, const ssi_char_t *column_class, const ssi_char_t *column_conf)
{
	if (!check_not_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (!file.hasHeader())
	{
		ssi_wrn("csv header is missing");
		return false;
	}

	std::vector<ssi_char_t*>::iterator from = file[column_from].begin();
	std::vector<ssi_char_t*>::iterator to = file[column_to].begin();
	std::vector<ssi_char_t*>::iterator id = file[column_class].begin();
	std::vector<ssi_char_t*>::iterator conf = file[column_conf].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*from++, *to++, *id++, *conf++);
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, const ssi_char_t *column_from, const ssi_char_t *column_to, const ssi_char_t *column_class, ssi_real_t conf)
{
	if (!check_not_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (!file.hasHeader())
	{
		ssi_wrn("csv header is missing");
		return false;
	}

	std::vector<ssi_char_t*>::iterator from = file[column_from].begin();
	std::vector<ssi_char_t*>::iterator to = file[column_to].begin();
	std::vector<ssi_char_t*>::iterator id = file[column_class].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*from++, *to++, *id++, conf);
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, ssi_size_t column_score, ssi_size_t column_conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

    ssi_size_t column_max = max(column_score, column_conf);
	if (column_max >= file.getColumnSize())
	{
		ssi_wrn("invalid column index '%u >= %u'", column_max, file.getColumnSize());
		return false;
	}

	std::vector<ssi_char_t*>::iterator score = file[column_score].begin();
	std::vector<ssi_char_t*>::iterator conf = file[column_conf].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*score++, *conf++);
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, ssi_size_t column_score, ssi_real_t conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (column_score >= file.getColumnSize())
	{
		ssi_wrn("invalid column index '%u >= %u'", column_score, file.getColumnSize());
		return false;
	}

	std::vector<ssi_char_t*>::iterator score = file[column_score].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*score++, conf);
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, const ssi_char_t *column_score, const ssi_char_t *column_conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (!file.hasHeader())
	{
		ssi_wrn("csv header is missing");
		return false;
	}

	std::vector<ssi_char_t*>::iterator score = file[column_score].begin();
	std::vector<ssi_char_t*>::iterator conf = file[column_conf].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*score++, *conf++);
	}

	return true;
}

bool Annotation::addCSV(FileCSV &file, const ssi_char_t *column_score, ssi_real_t conf)
{
	if (!check_type(SSI_SCHEME_TYPE::CONTINUOUS))
	{
		return false;
	}

	if (!file.hasHeader())
	{
		ssi_wrn("csv header is missing");
		return false;
	}

	std::vector<ssi_char_t*>::iterator score = file[column_score].begin();

	for (ssi_size_t i = 0; i < file.getRowSize(); i++)
	{
		add(*score++, conf);
	}

	return true;
}

void Annotation::print(FILE *file, ssi_size_t n_max)
{
	if (!_scheme)
	{
		return;
	}

	ssi_fprint(file, "-------------------------------------------------------\n");
	ssi_fprint(file, "%10s%10s%10s%15s%10s\n", "from(s)", "to(s)", "dur(s)", "label", "conf");
	ssi_fprint(file, "-------------------------------------------------------\n");
	
	switch(_scheme->type)
	{
	case SSI_SCHEME_TYPE::DISCRETE:
	{
		std::vector<ssi_label_t>::iterator iter;		
		ssi_size_t i = 0;
		for (iter = begin(); iter != end(); iter++)
		{
			ssi_fprint(file, "%10g%10g%10g%15s%10g\n", iter->discrete.from, iter->discrete.to, iter->discrete.to - iter->discrete.from, getClassName(iter->discrete.id), iter->confidence);
			if (n_max > 0 && ++i == n_max)
			{
				break;
			}
		}
		break;
	}
	case SSI_SCHEME_TYPE::CONTINUOUS:
	{
		ssi_time_t delta = 1.0 / _scheme->continuous.sr;
		ssi_time_t from = 0;
		ssi_time_t to = from + delta;		
		std::vector<ssi_label_t>::iterator iter;
		ssi_size_t i = 0;
		for (iter = begin(); iter != end(); iter++)
		{
			ssi_fprint(file, "%10g%10g%10g%15g%10g\n", from, to, delta, iter->continuous.score, iter->confidence);
			from = to;
			to += delta;
			if (n_max > 0 && ++i == n_max)
			{
				break;
			}
		}
		break;
	}	
	case SSI_SCHEME_TYPE::FREE:
	{
		std::vector<ssi_label_t>::iterator iter;
		ssi_size_t i = 0;
		for (iter = begin(); iter != end(); iter++)
		{
			ssi_fprint(file, "%10g%10g%10g%15s%10g\n", iter->free.from, iter->free.to, iter->free.to - iter->free.from, iter->free.name, iter->confidence);
			if (n_max > 0 && ++i == n_max)
			{
				break;
			}
		}
		break;
	}
	}

	ssi_fprint(file, "-------------------------------------------------------\n");
}

bool Annotation::save(const ssi_char_t *path, File::TYPE type)
{
	if (!_scheme)
	{
		return false;
	}

	FileAnnotationOut out;
	if (!out.open(path, *_scheme, type))
	{
		return false;
	}

	if (!out.write(*this))
	{
		return false;
	}

	for (std::map<String, String>::iterator it = _meta.begin(); it != _meta.end(); it++)
	{
		out.writeMeta(it->first.str(), it->second.str());
	}

	if (!out.close())
	{
		return false;
	}

	return true;
}

bool Annotation::loadScheme(const ssi_char_t *path)
{
	release();

	FileAnnotationIn in;
	if (!in.open(path))
	{
		return false;
	}

	if (!setScheme(*in.getScheme()))
	{
		return false;
	}

	return true;
}

bool Annotation::load(const ssi_char_t *path)
{
	release();

	FileAnnotationIn in;
	if (!in.open(path))
	{
		return false;
	}

	if (!setScheme(*in.getScheme()))
	{
		return false;
	}

	const ssi_label_t *label;
	while (label = in.next())
	{
		add(*label);
	}

	ssi_size_t n_meta = in.getMetaSize();
	for (ssi_size_t i = 0; i < n_meta; i++)
	{
		const ssi_char_t *key = in.getMetaKey(i);
		setMeta(key, in.getMeta(key));
	}

	if (!in.close())
	{
		return false;
	}

	return true;
}

void Annotation::normConfidence()
{
	ssi_real_t min_val = FLT_MAX;
	ssi_real_t max_val = -FLT_MAX;
	for (Annotation::iterator it = begin(); it != end(); it++)
	{
		if (it->confidence < min_val)
		{
			min_val = it->confidence;
		}
		if (it->confidence > max_val)
		{
			max_val = it->confidence;
		}
	}
	ssi_real_t offset = min_val;
	ssi_real_t divisor = max_val - min_val;
	if (divisor != 0)
	{
		for (Annotation::iterator it = begin(); it != end(); it++)
		{
			it->confidence = (it->confidence - offset) / divisor;
		}
	}	
}

void Annotation::setConfidence(ssi_real_t confidence)
{
	for (Annotation::iterator it = begin(); it != end(); it++)
	{
		it->confidence = confidence;
	}
}

bool Annotation::removeClass(const ssi_char_t *name)
{	
	StringList names;	
	names.add(name);
	return removeClass(names);
}

bool Annotation::removeClass(StringList names)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	bool remove_garbage = false;
	std::map<String, ssi_size_t> classes;
	ssi_char_t *scheme_name = ssi_strcpy(_scheme->name);
	for (std::vector<String>::iterator it = names.begin(); it != names.end(); it++)
	{
		if (ssi_strcmp(it->str(), SSI_SAMPLE_GARBAGE_CLASS_NAME))
		{
			remove_garbage = true;
			break;
		}
	}
	
	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		bool remove_class = false;
		for (std::vector<String>::iterator it = names.begin(); it != names.end(); it++)
		{
			if (ssi_strcmp(_scheme->discrete.names[i], it->str()))
			{			
				remove_class = true;
				break;
			}
		}
		if (!remove_class)
		{
			classes[_scheme->discrete.names[i]] = _scheme->discrete.ids[i];
		}
	}

	Annotation clone(*this);

	release();
	setDiscreteScheme(scheme_name, classes);

	for (Annotation::iterator it = clone.begin(); it != clone.end(); it++)
	{
		if (it->discrete.id < 0)
		{
			if (!remove_garbage)
			{
				//add(*it); safer
				push_back(*it); // faster
			}
		}
		else if (hasClassId(it->discrete.id))
		{
			//add(*it); safer
			push_back(*it); // faster
		}
	}

	delete[] scheme_name;

	return true;
}

bool Annotation::keepClass(const ssi_char_t *name)
{
	StringList names;
	names.add(name);
	return keepClass(names);
}

bool Annotation::keepClass(StringList names)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	bool keep_garbage = false;
	std::map<String, ssi_size_t> classes;
	ssi_char_t *scheme_name = ssi_strcpy(_scheme->name);
	for (std::vector<String>::iterator it = names.begin(); it != names.end(); it++)
	{
		if (ssi_strcmp(it->str(), SSI_SAMPLE_GARBAGE_CLASS_NAME))
		{
			keep_garbage = true;
		}
		else
		{
			for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
			{
				if (ssi_strcmp(_scheme->discrete.names[i], it->str()))
				{
					classes[*it] = _scheme->discrete.ids[i];
				}
			}
		}
	}

	Annotation clone(*this);

	release();
	setDiscreteScheme(scheme_name, classes);

	for (Annotation::iterator it = clone.begin(); it != clone.end(); it++)
	{
		if (it->discrete.id < 0)			
		{
			if (keep_garbage)
			{
				//add(*it); safer
				push_back(*it); // faster
			}
		}
		else if (hasClassId(it->discrete.id))
		{
			//add(*it); safer
			push_back(*it); // faster
		}
	}

	delete[] scheme_name;

	return true;
}

bool Annotation::renameClass(const ssi_char_t *from, const ssi_char_t *to)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	if (hasClassName(to))
	{
		ssi_wrn("class name already exists '%s'", to);
		return false;
	}

	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		if (ssi_strcmp(_scheme->discrete.names[i], from))
		{
			delete[] _scheme->discrete.names[i];
			_scheme->discrete.names[i] = ssi_strcpy(to);
			return true;
		}
	}

	ssi_wrn("invalid class name '%s'", from);

	return false;
}

bool Annotation::mapClass(const ssi_char_t *from, const ssi_char_t *to, bool keep)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	ssi_int_t from_id;
	if (!getClassId(from, from_id))
	{
		return false;
	}

	ssi_int_t to_id;
	if (!getClassId(to, to_id))
	{
		return false;
	}

	for (iterator it = begin(); it != end(); it++)
	{
		if (it->discrete.id == from_id)
		{
			it->discrete.id = to_id;
		}
	}

	if (!keep)
	{
		removeClass(from);
	}

	return true;
}

bool Annotation::extractStream(const ssi_stream_t &from, ssi_stream_t &to)
{	
	ssi_stream_init(to, from.num, from.dim, from.byte, from.type, from.sr);
	to.num = 0;
	to.tot = 0;
	ssi_byte_t *src = from.ptr;
	ssi_byte_t *dst = to.ptr;
	ssi_size_t n_bytes = from.byte * from.dim;

	for (Annotation::iterator it = begin(); it != end(); it++)
	{
        ssi_size_t start = ssi_cast(ssi_size_t, it->discrete.from * from.sr + 0.5);
        ssi_size_t stop = max(start, ssi_cast(ssi_size_t, it->discrete.to * from.sr + 0.5));

		if (stop >= from.num)
		{
			ssi_wrn("label exceeds stream '%lg-%lg'", it->discrete.from, it->discrete.to);
			continue;
		}

		ssi_size_t len = stop - start + 1;

		if (to.num + len >= from.num)
		{
			ssi_wrn("copied sample data exceeds input stream (do label overlap?)");
			return false;
		}

		ssi_size_t n_copy = len * n_bytes;
		memcpy(dst, src + start * n_bytes, n_copy);
		dst += n_copy;
		to.num += len;
		to.tot += n_copy;
	}

	return true;
}

bool Annotation::convertToStream(ssi_stream_t &stream,
	ssi_time_t sr,
	ssi_time_t duration_s)
{
	if (!_scheme || (_scheme->type != SSI_SCHEME_TYPE::DISCRETE && _scheme->type != SSI_SCHEME_TYPE::CONTINUOUS))
	{
		ssi_wrn("not a discrete or continuous scheme");
		return false;
	}	
	
	if (_scheme->type == SSI_SCHEME_TYPE::CONTINUOUS)
	{
		ssi_stream_init(stream, (ssi_size_t)size(), 1, sizeof(ssi_real_t), SSI_REAL, _scheme->continuous.sr);
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr); 
		for (iterator it = begin(); it != end(); it++)
		{
			*ptr++ = it->continuous.score;
		}
	}
	else 
	{
		if (sr <= 0.0)
		{
			ssi_wrn("invalid sample rate '%lf'", sr);
			return false;
		}

		if (duration_s <= 0.0)
		{
			duration_s = this->back().discrete.to;
		}

		Annotation anno(*this);
		anno.convertToFrames(1 / sr, SSI_SAMPLE_REST_CLASS_NAME, duration_s);
		ssi_int_t rest_id = 0;
		anno.getClassId(SSI_SAMPLE_REST_CLASS_NAME, rest_id);

		ssi_stream_init(stream, (ssi_size_t)anno.size(), 1, sizeof(ssi_real_t), SSI_REAL, sr);
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);
		for (iterator it = anno.begin(); it != anno.end(); it++)
		{
			*ptr++ = (ssi_real_t)(it->discrete.id == rest_id ? 0 : (ssi_real_t)it->discrete.id + 1);
		}
	}

	return true;
}

bool Annotation::addClass(const ssi_char_t *name)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	StringList names;
	names.add(name);
	return addClass(names);
}

bool Annotation::addClass(const ssi_char_t *name, ssi_int_t &id)
{
	if (addClass(name))
	{
		getClassId(name, id);
		return true;
	}
	return false;
}

bool Annotation::addClass(ssi_int_t class_id, const ssi_char_t *name)
{
	if (hasClassId(class_id))
	{
		ssi_int_t id;
		getClassId(name, id);
		if (id != class_id)
		{
			ssi_wrn("already has a class with id '%d'", class_id);
			return false;
		}
		return true;
	}

	if (addClass(name))
	{
		ssi_size_t index;
		getClassIndex(name, index);
		_scheme->discrete.ids[index] = class_id;

		return true;
	}

	return false;
}

bool Annotation::addClass(StringList names)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	std::map<String, ssi_size_t> new_classes;
	ssi_size_t max_id = 0;
	for (ssi_size_t i = 0; i < _scheme->discrete.n; i++)
	{
		new_classes[_scheme->discrete.names[i]] = _scheme->discrete.ids[i];
		if (_scheme->discrete.ids[i] >= max_id)
		{
			max_id = _scheme->discrete.ids[i];
		}
	}
	for (StringList::iterator it = names.begin(); it != names.end(); it++)
	{
		if (!hasClassName(it->str()) && !ssi_strcmp(it->str(), SSI_SAMPLE_GARBAGE_CLASS_NAME))
		{			
			new_classes[it->str()] = ++max_id;
		}
	}

	ssi_char_t *scheme_name = ssi_strcpy(_scheme->name);

	release_scheme();

	_scheme = new ssi_scheme_t;
	_scheme->name = scheme_name;
	_scheme->type = SSI_SCHEME_TYPE::DISCRETE;
	_scheme->discrete.n = (ssi_size_t)new_classes.size();
	_scheme->discrete.names = new ssi_char_t *[_scheme->discrete.n];
	_scheme->discrete.ids = default_ids(_scheme->discrete.n);
	ssi_size_t i = 0;
	for (std::map<String, ssi_size_t>::iterator it = new_classes.begin(); it != new_classes.end(); it++, i++)
	{
		_scheme->discrete.names[i] = ssi_strcpy(it->first.str());
		_scheme->discrete.ids[i] = it->second;
	}

	return true;
}

ssi_size_t Annotation::max_class_id()
{
	ssi_size_t max_id = _scheme->discrete.ids[0];
	for (ssi_size_t i = 1; i < _scheme->discrete.n; i++)
	{
		if (_scheme->discrete.ids[i] >= max_id)
		{
			max_id = _scheme->discrete.ids[i];
		}
	}
	return max_id;
}

bool Annotation::addOffset(ssi_time_t offset_left, ssi_time_t offset_right)
{
	if (!_scheme || _scheme->type == SSI_SCHEME_TYPE::CONTINUOUS)
	{
		ssi_wrn("not a discrete or free scheme");
		return false;
	}

	for (iterator it = begin(); it != end(); it++)
	{
		it->discrete.from += offset_left;
		it->discrete.to += offset_right;
	}

	return true;
}

bool Annotation::convertToFrames(ssi_time_t frame_s, 	
	const ssi_char_t *emptyClassName,
	ssi_time_t duration,
	ssi_real_t empty_percent)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	bool add_empty = emptyClassName != 0;	
	ssi_int_t empty_id = SSI_SAMPLE_GARBAGE_CLASS_ID;
	if (add_empty && !ssi_strcmp(emptyClassName, SSI_SAMPLE_GARBAGE_CLASS_NAME))
	{
		addClass(emptyClassName);
		getClassId(emptyClassName, empty_id);
	}	
	
	if (duration <= 0 && size() > 0)
	{
		duration = (end()-1)->discrete.to;		
	}

	if (duration <= 0)
	{
		ssi_wrn("new annotation has zero length");
		return false;
	}

	ssi_size_t n_frames = ssi_cast(ssi_size_t, duration / frame_s);	

	ssi_time_t frame_dur = frame_s;
	ssi_time_t frame_from = 0;
	ssi_time_t frame_to = frame_dur;
	
	ssi_size_t n_classes = _scheme->discrete.n;
	ssi_label_t new_label;
	ssi_time_t *percent_class = new ssi_time_t[n_classes];
	ssi_time_t percent_garbage;

	// create a map from class ids to indices	
	ssi_size_t n_id_map = max_class_id();
	ssi_size_t *id_map = new ssi_size_t[n_id_map+1];
	for (ssi_size_t i = 0; i < n_classes; i++)
	{
		id_map[_scheme->discrete.ids[i]] = i;
	}

	// copy labels and clear annotation
	Annotation clone(*this);
	clone.sort(); 	
	clear();
	reserve(n_frames);
	iterator label = clone.begin();
	iterator last_label = label;

	for (ssi_size_t i = 0; i < n_frames; i++) {		

		new_label.discrete.from = frame_from;
		new_label.discrete.to = frame_from + frame_dur;
		new_label.discrete.id = empty_id;
		new_label.confidence = 1.0f;
		
		for (ssi_size_t i = 0; i < n_classes; i++)
		{
			percent_class[i] = 0;
		}
		percent_garbage = 0;

		// skip labels before the current frame
		label = last_label;
		while (label != clone.end() && label->discrete.to < frame_from)
		{ 
			label++;
			last_label++;
		}

		if (label != clone.end())
		{
			bool found_at_least_one = false;

			// find all classes within the current frame
			while (label != clone.end() && label->discrete.from < frame_to)
            {
                ssi_time_t dur = (min(frame_to, label->discrete.to) - max(frame_from, label->discrete.from)) / frame_dur;
				if (label->discrete.id < 0)
				{
					percent_garbage += dur;
				}
				else
				{
					percent_class[id_map[label->discrete.id]] += dur;
				}
				label++;
				found_at_least_one = true;
			}
			
			if (found_at_least_one) 
			{

				// find dominant class
				ssi_time_t max_percent = percent_garbage;
				ssi_int_t max_label_id = SSI_SAMPLE_GARBAGE_CLASS_ID;
				ssi_time_t percent_sum = percent_garbage;
				for (ssi_size_t i = 0; i < n_classes; i++) 
				{
					if (max_percent < percent_class[i]) 
					{
						max_label_id = i;
						max_percent = percent_class[i];
					}
					percent_sum += percent_class[i];
				}

				// add label
				if (percent_sum > empty_percent)
				{					
					new_label.discrete.id = max_label_id >= 0 ? _scheme->discrete.ids[max_label_id] : SSI_SAMPLE_GARBAGE_CLASS_ID;
					add(new_label);
				}
				else if (add_empty) 
				{
					add(new_label);
				}
			}
			else if (add_empty)
			{
				add(new_label);
			}
		}
		else if (add_empty) {
			add(new_label);
		}

		frame_from += frame_s;
		frame_to += frame_s;
	}

	delete[] id_map;
	delete[] percent_class;

	return true;
}

bool Annotation::extractSamples(const ssi_stream_t &stream, 
	SampleList *samples, 
	const ssi_char_t *user)
{
	if (!_scheme || (_scheme->type != SSI_SCHEME_TYPE::CONTINUOUS && _scheme->type != SSI_SCHEME_TYPE::DISCRETE))
	{
		ssi_wrn("not a continous or discrete scheme");
		return false;
	}

	if (_scheme->type == SSI_SCHEME_TYPE::CONTINUOUS)
	{
		return extractSamplesFromContinuousScheme(stream, samples, 0, 0, user);
	}
	else
	{
		return extractSamplesFromDiscreteScheme(1u, &stream, samples, user);
	}
}

bool Annotation::extractSamplesFromContinuousScheme(const ssi_stream_t &stream,
	SampleList *samples,
	ssi_size_t context_left,
	ssi_size_t context_right,
	const ssi_char_t *user)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::CONTINUOUS)
	{
		ssi_wrn("not a continuous scheme");
		return false;
	}

	if (stream.sr != _scheme->continuous.sr)
	{
		ssi_wrn("sample rates do not fit '%g != %g'", stream.sr, _scheme->continuous.sr);
	}

    ssi_size_t n_samples = ssi_size_t(min((int)stream.num, (int)size()));
	if (n_samples <= context_left + context_right)
	{
		ssi_wrn("stream or annotation too short '%u'", n_samples);
		return false;
	}	
	n_samples -= context_left + context_right;

	ssi_time_t delta = 1 / _scheme->continuous.sr;

	ssi_sample_t *sample = new ssi_sample_t;
	ssi_sample_create(*sample,
		1,
		samples->addUserName(user),
		samples->addClassName(_scheme->name),
		0,
		0);

	ssi_stream_t chunk;
	ssi_size_t dim = stream.dim;
	ssi_size_t num = 1 + context_left + context_right;
	ssi_stream_init(chunk, 0, stream.dim, stream.byte, stream.type, stream.sr);
	chunk.ptr = stream.ptr;
	chunk.num = chunk.num_real = num;
	chunk.tot = chunk.tot_real = stream.dim * stream.byte * num;
	sample->streams[0] = &chunk;
	sample->time = delta * context_left;

	iterator it = begin();
	it += context_left;
	for (ssi_size_t i = 0; i < n_samples; i++)
	{
        if (!std::isnan(it->continuous.score))
		{
			sample->score = it->continuous.score;
			samples->addSample(sample, true);
		}

		sample->time += delta;
		chunk.ptr += stream.dim * stream.byte;
		it++;
	}

	sample->streams[0] = 0;
	ssi_sample_destroy(*sample);
	delete sample;

	return true;
}

bool Annotation::extractSamplesFromDiscreteScheme(ssi_size_t num,
	const ssi_stream_t *streams,
	SampleList *samples,
	const ssi_char_t *user)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	ssi_size_t n_classes = _scheme->discrete.n;
	ssi_size_t user_id = samples->addUserName(user);

	// create a map from class ids to indices	
	ssi_size_t n_id_map = max_class_id();
	ssi_size_t *id_map = new ssi_size_t[n_id_map + 1];
	for (ssi_size_t i = 0; i < n_classes; i++)
	{
		id_map[_scheme->discrete.ids[i]] = samples->addClassName(_scheme->discrete.names[i]);
	}

	// iterate through sample list
	for (iterator it = begin(); it != end(); it++)
	{
		ssi_sample_t *sample = new ssi_sample_t;
		ssi_stream_t **chops = new ssi_stream_t *[num];

		bool success = false;
		for (ssi_size_t j = 0; j < num; j++) 
		{
			// check if samples start index is smaller than the stop index and smaller than streams number of samples
			if (it->discrete.from < 0 || it->discrete.to <  it->discrete.from) {
				ssi_wrn("interval out of range [%lf..%lf]s", it->discrete.from, it->discrete.to);
				continue;
			}

			// calculate start and stop index
			ssi_size_t from = ssi_cast(ssi_size_t, it->discrete.from * streams[j].sr + 0.5);
			ssi_size_t to = ssi_cast(ssi_size_t, it->discrete.to * streams[j].sr + 0.5);

			// check if samples start index is smaller than the stop index and smaller than streams number of samples
			if (!(from <= to && to < streams[j].num)) {
				ssi_wrn("interval out of range [%lf..%lf]s", it->discrete.from, it->discrete.to);
				continue;
			}

			// extract sample
			chops[j] = new ssi_stream_t;
			ssi_stream_copy(streams[j], *chops[j], from, to);
			success = true;
		}

		if (success) 
		{
			// create and add new sample
			sample->class_id = it->discrete.id < 0 ? SSI_SAMPLE_GARBAGE_CLASS_ID : id_map[it->discrete.id];
			sample->num = num;
			sample->score = 0.0f;
			sample->streams = chops;
			sample->time = it->discrete.from;
			sample->user_id = user_id;
			samples->addSample(sample);
		}
		else 
		{
			delete sample;
			delete[] chops;
		}
	}

	delete[] id_map;

	return true;
}

bool Annotation::filter(double threshold, FILTER_PROPERTY::List prop, FILTER_OPERATOR::List op, const ssi_char_t *name)
{
	if (!_scheme || _scheme->type == SSI_SCHEME_TYPE::CONTINUOUS)
	{
		ssi_wrn("not a free or discrete scheme");
		return false;
	}

	if (size() == 0)
	{
		return true;
	}

	bool hot_class = !name || !hasClassName(name) ? false : true;
	ssi_int_t hot_class_id = SSI_SAMPLE_GARBAGE_CLASS_ID;
	if (hot_class)
	{
		getClassId(name, hot_class_id);
	}

	Annotation clone(*this);
	clear();

	for (iterator it = clone.begin(); it != clone.end(); it++)
	{
		bool keep = false;
		double value = 0;
		switch (prop)
		{
		case FILTER_PROPERTY::CONFIDENCE:
			value = (double) it->confidence;
			break;
		case FILTER_PROPERTY::DURATION:
			value = (double) _scheme->type == SSI_SCHEME_TYPE::DISCRETE ? it->discrete.to - it->discrete.from : it->free.to - it->free.from;
			break;
		case FILTER_PROPERTY::FROM:
			value = (double)_scheme->type == SSI_SCHEME_TYPE::DISCRETE ? it->discrete.from : it->free.from;
			break;
		case FILTER_PROPERTY::TO:
			value = (double)_scheme->type == SSI_SCHEME_TYPE::DISCRETE ? it->discrete.to : it->free.to;
			break;
		}
		
		switch (op)
		{
		case FILTER_OPERATOR::GREATER:
			keep = value > threshold;
			break;
		case FILTER_OPERATOR::GREATER_EQUAL:
			keep = value >= threshold;
			break;
		case FILTER_OPERATOR::LESSER:
			keep = value < threshold;
			break;
		case FILTER_OPERATOR::LESSER_EQUAL:
			keep = value <= threshold;
			break;
		case FILTER_OPERATOR::EQUAL:
			keep = value == threshold;
			break;
		case FILTER_OPERATOR::NOT_EQUAL:
			keep = value != threshold;
			break;
		}
		if (keep)
		{
			push_back(*it);
		}
	}

	return true;
}

bool Annotation::packClass(ssi_time_t max_time_gap, const ssi_char_t *name)
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	if (size() == 0)
	{
		return true;
	}

	bool hot_class = !name || !hasClassName(name) ? false : true;
	ssi_int_t hot_class_id = SSI_SAMPLE_GARBAGE_CLASS_ID;
	if (hot_class)
	{
		getClassId(name, hot_class_id);
	}

	Annotation clone(*this);
	clone.sort();
	clear();
	
	ssi_label_t label = *(clone.begin());
	ssi_real_t sum = label.confidence;
	ssi_size_t count = 1;
	for (iterator it = clone.begin()+1; it != clone.end(); it++)
	{
		if (label.discrete.id == it->discrete.id 
			&& it->discrete.from - label.discrete.to <= max_time_gap
			&& (!hot_class || hot_class_id == it->discrete.id))
		{
			count++;
			sum += it->confidence;
			label.discrete.to = it->discrete.to;
		}
		else
		{
			label.discrete.to = (it - 1)->discrete.to;
			label.confidence = sum / count;
			push_back(label);

			label = *it;
			sum = it->confidence;
			count = 1;
		}		
	}

	label.confidence = sum / count;
	push_back(label);	

	return true;
}

bool Annotation::removeOverlap()
{
	if (!_scheme || _scheme->type != SSI_SCHEME_TYPE::DISCRETE)
	{
		ssi_wrn("not a discrete scheme");
		return false;
	}

	if (size() == 0)
	{
		return true;
	}

	for (iterator it = begin(); it != end()-1; it++)
    {
        it->discrete.to = min(it->discrete.to, (it + 1)->discrete.from);
	}

	return true;
}





















// old implementation begins here (deprecated)

old::Annotation::Annotation () {
}

old::Annotation::~Annotation () {
	clear ();
}

void old::Annotation::clear () {

	for (ssi_size_t i = 0; i < labelSize (); i++) {
		delete[] labels[i];
	}
	labels.clear ();

	reset ();
	old::Annotation::Entry *entry;
	while (entry = next ()) {
		delete entry;
	}
	entries.clear ();
	reset ();
}

void old::Annotation::add (ssi_time_t start,
	ssi_time_t stop,
	const ssi_char_t *label) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "add new entry: %f %f %s\n", start, stop, label);

	// add label
	ssi_size_t label_index = addLabel (label);

	// add as new entry
	old::Annotation::Entry *entry = new old::Annotation::Entry;
	entry->start = start;
	entry->stop = stop;
	entry->label_index = label_index;
	std::pair<entries_set_t::iterator,bool> ret;
	ret = entries.insert (entry);
	if (ret.second == false) {
		delete entry;
	}
}

void old::Annotation::add (old::Annotation::Entry &e) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "add new entry: %f %f %s\n", e.start, e.stop, Annotation::getLabel (e.label_index));

	// add as new entry
	old::Annotation::Entry *entry = new old::Annotation::Entry;
	entry->start = e.start;
	entry->stop = e.stop;
	entry->label_index = e.label_index;
	std::pair<entries_set_t::iterator,bool> ret;
	ret = entries.insert (entry);
	if (ret.second == false) {
		delete entry;
	}
}

void old::Annotation::reset () {
	entries_iter = entries.begin ();
}

old::Annotation::Entry *old::Annotation::next () {

	if (entries_iter == entries.end ()) {
		return 0;
	}
	
	return *entries_iter++;
}

old::Annotation::Entry *old::Annotation::next (ssi_size_t label_index) {

	while (entries_iter != entries.end () && (*entries_iter)->label_index != label_index)
		entries_iter++;

	if (entries_iter == entries.end ()) {
		return 0;
	}
	
	return *entries_iter++;
}

const ssi_char_t *old::Annotation::getLabel (ssi_size_t index) {

	if (index >= labelSize ()) {
		ssi_wrn ("index (%u) out of boundary (%u)", index, labelSize ());
		return 0;
	}

	return labels[index];
}

ssi_size_t old::Annotation::size () {
	return ssi_cast (ssi_size_t , entries.size ());
}

ssi_size_t old::Annotation::addLabel (const ssi_char_t *label) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "add new label: %s\n", label);

	// check if label exists
	ssi_size_t label_index = 0;
	bool found = false;
	for (ssi_size_t i = 0; i < labels.size (); i++) {
		if (strcmp (label, labels[i]) == 0) {
			label_index = i;
			found = true;
		}
	}

	// if necessary add as new label
	if (!found) {
		ssi_char_t *new_label = ssi_strcpy (label);
		labels.push_back (new_label);
		label_index = labelSize () - 1;
	}

	return label_index;
}

old::Annotation::Entry *old::Annotation::last () {
	
	entries_set_t::iterator i_end = entries.end();
	return *(--i_end);
}

ssi_size_t old::Annotation::labelSize () {
	return ssi_cast (ssi_size_t , labels.size ());
}

void old::Annotation::print (FILE *file) {

	reset ();
	old::Annotation::Entry *entry;
	while (entry = next ()) {
		fprintf (file, "%.2lf\t%.2lf\t%s\n", entry->start, entry->stop, labels[entry->label_index]);
	}
	reset ();
}
/*
bool old::Annotation::entry_compare (old::Annotation::Entry *lhs, old::Annotation::Entry *rhs) {
	return lhs->start <= rhs->start;
}*/

old::Annotation::Entry *old::Annotation::getEntryAt(ssi_time_t time, ssi_time_t max_delay)
{	
	entries_set_t::iterator i = entries.begin();
	entries_set_t::iterator i_end = entries.end();
	while(i != i_end)
	{
		if(time >= (*i)->start && time < (*i)->stop + max_delay)			
			return *i;
		++i;
	}
	return 0;
}

void old::Annotation::trim(ssi_size_t cut_front, ssi_size_t cut_back)
{
	//trim front
	{
		entries_set_t::iterator i = entries.begin();
		entries_set_t::iterator i_end = entries.end();
		for (ssi_size_t k = 0; k < cut_front; k++)
			i++;

		entries.erase(entries.begin(), i);
	}

	//trim back
	{
		entries_set_t::iterator i = entries.end();
		entries_set_t::iterator i_end = entries.begin();
		for (ssi_size_t k = 0; k < cut_back; k++)
			i--;

		entries.erase(i, entries.end());
	}
}


}
