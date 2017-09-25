// ElanTier.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/04/08
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

#include "ElanTier.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif
#if __gnu_linux__
using std::min;
using std::max;
#endif
namespace ssi {

ssi_char_t *ElanTier::ssi_log_name = "elantier__";

ElanTier::ElanTier (String name) 
: _name (name) {
}

ElanTier::~ElanTier () {
};

void ElanTier::unify (const char *string) {

	ElanTier::iterator it;
	for (it = begin (); it != end (); it++) {
		it->value = string;
	}
}

ssi_size_t ElanTier::invert (ElanTier &tier, String value, ssi_size_t tiersize) {

	ssi_size_t n_added = 0;

	ElanTier::iterator it = begin ();
	if (it->from > 0) {
		ElanAnnotation a;
		a.from = 0;
		a.to = it->from;
		a.value = value;
		n_added++;
		tier.push_back (a);
	}
	ssi_size_t last_to = it->to;
	for (it++; it != end (); it++) {
		ElanAnnotation a;
		a.from = last_to;
		a.to = it->from;
		a.value = value;
		n_added++;
		tier.push_back (a);
		last_to = it->to;
	}

	if (tiersize > last_to) {
		ElanAnnotation a;
		a.from = last_to;
		a.to = tiersize;
		a.value = value;
		n_added++;
		tier.push_back (a);
	}

	return n_added;
}

void ElanTier::sort () {

	std::sort (begin (), end (), SortAnnotationFunction);
}

ssi_size_t ElanTier::split (ElanTier &tier, ssi_size_t framesize, ssi_size_t deltasize, ssi_size_t dropsize) {
	
	if (dropsize >= framesize) {
		ssi_wrn ("dropsize '%u' > framesize '%u'", framesize, dropsize);
		return 0;
	}

	if (size () == 0) {
		return 0;
	}

	ssi_size_t n_added = 0;

	ssi_size_t pos = 0;	
	ssi_size_t maxsize = (end ()-1)->to;
	ssi_size_t n_frames = (maxsize - (framesize-deltasize)) / framesize;
	ElanAnnotation a;
	ElanTier::iterator current = begin ();
	for (ssi_size_t i = 0; i < n_frames; i++, pos += framesize) {

		if (pos >= current->to) {
			current++;
		}

		if (current->from >= pos + framesize) {
			continue;
		}
		
		ssi_size_t real_from = max (current->from, pos);
		ssi_size_t real_to = min (current->to, pos + framesize + deltasize);		

		if (real_to - real_from >= dropsize) {
			a.from = pos;
			a.to = pos + framesize + deltasize;
			a.value = current->value;
			tier.push_back (a);
			++n_added;
		}
	}

	return n_added;
}

ssi_size_t ElanTier::split (ElanTier &tier, ElanTier &inverse , String inverseValue, ssi_size_t framesize, ssi_size_t deltasize, ssi_size_t dropsize, ssi_size_t tiersize) {
	
	if (dropsize >= framesize) {
		ssi_wrn ("dropsize '%u' > framesize '%u'", framesize, dropsize);
		return 0;
	}

	if (size () == 0) {
		return 0;
	}

	ssi_size_t n_added = 0;

	ssi_size_t pos = 0;	
	ssi_size_t maxsize = (end ()-1)->to;
	ssi_size_t n_frames = (maxsize - (framesize-deltasize)) / framesize;
	ElanAnnotation a;
	ElanTier::iterator current = begin ();
	for (ssi_size_t i = 0; i < n_frames; i++, pos += framesize) {

		if (pos >= current->to) {
			current++;
		}

		if (current->from >= pos + framesize) {
			a.from = pos;
			a.to = pos + framesize + deltasize;
			a.value = inverseValue;
			inverse.push_back (a);			
			continue;
		}
		
		ssi_size_t real_from = max (current->from, pos);
		ssi_size_t real_to = min (current->to, pos + framesize + deltasize);		

		if (real_to - real_from >= dropsize) {
			a.from = pos;
			a.to = pos + framesize + deltasize;
			a.value = current->value;
			tier.push_back (a);
			++n_added;
		} else {
			a.from = pos;
			a.to = pos + framesize + deltasize;
			a.value = inverseValue;
			inverse.push_back (a);
		}
	}

	if (tiersize > pos) {
		ssi_size_t tailsize = tiersize - pos;
		if (tailsize > framesize-deltasize) {
			n_frames = (tailsize - (framesize-deltasize)) / framesize;
			for (ssi_size_t i = 0; i < n_frames; i++, pos += framesize) {
				a.from = pos;
				a.to = pos + framesize + deltasize;
				a.value = inverseValue;
				inverse.push_back (a);					
			}
		}
	}

	return n_added;
}

ssi_size_t ElanTier::pack (ElanTier &tier, const char *delim) {

	ssi_size_t n_added = 0;
	ElanTier::iterator it = begin ();
	ssi_size_t last_to = it->to;
	ElanAnnotation a;
	a.from = it->from;
	a.value = it->value;
	for (it++; it != end (); it++) {
		if (last_to < it->from) {
			a.to = last_to;
			tier.push_back (a);
			n_added++;
			a.from = it->from;
			a.value = it->value;
		} else {
			a.value += String (delim) + it->value;
		}
		last_to = it->to;
	}
	a.to = last_to;	
	tier.push_back (a);

	return n_added;
}

void ElanTier::print (FILE *file) {
	
	ssi_fprint (file, "TIER: %s\n", _name.str ());
	ElanTier::iterator it;
	for (it = begin (); it != end (); it++) {
		ssi_fprint (file, "\t%u-%u:%s\n", it->from, it->to, it->value.str ());
	}
}

}


