// Bundle.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2015/02/20
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

#ifndef SSI_SIGNAL_BUNDLE_H
#define SSI_SIGNAL_BUNDLE_H

#include "base/IFilter.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class Bundle : public IFilter {

public:

	struct REPLACE {
		enum List {
			PREVIOUS,
			EXTREMUM,
			MEAN,
			THRESHOLD,
			SET
		};
	};

	class Options : public OptionList {

	public:

		Options () 
			: thres(0.0), hang_in(0), hang_out(0), replace_below(REPLACE::PREVIOUS), replace_above(REPLACE::PREVIOUS), set_below(0), set_above(0), set_all (false) {

			addOption ("thres", &thres, 1, SSI_REAL, "threshold to find outliers");	
			addOption ("hang_in", &hang_in, 1, SSI_SIZE, "#successive values above threshold to change direction");
			addOption ("hang_out", &hang_out, 1, SSI_SIZE, "#successive values below threshold to change direction");
			addOption ("replace_below", &replace_below, 1, SSI_INT, "method to replace outliers below threshold: 0=PREVIOUS, 1=EXTREMA, 2=MEAN, 3=THRESHOLD, 4=SET");
			addOption ("replace_above", &replace_above, 1, SSI_INT, "method to replace outliers above threshold: 0=PREVIOUS, 1=EXTREMA, 2=MEAN, 3=THRESHOLD, 4=SET");
			addOption ("set_below", &set_below, 1, SSI_REAL, "if 'replace_below=SET' this value will be used");
			addOption ("set_above", &set_above, 1, SSI_REAL, "if 'replace_above=SET' this value will be used");
			addOption ("set_all", &set_all, 1, SSI_BOOL, "set all values to either 'set_below' or 'set_above'");
		};

		ssi_real_t thres;
		ssi_size_t hang_in, hang_out;
		REPLACE::List replace_below, replace_above;
		ssi_real_t set_below, set_above;
		bool set_all;
	};


public:

	static const ssi_char_t *GetCreateName () { return "Bundle"; };
	static IObject *Create (const ssi_char_t *file) { return new Bundle (file); };
	~Bundle ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Removes outliers above or below a threshold."; };

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
		return sample_dimension_in;
	}

	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sizeof (ssi_real_t);
	}

	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

protected:

	Bundle (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;

	ssi_real_t _thres;
	ssi_size_t _hang_in, _hang_out;
	REPLACE::List _replace_below, _replace_above;
	ssi_real_t _set_below, _set_above;
	bool _set_all;
	bool *_above;
	bool _first_call;
	ssi_real_t *_last;
	REPLACE::List _replace;

	bool check(ssi_real_t *in, ssi_real_t *out, ssi_real_t &last, ssi_size_t dim, bool above, ssi_size_t n);
	void replace(ssi_real_t *in, ssi_real_t *out, ssi_real_t last, ssi_size_t dim, bool above, ssi_size_t n, REPLACE::List method);
};

}

#endif
