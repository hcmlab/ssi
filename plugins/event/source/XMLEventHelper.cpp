// XMLEventHelper.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/10/27
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

#include "XMLEventHelper.h"
#include "ioput/xml/tinyxml.h"
#include "event/EventAddress.h"
#include "base/Factory.h"
#include "thread/Lock.h"
#include "XMLEventSender.h"
#include "thread/Timer.h"
#include "graphic/Monitor.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *XMLEventHelper::ssi_log_name = "xmleventh_";

XMLEventHelper::XMLEventHelper(XMLEventSender *sender)
	: Thread (false),
	_sender (sender),
	_timer(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	_n_strbuf = _sender->_options.mbuf;
	_strbuf = new ssi_char_t[_n_strbuf];
	_row_delim = _sender->_options.rowdelim;
	_col_delim = _sender->_options.coldelim;

	_mutex = new Mutex();
}

XMLEventHelper::~XMLEventHelper() {

	clearMapping();
	delete[] _strbuf;
	delete _mutex;
}


void XMLEventHelper::initMapping(Mapping &map) {

	map.type = MapType::UNDEF;
	map.target = MapTarget::UNDEF;
	map.node = 0;
	map.stream_id = 0;
	map.n_event_ids = 0;
	map.event_ids = 0;
	map.n_sender_ids = 0;
	map.sender_ids = 0;
	map.n_select = 0;
	map.n_select_multiples = 0;
	map.select = 0;
	map.last = 0;
	map.span = -1;
	map.precision = -1;
	map.field = Field::VALUE;
	map.functional = Functional::NONE;
}

void XMLEventHelper::clearMapping() {

	std::vector <Mapping>::iterator it;
	for (it = mapping.begin(); it != mapping.end(); it++) {
		delete[] it->select;
		delete[] it->event_ids;
		delete[] it->sender_ids;
	}
	mapping.clear();
}

void XMLEventHelper::resetMapping(ssi_size_t time) {

	std::vector <Mapping>::iterator it;
	for (it = mapping.begin(); it != mapping.end(); it++) {
		switch (it->target) {
		case XMLEventHelper::MapTarget::ATTRIBUTE: {
			if (it->span != -1 && time - it->last > it->span) {
				ssi_pcast(TiXmlAttribute, it->node)->SetValue("");
			}
			break;
		}
		case XMLEventHelper::MapTarget::ELEMENT: {
			TiXmlNode *content = ssi_pcast(TiXmlElement, it->node)->FirstChild();
			if (it->span != -1 && time - it->last > it->span) {
				content->SetValue("");
			}
			break;
		}
		}
	}
}

bool XMLEventHelper::parse() {

	TiXmlElement *elem = 0;
	TiXmlAttribute *attr = 0;

	elem = _sender->_doc->FirstChildElement();
	parseChildren(elem);

	return true;
}

bool XMLEventHelper::parseChildren(TiXmlElement *elem) {

	TiXmlElement *child = elem->FirstChildElement();
	while (child) {
		parseChildren(child);
		child = child->NextSiblingElement();
	}

	parseAttributes(elem);
	parseMapping(MapTarget::ELEMENT, elem, elem->GetText());

	return true;
}

bool XMLEventHelper::parseAttributes(TiXmlElement *elem) {

	TiXmlAttribute *attr = elem->FirstAttribute();
	while (attr) {
		parseMapping(MapTarget::ATTRIBUTE, attr, attr->Value());
		attr = attr->Next();
	}

	return true;
}

