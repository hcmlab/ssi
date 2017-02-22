// Selector.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/10/28
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

/**

Provides second-order infinite impulse response (IIR) filtering.

*/

#pragma once

#ifndef SSI_SIGNAL_SELECTOR_H
#define SSI_SIGNAL_SELECTOR_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Selector : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options ()
			: sort(false), multiples(0){

			indices[0] = '\0';
			addOption ("indices", indices, SSI_MAX_CHAR, SSI_CHAR, "indices of dimensions that will be kept (i.e. 0,1,2,..) (leave empty to keep all)");
			addOption ("sort", &sort, 1, SSI_BOOL, "sort indices");
			addOption ("multiples", &multiples, 1, SSI_SIZE, "select multiples (e.g. indices=0,3 and multiples=5 the selection will be expanded to 0,3,5,8,10,13,...");
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
		bool sort;
		ssi_size_t multiples;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Selector"; };
	static IObject *Create (const ssi_char_t *file) { return new Selector (file); };
	~Selector ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Selects certain dimensions of the input stream in abitrary order."; };

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
		return Parse(_options.indices, sample_dimension_in) ? _n_select : sample_dimension_in;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		return sample_type_in;
	}

protected:

	Selector (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	bool Parse(const ssi_char_t *indices, ssi_size_t n_values);

	int *_offset;
	ssi_size_t *_select;
	ssi_size_t _n_select;
};

}

#endif
