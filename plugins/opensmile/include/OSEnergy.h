// OSEnergy.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/21 
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

// based on code of openSMILE 1.0.1
// http://opensmile.sourceforge.net/

/*F******************************************************************************
 *
 * openSMILE - open Speech and Music Interpretation by Large-space Extraction
 *       the open-source Munich Audio Feature Extraction Toolkit
 * Copyright (C) 2008-2009  Florian Eyben, Martin Woellmer, Bjoern Schuller
 *
 *
 * Institute for Human-Machine Communication
 * Technische Universitaet Muenchen (TUM)
 * D-80333 Munich, Germany
 *
 *
 * If you use openSMILE or any code from openSMILE in your research work,
 * you are kindly asked to acknowledge the use of openSMILE in your publications.
 * See the file CITING.txt for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ******************************************************************************E*/

/**

Provides energy analysis.

*/

#pragma once

#ifndef SSI_OPENSMILE_ENERGY_H
#define SSI_OPENSMILE_ENERGY_H

#include "base/IFeature.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSEnergy : public IFeature {

public:

	enum TYPE {
		RMS = 0,
		LOG = 1,
		BOTH = 2
	};

public:
	
	struct meta_s {
		int pos_rms;
		int pos_log;
	};

	class Options : public OptionList {

	public:

		Options ()
			: type (BOTH), htk (false), scaleLog (1.0f), scaleRms (1.0f), biasLog (0), biasRms (0) {

			addOption ("type", &type, 1, SSI_INT, "0=root-mean-square energy, 1=logarithmic energy, 2=both");		
			addOption ("htk", &htk, 1, SSI_BOOL, "compatible with HTK Speech Recognition Toolkit");		
			addOption ("scaleLog", &scaleLog, 1, SSI_REAL, "scale factor to multiply log energy by");
			addOption ("scaleRms", &scaleRms, 1, SSI_REAL, "scale factor to multiply log energy by");
			addOption ("biasLog", &biasLog, 1, SSI_REAL, "bias to add to log energy");
			addOption ("biasRms", &biasRms, 1, SSI_REAL, "bias to add to rms energy");
		};

		TYPE type;
		bool htk;
		ssi_real_t scaleLog, scaleRms, biasLog, biasRms;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSEnergy"; };
	static IObject *Create (const ssi_char_t *file) { return new OSEnergy (file); };
	~OSEnergy ();

	OSEnergy::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes logarithmic (log) and root-mean-square (rms) signal energy from PCM frames."; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in != 1) {
			ssi_err ("dimension > 1 not supported");
		}
		switch (_options.type) {
			case RMS:
			case LOG:
				return 1;
			case BOTH:
				return 2;
		}

		return 0;
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

	const void *getMetaData (ssi_size_t &size) { 

		OSEnergy::Options *options = ssi_pcast (OSEnergy::Options, getOptions ());

		//the position of each result in the output stream
		int n = 0;
		_meta.pos_rms = -1;
		_meta.pos_log = -1;
		
		switch (_options.type) {
			case RMS:
				_meta.pos_rms = 0;
				break;
			case LOG:
				_meta.pos_log = 0;
				break;
			case BOTH:
				_meta.pos_rms = 0;
				_meta.pos_log = 1;
				break;
		}

		size = sizeof (_meta);
		return &_meta;
	};

protected:

	OSEnergy (const ssi_char_t *file = 0);
	OSEnergy::Options _options;
	ssi_char_t *_file;
	meta_s _meta;

	int htkcompatible;
	int erms, elog;
    FLOAT_DMEM escaleRms, escaleLog;
    FLOAT_DMEM ebiasLog, ebiasRms;
};

}

#endif