bool XMLEventHelper::splitMapping(const ssi_char_t *string, ssi_char_t **input, ssi_char_t **options)
{
	*input = 0;
	*options = 0;

	if (!string)
	{
		return false;
	}

	ssi_size_t n_string = ssi_strlen(string);
	if (n_string == 0)
	{
		return false;
	}
	
	int pos_option_begin = 0;
	int pos_option_end = 0;
	ssi_size_t pos = 0;

	for (pos = 1; pos < n_string; pos++)
	{
		if (string[pos] == '{')
		{
			pos_option_begin = pos;
			break;
		}
	}

	if (pos_option_begin > 0)
	{
		for (pos = pos_option_begin + 1; pos < ssi_strlen(string); pos++)
		{
			if (string[pos] == '}')
			{
				pos_option_end = pos;
				break;
			}
		}

		if (pos_option_end > pos_option_begin)
		{
			int n_options = pos_option_end - (pos_option_begin + 1);
			*options = new ssi_char_t[n_options + 1];
			memcpy(*options, string + pos_option_begin + 1, n_options);
			(*options)[n_options] = '\0';
		}
	}

	int n_input = pos_option_begin > 0 ? pos_option_begin : n_string;
	*input = new ssi_char_t[n_input + 1];
	memcpy(*input, string, n_input);
	(*input)[n_input] = '\0';

	return true;
}

bool XMLEventHelper::parseMapping(MapTarget::List target, TiXmlBase *node, const ssi_char_t *string) 
{

	if (!string) {
		return false;
	}

	ssi_size_t len = ssi_strlen(string);
	if (len < 3) {
		return false;
	}

	if (string[0] != '$' && string[1] != '(' && string[len] != ')') {
		return false;
	}

	ssi_char_t *content = new ssi_char_t[len - 2];
	memcpy(content, string + 2, len - 3);
	content[len - 3] = '\0';

	Mapping map;
	initMapping(map);
	map.target = target;
	map.node = node;

	ssi_strtrim(content);

	ssi_char_t *input = 0;
	ssi_char_t *options = 0;
	
	if (!splitMapping(content, &input, &options))
	{
		ssi_wrn("could not parse mapping '%s'", content);
		return false;
	}

	bool is_event = false;
	for (ssi_size_t i = 0; i < ssi_strlen(input); i++)
	{
		if (input[i] == '@')
		{
			is_event = true;
		}
	}
	
	if (is_event)
	{
		map.type = MapType::EVENT;
		parseEvent(map, input);
	}
	else
	{
		map.type = MapType::STREAM;
		parseStream(map, input);
	}

	if (map.type == MapType::UNDEF)
	{
		ssi_wrn("undefined mapping '%s'", string);
		return false;
	}

	if (options)
	{
		ssi_size_t n_tokens = 0;
		ssi_char_t **tokens = 0;

		n_tokens = ssi_split_string_count(options, ';');
		if (n_tokens > 0)
		{
			tokens = new ssi_char_t *[n_tokens];
			ssi_split_string(n_tokens, tokens, options, ';');

			for (ssi_size_t i = 0; i < n_tokens; i++) {

				ssi_strtrim(tokens[i]);

				ssi_char_t *key = 0;
				ssi_char_t *value = 0;
				if (!ssi_split_keyvalue(tokens[i], &key, &value) || !value) {
					ssi_wrn("could not parse option '%s'", tokens[i]);
					continue;
				}
				ssi_strtrim(key);
				ssi_strtrim(value);

				if (ssi_strcmp(key, "span", false)) {
					parseTimeout(map, value);
				}
				else if (ssi_strcmp(key, "select", false, 6)) {
					ssi_size_t multiples = 0;
					if (ssi_strlen(key) > 6) {
						if (key[6] != '*' || sscanf(key + 7, "%u", &multiples) != 1) {
							ssi_wrn("could not parse multiples from '%s'", key + 6);
							multiples = 0;
						}
					}
					parseSelect(map, value, multiples);
				}
				else if (ssi_strcmp(key, "field", false)) {
					parseField(map, value);
				}
				else if (ssi_strcmp(key, "functional", false)) {
					parseFunction(map, value);
				}
				else if (ssi_strcmp(key, "precision", false)) {
					parsePrecision(map, value);
				}
				else {
					ssi_wrn("unknown option '%s'", key);
					return false;
				}

				delete[] key;
				delete[] value;
			}

			for (ssi_size_t i = 0; i < n_tokens; i++) {
				delete[] tokens[i];
			}
			delete[] tokens;
		}
	}

	mapping.push_back(map);

	switch (map.target) {
		case XMLEventHelper::MapTarget::ATTRIBUTE: {
			ssi_pcast(TiXmlAttribute, node)->SetValue("");
			break;
		}
		case XMLEventHelper::MapTarget::ELEMENT: {
			TiXmlNode *content = ssi_pcast(TiXmlElement, node)->FirstChild();
			content->SetValue("");
			break;
		}
	}

	delete[] input;
	delete[] options;
	delete[] content;

	return true;
}

