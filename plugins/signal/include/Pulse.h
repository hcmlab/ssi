// Pulse.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/01/16
// Copyright (C) 2007-10 University of Augsburg, Johannes Wagner
//
// *************************************************************************************************
//
// This file is part of Smart Sensor Integration (SSI) developed at the 
// Lab for Multimedia Concepts and Applications of the University of Augsburg
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

#ifndef SSI_FEATURE_PULSE_H
#define SSI_FEATURE_PULSE_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Pulse : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: min_rate (0), max_rate (0), delta (0) {

			addOption ("delta", &delta, 1, SSI_REAL, "minimum difference in amplitude between successive maxima (to filter local maxima)");		
			addOption ("minr", &min_rate, 1, SSI_REAL, "minimum rate in hz");	
			addOption ("maxr", &max_rate, 1, SSI_REAL, "maximum rate in hz");			
		};

		ssi_real_t delta;
		ssi_real_t min_rate;
		ssi_real_t max_rate;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Pulse"; };
	static IObject *Create (const ssi_char_t *file) { return new Pulse (file); };
	~Pulse ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes pulse (in Hz) based on distance of successive maxima."; };


	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		return sample_dimension_in;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		SSI_ASSERT (sample_bytes_in == sizeof (ssi_real_t));
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		SSI_ASSERT (sample_type_in == SSI_REAL);
		return sample_type_in;
	}

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

protected:
	
	Pulse (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	bool _first_call;
	bool *_lookformax;
	ssi_real_t *_minval, *_maxval;
	ssi_size_t *_minpos, *_maxpos;
	ssi_real_t _mark_min, _mark_max;
	ssi_real_t _delta;
	ssi_size_t _offset;
	ssi_size_t _min_dist, _max_dist;
	ssi_real_t _min_rate, _max_rate;
	ssi_size_t *_last;
	ssi_real_t *_new_rate, *_old_rate;
	ssi_size_t *_counter;
};

}

#endif
