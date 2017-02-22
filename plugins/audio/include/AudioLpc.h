// AudioLpc.h
// author: Ionut Damian <damian@hcm-lab.de>
// created: 2012/10/16 
// Copyright (C) 2007-12 University of Augsburg, Ionut Damian
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

LPC, compute LPC coefficients from wave data (PCM) frames 

*/

#pragma once

#ifndef SSI_AUDIO_AUDIOLPC_H
#define SSI_AUDIO_AUDIOLPC_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class AudioLpc : public IFeature {

public:

	enum TYPE {
		RMS = 0,
		LOG = 1,
		BOTH = 2
	};

public:

	struct meta_s {
		int numCoeff;
		int pos_lpCoeff;
		int pos_lpcGain;
		int pos_refCoeff;
		int pos_residual;
		int pos_lpSpectrum;
		int pos_lsp;
	};

	class Options : public OptionList {

	public:

		Options ()
			: p (8), saveLPCoeff (true), lpGain (false), saveRefCoeff (false), residual(false), forwardFilter(false), lpSpectrum(false), lsp(false), lpSpecDeltaF(10.0f), lpSpecBins(100)  {
		
			setMethod("acf");

			addOption ("method", &method, SSI_MAX_CHAR, SSI_CHAR, "This option sets the lpc method to use. Choose between: 'acf' acf (autocorrelation) method with Levinson-Durbin algorithm , 'burg' Burg method (N. Anderson (1978))");
			addOption ("p", &p, 1, SSI_SIZE, "Predictor order (= number of lpc coefficients)");
			addOption ("saveLPCoeff", &saveLPCoeff, 1, SSI_BOOL, "true = save LP coefficients to output");
			addOption ("lpGain", &lpGain, 1, SSI_BOOL, "true = save lpc gain (error) in output vector");
			addOption ("saveRefCoeff", &saveRefCoeff, 1, SSI_BOOL, "true = save reflection coefficients to output");
			addOption ("residual", &residual, 1, SSI_BOOL, "true = compute lpc residual signal and store in output frame");
			addOption ("forwardFilter", &forwardFilter, 1, SSI_BOOL, "true = apply forward instead of inverse filter when computing residual");
			addOption ("lpSpectrum", &lpSpectrum, 1, SSI_BOOL, "true = compute lp spectrum using 'lpSpecDeltaF' as frequency resolution or 'lpSpecBins' bins");
			addOption ("lpSpecDeltaF", &lpSpecDeltaF, 1, SSI_REAL, "frequency resolution of lp spectrum (only applicable if 'lpSpectrum=true')");
			addOption ("lpSpecBins", &lpSpecBins, 1, SSI_SIZE, "number of bins to compute lp spectrum for (overrides lpSpecDeltaF) (only applicable if 'lpSpectrum=true')");
			addOption ("lsp", &lsp, 1, SSI_BOOL, "compute LSP (line spectral pairs) from LPC coefficients");
		};
		
		void setMethod (const ssi_char_t *method) {
			ssi_strcpy (this->method, method);
		}

		ssi_char_t method[SSI_MAX_CHAR];
		bool saveLPCoeff, lpGain, saveRefCoeff, residual, forwardFilter, lpSpectrum, lsp;
		ssi_size_t p, lpSpecBins;
		ssi_real_t lpSpecDeltaF;
	};

public:

	static const ssi_char_t *GetCreateName () { return "OSLpc"; };
	static IObject *Create (const ssi_char_t *file) { return new AudioLpc (file); };
	~AudioLpc ();

	AudioLpc::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "This component computes linear predictive coding (LPC) coefficients from PCM frames. Burg's algorithm and the standard ACF/Durbin based method are implemented for LPC coefficient computation. The output of LPC filter coefficients, reflection coefficients, residual signal, LP spectrum and LSP is supported."; };

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

		int n = 0;
		if(_options.saveLPCoeff)	n += _options.p;
		if(_options.saveRefCoeff)	n += _options.p;
		if(_options.lpGain)			n += 1;
		if(_options.lpSpectrum)		n += _options.lpSpecBins;
		if(_options.lsp)			n += _options.p;
		if(_options.residual)		n += sample_dimension_in; //TODO: this should be "num" not "dim"

		return n;
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

		AudioLpc::Options *options = ssi_pcast (AudioLpc::Options, getOptions ());

		_meta.numCoeff = _options.p;

		//the position of each result in the output stream
		int n = 0;
		_meta.pos_lpCoeff = -1;
		_meta.pos_residual = -1;
		_meta.pos_refCoeff = -1;
		_meta.pos_lpcGain = -1;
		_meta.pos_lpSpectrum = -1;
		_meta.pos_lsp = -1;
		
		if(_options.saveLPCoeff)	
		{	
			_meta.pos_lpCoeff = n;
			n += _options.p;
		}
		if(_options.saveRefCoeff)	
		{		
			_meta.pos_refCoeff = n;
			n += _options.p;
		}
		if(_options.lsp)	
		{	
			_meta.pos_lsp = n;
			n += _options.p;
		}
		if(_options.lpGain)	
		{	
			_meta.pos_lpcGain = n;
			n += 1;
		}
		if(_options.lpSpectrum)	
		{	
			_meta.pos_lpSpectrum = n;
			n += _options.lpSpecBins;
		}
		if(_options.residual)	
		{	
			_meta.pos_residual = n;
		}

		size = sizeof (_meta);
		return &_meta;
	};

protected:

	AudioLpc (const ssi_char_t *file = 0);
	AudioLpc::Options _options;
	ssi_char_t *_file;
	meta_s _meta;

	ssi_real_t calcLpc(const ssi_real_t *x, long Nsrc, ssi_real_t * lpc, long nCoeff, ssi_real_t *refl);
	int lpc_to_lsp (const ssi_real_t *a, int lpcrdr, ssi_real_t *freq, int nb, ssi_real_t delta);
	ssi_real_t cheb_poly_eva(ssi_real_t *coef, ssi_real_t x, int m);
	ssi_real_t smileDsp_invLattice(ssi_real_t *k, ssi_real_t *b, int M, ssi_real_t out);
	ssi_real_t smileDsp_lattice(ssi_real_t *k, ssi_real_t *b, int M, ssi_real_t in, ssi_real_t *bM);
	void smileDsp_autoCorr(const ssi_real_t *x, const int n, ssi_real_t *acf, int lag);
	int smileDsp_calcLpcAcf(ssi_real_t * r, ssi_real_t *a, int p, ssi_real_t *gain, ssi_real_t *k);
	int smileDsp_calcLpcBurg(const ssi_real_t *x, long n, ssi_real_t *a, int m, ssi_real_t *gain, ssi_real_t **burgB1, ssi_real_t **burgB2, ssi_real_t **burgAA);

	int p;
	bool saveLPCoeff, saveRefCoeff, residual, lpGain, lpSpectrum, forwardRes;
	int method;

	double lpSpecDeltaF;
	int lpSpecBins;

	ssi_real_t *latB;
	ssi_real_t lastGain;
	ssi_real_t *lSpec;

	int *_ip;
	ssi_real_t *_w;

	ssi_real_t *acf;
	ssi_real_t *lpCoeff, *lastLpCoeff, *refCoeff;

	ssi_real_t *burgB1, *burgB2, *burgAA;
};

}

#endif
