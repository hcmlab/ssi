// MyModel.cpp
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/03/23
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

#include "MyModel.h"
#include "ioput/file/File.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

MyModel::MyModel () 
	: _n_samples (0),	
	_n_features (0),
	_n_classes (0),
	_centers (0) {
} 

MyModel::~MyModel () { 
}

void MyModel::release () 
{
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		delete[] _centers[i];
	}
	delete[] _centers;
	_centers = 0;
	_n_features = 0;
	_n_samples = 0;
	_n_classes = 0;
}

bool MyModel::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = samples.getClassSize ();
	_n_features = samples.getStream (stream_index).dim;
	_centers = new ssi_real_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_centers[i] = new ssi_real_t[_n_features];
		for (ssi_size_t j = 0; j < _n_features; j++) {
			_centers[i][j] = 0;
		}
	}

	ssi_sample_t *sample;	
	samples.reset ();
	ssi_real_t *ptr = 0;
	while (sample = samples.next ()) {				
		ssi_size_t id = sample->class_id;	
		ptr = ssi_pcast (ssi_real_t, sample->streams[stream_index]->ptr);
		for (ssi_size_t j = 0; j < _n_features; j++) {
			_centers[id][j] += ptr[j];
		}
	}	 

	for (ssi_size_t i = 0; i < _n_classes; i++) {
		ssi_size_t num = samples.getSize (i);
		for (ssi_size_t j = 0; j < _n_features; j++) {
			_centers[i][j] /= num;
		}
	}

	return true;
}

bool MyModel::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_centers) {
		ssi_wrn ("not trained");
		return false;
	}

	if (n_probs != _n_classes) {
		ssi_wrn ("#classes differs");
		return false;
	}

	if (stream.type != SSI_REAL) {
		ssi_wrn ("type differs");
		return false;
	}

	if (stream.dim != _n_features) {
		ssi_wrn ("feature dimension differs");
		return false;
	}

	ssi_real_t *ptr = ssi_pcast (ssi_real_t, stream.ptr);
	ssi_real_t sum = 0;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		probs[i] = 1 / dist (ptr, _centers[i], _n_features);
		sum += probs[i];
	}
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		probs[i] /= sum;
	}

	return true;
}

bool MyModel::load (const ssi_char_t *filepath) {
	
	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	release ();

	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_samples, sizeof (_n_samples), 1);
	file->read (&_n_features, sizeof (_n_features), 1);	
	_centers = new ssi_real_t *[_n_classes];
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		_centers[i] = new ssi_real_t[_n_features];
		file->read (_centers[i], sizeof (ssi_real_t), _n_features);
	}
	
	delete file;

	return true;
}

bool MyModel::save (const ssi_char_t *filepath) {

	if (!_centers) {
		ssi_wrn ("not trained");
		return false;
	}

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_samples, sizeof (_n_samples), 1);
	file->write (&_n_features, sizeof (_n_features), 1);
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		file->write (_centers[i], sizeof (ssi_real_t), _n_features);
	}

	delete file;

	return true;
}

SSI_INLINE ssi_real_t MyModel::dist (ssi_real_t *x1, ssi_real_t *x2, ssi_size_t n_dim) {

	ssi_real_t retVal = 0.0f;
	ssi_real_t tempDifference = 0.0f;
	for(ssi_size_t i = 0; i < n_dim; ++i)
	{
		tempDifference = *x1++ - *x2++;
		retVal += tempDifference * tempDifference;
	}

	return retVal;
}

}
