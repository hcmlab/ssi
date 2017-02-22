// OSMelspec.h
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

#ifndef SSI_OPENSMILE_MELSPEC_H
#define SSI_OPENSMILE_MELSPEC_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSMelspec : public IFilter {

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

	enum BWMETHOD {
		LR,
		ERB
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: nBands (26), loFreq (20.0f), hiFreq (8000.0f), usePower (false), showBank (false), htk (false), scale (MEL), bwMethod (LR), logScaleBase (2.0), firstNote (27.05) {

			addOption ("nBands", &nBands, 1, SSI_SIZE, "The number of Mel/Bark/Semitone band filters the filterbank from 'lofreq'-'hifreq' contains.");		
			addOption ("loFreq", &loFreq, 1, SSI_REAL, "The upper cut-off frequency of the filterbank (Hz).");
			addOption ("hiFreq", &hiFreq, 1, SSI_REAL, "The upper cut-off frequency of the filterbank (Hz).");
			addOption ("usePower", &usePower, 1, SSI_BOOL, "Use the power spectrum instead of magnitude spectrum, i.e. if set this squares the input data.");
			addOption ("showBank", &showBank, 1, SSI_BOOL, "Bandwidths and centre frequencies of the filters in the filterbank are printed to openSMILE log output.");
			addOption ("htk", &htk, 1, SSI_BOOL, "Enable htk compatible output (audio sample scaling -32767..+32767.");
			addOption ("scale", &scale, 1, SSI_INT, "The frequency scale to design the critical band filterbank in (this is the scale in which the filter centre frequencies are placed equi-distant): 3=Mel-frequency scale (m = 1127 ln (1+f/700)), 2=Bark scale approximation (Critical band rate z): z = [26.81 / (1.0 + 1960/f)] - 0.53, 5=Bark scale approximation due to Schroeder (1977): 6*ln( f/600 + [(f/600)^2+1]^0.5 ), 6=Bark scale approximation as used in Speex codec package, 4=semi-tone scale with first note (0) = 'firstNote' (default 27.5Hz)  (s=12*log(f/firstNote)/log(2)) [experimental], 1=logarithmic scale with base 'logScaleBase'.");
			addOption ("bwMethod", &bwMethod, 1, SSI_INT, "The method to use to compute filter bandwidth: 0=use centre frequencies of left and right neighbours (standard way for mel-spectra and mfcc), 1=bandwidth based on critical bandwidth approximation (ERB), choose this option for computing HFCC instead of MFCC.");
			addOption ("logScaleBase", &logScaleBase, 1, SSI_DOUBLE, "The base for log scales (a log base of 2.0 - the default - corresponds to an octave target scale)");
			addOption ("firstNote", &firstNote, 1, SSI_DOUBLE, "The first note (in Hz) for a semi-tone scale");
		};

		ssi_size_t nBands;
		ssi_real_t loFreq, hiFreq;
		bool usePower, showBank, htk;
		SPECTSCALE scale;
		BWMETHOD bwMethod;
		double logScaleBase, firstNote;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSMelspec"; };
	static IObject *Create (const ssi_char_t *file) { return new OSMelspec (file); };
	~OSMelspec ();

	OSMelspec::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes an N-band Mel/Bark/Semitone-frequency spectrum (critical band spectrum) by applying overlapping triangular filters equidistant on the Mel/Bark/Semitone-frequency scale to an FFT magnitude or power spectrum."; };

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
		if (sample_dimension_in < _options.nBands) {
			ssi_err ("dimension '%u' < #bands '%u'", sample_dimension_in, _options.nBands);
		}
		return _options.nBands;
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

protected:

	OSMelspec (const ssi_char_t *file = 0);
	OSMelspec::Options _options;
	ssi_char_t *_file;

	int hfcc;
    int nBands;
	int htkcompatible, usePower, showFbank;
    long bs;
    FLOAT_DMEM lofreq, hifreq;
    long nLoF, nHiF;
    SPECTSCALE specScale;
    double firstNote, logScaleBase, param;

	int computeFilters( long blocksize, double frameSizeSec);
	double specScaleTransfFwd (double x, SPECTSCALE scale, double param);
	double specScaleTransfInv(double x, SPECTSCALE scale, double param);
	void releaseFilters ();

	FLOAT_DMEM *_filterCoeffs;
    FLOAT_DMEM *_filterCfs;
    long *_chanMap;
	FLOAT_DMEM *_src;

	// convert frequency (hz) to FFT bin number
    long FtoN(FLOAT_DMEM fhz, FLOAT_DMEM baseF)
    {
      return (long) (fhz/baseF+0.5);
    }

    // convert FFT bin number to frequency (hz)
    FLOAT_DMEM NtoF(long bin, FLOAT_DMEM baseF)
    {
      return ((FLOAT_DMEM)bin * baseF);
    }

    // convert bin number to frequency in Mel/Bark/...
    FLOAT_DMEM NtoFmel(long N, FLOAT_DMEM baseF)
    {
      return (FLOAT_DMEM) specScaleTransfFwd (N*baseF , specScale, param );
    }
};

}

#endif
