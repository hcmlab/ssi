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

#define SSI_CLASSIFIER_MAXHANDLER 5

namespace ssi {

class Trainer;

class Classifier : public IConsumer {

public:

	class EventHandler {

	public:

		EventHandler(IEventListener *listener,
			ssi_size_t sid,
			ssi_size_t eid,
			bool winner_only,
			ssi_char_t *select);
		virtual ~EventHandler();
		void handle(ssi_time_t time,
			ssi_time_t duration,
			ssi_size_t n_classes,
			ssi_size_t class_index,
			const ssi_real_t *probs,
			ssi_char_t *const*class_names,
			ssi_size_t n_metas,
			ssi_real_t *metas);

	protected:

		ssi_event_t _event;
		IEventListener *_listener;
		bool _winner_only;
		ssi_size_t _n_select;
		int *_select;
		ssi_size_t *_class_ids;
	};

public:

	class Options : public OptionList {

	public:

		Options () 
			: console(false), merge(false), winner(false), pthres(0) {

			setAddress("");

			trainer[0] = '\0';
			ename[0] = '\0';
			sname[0] = '\0';
			select[0] = '\0';

			SSI_OPTIONLIST_ADD_ADDRESS(address);

			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board) [deprecated, see address]");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board) [deprecated, see address]");
			addOption ("trainer", trainer, SSI_MAX_CHAR, SSI_CHAR, "filepath of trainer");			
			addOption ("pthres", &pthres, 1, SSI_REAL, "probablity threshold");			
			addOption ("merge", &merge, 1, SSI_BOOL, "in case of multiple streams merge to single stream");	
			addOption ("console", &console, 1, SSI_BOOL, "output classification to console");			
			addOption ("winner", &winner, 1, SSI_BOOL, "send winning class only");		
			addOption ("select", select, SSI_MAX_CHAR, SSI_CHAR, "foward only specific classes (indices separated by ',') [ignored if winner=true]");
		};

		void setTrainer (const ssi_char_t *filepath) {
			ssi_strcpy (trainer, filepath);
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

	virtual void setTrainer (Trainer *trainer);
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

	Trainer *_trainer;
	bool _is_loaded;
	bool _del_trainer;
	ssi_size_t _n_classes;
	ssi_real_t *_probs;
	static void LoadTrainer(void *arg);
	bool loadTrainer(Trainer *trainer = 0); // if 0 loaded from options
	bool callTrainer(ssi_time_t time,
		ssi_time_t dur,
		ssi_size_t n_streams,
		ssi_stream_t stream_in[]);
	void releaseTrainer();
	Mutex _mutex;

	ssi_size_t _n_metas;
	ssi_real_t *_metas;

	ssi_size_t _merged_sample_dimension;
	
	EventHandler *_handler;

	ssi_time_t _consumer_sr;
	ssi_size_t _consumer_byte;
	ssi_size_t _consumer_dim;;
	ssi_size_t _consumer_num;	

};

}

#endif