bool XMLEventHelper::parseField(Mapping &map, const ssi_char_t *string) {

	if (ssi_strcmp(string, FieldName(Field::VALUE), false)) {
		map.field = Field::VALUE;
	} else if (ssi_strcmp(string, FieldName(Field::NAME), false)) {
		map.field = Field::NAME;
	} else if (ssi_strcmp(string, FieldName(Field::TIME), false)) {
		map.field = Field::TIME;
	} else if (ssi_strcmp(string, FieldName(Field::TIME_SYSTEM), false)) {
		map.field = Field::TIME_SYSTEM;
	} else if (ssi_strcmp(string, FieldName(Field::TIME_RELATIVE), false)) {
		map.field = Field::TIME_RELATIVE;
	} else if (ssi_strcmp(string, FieldName(Field::DURATION), false)) {
		map.field = Field::DURATION;
	} else if (ssi_strcmp(string, FieldName(Field::STATE), false)) {
		map.field = Field::STATE;
	} else if (ssi_strcmp(string, FieldName(Field::EVENT), false)) {
		map.field = Field::EVENT;
	} else if (ssi_strcmp(string, FieldName(Field::SENDER), false)) {
		map.field = Field::SENDER;
	} else {
		ssi_wrn("unkown field '%s', setting to 'value'", string);
		map.field = Field::VALUE;
	}

	return true;
}

bool XMLEventHelper::parseFunction(Mapping &map, const ssi_char_t *string) {

	if (ssi_strcmp(string, FunctionalName(Functional::NONE), false)) {
		map.functional = Functional::NONE;
	}
	else if (ssi_strcmp(string, FunctionalName(Functional::MEAN), false)) {
		map.functional = Functional::MEAN;
	}
	else if (ssi_strcmp(string, FunctionalName(Functional::STD), false)) {
		map.functional = Functional::STD;
	}
	else if (ssi_strcmp(string, FunctionalName(Functional::MIN), false)) {
		map.functional = Functional::MIN;
	}
	else if (ssi_strcmp(string, FunctionalName(Functional::MAX), false)) {
		map.functional = Functional::MAX;
	}
	else if (ssi_strcmp(string, FunctionalName(Functional::FIRST), false)) {
		map.functional = Functional::FIRST;
	}
	else if (ssi_strcmp(string, FunctionalName(Functional::LAST), false)) {
		map.functional = Functional::LAST;
	}
	else {
		ssi_wrn("unknown field '%s', setting to 'none'", string);
		map.functional = Functional::NONE;
	}

	return true;
}

bool XMLEventHelper::parsePrecision(Mapping &map, const ssi_char_t *string) {

	if (sscanf(string, "%d", &map.precision) != 1) {
		ssi_wrn("could not parse precision value '%s'", string);
		return false;
	}

	return true;
}

bool XMLEventHelper::parseTimeout(Mapping &map, const ssi_char_t *string) {

	if (sscanf(string, "%u", &map.span) != 1) {
		ssi_wrn("could not parse span value '%s'", string);
		return false;
	}

	return true;
}

