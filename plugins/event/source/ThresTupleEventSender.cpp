#include "ThresTupleEventSender.h"
#include "ioput/file/FileTools.h"
#include "base/ITheFramework.h"
#include "base/Factory.h"
#include "SSI_Tools.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	ssi_char_t *ThresTupleEventSender::ssi_log_name = "mapesend__";

	ThresTupleEventSender::ThresTupleEventSender(const ssi_char_t *file)
		: _file(0),
		_listener(0),
		ssi_log_level(SSI_LOG_LEVEL_DEFAULT) {

		if (file) {
		/*	if (!OptionList::LoadXML(file, _options)) {
				OptionList::SaveXML(file, _options);
			}
			_file = ssi_strcpy(file);*/
		}

		ssi_event_init(_event, SSI_ETYPE_MAP);
	}

	ThresTupleEventSender::~ThresTupleEventSender() {

		ssi_event_destroy(_event);

		if (_file) {
		/*	OptionList::SaveXML(_file, _options);*/
			delete[] _file;
		}
	}

	bool ThresTupleEventSender::setEventListener(IEventListener *listener) {

		_listener = listener;

		if (_options.address[0] != '\0') {

			SSI_OPTIONLIST_SET_ADDRESS(_options.address, _event_address, _event);

		}
		else {

			ssi_wrn("use of deprecated option 'sname' and 'ename', use 'address' instead")

			_event.sender_id = Factory::AddString(_options.sname);
			_event.sender_id = Factory::AddString(_options.ename);
			if (_event.sender_id == SSI_FACTORY_STRINGS_INVALID_ID) {
				return false;
			}
			_event.event_id = Factory::AddString(_options.ename);
			if (_event.event_id == SSI_FACTORY_STRINGS_INVALID_ID) {
				return false;
			}

			_event_address.setSender(_options.sname);
			_event_address.setEvents(_options.ename);
		}

		return true;
	}

	void ThresTupleEventSender::consume_enter(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		if (stream_in[0].dim > 1)
			ssi_wrn("Dimension > 1 unsupported");

		_classes = parseStrings(_options.classes, _num_classes);
		_thres = parseFloats(_options.thres, _num_thres, true);
		
		ssi_event_adjust(_event, _num_classes * (sizeof(ssi_event_map_t)));
		

		if (_num_thres != _num_classes + 1)
			ssi_wrn("invalid thresholds (%d classes, %d thresholds)", _num_classes, _num_thres);

	}

	void ThresTupleEventSender::consume(IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		ssi_real_t sum = 0;
		ssi_real_t value;
		
	

		if (_listener) {

			for (ssi_size_t k = 0; k < stream_in[0].num; k++)
			{
				value = *ssi_pcast(ssi_real_t, stream_in[0].ptr + k * stream_in[0].dim * stream_in[0].byte);

				if (_options.mean) {
					sum = sum + value;
				}
			}

			if (_options.mean) {

				value = sum / stream_in[0].num;

			}


			for (int i = _num_thres - 1; i > 0; i--) {
									
				if (value <= _thres[i] && value > _thres[i - 1]) {

					ssi_event_map_t *ptr = ssi_pcast(ssi_event_map_t, _event.ptr);

					for (int j = 0; j < _num_classes; j++) {

						ptr->id = Factory::AddString(_classes[j]);

						if (i - 1 == j) {
							ptr->value = 1;
						}
						else {
							ptr->value = 0;
						}
						
						ptr++;

					}

					_event.time = ssi_sec2ms(consume_info.time);
					_event.dur = ssi_sec2ms(consume_info.dur);

					_listener->update(_event);
				}
			}
		}

	}

	void ThresTupleEventSender::consume_flush(ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]) {

		ssi_event_reset(_event);
	}

	ssi_real_t *ThresTupleEventSender::parseFloats(const ssi_char_t *str, ssi_size_t &n, bool sort, const ssi_char_t *delims) {

		n = 0;

		if (!str || str[0] == '\0') {
			return 0;
		}

		ssi_char_t *string = ssi_strcpy(str);

		char *pch;
		strcpy(string, str);
		pch = strtok(string, delims);
		float index;

		std::vector<float> items;

		while (pch != NULL) {
			index = (float)atof(pch);
			items.push_back(index);
			pch = strtok(NULL, delims);
		}

		if (sort) {
			std::sort(items.begin(), items.end());
		}

		n = (ssi_size_t)items.size();
		float *values = new float[n];

		for (size_t i = 0; i < items.size(); i++) {
			values[i] = items[i];
		}

		delete[] string;

		return values;
	}

	const ssi_char_t **ThresTupleEventSender::parseStrings(const ssi_char_t *str, ssi_size_t &n, const ssi_char_t *delims) {

		n = 0;

		if (!str || str[0] == '\0') {
			return 0;
		}

		ssi_char_t *string = ssi_strcpy(str);

		char *pch;
		strcpy(string, str);
		pch = strtok(string, delims);

		std::vector<char*> items;

		while (pch != NULL) {
			items.push_back(pch);
			pch = strtok(NULL, delims);
		}

		n = (ssi_size_t)items.size();
		char **values = new char*[n];

		for (size_t i = 0; i < items.size(); i++) {
			values[i] = new char[SSI_MAX_CHAR];
			strcpy(values[i], items[i]);
		}
		delete[] string;

		return (const ssi_char_t**)values;
	}

}
