// VectorSenderContinuous.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2012/04/25
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

#ifndef _VECTORSENDERCONTINUOUS_H
#define _VECTORSENDERCONTINUOUS_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"
#include "base/ISamples.h"

namespace alglib {
	class linearmodel;
}

namespace ssi {

class VectorSenderContinuous : public IObject{

	class Options : public OptionList {

	public:

		Options () : dimension(0){

			setSenderName ("vector_sender");
			setEventName ("vector_event");	

			addOption("dimension", &dimension, 1, SSI_INT, "dimension of output-vector");
			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");

		};

		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}
		void setEventName (const ssi_char_t *ename) {
			if (ename) {
				ssi_strcpy (this->ename, ename);
			}
		}

		ssi_size_t dimension;
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];		
	};

public:

	static const ssi_char_t *GetCreateName () { return "VectorSenderContinuous"; };
	static IObject *Create (const ssi_char_t *file) { return new VectorSenderContinuous (file); };
	~VectorSenderContinuous ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "takes events, mapps them into the vector space - using pretrained linear regression - and forwards them to the vector fusion."; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	virtual bool train (ISamples &samples, ssi_size_t data_index, ssi_size_t anno_index);
	virtual void save_lm(const ssi_char_t *filepath);
	virtual void load_lm(const ssi_char_t *filepath);

	virtual void createMapping (ssi_size_t ndim);
	virtual void setMapping (ssi_size_t nvalues, ssi_real_t* values);
	virtual void releaseMapping ();

protected:

	VectorSenderContinuous (const ssi_char_t *file = 0);
	Options _options;
	static char ssi_log_name[];
	ssi_char_t *_file;
	int ssi_log_level;

	EventAddress _event_address;

	IEventListener *_listener;
	ssi_event_t _event;

	ssi_size_t _dim;
	ssi_size_t _data_index;
	ssi_size_t _anno_index;
	bool _trained;

	alglib::linearmodel* _lm;

	ssi_real_t *_mapping;

};

}

#endif
