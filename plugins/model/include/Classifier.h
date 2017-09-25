// Classifier.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/03/04
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

#ifndef SSI_MODEL_CLASSIFIER_H
#define SSI_MODEL_CLASSIFIER_H

#include "base/IConsumer.h"
#include "signal/SignalCons.h"
#include "ioput/file/File.h"
#include "ioput/socket/SocketOsc.h"
#include "ioput/option/OptionList.h"
#include "base/IEvents.h"
#include "event/EventAddress.h"
#include "thread/Lock.h"
#include "ClassifierHelper.h"

#define SSI_CLASSIFIER_MAXHANDLER 5

namespace ssi {

class Trainer;

class Classifier : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options () 
			: console(false), merge(false), flat(false), winner(false), pthres(0) {

			setAddress("");

			trainer[0] = '\0';
			ename[0] = '\0';
			sname[0] = '\0';
			select[0] = '\0';

			SSI_OPTIONLIST_ADD_ADDRESS(address);

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");			
			addOption("path", trainer, SSI_MAX_CHAR, SSI_CHAR, "path to trainer 'name:filepath' (if several separate by ;)");			
			addOption("pthres", &pthres, 1, SSI_REAL, "probablity threshold");			
			addOption("merge", &merge, 1, SSI_BOOL, "in case of multiple streams merge to single stream");	
			addOption("flat", &flat, 1, SSI_BOOL, "in case of multiple samples merge to single sample");
			addOption("console", &console, 1, SSI_BOOL, "output classification to console");			
			addOption("winner", &winner, 1, SSI_BOOL, "send winning class only");		
			addOption("select", select, SSI_MAX_CHAR, SSI_CHAR, "foward only specific classes (indices separated by ',') [ignored if winner=true]");

			addOption("trainer", trainer, SSI_MAX_CHAR, SSI_CHAR, "filepath of trainer [deprecated use 'path']");
		};

		void addTrainer (const ssi_char_t *name, const ssi_char_t *path) {
			if (trainer[0] == '\0')
			{
				ssi_sprint(trainer, "%s:%s", name, path);
			}
			else
			{
				ssi_sprint(trainer, "%s;%s:%s", trainer, name, path);
			}
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
		
		ssi_char_t trainer[SSI_MAX_CHAR];	
		ssi_real_t pthres;
		bool merge;
		bool console;
		bool flat;
		bool winner;
		ssi_char_t select[SSI_MAX_CHAR];
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "Classifier"; };
	static IObject *Create (const ssi_char_t *file) { return new Classifier (file); };
	~Classifier ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Applies classifier to a stream and outputs result as an event."; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void listen_enter();
	bool update(IEvents &events, 
		ssi_size_t n_new_events, 
		ssi_size_t time_ms);
	void listen_flush();

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

    bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	virtual void addTrainer (const ssi_char_t *name, Trainer *trainer);
	void wait () {
		fflush (stdin);
		ssi_print("\n");
		ssi_print_off("press enter to stop!\n\n");
		getchar ();
	};
		
	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	Classifier (const ssi_char_t *file = 0);
	Classifier::Options _options;
	ssi_char_t *_file;

	EventAddress _event_address;

	int ssi_log_level;
	static ssi_char_t ssi_log_name[];
	static ssi_char_t ssi_log_name_static[];

	ClassifierHelper _helper;
	bool loadTrainerFromOptions();
	bool _loadedTrainerFromOptions;
	bool predict(ssi_time_t time,
		ssi_time_t dur,
		ssi_size_t n_streams,
		ssi_stream_t stream_in[]);
	void release();

	ssi_event_t _event;
	IEventListener *_listener;
	bool _winner_only;
	ssi_size_t _n_select;
	int *_select;

	ssi_size_t _merged_sample_dimension;

};

}

#endif


