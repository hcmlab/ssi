// FileAnnotationWriter.h
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

#pragma once

#ifndef SSI_IOPUT_FILEANNOTATIONWRITER_H
#define SSI_IOPUT_FILEANNOTATIONWRITER_H

#include "thread/Mutex.h"
#include "thread/Lock.h"
#include "base/IEvents.h"
#include "ioput/option/OptionList.h"
#include "base/Factory.h"

namespace ssi {
	class FileAnnotationWriter : public IEventListener {
	public:

		class Options : public OptionList {
		public:

			Options() : eventNameAsLabel(false), eventNameaAsTier(false), senderNameAsTier(false) {
				addOption("eventnameaslabel", &eventNameAsLabel, 1, SSI_BOOL, "use the eventname as label");
				addOption("eventnameastier", &eventNameaAsTier, 1, SSI_BOOL, "use the eventname as tier");
				addOption("sendernameastier", &senderNameAsTier, 1, SSI_BOOL, "use the sendername as tier");
			};

			bool eventNameAsLabel;
			bool eventNameaAsTier;
			bool senderNameAsTier;
		};

	public:

		FileAnnotationWriter(const char *filename = 0,
			const ssi_char_t *label = 0, const ssi_char_t *tier = 0);
		~FileAnnotationWriter();
		FileAnnotationWriter::Options *getOptions() { return &_options; };

		bool update(ssi_event_t &e);
		ssi_size_t getStringId(const ssi_char_t *str) {
			return 0;
		}
		ssi_size_t getGlueId() {
			return SSI_FACTORY_UNIQUE_INVALID_ID;
		}
		void setLabel(const ssi_char_t *label);
		void setTier(const ssi_char_t *tier);

	protected:

		FILE *_file;
		ssi_char_t _string[SSI_MAX_CHAR];
		FileAnnotationWriter::Options _options;
		ssi_char_t *_filename;

		ssi_time_t _time;
		ssi_time_t _duration;

		ssi_char_t *_label;
		ssi_char_t *_tier;
		ssi_char_t *_meta;
		Mutex _label_mutex;
		Mutex _tier_mutex;
	};
}

#endif
