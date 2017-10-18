// OnlineClassifier.h
// author: Raphael Schwarz <raphaelschwarz@student.uni-augsbrug.de>
// created: 2017/04/12
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

#pragma once

#ifndef SSI_MODEL_ONLINECLASSIFIER_H
#define SSI_MODEL_ONLINECLASSIFIER_H

#include "ioput/file/FilePath.h"
#include "base/IConsumer.h"
#include "signal/SignalCons.h"
#include "ioput/file/File.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/option/OptionList.h"
#include "base/IEvents.h"
#include "event/EventAddress.h"
#include "thread/Lock.h"
#include "base/IModel.h"

#define SSI_CLASSIFIER_MAXHANDLER 5



namespace ssi {
class OnlineClassifier : public IConsumer {

public:

	class EventHandler {

	public:

		EventHandler(IEventListener *listener,
					 ssi_size_t sid,
					 ssi_size_t eid);

		virtual ~EventHandler();

		void handle(ssi_time_t time,
					ssi_time_t duration,
					ssi_size_t n_classes,
					ssi_size_t class_index,
					const ssi_real_t *probs,
					ssi_char_t *const *class_names,
					ssi_size_t n_metas,
					ssi_real_t *metas,
					ssi_size_t glue);
        void handleNotify(ssi_time_t time,
                          ssi_time_t duration,
                          ssi_size_t n_classes,
                          ssi_size_t class_index,
                          const ssi_real_t *probs,
                          ssi_char_t *const *class_names,
                          ssi_size_t n_metas,
                          ssi_real_t *metas,
                          ssi_size_t glue);

	protected:

		ssi_event_t _event;
        ssi_event_t _classificationReport;
		IEventListener *_listener;
		bool _winner_only;
		ssi_size_t _n_select;
		int *_select;
		ssi_size_t *_class_ids;


	};

public:

	class Options : public OptionList {

	public:

		Options() : console(false),
					confidence(0.5f),
					training(true),
					interval(false),
					intervalMin(0.0f),
					intervalMax(1.0f),
                    oldSamplesUsage(false){

			setAddress("");
            setBaseModelName("model.model");
            setActualModelName("actual.model");

			model[0] = '\0';
			modelOutputPath[0] = '\0';
			sampleOutputPath[0] = '\0';
            decisionClass[0] = '\0';
            answerValueName[0] = '\0';
            annoFileLocation[0] = '\0';
            oldSamplesClass[0] = '\0';

			SSI_OPTIONLIST_ADD_ADDRESS(address);

			addOption("baseModel", model, SSI_MAX_CHAR, SSI_CHAR,
					  "path and name of warm start model (by default: file model.model in application directory)");
            addOption("actualModel", actualModel, SSI_MAX_CHAR, SSI_CHAR,
                      "name of the newest lerned model (by default: file actual.model in application directory; must be in the application directory");
			addOption("modelOut", modelOutputPath, SSI_MAX_CHAR, SSI_CHAR,
					  "path where incremented model will be saved (excluding filename)");
			addOption("sampleOut", sampleOutputPath, SSI_MAX_CHAR, SSI_CHAR,
					  "path where sampleLists will be saved");
			addOption("interval", &interval, 1, SSI_BOOL, "switch for confidence interval");
			addOption("intervalMin", &intervalMin,1, SSI_REAL, "Min confidence of interval (if interval = true)");
			addOption("intervalMax", &intervalMax,1, SSI_REAL, "Max confidence of interval (if interval = true)");
			addOption("confidence", &confidence, 1, SSI_REAL,
					  "Set minimal value confidence must reach to be interesting (if interval = false)");
			addOption("console", &console, 1, SSI_BOOL, "output classification to console");
			addOption("useDecisionClass", &useDecisionClass, 1, SSI_BOOL, "Switch for using decision class only to forward if false each class will be forwarded");
            addOption("decisionClass", decisionClass, SSI_MAX_CHAR, SSI_CHAR, "Set name of class for which classifier should react (if useDecisionClass = true" );
            addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
            addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");
            addOption("valueName", answerValueName, SSI_MAX_CHAR, SSI_CHAR,
                      "Name of value to react on");
            addOption("oldSamplesUsage", &oldSamplesUsage, 1, SSI_BOOL, "if true: use old samples, which are not labeled by user for oldSamplesClass; if false: discard older samples not labled by user");
            addOption("oldSamplesClass", &oldSamplesClass, SSI_MAX_CHAR, SSI_CHAR, "Class for which old samples should be used for training");
            addOption("annoOut", annoFileLocation, SSI_MAX_CHAR, SSI_CHAR, "location and name of annotation file");
			addOption("training", &training, 1, SSI_BOOL, "Enable (true) or disable (false) online training of model");
		}

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}
        void setEventName (const ssi_char_t *ename) {
            ssi_strcpy (this->ename, ename);
        }
        void setSenderName (const ssi_char_t *sname) {
            ssi_strcpy (this->sname, sname);
        }

