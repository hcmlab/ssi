// Spectrogram.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/02
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

#ifndef SSI_SIGNAL_SPECTROGRAM_H
#define SSI_SIGNAL_SPECTROGRAM_H

#include "base/IFeature.h"
#include "FilterTools.h"
#include "signal/Matrix.h"
#include "ioput/option/OptionList.h"

namespace ssi {

class FFT;

class Spectrogram : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: nfft (512), nbanks (50), minfreq (0.0), maxfreq (0.0), wintype (WINDOW_TYPE_HAMMING), dolog (true) {

			file[0] = '\0';
			banks[0] = '\0';

			addOption ("nfft", &nfft, 1, SSI_SIZE, "#fft coefficients");		
			addOption ("nbanks", &nbanks, 1, SSI_UINT, "#filter banks");	
			addOption ("minfreq", &minfreq, 1, SSI_TIME, "mininmum frequency");
			addOption ("maxfreq", &maxfreq, 1, SSI_TIME, "maximum frequency (nyquist if 0)");
			addOption ("wintype", &wintype, 1, SSI_INT, "window type (0=rectangle,1=triangle,2=gauss,3=hamming");
			addOption ("dolog", &dolog, 1, SSI_BOOL, "apply logarithm");
			addOption("file", &file, SSI_MAX_CHAR, SSI_CHAR, "file with filter banks (plain text file with each line defining one band {<start in hz> <stop in hz>})");
			addOption("banks", &banks, SSI_MAX_CHAR, SSI_CHAR, "string with filter banks that gets applied if no file was set (example: \"0.003 0.040\n0.040 0.150\n0.150 0.400\").");
		};

		ssi_size_t nfft;
		ssi_size_t nbanks;
		ssi_time_t minfreq;
		ssi_time_t maxfreq;
		WINDOW_TYPE wintype;
		bool dolog;
		ssi_char_t file[SSI_MAX_CHAR];
		ssi_char_t banks[SSI_MAX_CHAR];
	};

public:

	static const ssi_char_t *GetCreateName () { return "Spectrogram"; };
	static IObject *Create (const ssi_char_t *file) { return new Spectrogram (file); };
	~Spectrogram ();

	Spectrogram::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Computes spectrogram of input stream."; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	void transform (Matrix<ssi_real_t> *matrix_in, 
		Matrix<ssi_real_t> *matrix_out);
		
	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in) {
		if (sample_dimension_in > 1) {
			ssi_err ("dimension > 1 not supported");
		}		
		if (_filterbank) {
			return _filterbank->cols;
		} else {
			return _options.nbanks;
		}
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

	virtual void readFilterbank (const ssi_char_t *file, ssi_time_t sr, bool fromFile = true);
	virtual void setFilterbank (const Matrix<ssi_real_t> *_filterbank, 
		WINDOW_TYPE _win_type = WINDOW_TYPE_HAMMING,
		bool _apply_log = true);
	virtual const Matrix<ssi_real_t> *getFilterbank () { return _filterbank; };

protected:

	Spectrogram (const ssi_char_t *file = 0);
	Options _options;
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

	int _fft_size;
	WINDOW_TYPE _win_type;
	ssi_size_t _win_size;
	Matrix<ssi_real_t> *_filterbank;
	FFT *_fft;
	Matrix<ssi_real_t> *_fftmag;
	Matrix<ssi_real_t> *_window;
	bool _apply_log;
};

}

#endif