bool XMLEventHelper::parseSelect(Mapping &map, const ssi_char_t *string, ssi_size_t multiples) {

	ssi_size_t n;
	int *indices = ssi_parse_indices(string, n);
	if (!indices) {
		ssi_wrn("could not parse selection '%s'", string);
		return false;
	}

	map.n_select = n;
	map.select = new ssi_size_t[n];
	for (ssi_size_t i = 0; i < n; i++) {
		map.select[i] = indices[i];
	}
	map.n_select_multiples = multiples;

	delete[] indices;

	return true;
}

bool XMLEventHelper::parseStream(Mapping &map, const ssi_char_t *string) {

	if (sscanf(string, "%u", &map.stream_id) != 1) {
		ssi_wrn("could not parse stream id '%s'", string);
		return false;
	}

	return true;
}

bool XMLEventHelper::parseEvent(Mapping &map, const ssi_char_t *string) {

	EventAddress address;
	address.setAddress(string);

	map.n_event_ids = address.getEventsSize();
	if (map.n_event_ids > 0) {
		map.event_ids = new ssi_size_t[map.n_event_ids];
		for (ssi_size_t i = 0; i < map.n_event_ids; i++) {
			map.event_ids[i] = Factory::AddString(address.getEvent(i));
		}
	}
	map.n_sender_ids = address.getSenderSize();
	if (map.n_sender_ids > 0) {
		map.sender_ids = new ssi_size_t[map.n_sender_ids];
		for (ssi_size_t i = 0; i < map.n_sender_ids; i++) {
			map.sender_ids[i] = Factory::AddString(address.getSender(i));
		}
	}

	return true;
}

bool XMLEventHelper::checkEvent(Mapping &map, ssi_event_t &e) {

	bool sender_ok = false;
	bool event_ok = false;

	if (map.n_sender_ids > 0) {
		for (ssi_size_t i = 0; i < map.n_sender_ids; i++) {
			if (e.sender_id == map.sender_ids[i]) {
				sender_ok = true;
				break;
			}
		}
	}
	else {
		sender_ok = true;
	}
	if (map.n_event_ids > 0) {
		for (ssi_size_t i = 0; i < map.n_event_ids; i++) {
			if (e.event_id == map.event_ids[i]) {
				event_ok = true;
				break;
			}
		}
	}
	else {
		event_ok = true;
	}

	return sender_ok && event_ok;
}

