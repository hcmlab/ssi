// OSVad.h
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2012/10/16 
// Copyright (C) 2007-12 University of Augsburg, Ionut Damian
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

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *******************************************************************************/

/**

agorithmic voice activity detection

*/

#pragma once

#ifndef SSI_OPENSMILE_VAD_H
#define SSI_OPENSMILE_VAD_H

#define SSI_OPENSMILE_VAD_NINIT  50  // NINIT must be < FTBUF!!
#define SSI_OPENSMILE_VAD_FTBUF  100
#define SSI_OPENSMILE_VAD_FUZBUF 10
#define VAD_FLOAT float

#include "base/IConsumer.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class OSVad : public IConsumer {

public:

	enum TYPE {
		RMS = 0,
		LOG = 1,
		BOTH = 2
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: threshold (-13.0f), disableDynamicVAD(false), minvoicedur(0.1f), minsilencedur(0.5f) {
				
			setSender ("OSVad");
			setEvent ("VoiceActivity");	
			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board)");
			addOption ("minvoicedur", &minvoicedur, 1, SSI_REAL, "minimum voice activity duration to trigger event (in seconds)");
			addOption ("minsilencedur", &minsilencedur, 1, SSI_REAL, "minimum silence duration to trigger event (in seconds)");
			addOption ("threshold", &threshold, 1, SSI_REAL, "The minimum rms/log energy threshold to use (or the actual rms energy threshold, if disableDynamicVAD==1)"); // 0.0005	
			addOption ("disableDynamicVAD", &disableDynamicVAD, 1, SSI_BOOL, "1/0 = yes/no, whether dynamic VAD is disabled (default is enabled)");	
		};

		void setSender (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}

		void setEvent (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}
		
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];	
	ssi_real_t minvoicedur;
	ssi_real_t minsilencedur;
		bool disableDynamicVAD;
		ssi_real_t threshold;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSVad"; };
	static IObject *Create (const ssi_char_t *file) { return new OSVad (file); };
	~OSVad ();

	OSVad::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "A voice activity detector based on Line-Spectral-Frequencies, energy and pitch. This component requires input of the following type in the following order: LSP, pitch, energy. Requires 10ms blocks!"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	//event sender
	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}
	
	void setMetaData (ssi_size_t size, const void *meta) {
		memcpy (_meta_ptr, meta, size);
		_meta_ptr += size; //if there are multiple meta's, we will concatenate them
	}

protected:

	OSVad (const ssi_char_t *file = 0);
	bool handleEvent(bool condition, ssi_time_t min_dur, ssi_time_t time, const char* ev_value, ssi_estate_t ev_state);
	VAD_FLOAT pitchVariance(VAD_FLOAT curF0raw);
	
	ssi_real_t _meta[256];
	ssi_real_t *_meta_ptr;

	OSVad::Options _options;
	ssi_char_t *_file;
	static ssi_char_t ssi_log_name[];
	
	IEventListener *_elistener;
	EventAddress _event_address;
	ssi_event_t _event;	
	bool _has_voiceactivity;

	ssi_time_t _ev_tstart;
	ssi_time_t _ev_tstart_old;
	ssi_time_t _ev_dur;
	
	long specN, lsfN;
	unsigned int spec_streamId, voiceProb_streamId, e_streamId;
	unsigned int spec_offset, voiceProb_offset, e_offset;

	VAD_FLOAT *spec;
	int t0histIdx; int vadBin;
	VAD_FLOAT t0hist[8];
	VAD_FLOAT div0;
	VAD_FLOAT turnSum, turnN;

	// history for smooting:
	VAD_FLOAT f0v_0, ent_0, E_0;
	VAD_FLOAT ar0, ar1, arU, arV;
	
	VAD_FLOAT vadFuzH[SSI_OPENSMILE_VAD_FUZBUF];
	int vadFuzHidx;

	VAD_FLOAT minE, minEn, maxEn;

	VAD_FLOAT F0vH[SSI_OPENSMILE_VAD_FTBUF], entH[SSI_OPENSMILE_VAD_FTBUF], EH[SSI_OPENSMILE_VAD_FTBUF];

	// adaptive thresholds:
	long nInit;
	VAD_FLOAT uF0v, uEnt, uE;
	VAD_FLOAT vF0v, vEnt, vE;
	VAD_FLOAT tuF0v, tuEnt, tuE;
	VAD_FLOAT tvF0v, tvEnt, tvE;
	int F0vHidx, entHidx, EHidx;
	VAD_FLOAT tF0vH[SSI_OPENSMILE_VAD_FTBUF], tentH[SSI_OPENSMILE_VAD_FTBUF], tEH[SSI_OPENSMILE_VAD_FTBUF];
	int tF0vHidx, tentHidx, tEHidx;
	int nInitT, nInitN;
};

}

#endif
