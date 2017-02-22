// MyOscListener.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/11/02
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

#include "ioput/socket/SocketOscListener.h"

class MyOscListener : public ssi::SocketOscListener {
	
public:

	void message (const char *from,
		const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		ssi::osc_int32 time,
		ssi::osc_int32 dur,
		const ssi_char_t *msg);

	void stream (const ssi_char_t *from,
		const ssi_char_t *id,
		ssi::osc_int32 time,
		float sr,
		ssi::osc_int32 num, 		
		ssi::osc_int32 dim,
		ssi::osc_int32 bytes,
		ssi::osc_int32 type,		
		void *data);

	void event (const char *from,
		const ssi_char_t *sender_id,
		const ssi_char_t *event_id,
		ssi::osc_int32 time,
		ssi::osc_int32 dur,
		ssi::osc_int32 state,
		ssi::osc_int32 n_events,
		const ssi_char_t **event_name,
		const ssi_real_t *event_value);

};
