#pragma once

#ifndef THRESTUPLEEVENTSENDER_H
#define _THRESTUPLEEVENTSENDER_H

#include "base/IConsumer.h"
#include "base/IEvents.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/IObject.h"


namespace ssi {

	class ThresTupleEventSender : public IConsumer {

	public:

		class Options : public OptionList {

		public:

			Options() : mean(true), extend(0) {

				setAddress("");
				setSenderName("threstuplesender");
				setEventName("class");

				setClasses("low, medium, high");
				setThresholds("0.1, 0.3, 0.8");

				SSI_OPTIONLIST_ADD_ADDRESS(address);

				mean = true;

				addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
				addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");
				addOption("classes", classes, SSI_MAX_CHAR, SSI_CHAR, "names of the classes event (e.g. low,medium,high)");
				addOption("thres", &thres, SSI_MAX_CHAR, SSI_CHAR, "thresholds (e.g. 0.1,0.3,0.8)");
				addOption("minDiff", &minDiff, 1, SSI_FLOAT, "minimum difference to previous value");
				addOption("mean", &mean, 1, SSI_BOOL, "send mean value (only a single event will be sent)");
			};

			void setAddress(const ssi_char_t *address) {
				if (address) {
					ssi_strcpy(this->address, address);
				}
			}
			void setSenderName(const ssi_char_t *sname) {
				if (sname) {
					ssi_strcpy(this->sname, sname);
				}
			}
			void setEventName(const ssi_char_t *ename) {
				if (ename) {
					ssi_strcpy(this->ename, ename);
				}
			}

			void setClasses(const ssi_char_t *classes) {
				if (classes) {
					ssi_strcpy(this->classes, classes);
				}
			}

			void setThresholds(const ssi_char_t *thres) {
				if (thres) {
					ssi_strcpy(this->thres, thres);
				}
			}

			ssi_char_t address[SSI_MAX_CHAR];
			ssi_char_t sname[SSI_MAX_CHAR];
			ssi_char_t ename[SSI_MAX_CHAR];
			ssi_char_t classes[SSI_MAX_CHAR];
			ssi_char_t thres[SSI_MAX_CHAR];	
			ssi_real_t minDiff;
			bool mean;
			ssi_size_t extend;
		};

	public:

		static const ssi_char_t *GetCreateName() { return "ThresTupleEventSender"; };
		static IObject *Create(const ssi_char_t *file) { return new ThresTupleEventSender(file); };
		~ThresTupleEventSender();

		ThresTupleEventSender::Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Converts a stream to a map event (key1=value1,key2=value2,...)."; };

		void consume_enter(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume(IConsumer::info consume_info,
			ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);
		void consume_flush(ssi_size_t stream_in_num,
			ssi_stream_t stream_in[]);

		bool setEventListener(IEventListener *listener);
		const ssi_char_t *getEventAddress() {
			return _event_address.getAddress();
		}

		void setLogLevel(int level) {
			ssi_log_level = level;
		}

	protected:

		ThresTupleEventSender(const ssi_char_t *file = 0);
		ThresTupleEventSender::Options _options;
		ssi_char_t *_file;

		static ssi_char_t *ssi_log_name;
		int ssi_log_level;

		ssi_real_t *_thres;
		ssi_size_t _num_classes;
		ssi_size_t _num_thres;
		const ssi_char_t **_classes;

		IEventListener *_listener;
		EventAddress _event_address;
		ssi_event_t _event;

		ssi_real_t *parseFloats(const ssi_char_t *str, ssi_size_t &n_indices, bool sort = false, const ssi_char_t *delims = " ,");
		const ssi_char_t **parseStrings(const ssi_char_t *str, ssi_size_t &n_indices, const ssi_char_t *delims = " ,");

	};

}

#endif
