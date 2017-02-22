// ITransformer.cpp
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

#include "ARTKPlusSelector.h"

namespace ssi {

ARTKPlusSelector::ARTKPlusSelector (const ssi_char_t *file) 
	: _file (0),
	_n_selected (0) {

	for (ssi_size_t i = 0; i < MAX_SELECT; i++) {
		_selector[i] = 0;
	}

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

ARTKPlusSelector::~ARTKPlusSelector () {

	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void ARTKPlusSelector::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	if (_options.indices[0] != '\0') {
		releaseSelection ();
		parseSelection ();
	}
}

void ARTKPlusSelector::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ARTKPlusTools::marker_s *ss = ssi_pcast (ARTKPlusTools::marker_s, stream_in.ptr);
	ssi_real_t *outptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	for (ssi_size_t i = 0; i < stream_in.num; i++) {
		for (ssi_size_t j = 0; j < stream_in.dim; j++) {
			for (ssi_size_t k = 0; k < _n_selected; k++) {
				*outptr++ = _selector[k] (ss[j]);
			}
		}
		ss += stream_in.dim;
	}
}

void ARTKPlusSelector::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {
}

void ARTKPlusSelector::parseSelection () {

	ssi_size_t n_inds = 0;
	int *inds = ssi_parse_indices (_options.indices, n_inds);
	for (ssi_size_t i = 0; i < n_inds; i++) {
		switch (inds[i]) {
			case 0 : visible (); break;
			case 1 : id (); break;
			case 2 : center_X (); break;
			case 3 : center_Y (); break;			
			default : ssi_wrn ("index out of range '%d'", inds[i]);
		}
	}
	delete[] inds;
}

void ARTKPlusSelector::releaseSelection () {

	for (ssi_size_t i = 0; i < MAX_SELECT; i++) {
		_selector[i] = 0;
	}
	_n_selected = 0;
}

}
