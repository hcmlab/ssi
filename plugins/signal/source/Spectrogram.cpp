// Spectrogram.cpp
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

#include "Spectrogram.h"
#include "FilterTools.h"
#include "signal/MatrixOps.h"
#include "FFT.h"
#include <string>

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *Spectrogram::ssi_log_name = "spectrogra";

Spectrogram::Spectrogram (const ssi_char_t *file)
	: _fft_size (0),
	_win_type (WINDOW_TYPE_HAMMING),
	_win_size (0),
	_fft (0),
	_fftmag (0),
	_window (0),
	_filterbank (0),
	_apply_log (false),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
}

Spectrogram::~Spectrogram () {
	
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}

	delete _fft;
	delete _fftmag;
	delete _filterbank;
}

void Spectrogram::readFilterbank (const ssi_char_t *file, ssi_time_t sr, bool fromFile) {

	ssi_size_t n_banks = 0;
	Matrix<ssi_real_t> *intervals;

	if (fromFile){
		if (!ssi_exists(file)) {
			ssi_err("file '%s' does not exist", file);
		}

		File *fp = File::CreateAndOpen(File::ASCII, File::READ, file);
		if (!fp) {
			ssi_err("cannot open file '%s'", file);
		}

		fp->setType(SSI_REAL);
		ssi_real_t bank[2];
		while (fp->ready()) {
			if (fp->read(bank, 0, 2)) {
				n_banks++;
			}
		}

		if (_options.nbanks != n_banks) {
			ssi_err("#banks (%u) in file '%s' differs from #banks (%u) in options", n_banks, file, _options.nbanks);
		}

		intervals = new Matrix<ssi_real_t>(n_banks, 2);
		ssi_real_t *ptr = intervals->data;

		fp->seek(0, File::BEGIN);
		while (fp->ready()) {
			if (fp->read(ptr, 0, 2)) {
				ptr += 2;
			}
		}

		delete fp;

	}
	else{
    #if __ANDROID__
        ssi_wrn("Filterbank from string not supported on Android");
    #else
		std::string strBanks(file);
		n_banks = std::count(strBanks.begin(), strBanks.end(), ' ');

		if (_options.nbanks != n_banks) {
			ssi_err("#banks (%u) in string '%s' differs from #banks (%u) in options", n_banks, file, _options.nbanks);
		}

		intervals = new Matrix<ssi_real_t>(n_banks, 2);
		int delim = 0;
		int delim2 = 0;
		int current_bank = 0;

		while (strBanks.size() > 1 && delim2 >= 0){
			delim = strBanks.find_first_of(" ");
			delim2 = strBanks.find_first_of("\n");
			
			intervals->data[current_bank * 2] = std::stof(strBanks.substr(0, delim));
			int substrlen = (delim2 < 0) ? (strBanks.size() - delim) : (delim2 - delim);
			intervals->data[current_bank * 2 + 1] = std::stof(strBanks.substr(delim, substrlen));

			current_bank++;
			strBanks = strBanks.substr(delim2 + 1, strBanks.size() - delim2);

		}
      #endif
	}
	

	Matrix<ssi_real_t> *filterbank = FilterTools::Filterbank(_options.nfft, sr, intervals, _options.wintype);
	setFilterbank(filterbank, _options.wintype, _options.dolog);

	delete filterbank;
	delete intervals;
}

void Spectrogram::setFilterbank (const Matrix<ssi_real_t> *filterbank, 
	WINDOW_TYPE win_type,
	bool apply_log) {

	delete _fft; _fft = 0;
	delete _fftmag; _fftmag = 0;
	delete _filterbank; _filterbank = 0;
	delete _window; _window = 0;

	_fft_size = (filterbank->cols - 1) << 1;
	_win_type = win_type;
	_apply_log = apply_log;

	_filterbank = MatrixOps<ssi_real_t>::Clone (filterbank);
	MatrixOps<ssi_real_t>::Transpose (_filterbank);

	_fft = new FFT (_fft_size, 1);
	_fftmag = new Matrix<ssi_real_t> (1, _fft->rfft);
}

void Spectrogram::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {	

	if (!_filterbank) {
		if (_options.file[0] != '\0') {
			readFilterbank (_options.file, stream_in.sr, true);
		}
		else if(_options.banks[0] != '\0'){
			readFilterbank(_options.banks, stream_in.sr, false);
		}
		else {
			ssi_time_t maxfreq = _options.maxfreq > 0 ? _options.maxfreq : stream_in.sr / 2;
			Matrix<ssi_real_t> *filterbank = FilterTools::Filterbank (_options.nfft, stream_in.sr, _options.nbanks, _options.minfreq, maxfreq, _options.wintype);
			setFilterbank (filterbank, _options.wintype, _options.dolog);		
			delete filterbank;
		}		
	}
}

void Spectrogram::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	/* Matlab code:

	[wdata, steps] = wins (data, fs, flen, wlen, winname);
	coefs = zeros (steps, nbands);
	for i = 1:steps
	   ffts = _fft (wdata(i,:), fftsize);% .^2;
	   _fftmag = abs (ffts(1:bandsize));
	   coefs(i,:) = bands * _fftmag';
	end

	*/

	// some temporary help vars
	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;
	ssi_size_t sample_elements = sample_number * sample_dimension;
	ssi_real_t *srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);

	Matrix<ssi_real_t> matrix_in (sample_number, sample_dimension, srcptr);
	Matrix<ssi_real_t> matrix_out (1, _filterbank->cols, dstptr);

	transform (&matrix_in, &matrix_out);

	matrix_in.data = 0;
	matrix_out.data = 0;
}

void Spectrogram::transform (Matrix<ssi_real_t> *matrix_in, 
	Matrix<ssi_real_t> *matrix_out) {

	if (_win_size != matrix_in->rows) {
		_win_size = matrix_in->rows;
		if (_win_type != WINDOW_TYPE_RECTANGLE) {
			_window = FilterTools::Window (_win_size, _win_type, MATRIX_DIMENSION_COL);
		}
	}

	if (_win_type != WINDOW_TYPE_RECTANGLE) {
		MatrixOps<ssi_real_t>::Mult (matrix_in, _window);
	}

	_fft->transform (matrix_in, _fftmag);
	
	MatrixOps<ssi_real_t>::MultM (_fftmag, _filterbank, matrix_out);

	if (_apply_log) {
		MatrixOps<ssi_real_t>::Log10 (matrix_out);
	}
}

void Spectrogram::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	delete _window; _window = 0;
	_win_size = 0;
}

}
