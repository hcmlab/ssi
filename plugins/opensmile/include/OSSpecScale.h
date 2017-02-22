// OSSpecScale.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/09/22
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

#pragma once

#ifndef SSI_OPENSMILE_SPECSCALE_H
#define SSI_OPENSMILE_SPECSCALE_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSSpecScale : public IFilter {

public:

	enum SPECTSCALE {
		LINEAR = 0,
		LOG,
		BARK,
		MEL,
		SEMITONE,
		BARK_SCHROED,
		BARK_SPEEX
	};
	
public:

	class Options : public OptionList {

	public:

		Options ()
			: dstScale (LOG), srcScale (LINEAR), dstLogScaleBase (2.0), srcLogScaleBase (2.0), firstNote (55.0), minF (25.0), maxF (-1.0), nPoints (0), smooth (false), enhance (false), weight (false), fsSec (-1.0) {

			addOption ("dstScale", &dstScale, 1, SSI_INT, "The target scale, one of the following: 0=linear scale, 1=logarithmic, (see 'logScaleBase'), 2=bark scale, 3=mel frequency scale, 4=musical semi-tone scale");		
			addOption ("srcScale", &srcScale, 1, SSI_INT, "The source scale (currently only '0=linear' is supported, all other options (as found for target scale) are experimental.");		
			addOption ("dstLogScaleBase", &dstLogScaleBase, 1, SSI_DOUBLE, "The base for log scales (a log base of 2.0 - the default - corresponds to an octave target scale).");		
			addOption ("srcLogScaleBase", &srcLogScaleBase, 1, SSI_DOUBLE, "The base for log source scales (a log base of 2.0 - the default - corresponds to an octave target scale).");		
			addOption ("firstNote", &firstNote, 1, SSI_DOUBLE, "The first note (in Hz) for a semi-tone scale.");		
			addOption ("minF", &minF, 1, SSI_DOUBLE, "The minimum frequency of the target scale.");		
			addOption ("maxF", &maxF, 1, SSI_DOUBLE, "The maximum frequency of the target scale (-1.0 : set to maximum frequency of the source spectrum).");
			addOption ("nPoints", &nPoints, 1, SSI_SIZE, "The number of frequency points in target spectrum (<= 0 : same as input spectrum).");
			addOption ("smooth", &smooth, 1, SSI_BOOL, "Perform spectral smoothing before applying the scale transformation.");
			addOption ("enhance", &enhance, 1, SSI_BOOL, "Perform spectral peak enhancement before applying smoothing (if enabled) and scale transformation.");
			addOption ("weight", &weight, 1, SSI_BOOL, "Perform post-scale auditory weighting (this is currently only supported for octave (log2) scales).");
			addOption ("fsSec", &fsSec, 1, SSI_DOUBLE, "Frame size in seconds (if -1.0 calculated from sample rate of input stream).");
		};

		SPECTSCALE dstScale, srcScale;
		double dstLogScaleBase, srcLogScaleBase, firstNote;
		double minF, maxF, fmin_t, fmax_t;
		ssi_size_t nPoints;
		bool smooth, enhance, weight;
		double fsSec;
  
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSSpecScale"; };
	static IObject *Create (const ssi_char_t *file) { return new OSSpecScale (file); };
	~OSSpecScale ();

	OSSpecScale::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component performs linear/non-linear axis scaling of FFT magnitude spectra with spline interpolation."; };

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
		return _options.nPoints == 0 ? sample_dimension_in : _options.nPoints;
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
		size = sizeof (pmdata); 
		return &pmdata;
	};


protected:

	OSSpecScale (const ssi_char_t *file = 0);
	OSSpecScale::Options _options;
	ssi_char_t *_file;

	void release ();
	double specScaleTransfFwd (double x, SPECTSCALE scale, double param);	

	SPECTSCALE scale;
    SPECTSCALE sourceScale;
    int specSmooth, specEnhance;
    int auditoryWeighting;
    double logScaleBase, logSourceScaleBase;
    double minF, maxF, fmin_t, fmax_t;
    long nPointsTarget;
    double firstNote, param;

    long nMag, magStart;
    double fsSec;
    double deltaF, deltaF_t;

    double *f_t;
    double *spline_work;
    double *y, *y2;
    double *audw;

	FLOAT_DMEM mdata[8];
	FLOAT_DMEM *pmdata;
};

}

#endif