        void setBaseModelName(const ssi_char_t *name){
            ssi_strcpy (this->model, name);
        }

        void setActualModelName(const ssi_char_t *name){
            ssi_strcpy (this->actualModel, name);
        }

		bool console;
		bool training;
		bool interval;
		bool useDecisionClass;
        bool oldSamplesUsage;
        ssi_char_t oldSamplesClass[SSI_MAX_CHAR];
		ssi_real_t intervalMin;
		ssi_real_t intervalMax;
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t model[SSI_MAX_CHAR];
		ssi_real_t confidence;
		ssi_char_t modelOutputPath[SSI_MAX_CHAR];
		ssi_char_t sampleOutputPath[SSI_MAX_CHAR];
        ssi_char_t decisionClass[SSI_MAX_CHAR];
        ssi_char_t ename[SSI_MAX_CHAR];
        ssi_char_t sname[SSI_MAX_CHAR];
        ssi_char_t answerValueName[SSI_MAX_CHAR];
        ssi_char_t annoFileLocation[SSI_MAX_CHAR];
        ssi_char_t actualModel[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName() { return "OnlineClassifier"; }

	static IObject *Create(const ssi_char_t *file) { return new OnlineClassifier(file); }

	~OnlineClassifier();

	Options *getOptions() { return &_options; }

	const ssi_char_t *getName() { return GetCreateName(); }

	const ssi_char_t *
	getInfo() { return "Applies classifier to a stream and outputs result as an event."; }

	void consume_enter(ssi_size_t stream_in_num,
					   ssi_stream_t stream_in[]);

	void consume(IConsumer::info consume_info,
				 ssi_size_t stream_in_num,
				 ssi_stream_t stream_in[]);

	void consume_flush(ssi_size_t stream_in_num,
					   ssi_stream_t stream_in[]);

	void listen_enter();

	bool update(IEvents &events,
				ssi_size_t n_new_events,
				ssi_size_t time_ms);

	void listen_flush();

	bool setEventListener(IEventListener *listener);

	const ssi_char_t *getEventAddress() {
		return _event_address.getAddress();
	}

	void wait() {
		fflush(stdin);
		ssi_print("\n");
		ssi_print_off("press enter to stop!\n\n");
		getchar();
	}

	void setLogLevel(int level) {
		ssi_log_level = level;
	}

protected:

	OnlineClassifier(const ssi_char_t *file = 0);

	OnlineClassifier::Options _options;
	ssi_char_t *_file;

    FILE* _annoFile;

	EventAddress _event_address;

	int ssi_log_level;
	static ssi_char_t ssi_log_name[];
	static ssi_char_t ssi_log_name_static[];

	bool dirCheck(const ssi_char_t *path);

	bool _is_loaded;
	ssi_size_t _n_classes;
	ssi_real_t *_probs;

	bool annotate(ssi_sample_t sample);

	void lernFromPredictionAndAnnotate(const int index, bool answer);

	int getOtherClass(const int classID);

	void handleAndTrain(ssi_time_t time,
						ssi_time_t dur,
						ssi_size_t max_ind,
						ssi_size_t glue,
						ssi_size_t n_streams,
						ssi_stream_t stream_in[]);

	ssi_time_t _time;
	ssi_time_t _duration;

	ssi_char_t *_label;
	ssi_char_t *_tier;
	ssi_char_t *_meta;
	ssi_char_t _string[SSI_MAX_CHAR];

	Mutex _mutex;
	Mutex _label_mutex;
    Mutex _deleteLock;

	ssi_size_t _n_metas;
	ssi_real_t *_metas;

	ssi_size_t glue_d = 0;
	IEventListener *_elistener = 0;

	ssi_size_t _merged_sample_dimension;

	EventHandler *_handler;

	ssi_time_t _consumer_sr;
	ssi_size_t _consumer_byte;
	ssi_size_t _consumer_dim;
	ssi_size_t _consumer_num;

	ssi_char_t *_lastTimestamp;
	ssi_size_t _timestampInkrement;



	IModel *_model;
	std::vector<ssi_char_t *> _n_class_names;

	bool readLine(FILE *fp = 0, ssi_size_t num = 0, ssi_char_t *string = 0);

	bool initModel(char *modelName);

	bool callModel(ssi_time_t time,
				   ssi_time_t dur,
				   ssi_size_t n_streams,
				   ssi_stream_t stream_in[],
				   ssi_size_t glue);

	ssi_size_t cycle_counter;

	char *getValue(const char *str, const char *delimiter, const char *value);

	std::map<ssi_size_t, std::pair<ssi_sample_t, ssi_time_t >> m;


};

}

#endif


