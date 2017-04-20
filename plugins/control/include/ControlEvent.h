// ControlEvent.h
// author: Dominik Schiller <dominik.schiller@student.uni-augsburg.de>
// created: 2014/08/03
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
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_ControlEvent_H
#define SSI_ControlEvent_H

#include "SSI_Define.h"

#ifndef SSI_USE_SDL

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"


namespace ssi {

	class ControlEvent : public IObject {

		class Options : public OptionList {

		public:

			Options()  {

			};
		};

	public:

		static const ssi_char_t *GetCreateName() { return "ControlEvent"; };
		static IObject *Create(const ssi_char_t *file) { return new ControlEvent(file); };
		~ControlEvent();

		Options *getOptions() { return &_options; };
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "..."; };

		static ssi_char_t *ssi_log_name;

		void listen_enter();
		bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
		void listen_flush();

	protected:
		ControlEvent(const ssi_char_t *file = 0);

		IObject *ControlEvent::parseCommand(const ssi_char_t *line, ssi_char_t **id, ssi_char_t **arg);
		bool ControlEvent::sendCommand(IObject *object, const ssi_char_t *id, const ssi_char_t *arg);
		Options _options;
		ssi_char_t *_file;

	};

}

#endif

#endif
