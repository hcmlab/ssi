// TupleMap.h
// author: Florian Lingenfelser <florian.lingenfelser@informatik.uni-augsburg.de>
// created: 2013/10/15
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


#ifndef SSI_TUPLEMAP_H
#define SSI_TUPLEMAP_H

#include "base/IObject.h"
#include "ioput/option/OptionList.h"
#include "event/EventAddress.h"

namespace ssi {

class TupleMap : public IObject {

	class Options : public OptionList {

	public:

		Options () : dimension (1), mapped (true), negative (true) {

			setAddress("");
			setSenderName ("sender");
			setEventName ("event");	

			addOption("dimension", &dimension, 1, SSI_INT, "dimension of output-vector");
			addOption("mapped", &mapped, 1, SSI_BOOL, "use mapping or received value");
			addOption("negative", &negative, 1, SSI_BOOL, "allow negative values for unmapped events");
			mapping[0] = '\0';
			addOption("mapping", mapping, SSI_MAX_CHAR, SSI_CHAR, "mapping of dimensions (0.7, 1.0, 0.0, ...)");
			addOption("address", address, SSI_MAX_CHAR, SSI_CHAR, "event address (if sent to event board) (event@sender)");
			addOption("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event");

		};

		void setAddress(const ssi_char_t *address) {
			if (address) {
				ssi_strcpy(this->address, address);
			}
		}
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
		bool mapped;
		bool negative;
		ssi_char_t address[SSI_MAX_CHAR];
		ssi_char_t mapping[SSI_MAX_CHAR];
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t ename[SSI_MAX_CHAR];		
	};

public:

	static const ssi_char_t *GetCreateName () { return "TupleMap"; };
	static IObject *Create (const ssi_char_t *file) { return new TupleMap (file); };
	~TupleMap ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "..."; };

	void listen_enter ();
	bool update (IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);
	void listen_flush ();

	bool setEventListener (IEventListener *listener);
	const ssi_char_t *getEventAddress () {
		return _event_address.getAddress ();
	}

	virtual void createMapping (ssi_size_t ndim);
	virtual void setMapping (ssi_size_t nvalues, ssi_real_t* values);
	virtual void releaseMapping ();

protected:

	TupleMap (const ssi_char_t *file = 0);
	Options _options;
	static char ssi_log_name[];
	ssi_char_t *_file;
	int ssi_log_level;

	EventAddress _event_address;

	IEventListener *_listener;
	ssi_event_t _event;

	ssi_size_t _dim;
	bool _mapped;
	ssi_real_t *_mapping;

};

}

#endif
