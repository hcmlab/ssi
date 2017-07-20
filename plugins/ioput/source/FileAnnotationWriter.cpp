// FileAnnotationWriter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/12
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

#include "FileAnnotationWriter.h"
#include "ioput/file/FilePath.h"
#include "ssiml/include/Annotation.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	ssi_char_t *FileAnnotationWriter::ssi_log_name = "fannowriter";

	FileAnnotationWriter::FileAnnotationWriter(const ssi_char_t *file)
		: _file(0),
		_annotation(0),	
		_borrowed(false),
		_confidence(0),
		_label(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}		
	}	

	FileAnnotationWriter::~FileAnnotationWriter() {

		delete[] _label; _label = 0;
		if (!_borrowed)
		{
			delete _annotation; _annotation = 0;
		}

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	bool FileAnnotationWriter::load_scheme()
	{
		if (_annotation)
		{
			return false;
		}

		Annotation *annotation = new Annotation();
		if (_options.schemePath[0] != '\0')
		{			
			if (!annotation->loadScheme(_options.schemePath))
			{
				ssi_wrn("invalid scheme '%s'", _options.schemePath);
				return false;
			}

			if (_options.meta[0] != '\0')
			{
				parse_meta(annotation, _options.meta, ';');
			}
			
			_annotation = annotation;
		}
		else
		{			
			if (annotation->setDiscreteScheme("noname"))
			{
				_annotation = annotation;
			}
		}

		return true;
	}

	void FileAnnotationWriter::save_annotation()
	{
		if (!_borrowed && _annotation)
		{
			if (_options.annotationPath[0] != '\0')
			{
				if (!ssi_pcast(Annotation, _annotation)->save(_options.annotationPath, File::ASCII))
				{
					ssi_wrn("could not save annotation file '%s'", _options.annotationPath);
				}
			}
			delete _annotation; _annotation = 0;			
		}
	}

	void FileAnnotationWriter::listen_enter()
	{		
		load_scheme();
		if (!_label)
		{
			_label = ssi_strcpy(_options.defaultLabel);
			_confidence = _options.defaultConfidence;
		}
	}

	void FileAnnotationWriter::setLabel(const ssi_char_t *label, ssi_real_t confidence) {

		Lock lock(_mutex);
		
		if (label) {
			delete[] _label;
			_label = ssi_strcpy(label);		
		}
		_confidence = confidence;
	}

	bool FileAnnotationWriter::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {

		if (n_new_events > 0) {
			ssi_event_t **es = new ssi_event_t *[n_new_events];
			for (ssi_size_t i = 0; i < n_new_events; i++) {
				es[i] = events.next();
			}
			for (ssi_size_t i = n_new_events; i > 0; i--) {
				update(*es[i - 1]);
			}
			delete[] es;
		}

		return true;
	}

	bool FileAnnotationWriter::get_class_id(Annotation *annotation, const ssi_char_t *string, ssi_int_t &id)
	{
		if (!string || string[0] == '\0')
		{
			return false;
		}

		if (!annotation->hasClassName(string))
		{
			if (!_options.addUnkownLabel)
			{
				return false;
			}
			else
			{
				return annotation->addClass(string, id);
			}
		}

		return annotation->getClassId(string, id);
	}

	bool FileAnnotationWriter::update(ssi_event_t &e) {

		Lock lock(_mutex);

		if (!_annotation)
		{
			return false;
		}		

		if (e.state != SSI_ESTATE_COMPLETED)
		{
			return false;		
		}

		Annotation *annotation = ssi_pcast(Annotation, _annotation);

		ssi_label_t label;
		label.discrete.from = e.time / 1000.0;
		label.discrete.to = (e.time + e.dur) / 1000.0;
		label.confidence = _confidence;

		if (annotation->getScheme()->type == SSI_SCHEME_TYPE::CONTINUOUS)
		{
			label.continuous.score = 0;

			if (e.type == SSI_ETYPE_MAP)
			{
				ssi_size_t n = e.tot / sizeof(ssi_event_map_t);
				ssi_event_map_t *t = ssi_pcast(ssi_event_map_t, e.ptr);
				if (_options.streamScoreIndex < n)
				{
					label.continuous.score = t[_options.streamScoreIndex].value;
				}
				if (!_options.forceDefaultConfidence && _options.streamConfidenceIndex < n)
				{
					label.confidence = t[_options.streamConfidenceIndex].value;
				}
			}
			else if (e.type == SSI_ETYPE_TUPLE)
			{
				ssi_size_t n = e.tot / sizeof(ssi_event_tuple_t);
				ssi_event_tuple_t *t = ssi_pcast(ssi_event_tuple_t, e.ptr);
				if (_options.streamScoreIndex < n)
				{
					label.continuous.score = t[_options.streamConfidenceIndex];
				}
				if (!_options.forceDefaultConfidence && _options.streamConfidenceIndex < n)
				{
					label.confidence = t[_options.streamConfidenceIndex];
				}
			}
			else
			{
				ssi_wrn("event type not supported '%s'", SSI_ETYPE_NAMES[e.type]);
				return false;
			}

		} 
		else if (annotation->getScheme()->type == SSI_SCHEME_TYPE::DISCRETE)
		{
			ssi_int_t id = SSI_SAMPLE_GARBAGE_CLASS_ID;

			if (_options.eventNameAsLabel)
			{
				const ssi_char_t *string = Factory::GetString(e.event_id);
				if (!get_class_id(annotation, string, id))
				{
					return false;
				}
			}
			else if (_options.eventNameAsLabel)
			{
				const ssi_char_t *string = Factory::GetString(e.sender_id);
				if (!get_class_id(annotation, string, id))
				{
					return false;
				}
			}
			else if (e.type == SSI_ETYPE_EMPTY || _options.forceDefaultLabel)
			{
				if (!get_class_id(annotation, _label, id))
				{
					return false;
				}
			}
			else if (e.type == SSI_ETYPE_STRING)
			{
				if (!get_class_id(annotation, e.ptr, id))
				{
					return false;
				}
			}
			else if (e.type == SSI_ETYPE_MAP)
			{
				ssi_size_t n = e.tot / sizeof(ssi_event_map_t);
				ssi_event_map_t *t = ssi_pcast(ssi_event_map_t, e.ptr);
				if (n > 0)
				{
					if (_options.mapKeySelectMax)
					{
						ssi_size_t max_index = 0;
						ssi_real_t max_value = t[0].value;
						for (ssi_size_t i = 1; i < n; i++)
						{
							if (t[i].value > max_value)
							{
								max_value = t[i].value;
								max_index = i;
							}
						}
						const ssi_char_t *string = Factory::GetString(t[max_index].id);
						if (!get_class_id(annotation, string, id))
						{
							return false;
						}
						if (!_options.forceDefaultConfidence)
						{
							label.confidence = t[max_index].value;
						}
					}
					else
					{
						const ssi_char_t *string = Factory::GetString(t[min(n, _options.mapKeyIndex)].id);
						if (!get_class_id(annotation, string, id))
						{
							return false;
						}
						if (!_options.forceDefaultConfidence)
						{
							label.confidence = t[min(n, _options.mapKeyIndex)].value;
						}
					}
				}
			}
			else
			{
				ssi_wrn("event type not supported '%s'", SSI_ETYPE_NAMES[e.type]);
				return false;
			}

			if (id != SSI_SAMPLE_GARBAGE_CLASS_ID)
			{
				label.discrete.id = id;
			}
			else
			{
				return false;
			}
		}
		else if (annotation->getScheme()->type == SSI_SCHEME_TYPE::FREE)
		{
			if (e.type == SSI_ETYPE_EMPTY || _options.forceDefaultLabel)
			{
				label.free.name = ssi_strcpy(_label);
			}
			else if (_options.eventNameAsLabel)
			{
				label.free.name = ssi_strcpy(Factory::GetString(e.event_id));
			}
			else if (_options.eventNameAsLabel)
			{
				label.free.name = ssi_strcpy(Factory::GetString(e.sender_id));
			}
			else if (e.type == SSI_ETYPE_STRING)
			{
				label.free.name = ssi_strcpy(e.ptr);
			}
			else if (e.type == SSI_ETYPE_MAP)
			{
				ssi_size_t n = e.tot / sizeof(ssi_event_map_t);
				ssi_event_map_t *t = ssi_pcast(ssi_event_map_t, e.ptr);
				if (n > 0)
				{
					label.free.name = ssi_strcpy(Factory::GetString(t[min(n, _options.mapKeyIndex)].id));
					if (!_options.forceDefaultConfidence)
					{
						label.confidence = t[min(n, _options.mapKeyIndex)].value;
					}
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			ssi_wrn("scheme type not supported '%s'", SSI_SCHEME_NAMES[annotation->getScheme()->type]);
			return false;
		}


		annotation->add(label);

		return true;
	}

	void FileAnnotationWriter::listen_flush()
	{
		save_annotation();
		delete _label; _label = 0;
	}

	void FileAnnotationWriter::consume_enter(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{
		load_scheme();
		if (!_label)
		{
			_label = ssi_strcpy(_options.defaultLabel);
			_confidence = _options.defaultConfidence;
		}

		if (_annotation)
		{
			Annotation *annotation = ssi_pcast(Annotation, _annotation);
			if (annotation->getScheme()->type != SSI_SCHEME_TYPE::CONTINUOUS)
			{
				if (annotation->getScheme()->continuous.sr != stream_in[0].sr)
				{
					ssi_wrn("sample rates in stream and annotation differ '%g != %g'", stream_in[0].sr, annotation->getScheme()->continuous.sr);
				}
			}
		}

		if (stream_in[0].type != SSI_REAL)
		{
			ssi_wrn("stream type not supported '%s'", SSI_TYPE_NAMES[stream_in[0].type]);
		}
	}

	void FileAnnotationWriter::consume(IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{
		if (!_annotation)
		{
			return;
		}

		if (stream_in[0].type != SSI_REAL)
		{
			return;
		}

		Annotation *annotation = ssi_pcast(Annotation, _annotation);

		if (annotation->getScheme()->type != SSI_SCHEME_TYPE::CONTINUOUS)
		{
			return;
		}

		ssi_size_t n = stream_in[0].num;
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream_in[0].ptr);
		ssi_real_t *score = ptr + min(stream_in[0].dim - 1, _options.streamScoreIndex);
		ssi_real_t *confidence = _options.forceDefaultConfidence ? 0 : ptr + min(stream_in[0].dim - 1, _options.streamConfidenceIndex);

		ssi_label_t label;
		for (ssi_size_t i = 0; i < n; i++)
		{			
			label.continuous.score = *score;
			score += stream_in[0].dim;
		
			if (confidence)
			{
				label.confidence = *confidence;
				confidence += stream_in[0].dim;
			}
			else
			{
				label.confidence = _confidence;
			}

			annotation->add(label);
		}

	}

	void FileAnnotationWriter::consume_flush(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[])
	{
		save_annotation();
		delete _label; _label = 0;
	}

	void FileAnnotationWriter::parse_meta(Annotation *annotation, const ssi_char_t *string, char delim)
	{
		ssi_size_t n_split = ssi_split_string_count(string, delim);
		if (n_split > 0)
		{
			ssi_char_t **split = new ssi_char_t *[n_split];
			ssi_split_string(n_split, split, string, delim);
			for (ssi_size_t n = 0; n < n_split; n++)
			{
				ssi_char_t *key = 0;
				ssi_char_t *value = 0;
				if (ssi_parse_option(split[n], &key, &value))
				{
					annotation->setMeta(key, value);
				}
				delete[] key;
				delete[] value;
				delete[] split[n];
			}
			delete[] split;
		}
	}

	bool FileAnnotationWriter::setAnnotation(Annotation *annotation)	
	{
		if (!_annotation)
		{
			if (annotation->getScheme())
			{
				_annotation = annotation;
				_borrowed = true;

				return true;
			}
			ssi_wrn("annotation has no scheme");			
		}
		else
		{
			ssi_wrn("already has an annotation");
		}

		if (!_label)
		{
			_label = ssi_strcpy(_options.defaultLabel);
			_confidence = _options.defaultConfidence;
		}

		return false;
	}
}