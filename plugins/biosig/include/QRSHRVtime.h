// QRSHRVtime.h
// author: Florian Lingenfelser <lingenfelser@hcm-lab.de>
// created: 2013/03/08
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
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

/**

Converts heart rate spectogram to lf/hf ratio.

*/

#pragma once

#ifndef SSI_BIOSIG_QRSHRVTIME_H
#define SSI_BIOSIG_QRSHRVTIME_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"
#include <vector>

namespace ssi {

class QRSHRVtime : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options () 
			: print (false) {

			addOption ("print", &print, 1, SSI_BOOL, "print features to console");	

		};

		bool print;

	};

public:

	static const ssi_char_t *GetCreateName () { return "QRSHRVtime"; };
	static IObject *Create (const ssi_char_t *file) { return new QRSHRVtime (file); };
	~QRSHRVtime ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "..."; };

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in != 1) {
			ssi_err ("sample dimension != 1 not supported");
		}
		return 8;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("sample type != %s not supported", SSI_TYPE_NAMES[SSI_REAL]);
		}		
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
	
	QRSHRVtime (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	bool calibrated;

	ssi_real_t _time_per_sample;
	ssi_size_t _first_R;
	ssi_size_t _samples_since_last_R;

	std::vector <ssi_real_t> _RR_intervals;
};

}

#endif
