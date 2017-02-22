// Annotation.cpp
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

#include "model/Annotation.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Annotation::Annotation () {
}

Annotation::~Annotation () {
	clear ();
}

void Annotation::clear () {

	for (ssi_size_t i = 0; i < labelSize (); i++) {
		delete[] labels[i];
	}
	labels.clear ();

	reset ();
	Annotation::Entry *entry;
	while (entry = next ()) {
		delete entry;
	}
	entries.clear ();
	reset ();
}

void Annotation::add (ssi_time_t start,
	ssi_time_t stop,
	const ssi_char_t *label) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "add new entry: %f %f %s\n", start, stop, label);

	// add label
	ssi_size_t label_index = addLabel (label);

	// add as new entry
	Annotation::Entry *entry = new Annotation::Entry;
	entry->start = start;
	entry->stop = stop;
	entry->label_index = label_index;
	std::pair<entries_set_t::iterator,bool> ret;
	ret = entries.insert (entry);
	if (ret.second == false) {
		delete entry;
	}
}

void Annotation::add (Annotation::Entry &e) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "add new entry: %f %f %s\n", e.start, e.stop, getLabel (e.label_index));

	// add as new entry
	Annotation::Entry *entry = new Annotation::Entry;
	entry->start = e.start;
	entry->stop = e.stop;
	entry->label_index = e.label_index;
	std::pair<entries_set_t::iterator,bool> ret;
	ret = entries.insert (entry);
	if (ret.second == false) {
		delete entry;
	}
}

void Annotation::reset () {
	entries_iter = entries.begin ();
}

Annotation::Entry *Annotation::next () {

	if (entries_iter == entries.end ()) {
		return 0;
	}
	
	return *entries_iter++;
}

Annotation::Entry *Annotation::next (ssi_size_t label_index) {

	while (entries_iter != entries.end () && (*entries_iter)->label_index != label_index)
		entries_iter++;

	if (entries_iter == entries.end ()) {
		return 0;
	}
	
	return *entries_iter++;
}

const ssi_char_t *Annotation::getLabel (ssi_size_t index) {

	if (index >= labelSize ()) {
		ssi_wrn ("index (%u) out of boundary (%u)", index, labelSize ());
		return 0;
	}

	return labels[index];
}

ssi_size_t Annotation::size () {
	return ssi_cast (ssi_size_t , entries.size ());
}

ssi_size_t Annotation::addLabel (const ssi_char_t *label) {

	SSI_DBG (SSI_LOG_LEVEL_DEBUG, "add new label: %s\n", label);

	// check if label exists
	ssi_size_t label_index = 0;
	bool found = false;
	for (ssi_size_t i = 0; i < labels.size (); i++) {
		if (strcmp (label, labels[i]) == 0) {
			label_index = i;
			found = true;
		}
	}

	// if necessary add as new label
	if (!found) {
		ssi_char_t *new_label = ssi_strcpy (label);
		labels.push_back (new_label);
		label_index = labelSize () - 1;
	}

	return label_index;
}

Annotation::Entry *Annotation::last () {
	
	entries_set_t::iterator i_end = entries.end();
	return *(--i_end);
}

ssi_size_t Annotation::labelSize () {
	return ssi_cast (ssi_size_t , labels.size ());
}

void Annotation::print (FILE *file) {

	reset ();
	Annotation::Entry *entry;
	while (entry = next ()) {
		fprintf (file, "%.2lf\t%.2lf\t%s\n", entry->start, entry->stop, labels[entry->label_index]);
	}
	reset ();
}
/*
bool Annotation::entry_compare (Annotation::Entry *lhs, Annotation::Entry *rhs) {
	return lhs->start <= rhs->start;
}*/

Annotation::Entry *Annotation::getEntryAt(ssi_time_t time, ssi_time_t max_delay)
{	
	entries_set_t::iterator i = entries.begin();
	entries_set_t::iterator i_end = entries.end();
	while(i != i_end)
	{
		if(time >= (*i)->start && time < (*i)->stop + max_delay)			
			return *i;
		++i;
	}
	return 0;
}

void Annotation::trim(ssi_size_t cut_front, ssi_size_t cut_back)
{
	//trim front
	{
		entries_set_t::iterator i = entries.begin();
		entries_set_t::iterator i_end = entries.end();
		for (ssi_size_t k = 0; k < cut_front; k++)
			i++;

		entries.erase(entries.begin(), i);
	}

	//trim back
	{
		entries_set_t::iterator i = entries.end();
		entries_set_t::iterator i_end = entries.begin();
		for (ssi_size_t k = 0; k < cut_back; k++)
			i--;

		entries.erase(i, entries.end());
	}
}

}