bool XMLEventHelper::applyEvent(Mapping &map, ssi_event_t &e) {

	switch (map.field) {

		case Field::VALUE: {

			switch (e.type) {

				case SSI_ETYPE_EMPTY: {

					ssi_sprint(_strbuf, "1");

					break;
				}

				case SSI_ETYPE_STRING: {

					ssi_sprint(_strbuf, "%s", ssi_pcast(ssi_char_t, e.ptr));

					break;
				}

				case SSI_ETYPE_TUPLE: {

					ssi_size_t n_values = e.tot / sizeof(ssi_event_tuple_t);
					ssi_size_t n = map.n_select == 0 ? n_values : map.n_select;
					ssi_event_tuple_t *ptr = ssi_pcast(ssi_event_tuple_t, e.ptr);
					ssi_size_t n_str = _n_strbuf;
					ssi_char_t *str = _strbuf;
					ssi_size_t n_add = 0;					

					// calculate iterations
					ssi_size_t n_iter = (map.n_select_multiples > 0 && n_values > map.n_select_multiples ) ? n_values / map.n_select_multiples : 1;

					for (ssi_size_t j = 0; j < n_iter; j++) {
						for (ssi_size_t i = 0; i < n; i++) {

							if (map.select) {								
								ssi_size_t n_offset = min(n_values - 1, map.select[i] + j * map.n_select_multiples);
								ptr = ssi_pcast(ssi_event_tuple_t, e.ptr) + n_offset;
							}

							n_add = ssi_val2str(SSI_REAL, ptr, n_str, str, map.precision);
							if (n_add == 0) {
								ssi_wrn("string buffer too short");
								return false;
							}
							str += n_add;
							n_str -= n_add;

							if (i < n - 1) {
								if (n_str == 0) {
									ssi_wrn("string buffer too short");
									return false;
								}
								str[0] = _col_delim;
								str[1] = '\0';
								str++;
								n_str--;
							}

							if (map.select == 0) {
								ptr++;
							}
						}
					}
	
					break;
				}

				case SSI_ETYPE_MAP: {

					ssi_size_t n_values = e.tot / sizeof(ssi_event_map_t);
					ssi_size_t n = map.n_select == 0 ? n_values : map.n_select;
					ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, e.ptr);
					ssi_size_t n_str = _n_strbuf;
					ssi_char_t *str = _strbuf;
					ssi_size_t n_add = 0;

					// calculate iterations
					ssi_size_t n_iter = (map.n_select_multiples > 0 && n_values > map.n_select_multiples) ? n_values / map.n_select_multiples : 1;

					bool first = true;
					for (ssi_size_t j = 0; j < n_iter; j++) {
						for (ssi_size_t i = 0; i < n; i++) {

							if (map.n_select > 0) {
								ssi_size_t n_offset = min(n_values - 1, map.select[i] + j * map.n_select_multiples);
								ptr = ssi_pcast(ssi_event_map_t, e.ptr) + n_offset;
							} else {
								ptr = ssi_pcast(ssi_event_map_t, e.ptr) + i;
							}

							if (first) {
								first = false;
							} else {
								if (n_str > 0) {
									str[0] = _col_delim;
									str[1] = '\0';
									str++;
									n_str--;
								} else {
									ssi_wrn("string buffer too short");
									return false;
								}
							}

							n_add = ssi_val2str(SSI_REAL, &ptr->value, n_str, str, map.precision);
							if (n_add == 0) {
								ssi_wrn("string buffer too short");
								return false;
							}
							str += n_add;
							n_str -= n_add;
						}
					}

					break;
				}
			}

			break;
		}

		case Field::NAME: {

			switch (e.type) {

				case SSI_ETYPE_MAP: {

					ssi_size_t n = map.n_select == 0 ? e.tot / sizeof(ssi_event_map_t) : map.n_select;
					ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, e.ptr);
					ssi_size_t n_str = _n_strbuf;
					ssi_char_t *str = _strbuf;

					bool first = true;
					for (ssi_size_t i = 0; i < n; i++) {

						if (map.n_select > 0) {
							ptr = ssi_pcast(ssi_event_map_t, e.ptr) + map.select[i];
						} else {
							ptr = ssi_pcast(ssi_event_map_t, e.ptr) + i;
						}

						const ssi_char_t *name = Factory::GetString(ptr->id);
						if (n_str < ssi_strlen(name) + 1) {
							ssi_wrn("string buffer too short");
							return false;
						}
						ssi_sprint(str, (first ? "%s" : " %s"), name);
						str += ssi_strlen(name) + (first ? 0 : 1);
						n_str -= ssi_strlen(name) + (first ? 0 : 1);

						first = false;
					}

					break;
				}

				default:
					ssi_wrn("field 'name' cannot be applied to an event of type '%s'", SSI_ETYPE_NAMES[e.type]);

			}

			break;
		}

		case Field::TIME: {

			ssi_size_t time = e.time;
			ssi_sprint(_strbuf, "%d", time);

			break;
		}

		case Field::TIME_SYSTEM: {

			ssi_size_t time = e.time + Factory::GetFramework()->GetStartTimeMs();
			ssi_sprint(_strbuf, "%d", time);

			break;
		}

		case Field::TIME_RELATIVE: {

			ssi_size_t time = Factory::GetFramework()->GetElapsedTimeMs() - e.time;
			ssi_sprint(_strbuf, "%d", time);

			break;
		}

		case Field::DURATION: {

			ssi_size_t duration = e.dur;
			ssi_sprint(_strbuf, "%d", duration);

			break;
		}

		case Field::STATE: {

			ssi_sprint(_strbuf, "%d", e.state == SSI_ESTATE_COMPLETED ? 0 : 1);

			break;
		}

		case Field::EVENT: {

			ssi_sprint(_strbuf, "%s", Factory::GetString(e.event_id));

			break;
		}

		case Field::SENDER: {

			ssi_sprint(_strbuf, "%s", Factory::GetString(e.sender_id));

			break;
		}
	}

	switch (map.target) {
		case XMLEventHelper::MapTarget::ATTRIBUTE: {
			ssi_pcast(TiXmlAttribute, map.node)->SetValue(_strbuf);
			break;
		}
		case XMLEventHelper::MapTarget::ELEMENT: {
			TiXmlNode *content = ssi_pcast(TiXmlElement, map.node)->FirstChild();
			content->SetValue(_strbuf);
			break;
		}
	}

	return true;
}

