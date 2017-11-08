// INotify.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/11/20
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

#ifndef SSI_INOTIFY_H
#define SSI_INOTIFY_H

#include "SSI_Cons.h"

namespace ssi {

	class INotify {

	public:		

		struct COMMAND {
			enum List {
				MESSAGE = 0,
				SLEEP_INIT,
				SLEEP_PRE,
				SLEEP_POST,
				WAKE_INIT,
				WAKE_PRE,
				WAKE_POST,
				OPTIONS_CHANGE,
				RESET,
				WINDOW_MOVE,
				WINDOW_SHOW,
				WINDOW_HIDE,
				MINMAX_SHOW,
				MINMAX_HIDE,
			};
		};

		virtual bool notify(COMMAND::List command, const ssi_char_t *message = 0) {
			return false;
		};

	};

}

#endif
