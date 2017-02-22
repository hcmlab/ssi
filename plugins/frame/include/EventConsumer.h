// EventConsumer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/10/14
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

#pragma once

#ifndef SSI_EVENT_EVENTCONSUMER_H
#define SSI_EVENT_EVENTCONSUMER_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "ConsumerBase.h"
#include "base/ITransformable.h"
#include "base/ITheEventBoard.h"
#include "thread/Thread.h"
#include "thread/Event.h"

namespace ssi {

class Monitor;

class EventConsumer : public IObject, public Thread {

friend class TheFramework;

public:

	class Options : public OptionList {

	public:

		Options () 
			: async (false) {

			addOption ("async", &async, 1, SSI_BOOL, "process consumer asynchronously (update call is not blocked)");			
		};

		bool async;
	};

public: 	

	static const ssi_size_t MAX_CONSUMER;

	static const ssi_char_t *GetCreateName () { return "EventConsumer"; };
	static IObject *Create (const ssi_char_t *file) { return new EventConsumer (file); };
	~EventConsumer ();

	EventConsumer::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Turns a regular consumer into an event listener."; };

	virtual bool AddConsumer (ITransformable *source, IConsumer *consumer, ITransformer *transformer = 0);
	virtual bool AddConsumer (ssi_size_t n_sources, ITransformable **sources, IConsumer *consumer, ITransformer **transformers = 0);
	virtual void clear ();

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	EventConsumer (const ssi_char_t *file = 0);
	EventConsumer::Options _options;
	ssi_char_t *_file;

	bool _async;
	void consume (IConsumer::info info);

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	void terminate ();
	void run ();

	ConsumerBase **_consumer;
	ssi_size_t _consumer_count;
	
	bool _terminate;

	Mutex _update_mutex;
	Event _update_event;
	IConsumer::info _update_info;

	TheFramework *_frame;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
};

}

#endif
