// GazePointShifter.h
// author: Daniel Schork
// created: 2015
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

#ifndef SSI_GAZEPOINTSHIFTER_H
#define SSI_GAZEPOINTSHIFTER_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

	class GazePointShifter : public IFilter {

public:

	class Options : public OptionList {
	public:

		Options()
			: scale(1), sliders(false) {

			addOption("scale", &scale, 1, SSI_DOUBLE, "scale of shift");
			addOption("sliders", &sliders, 1, SSI_DOUBLE, "use sliders instead if clickable image");
		};

		double scale;
		bool sliders;
	};

public:

	static const ssi_char_t *GetCreateName () { return "GazePointShifter"; };
	static IObject *Create (const ssi_char_t *file) { return new GazePointShifter (file); };
	~GazePointShifter();
	GazePointShifter::Options *getOptions() { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Shifts Gazepoint if glasses are moved during live recording"; };

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


	ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) {
		
		return 23;
	}

	ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) {
		
		return sizeof(ssi_real_t);
	}

	ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) {
		
		return SSI_REAL;
	}

protected:

	GazePointShifter(const ssi_char_t *file = 0);
	GazePointShifter::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;



};

}

#endif
