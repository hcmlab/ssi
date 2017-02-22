// OSPitchDirection.h
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2013/07/01 
// Copyright (C) University of Augsburg, Ionut Damian
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

/*  openSMILE component:

read pitch data and compute pitch direction estimates
input fields: F0 or F0env


This component detects:

pseudo-syllables, pseudo-syllable rate

pitch contour per pseudo syllable:
rise/fall
fall/rise
rise
fall
flat

genral pitch contour per syllable
rise
fall
flat

syllable length, mean syllable length (TODO: use energy here and adjacent syllables for more accurate results..?)

*/

#pragma once

#ifndef SSI_OPENSMILE_PITCHDIR_H
#define SSI_OPENSMILE_PITCHDIR_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class OSPitchDirection : public IFilter {

public:
	struct meta_s {
		int pos_F0direction;
		int pos_directionScore;
		int pos_speakingRate;
		int pos_F0avg;
		int pos_F0smooth;
	};

public:

	class Options : public OptionList {

	public:

		Options ()
		{	
			setSender ("OSPitchDirection");
			setEvent ("PitchDirection");
			
			ltbs = 0.20f;
			stbs = 0.05f;
			speakingRateBsize = 100;
			F0direction = true;
			directionScore = true;
			speakingRate = false;
			F0avg = false;
			F0smooth = false;
			onlyTurn = false;

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board)");

			addOption("ltbs", &ltbs, 1, SSI_REAL, "The size of the long-term average buffer in seconds");
			addOption("stbs", &stbs, 1, SSI_REAL, "The size of the short-term average buffer in seconds");
			addOption("speakingRateBsize", &speakingRateBsize, 1, SSI_SIZE, "The buffer size for computation of speaking rate (in input frames, typical frame rate 100 fps)");
			addOption("F0direction", &F0direction, 1, SSI_BOOL, "1 = enable output of F0 direction as numeric value (fall: -1.0 / flat: 0.0 / rise: 1.0)");
			addOption("directionScore", &directionScore, 1, SSI_BOOL, "1 = enable output of F0 direction score (short term mean - long term mean)");
			addOption("speakingRate", &speakingRate, 1, SSI_BOOL, "1 = enable output of current speaking rate in Hz (is is output for every frame, thus, a lot of redundancy here)");
			addOption("F0avg", &F0avg, 1, SSI_BOOL, "1 = enable output of long term average F0");
			addOption("F0smooth", &F0smooth, 1, SSI_BOOL, "1 = enable output of exponentially smoothed F0");
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

		ssi_real_t ltbs;
		ssi_real_t stbs;		
		ssi_size_t speakingRateBsize;
		bool F0direction;
		bool directionScore;
		bool speakingRate;
		bool F0avg;
		bool F0smooth;
		bool onlyTurn;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSPitchDirection"; };
	static IObject *Create (const ssi_char_t *file) {  return new OSPitchDirection (file); };
	~OSPitchDirection ();

	OSPitchDirection::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Reads pitch data and compute pitch direction estimates"; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in < 2) {
			ssi_err ("dimension < 2 not supported. Pitch f0 and f0env required.");
		}
		
		int n = 0;
		if(_options.F0direction)	++n;
		if(_options.directionScore)	++n;
		if(_options.F0avg)			++n;
		if(_options.F0smooth)		++n;
		if(_options.speakingRate)	++n;

		return n;
	};

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	};

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	};
	
	//event sender
	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	};

	const void *getMetaData (ssi_size_t &size) { 

		//the position of each result in the output stream
		int n = 0;
		_meta_out.pos_directionScore = -1;
		_meta_out.pos_F0avg = -1;
		_meta_out.pos_F0direction = -1;
		_meta_out.pos_F0smooth = -1;
		_meta_out.pos_speakingRate = -1;
		
		if(_options.F0direction)	_meta_out.pos_F0direction = n++;
		if(_options.directionScore)	_meta_out.pos_directionScore = n++;
		if(_options.speakingRate)	_meta_out.pos_speakingRate = n++;
		if(_options.F0avg)			_meta_out.pos_F0avg = n++;
		if(_options.F0smooth)		_meta_out.pos_F0smooth = n++;
		
		size = sizeof (_meta_out);
		return &_meta_out;
	};
	
	void setMetaData (ssi_size_t size, const void *meta) {
		memcpy (_meta_in_ptr, meta, size);
		_meta_in_ptr += size; //if there are multiple meta's, we will concatenate them
	};

protected:

	OSPitchDirection (const ssi_char_t *file = 0);

	bool handleEvent(IEventListener *listener, ssi_event_t* ev, ssi_size_t class_id, ssi_time_t time);
	inline double round(double x) { return floor(x + 0.5); };
	
	ssi_real_t _meta_in[256];
	ssi_real_t *_meta_in_ptr;
	meta_s _meta_out;

	OSPitchDirection::Options _options;
	ssi_char_t *_file;
	static ssi_char_t ssi_log_name[];
	
	bool _first_call;

	IEventListener *_elistener;
	EventAddress _event_address;
	ssi_event_t _event;	

	//ssi_time_t _ev_tstart;
	//ssi_time_t _ev_tstart_old;
	//ssi_time_t _ev_dur;
	
	unsigned int pitch_streamId, e_streamId;
	unsigned int f0_offset, f0env_offset, e_offset;

    long stbsFrames, ltbsFrames;
    FLOAT_DMEM * stbuf, * ltbuf;
    FLOAT_DMEM F0non0, lastF0non0, f0s;
    long stbufPtr, ltbufPtr; /* ring-buffer pointers */
    long bufInit; /* indicates wether buffer has been filled and valid values are to be expected */
    double ltSum, stSum;
    FLOAT_DMEM longF0Avg;
    long nFall,nRise,nFlat;

    int insyl;
    int f0cnt;
    FLOAT_DMEM lastE;
    FLOAT_DMEM startE, maxE, minE, endE;
    long sylen, maxPos, minPos, sylenLast;
    long sylCnt;
    double inpPeriod, timeCnt, lastSyl;

    FLOAT_DMEM startF0, lastF0, maxF0, minF0;
    long maxF0Pos, minF0Pos;

    /* speaking rate variables (buf0 is always the first half of buf1) */
    int nBuf0,nBuf1; /* number of frames collected in bufferA and bufferB */
    int nSyl0,nSyl1; /* number of syllable starts in bufferA and bufferB */
    double curSpkRate;
};

}

#endif
