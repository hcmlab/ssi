// GazeType.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/06/23
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

#ifndef SSI_SMI_GAZETYPE_H
#define SSI_SMI_GAZETYPE_H

#include "base/ITransformer.h"
#include "../../shore/include/ShoreTools.h"
#include "ioput/option/OptionList.h" 
#include "event/EventAddress.h"


typedef struct _IplImage IplImage;


namespace ssi{
class GazeType : public ITransformer {

public:

	class Options : public OptionList {
	public:

		Options ()
			: detectFaces(false), detectGazeKind(false), percent(false), dataIsFlipped(true) {
			
			setSender ("GazeType");
			setEvent ("DetectedObject");	
			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if sent to event board)");
			addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if sent to event board)");
			addOption ("detectFaces",&detectFaces,1, SSI_BOOL, "detect faces ");
			addOption ("detectGazeKind",&detectGazeKind,1, SSI_BOOL, "detects Business, Social, Intiminate Gaze");			
			addOption ("percent",&percent,1, SSI_BOOL, "outputs gaze target durations in percent rather than seconds");	
			addOption ("dataIsFlipped",&dataIsFlipped,1, SSI_BOOL, "flag indicating whether video/gaze data is flipped");
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
		bool detectFaces;
		bool detectGazeKind;
		bool percent;
		bool dataIsFlipped;
	};

public:

	static const ssi_char_t *GetCreateName () { return "GazeType"; };
	static IObject *Create (const ssi_char_t *file) { return new GazeType (file); };
	~GazeType ();
	GazeType::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Sends events whenever a object is detected"; };
	
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

		ssi_size_t n = 0;
		if(_options.detectFaces)		n += 1;
		if(_options.detectGazeKind)		n += 3;

		return n; 
	};

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 

		return sizeof(ssi_real_t); 
	};	

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {

		return SSI_REAL;
	};

	ssi_size_t getSampleNumberOut (ssi_size_t sample_number_in) {
		
		return 1;
	};

	//event sender
	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

protected:

	GazeType (const ssi_char_t *file = 0);
	GazeType::Options _options;
	ssi_char_t *_file;

	IEventListener *_elistener;
	EventAddress _event_address;
	ssi_time_t _start, _last;
	
	ssi_event_t _ev_businessgaze;
	ssi_event_t _ev_socialgaze;
	ssi_event_t _ev_intimategaze;
	ssi_event_t _ev_face;

	ssi_size_t tempglueidface;
	ssi_size_t tempglueidobject;
	ssi_size_t tempglueidcircle;
	ssi_size_t tempstate;
	ssi_size_t tempdurface, tempdurobject,tempdurcircle, tempdurbusiness, tempdursocial, tempdurintimate;

	bool facetracked;
	bool businessgazetracked;
	bool socialgazetracked;
	bool intimategazetracked;
	bool facetrackinglost;
	bool lookatface;
};

}

#endif
