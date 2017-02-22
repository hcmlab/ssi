// Functionals.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/12/11
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

#ifndef SSI_SIGNAL_FUNCTIONALS_H
#define SSI_SIGNAL_FUNCTIONALS_H

#include "base/IFeature.h"
#include "signal/SignalCons.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class IFunctionals : public IFeature {

public:

	enum FORMAT : ssi_bitmask_t { 
		NONE		= 0,
		MEAN		= 1<<0,
		ENERGY		= 1<<1,
		STD			= 1<<2,
		MIN			= 1<<3,
		MAX			= 1<<4,
		RANGE		= 1<<5,
		MINPOS		= 1<<6,
		MAXPOS		= 1<<7,
		ZEROS		= 1<<8,
		PEAKS		= 1<<9,
		LEN			= 1<<10,	
		ALL			= MEAN | ENERGY | STD | MIN | MAX | RANGE | MINPOS | MAXPOS | ZEROS | PEAKS | LEN
	};

	virtual ssi_size_t getSize () = 0;
	virtual const ssi_char_t *getName (ssi_size_t index) = 0;

protected:

	static const ssi_size_t FORMAT_SIZE;
	static const ssi_char_t *FORMAT_NAMES[];
	static ssi_bitmask_t Names2Format (const ssi_char_t *names);
	static ssi_bitmask_t Name2Format (const ssi_char_t *name);
	static ssi_size_t CountSetBits (ssi_bitmask_t _format);
};

class Functionals : public IFunctionals {

public:

	class Options : public OptionList {

	public:

		Options ()
			: delta (2) {

			names[0] = '\0';
			addOption ("names", names, SSI_MAX_CHAR, SSI_CHAR, "names of functionals separated by comma (mean,energy,std,min,max,range,minpos,maxpos,zeros,peaks,len) or leave empty to select all");		
			addOption ("delta", &delta, 1, SSI_UCHAR, "zero/peaks search offset");
		};

		void addName (const ssi_char_t *name) {
			if (!name || name[0] == '\0') {
				names[0] = '\0';
			} else {
				size_t old_len = strlen (names) + 1;
				size_t new_len = old_len + strlen (name) + 1;
				if (new_len <= SSI_MAX_CHAR) {
					names[old_len - 1] = ',';
					memcpy (names + old_len, name, strlen (name) + 1);
				} else {
					ssi_wrn ("could not add name");
				}
			}
		};

		ssi_char_t names[SSI_MAX_CHAR];
		ssi_size_t delta;
	};

public:

	static const ssi_char_t *GetCreateName () { return "Functionals"; };
	static IObject *Create (const ssi_char_t *file) { return new Functionals (file); };
	~Functionals ();

	Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes a series of functionals from input stream."; };
	
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
		return sample_dimension_in * getSize ();
	}
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sample_bytes_in;
	}
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
		}
		return SSI_REAL;
	}

	ssi_size_t getSize () { return IFunctionals::CountSetBits (Names2Format (_options.names)); };
	const ssi_char_t *getName (ssi_size_t index);

protected:

	Functionals (const ssi_char_t *file = 0);
	Functionals::Options _options;
	ssi_char_t *_file;

	void calc (ssi_size_t sample_dimension, 
		ssi_size_t sample_number, 
		ssi_real_t *data_in, 
		ssi_real_t *&dstptr);

	ssi_bitmask_t		_format;
	ssi_size_t			_delta;

	ssi_real_t			*_mean_val;
	ssi_real_t			*_energy_val;
	ssi_real_t			*_std_val;
	ssi_real_t			*_min_val;
	ssi_real_t			*_max_val;
	ssi_size_t			*_min_pos;
	ssi_size_t			*_max_pos;
	ssi_size_t			*_zeros;
	ssi_size_t			*_peaks;
	ssi_real_t			*_left_val;
	ssi_real_t			*_mid_val;
	ssi_real_t			_val;
};

}

#endif
