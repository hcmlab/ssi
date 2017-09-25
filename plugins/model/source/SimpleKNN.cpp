// SimpleKNN.cpp
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

#include "SimpleKNN.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

#if __gnu_linux__
using std::min;
using std::max;
#endif

namespace ssi {

SimpleKNN::SimpleKNN (const ssi_char_t *file) 
	: _n_samples (0),	
	_n_features (0),
	_data (0),
	_classes (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

SimpleKNN::~SimpleKNN () { 

	release ();
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void SimpleKNN::release () 
{
	delete[] _data;	
	_data = 0;
	delete[] _classes;
	_classes = 0;
	_n_features = 0;
	_n_samples = 0;
	_n_classes = 0;
}

bool SimpleKNN::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	if (samples.getSize () < _options.k) {
		ssi_wrn ("sample list has less than '%u' entries", _options.k);
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = samples.getClassSize ();
	_n_samples = samples.getSize ();	
	_n_features = samples.getStream (stream_index).dim;
	_data = new ssi_real_t[_n_features*_n_samples];
	_classes = new ssi_size_t[_n_samples];

	ssi_sample_t *sample;	
	samples.reset ();
	ssi_real_t *data_ptr = _data;
	ssi_size_t *class_ptr = _classes;
	ssi_stream_t *stream_ptr = 0;
	ssi_size_t bytes_to_copy = _n_features * sizeof (ssi_real_t);
	while (sample = samples.next ()) {				
		memcpy (data_ptr, sample->streams[stream_index]->ptr, bytes_to_copy);
		*class_ptr++ = sample->class_id;
		data_ptr += _n_features;
	}	 

	return true;
}

bool SimpleKNN::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {

	if (!_data) {
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

	ssi_size_t k = _options.k;
	DistanceMeasureFunc dist_fptr;
	switch (_options.dist) {
		case SimpleKNN::EUCLIDIAN:
			dist_fptr = SimpleKNN::EuclideanSquaredReal;
			break;
		default:
			ssi_err ("unkown distance measure function");
			return false;			
	}

	std::multimap<ssi_real_t, ssi_size_t> closestSamplesMap;
	typedef std::pair<ssi_real_t, ssi_size_t> samplePair;
	std::multimap<ssi_real_t, ssi_size_t>::iterator mapIter;
	
	ssi_real_t *probptr = probs;
	for (ssi_size_t i = 0; i < n_probs; i++)
	{
		*probptr++ = 0;
	}
	
	ssi_real_t *data_ptr = _data;	
	ssi_size_t *class_ptr = _classes;
	ssi_real_t currentMaxDistance = 0.0f;
	ssi_real_t currentDistance = 0.0f;
	ssi_real_t *sample_ptr = ssi_pcast (ssi_real_t, stream.ptr);

	for (ssi_size_t i = 0; i < k; ++i)
	{		
		currentDistance = (*dist_fptr) (sample_ptr, data_ptr, _n_features);
		closestSamplesMap.insert (samplePair (currentDistance, *class_ptr));
		currentMaxDistance = max (currentMaxDistance, currentDistance);

		class_ptr++;
		data_ptr += _n_features;
	}

	for (ssi_size_t i = k; i < _n_samples; ++i)
	{
		currentDistance = (*dist_fptr) (sample_ptr, data_ptr, _n_features);
		if (currentDistance < currentMaxDistance)
		{
			closestSamplesMap.insert (samplePair (currentDistance, *class_ptr));
			mapIter = --closestSamplesMap.end();
			closestSamplesMap.erase (mapIter--);
			currentMaxDistance = mapIter->first;
		}

		class_ptr++;
		data_ptr += _n_features;
	}

	probptr = probs;
	for (mapIter = closestSamplesMap.begin(); mapIter != closestSamplesMap.end(); ++mapIter)
	{
		ssi_size_t index = mapIter->second;
		*(probptr + index) += 1.0f;
	}
	// normalize
	probptr = probs;
	for (ssi_size_t i = 0; i < n_probs; i++)
	{
		*probptr++ /= k;
	}

	return true;
}

bool SimpleKNN::load (const ssi_char_t *filepath) {
	
	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	release ();

	file->read (&_options.k, sizeof (_options.k), 1);
	file->read (&_options.dist, sizeof (_options.dist), 1);
	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_samples, sizeof (_n_samples), 1);
	file->read (&_n_features, sizeof (_n_features), 1);	
	_data = new ssi_real_t[_n_samples * _n_features];
	file->read (_data, sizeof (ssi_real_t), _n_samples * _n_features);
	_classes = new ssi_size_t[_n_samples];
	file->read (_classes, sizeof (ssi_size_t), _n_samples);

	delete file;

	return true;
}

bool SimpleKNN::save (const ssi_char_t *filepath) {

	if (!_data) {
		ssi_wrn ("not trained");
		return false;
	}

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&_options.k, sizeof (_options.k), 1);
	file->write (&_options.dist, sizeof (_options.dist), 1);
	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_samples, sizeof (_n_samples), 1);
	file->write (&_n_features, sizeof (_n_features), 1);
	file->write (_data, sizeof (ssi_real_t), _n_features * _n_samples);
	file->write (_classes, sizeof (ssi_size_t), _n_samples);

	delete file;

	return true;
}

SSI_INLINE ssi_real_t SimpleKNN::EuclideanSquaredReal (ssi_real_t *x1, ssi_real_t *x2, ssi_size_t numberDimensions) {

	ssi_real_t retVal = 0.0f;
	ssi_real_t tempDifference = 0.0f;
	for(ssi_size_t i = 0; i < numberDimensions; ++i)
	{
		tempDifference = *x1++ - *x2++;
		retVal += tempDifference * tempDifference;
	}

	return retVal;
}

}
