// ARTKPlusSelector.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/08
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

#ifndef SSI_ARTKPLUS_SELECTOR_H
#define SSI_ARTKPLUS_SELECTOR_H

#include "base/IFilter.h"
#include "ARTKPlusTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class ARTKPlusSelector : public IFilter {

static const ssi_size_t MAX_SELECT = 32;
#define SSI_ARTKPLUS_SELECTOR_CHECK(_n_selected) {\
	if (++_n_selected >= MAX_SELECT) { \
		ssi_wrn ("too many selections"); \
		return; \
	}; \
}

public:


class Options : public OptionList {

	public:

		Options () {

			indices[0] = '\0';
			addOption ("indices", indices, SSI_MAX_CHAR, SSI_CHAR, "indices of selected fields (0,1,2,..) (0=visible, 1=id, 2=center_X 3=center_Y");		
		};

		void set (ssi_size_t index) {
			set (1, &index);
		}

		void set (ssi_size_t n_inds, ssi_size_t *inds) {
			indices[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%u", inds[0]);
				strcat (indices, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%u", inds[i]);
					strcat (indices, s);
				}
			}
		}

		ssi_char_t indices[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "ARTKPlusSelector"; };
	static IObject *Create (const ssi_char_t *file) { return new ARTKPlusSelector (file); };
	~ARTKPlusSelector ();
	ARTKPlusSelector::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "selects properties from a marker stream"; };

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
		if (_options.indices[0] != '\0') {
			releaseSelection ();
			parseSelection ();
		}
		return sample_dimension_in * _n_selected; 
	};
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) { 
		if (sample_bytes_in != sizeof (ARTKPlusTools::marker_s)) {
			ssi_err ("invalid struct size");
		}
		return sizeof (ssi_real_t); 
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_STRUCT) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	void visible () { SSI_ARTKPLUS_SELECTOR_CHECK (_n_selected); _selector[_n_selected-1] = &get_visible; };
	void id () { SSI_ARTKPLUS_SELECTOR_CHECK (_n_selected); _selector[_n_selected-1] = &get_id; };
	void center_X () { SSI_ARTKPLUS_SELECTOR_CHECK (_n_selected); _selector[_n_selected-1] = &get_center_X; };
	void center_Y () { SSI_ARTKPLUS_SELECTOR_CHECK (_n_selected); _selector[_n_selected-1] = &get_center_Y; };
	
protected:

	ARTKPlusSelector (const ssi_char_t *file = 0);
	ARTKPlusSelector::Options _options;
	ssi_char_t *_file;

	typedef float (*get_field)(ARTKPlusTools::marker_s &s);  	
	get_field _selector[MAX_SELECT];

	static float get_visible (ARTKPlusTools::marker_s &s) { return s.visible ? 1.0f : 0.0f; };
	static float get_id (ARTKPlusTools::marker_s &s) { return ssi_cast (float, s.id); };
	static float get_center_X (ARTKPlusTools::marker_s &s) { return s.center.x; };
	static float get_center_Y (ARTKPlusTools::marker_s &s) { return s.center.y; };

	ssi_size_t _n_selected;

	void parseSelection ();
	void releaseSelection ();
};

}

#endif
