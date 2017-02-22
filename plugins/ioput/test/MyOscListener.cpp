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

#include "MyOscListener.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

void MyOscListener::message (const char *from,
	const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	ssi::osc_int32 time,
	ssi::osc_int32 dur,
	const ssi_char_t *msg) {

	ssi_print("received message %s@%s from %s:\n%s\n", sender_id, event_id, from, msg);
};

void MyOscListener::stream (const ssi_char_t *from,
	const ssi_char_t *id,
	ssi::osc_int32 time,
	float sr,
	ssi::osc_int32 num, 		
	ssi::osc_int32 dim,
	ssi::osc_int32 bytes,
	ssi::osc_int32 type,		
	void *data) {

	ssi_print ("received stream from %s:\n", from);
	ssi_print ("  rate\t= %.2fhz\n\
  dim\t= %d\n\
  bytes\t= %d\n\
  num\t= %d\n\
  time\t= %dms\n\
  type\t= %s\n",
	sr, dim, bytes, num, time, SSI_TYPE_NAMES[type]);

	float *ptr = ssi_pcast (float, data);
	for (ssi::osc_int32 i = 0; i < num; i++) {
		for (ssi::osc_int32 j = 0; j < dim; j++) {			
			ssi_print ("%.2f ", *ptr++);
		}
		ssi_print ("\n"); 
	}	
};

void MyOscListener::event (const char *from,
	const ssi_char_t *sender_id,
	const ssi_char_t *event_id,
	ssi::osc_int32 time,
	ssi::osc_int32 dur,
	ssi::osc_int32 state,
	ssi::osc_int32 n_events,
	const ssi_char_t **names,
	const ssi_real_t *values) {

	ssi_print("received event %s@%s from %s (time=%dms, dur=%dms, %s):\n", sender_id, event_id, from, time, dur, state == SSI_ESTATE_COMPLETED ? "completed" : "continued");
	for (ssi::osc_int32 i = 0; i < n_events; i++) {
		ssi_print ("%s: %.2f\t", names[i], values[i]);
	}
	ssi_print ("\n");
};