bool XMLEventHelper::forward(IEvents &events,
	ssi_size_t n_new_events,
	ssi_size_t time) {

	if (n_new_events == 0) {
		return true;
	}

	ssi_event_t **es = new ssi_event_t *[n_new_events];
	for (ssi_size_t i = 0; i < n_new_events; i++) {
		es[i] = events.next();
	}

	for (ssi_size_t i = 0; i < n_new_events; i++) {

		ssi_event_t *e = es[n_new_events - i - 1];

		std::vector <Mapping>::iterator it;

		for (it = mapping.begin(); it != mapping.end(); it++) {

			if (it->type != MapType::EVENT) {
				continue;
			}

			if (checkEvent(*it, *e)) {
				if (!applyEvent(*it, *e)) {
					continue;
				}
				it->last = time;
			}

		}
	}

	delete[] es;

	return true;
}

bool XMLEventHelper::applyStreamFunctional(Functional::List functional, ssi_stream_t &stream, ssi_real_t *values)
{
	switch (functional)
	{
	case Functional::MEAN:
	{
		ssi_mean(stream.num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), values);
		break;
	}
	case Functional::STD:
	{
		ssi_stdv(stream.num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), values);
		break;
	}
	case Functional::MIN:
	{
		ssi_min(stream.num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), values);
		break;
	}
	case Functional::MAX:
	{
		ssi_max(stream.num, stream.dim, ssi_pcast(ssi_real_t, stream.ptr), values);
		break;
	}
	case Functional::FIRST:
	{
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);
		for (ssi_size_t i = 0; i < stream.dim; i++)
		{
			values[i] = ptr[i];
		}		
		break;
	}
	case Functional::LAST:
	{
		ssi_real_t *ptr = ssi_pcast(ssi_real_t, stream.ptr);
		ptr += stream.dim * (stream.num - 1);
		for (ssi_size_t i = 0; i < stream.dim; i++)
		{
			values[i] = ptr[i];
		}
		break;
	}
	}

	return true;
}

