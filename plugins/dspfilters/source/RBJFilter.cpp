// RBJFilter.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/06/26
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

#include "DspFilters\Dsp.h"
#include "RBJFilter.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *RBJFilter::ssi_log_name = "rbjfilt___";

RBJFilter::RBJFilter (const ssi_char_t *file)
	: _file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

RBJFilter::~RBJFilter () {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

Dsp::Filter *RBJFilter::createFilter (DspFiltersTools::FilterProperty &prop) {

	Dsp::Filter *f = 0;

	prop.family = DspFiltersTools::RBJ;	
	prop.type = ssi_cast (DspFiltersTools::FILTER_TYPE, _options.type);
	prop.state = _options.state;	
	prop.smooth = _options.smooth;
	prop.offset = _options.offset;
	
	DspFiltersTools::CreateFilter (&f, _prop);	
	
	f->setParams (f->getDefaultParams ());		
	f->setParam (0, prop.rate);		
	if (_options.type == HIGHSHELF) {
		double freq = (prop.rate/2) - _options.freq; // seems to be a bug
		f->setParam (1, _options.norm ? freq * (prop.rate / 2) : freq);
	} else {
		f->setParam (1, _options.norm ? _options.freq * (prop.rate / 2) : _options.freq);
	}
	if (_options.type == LOW || _options.type == HIGH || _options.type == ALL) {
		f->setParam (2, _options.q);
	}
	if (_options.type == BAND1 || _options.type == BAND2 ||_options.type == STOP) {
		f->setParam (2, _options.norm ? _options.width * (prop.rate / 2) : _options.width);
	}	
	if (_options.type == LOWSHELF || _options.type == HIGHSHELF ) {
		f->setParam (2, _options.gain);
		f->setParam (3, _options.slope);
	}	
	if (_options.type == BANDSHELF) {
		f->setParam (2, _options.gain);
		f->setParam (3, _options.norm ? _options.width * (prop.rate / 2) : _options.width);
	}	

	return f;
}		

}
