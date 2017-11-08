// MicrosoftKinectSmileDetector.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2013/04/14
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

#ifndef SSI_FILTER_MICROSOFTKINECTSMILEDETECTOR_H
#define SSI_FILTER_MICROSOFTKINECTSMILEDETECTOR_H

#include "base/IFilter.h"
#include "MicrosoftKinect.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class MicrosoftKinectSmileDetector : public IFilter {

public:

	struct JOIN {
		enum List {
			OFF = 0,
			MULT = 1,
			SUM = 2,
			SUMSQUARE = 3
		};
	};

public:

	class Options : public OptionList {

	public:

		Options () {
			
			factors[0] = '\0';									
			addOption ("factors", factors, SSI_MAX_CHAR, SSI_CHAR, "multiplication factors separated by comma (default: 1.0)");								
		};

		void setFactors (ssi_size_t n_inds, ssi_real_t *values) {
			factors[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%.f", values[0]);
				strcat (factors, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%.f", values[i]);
					strcat (factors, s);
				}
			}
		}

		ssi_char_t factors[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "MicrosoftKinectSmileDetector"; };
	static IObject *Create (const ssi_char_t *file) { return new MicrosoftKinectSmileDetector (file); };
	~MicrosoftKinectSmileDetector ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Smile detector for Microsoft Kinect (works on action unit provided by Kinect SDK's face detection)."; };

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
		if (sample_dimension_in != MicrosoftKinect::ACTIONUNIT::NUM) {
			ssi_err ("input dimension %u != %u", sample_dimension_in,  MicrosoftKinect::ACTIONUNIT::NUM);
		}
		return 1;
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_REAL;
	}

protected:

	MicrosoftKinectSmileDetector (const ssi_char_t *file = 0);
	MicrosoftKinectSmileDetector::Options _options;
	ssi_char_t *_file;
	
	ssi_real_t _factors[3];
};

}

#endif