bool XMLEventHelper::applyStream(Mapping &map, ssi_stream_t &stream) {

	Lock lock(*_mutex);

	if (map.n_select > 0) {

		ssi_size_t n_values = stream.dim;

		ssi_stream_t stream_select;
		ssi_stream_select(stream, stream_select, map.n_select, map.select, map.n_select_multiples);
		
		if (map.functional != Functional::NONE) 
		{
			if (stream.type != SSI_REAL) 
			{
				ssi_wrn("'functional is not supported for type '%s'", SSI_TYPE_NAMES[stream.type]);
				return false;
			}
			else
			{
				ssi_real_t *values = new ssi_real_t[stream.dim];
				applyStreamFunctional(map.functional, stream_select, values);
				if (ssi_stream_print(values, SSI_REAL, 1, stream_select.dim, stream_select.byte, _n_strbuf, _strbuf, _col_delim, _row_delim, map.precision) == 0)
				{
					ssi_wrn("string buffer too short");
					return false;
				}
				delete[] values;
			}
		}
		else
		{
			if (ssi_stream_print(stream_select, _n_strbuf, _strbuf, _col_delim, _row_delim, map.precision) == 0) {
				ssi_wrn("string buffer too short");
				return false;
			}
		}

		ssi_stream_destroy(stream_select);
	}
	else 
	{
		if (map.functional != Functional::NONE)
		{
			if (stream.type != SSI_REAL)
			{
				ssi_wrn("'functional is not supported for type '%s'", SSI_TYPE_NAMES[stream.type]);
				return false;
			}
			else
			{
				ssi_real_t *values = new ssi_real_t[stream.dim];
				applyStreamFunctional(map.functional, stream, values);
				if (ssi_stream_print(values, SSI_REAL, 1, stream.dim, stream.byte, _n_strbuf, _strbuf, _col_delim, _row_delim, map.precision) == 0)
				{
					ssi_wrn("string buffer too short");
					return false;
				}
				delete[] values;
			}
		}
		else
		{
			if (ssi_stream_print(stream, _n_strbuf, _strbuf, _col_delim, _row_delim, map.precision) == 0) {
				ssi_wrn("string buffer too short");
				return false;
			}
		}
	}

	switch (map.target) {
		case XMLEventHelper::MapTarget::ATTRIBUTE: {
			ssi_pcast(TiXmlAttribute, map.node)->SetValue(_strbuf);
			break;
		}
		case XMLEventHelper::MapTarget::ELEMENT: {
			TiXmlNode *content = ssi_pcast(TiXmlElement, map.node)->FirstChild();
			content->SetValue(_strbuf);
			break;
		}
	}
	return true;
}

bool XMLEventHelper::forward(ssi_size_t n_streams,
	ssi_stream_t streams[],
	ssi_size_t time) {

	std::vector <Mapping>::iterator it;

	for (it = mapping.begin(); it != mapping.end(); it++) {

		if (it->type != MapType::STREAM) {
			continue;
		}

		if (it->stream_id < n_streams) {
			if (!applyStream(*it, streams[it->stream_id])) {
				continue;
			}
			it->last = time;
		}
	}

	return true;
}

void XMLEventHelper::enter() {

	_timer = new Timer(_sender->_options.update);
}

void XMLEventHelper::flush() {

	delete _timer; _timer = 0;
}

void XMLEventHelper::run() {

	{
		Lock lock(*_mutex);

		TiXmlPrinter _printer;
		_printer.SetLineBreak("\r\n");
		_sender->_doc->Accept(&_printer);
		const char *string = _printer.CStr();

		if (_sender->_listener) {
			ssi_event_adjust(_sender->_event, ssi_strlen(string) + 1);
			ssi_sprint(_sender->_event.ptr, "%s", string);
			_sender->_listener->update(_sender->_event);
		}

		if (_sender->_monitor) {
			_sender->_monitor->clear();
			_sender->_monitor->print(string);
			_sender->_monitor->update();
		}

		if (_sender->_options.console) {
			_sender->_doc->Print();
		}

		ssi_size_t time = Factory::GetFramework ()->GetElapsedTimeMs ();
		resetMapping(time);
	}

	_timer->wait();
}

void XMLEventHelper::send(ssi_size_t time, ssi_size_t dur) {

	Lock lock(*_mutex);

	TiXmlPrinter _printer;
	_printer.SetLineBreak("\r\n");
	_sender->_doc->Accept(&_printer);
	const char *string = _printer.CStr();

	if (_sender->_listener) {
		_sender->_event.time = time;
		_sender->_event.dur = dur;
		ssi_event_adjust(_sender->_event, ssi_strlen(string) + 1);
		ssi_sprint(_sender->_event.ptr, "%s", string);
		_sender->_listener->update(_sender->_event);
	}

	if (_sender->_monitor) {
		_sender->_monitor->clear();
		_sender->_monitor->print(string);
		_sender->_monitor->update();
	}

	if (_sender->_options.console) {
		_sender->_doc->Print();
	}

	resetMapping(Factory::GetFramework()->GetElapsedTimeMs());

}

}
