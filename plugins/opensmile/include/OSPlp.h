// OSPlp.h
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

/**

Computes PLP Coefficients from Mel-Spectrum

*/

#pragma once

#ifndef SSI_OPENSMILE_PLP_H
#define SSI_OPENSMILE_PLP_H

#include "base/IFilter.h"
#include "OSTools.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class OSPlp : public IFilter {

public:

	class Options : public OptionList {

	public:

		Options ()
			: first (0), last (12), lporder(12), 
			melFloor (0.00000000093f), cepLifter (0.0f), compression(0.33f), rastaUcutoff(29.0f), rastaLcuttoff(1.0f), 
			doLog(true), doAud(true), htk (false), rasta(false), newRasta(false), doInvLog(true), doIDFT(true), doLP(true), doLpToCeps(true) {
					
			addOption("lpOrder", &lporder, 1, SSI_INT, "The order of the linear predictor (5th order is optimal according to Hermansky 1990, JASA)");
			addOption("firstCC", &first, 1, SSI_INT, "The first cepstral coefficient to compute (set to 0 to include the 0th coefficient, which is defined as -log(1/lpcGain) )");
			addOption("lastCC", &last, 1, SSI_INT,"The last cepstral coefficient to compute");

			addOption("doLog", &doLog, 1 , SSI_BOOL, "Take the log of input bands (1=yes / 0=no)");
			addOption("doAud", &doAud, 1, SSI_BOOL, "Do auditory processing (equal loudness curve and loudness compression) (1=yes / 0=no)");

			addOption("RASTA", &rasta, 1, SSI_BOOL, "Perform RASTA (temporal) filtering (1=yes / 0=no)");
			addOption("newRASTA", &newRasta, 1, SSI_BOOL, "Perform RASTA (temporal) filtering (more stable filter, Type-II, initial filtering only with FIR part; thanks to Chris Landsiedl for this code!) (1=enable / 0=disable) Note: this option (if set to 1) will disable the 'RASTA' option.");
			addOption("rastaUpperCutoff", &rastaUcutoff, 1 , SSI_REAL, "Upper cut-off frequency of RASTA bandpass filter in Hz");
			addOption("rastaLowerCutoff",&rastaLcuttoff, 1, SSI_REAL, "Lower cut-off frequency of RASTA bandpass filter in Hz");

			addOption("doInvLog", &doInvLog, 1, SSI_BOOL, "Apply inverse logarithm after power compression (1=yes / 0=no)");
			addOption("doIDFT", &doIDFT, 1, SSI_BOOL, "Apply I(nverse)DFT after power compression and inverse log (1=yes / 0=no)");
			addOption("doLP", &doLP, 1, SSI_BOOL, "Do lp analysis on autocorrelation function (1=yes / 0=no)");
			addOption("doLpToCeps", &doLpToCeps, 1, SSI_BOOL, "Convert lp coefficients to cepstral coefficients (1=yes / 0=no)");

			addOption("cepLifter", &cepLifter, 1 , SSI_REAL, "Parameter for cepstral 'liftering', set to 0.0 to disable cepstral liftering");
			addOption("compression", &compression, 1, SSI_REAL, "Compression factor for 'power law of hearing'");

			addOption("melfloor",&melFloor, 1 , SSI_REAL, "Minimum value of melspectra when computing mfcc (will be forced to 1.0 when htkcompatible=1)");
			addOption("htkcompatible", &htk, 1 , SSI_BOOL, "Set correct mel-floor and force HTK compatible PLP output (1/0 = yes/no)\n  htkcompatible == 1, forces the following settings:\n  - melfloor = 1.0 (signal scaling 0..32767*32767)\n  - append 0th coeff instead of having it as first value\n  - doAud = 1 , doLog=0 , doInvLog=0   (doIDFT, doLP, and doLpToCeps are not forced to 1, this enables generation of HTK compatible auditory spectra, etc. (these, of course, are not compatible, i.e. are not the same as HTK's PLP))\n  - the 0th audspec component is used as dc component in IDFT (else the DC component is zero)");


			};

		ssi_size_t first, last, lporder;
		ssi_real_t melFloor, cepLifter, compression, rastaUcutoff, rastaLcuttoff;
		bool htk, doLog, doAud, rasta, newRasta, doInvLog, doIDFT, doLP, doLpToCeps;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSPlp"; };
	static IObject *Create (const ssi_char_t *file) { return new OSPlp (file); };
	~OSPlp ();

	OSPlp::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes PLP and RASTA-PLP (currently the RASTA filter is not yet implemented) cepstral coefficients from a critical band spectrum (generated by the cMelspec component, for example)"; };

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
		return _options.last - _options.first + 1;
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

	OSPlp (const ssi_char_t *file = 0);
	OSPlp::Options _options;
	ssi_char_t *_file;

	int htkcompatible;
    
	int initTables (long blocksize);
	void releaseTables ();

	FLOAT_DMEM *_costable;
    FLOAT_DMEM *_sintable;
    FLOAT_DMEM melfloor;
    FLOAT_DMEM cepLifter;

	int nAuto, nFreq, nScale;
    int lpOrder;
    int nCeps, firstCC, lastCC;

    int doLog, doAud, doInvLog, doIDFT, RASTA, newRASTA, doLP, doLpToCeps;
    FLOAT_DMEM rastaUpperCutoff, rastaLowerCutoff;

    FLOAT_DMEM rasta_iir;
    FLOAT_DMEM rasta_fir[5];
    // rasta filter history:
    FLOAT_DMEM *rasta_buf_iir; /* size: n bands */
    FLOAT_DMEM *rasta_buf_fir; /* size: n bands * 5 (nCoeff) */
    int rasta_buf_fir_ptr; // buffer pointer (cyclic shifting)
    int rasta_init;

    FLOAT_DMEM *_acf;
    FLOAT_DMEM *_lpc;
    FLOAT_DMEM *_ceps;

    FLOAT_DMEM compression;
    FLOAT_DMEM *_eqlCurve; // equal loudness curve
   
};

}

#endif
