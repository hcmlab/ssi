// QRSHeartRate.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/01/17
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

Converts QRS signal to heart rate.

*/

#pragma once

#ifndef SSI_BIOSIG_QRSHEARTRATE_H
#define SSI_BIOSIG_QRSHEARTRATE_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class QRSHeartRate : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: smooth (false), defhr (75.0f) {

			addOption ("defhr", &defhr, 1, SSI_REAL, "inital heart rate value (bpm)");
			addOption ("smooth", &smooth, 1, SSI_BOOL, "applying smoothing between successive heart rate values");				
		};

		bool smooth;
		ssi_real_t defhr;
	};

public:

	static const ssi_char_t *GetCreateName () { return "QRSHeartRate"; };
	static IObject *Create (const ssi_char_t *file) { return new QRSHeartRate (file); };
	~QRSHeartRate ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "converts QRS signal to heart rate"; };

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in != 1) {
			ssi_err ("sample dimension != 1 not supported");
		}
		return sample_dimension_in;
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
	
	QRSHeartRate (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	ssi_size_t _last_r;    // position in samples of the last r peak
	ssi_real_t _last_hr;   // last heart rate value in bpm
	ssi_size_t _count;     // total number of processed samples
};

}

#endif
