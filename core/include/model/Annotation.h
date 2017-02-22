// Annotation.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/11/04
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

#ifndef SSI_MODEL_ANNOTATION_H
#define SSI_MODEL_ANNOTATION_H

#include "SSI_Cons.h"

namespace ssi {

class Annotation {

public:

	Annotation ();
	virtual ~Annotation ();

	struct Entry {
		ssi_time_t start;
		ssi_time_t stop;
		ssi_size_t label_index;
	};

	void add (ssi_time_t start,
		ssi_time_t stop,
		const ssi_char_t *label);
	void add (Annotation::Entry &entry);

	void reset ();
	Annotation::Entry *next ();
	Annotation::Entry *next (ssi_size_t label_index);
	Annotation::Entry *last ();

	ssi_size_t size ();
	void clear ();

	ssi_size_t addLabel (const ssi_char_t *label);
	const ssi_char_t *getLabel (ssi_size_t index);
	ssi_size_t labelSize ();

	void print (FILE *file = stdout);

	/**
	 * Returns the entry corresponding to the given time
	 * @param time the timestamp at which we want to look (in seconds)
	 * @param buffer a time buffer zone (in seconds). This zone causes timestamps after an entries stop-value to be still matched if they are within this buffer zone.
	 * @return pointer to the matched entry
	 */
	Annotation::Entry *getEntryAt(ssi_time_t time, ssi_time_t max_delay = 0);

	void Annotation::trim(ssi_size_t offset, ssi_size_t size);

protected:

	struct entry_compare {
		bool operator() (const Annotation::Entry *lhs, const Annotation::Entry *rhs) const {
			if (lhs->start == rhs->start) {
				return lhs->stop < rhs->stop;
			} else {
				return lhs->start < rhs->start;
			}
		}
	};

	typedef std::set<Annotation::Entry *,entry_compare> entries_set_t;
	entries_set_t entries;
	entries_set_t::iterator entries_iter;
	std::vector<ssi_char_t *> labels;
	ssi_size_t counter;
};

}

#endif
