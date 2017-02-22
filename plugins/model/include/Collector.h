// Collector.h
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

#ifndef SSI_MODEL_COLLECTOR_H
#define SSI_MODEL_COLLECTOR_H

#include "base/IConsumer.h"
#include "signal/SignalCons.h"
#include "model/SampleList.h"
#include "thread/Event.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Collector : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: ask (false), iter (5) {

			user[0] = '\0';		
			classes[0] = '\0';
			addOption ("user", user, SSI_MAX_CHAR, SSI_CHAR, "user name");
			addOption ("classes", classes, SSI_MAX_CHAR, SSI_CHAR, "list of class names (separated by blank)");
			addOption ("iter", &iter, 1, SSI_INT, "number of iterations per class");
			addOption ("ask", &ask, 1, SSI_BOOL, "always ask before recording next sample");
		};

		void addClassName (const ssi_char_t *name) {			
			if (name) {
				size_t old_len = strlen (classes) + 1;
				size_t new_len = old_len + strlen (name) + 1;
				if (new_len <= SSI_MAX_CHAR) {
					classes[old_len - 1] = ' ';
					memcpy (classes + old_len, name, strlen (name) + 1);
				} else {
					ssi_wrn ("could not add class name");
				}
			}
		}
		void setUserName (const ssi_char_t *user) {
			this->user[0] = '\0';			
			if (user) {
				ssi_strcpy (this->user, user);
			}
		}

		ssi_char_t user[SSI_MAX_CHAR];
		ssi_char_t classes[SSI_MAX_CHAR];
		ssi_size_t iter;
		bool ask;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Collector"; };
	static IObject *Create (const ssi_char_t *file) { return new Collector (file); };
	~Collector ();

	Collector::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Transforms current stream to a sample and stores it to a sample list."; };

	void setSampleList (SampleList &samples) { _sample_list = &samples; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	void wait () {
		_event.wait ();
	};

protected:

	Collector (const ssi_char_t *file = 0);
	Collector::Options _options;
	ssi_char_t *_file;

	ssi_size_t _class_count;
	ssi_size_t _iter_count;
	ssi_size_t _n_classes;
	ssi_size_t _user_index;
	ssi_size_t *_class_index; 
	SampleList *_sample_list;

	Event _event;
	char _input[SSI_MAX_CHAR];
};

}

#endif
