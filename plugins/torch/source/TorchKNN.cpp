// TorchKNN.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/19
// Copyright (C) University of Augsburg

#include "TorchKNN.h"

#include "ioput/file/FileBinary.h"
#include "TorchTools.h"
#include "NPTrainer.h"
#include "DiskXFile.h"

#include "MemoryDataSet.h"
#include "KNN.h"

namespace ssi {

TorchKNN::TorchKNN (const ssi_char_t *file) 
	:_knn (0), 
	_data_set (0),
	_n_classes (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

TorchKNN::~TorchKNN () { 

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool TorchKNN::train (ISamples &samples, ssi_size_t stream_index) {

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

	ssi_size_t sample_size = samples.getSize ();
	_n_classes = samples.getClassSize ();
	_n_features = samples.getStream (stream_index).dim;

	samples.reset ();

	_data_set = TorchTools::CreateMemoryDataSet (samples, stream_index, true);
	_knn = new Torch::KNN (_data_set->n_targets, _options.k);
	Torch::NPTrainer trainer (_knn);
	trainer.train (_data_set, NULL);

	return true;
}

bool TorchKNN::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence) {

	if (!_knn) {
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

	ssi_size_t sample_dimension = stream.dim;
	ssi_size_t sample_number = stream.num;
	ssi_real_t *sample_data = ssi_pcast (ssi_real_t, stream.ptr);

	Torch::Sequence *sequence = TorchTools::CreateSequence (sample_dimension, sample_number, sample_data);
	_knn->forward (sequence);
	delete sequence;
	
	ssi_real_t *probptr = probs;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		*probptr++ = 0;
	}
	probptr = probs;
	for (ssi_size_t i = 0; i < _options.k; i++) {
		_data_set->setExample (_knn->indices[i], false, true);
		ssi_size_t index = _data_set->targets->frames[0][0];
		*(probptr + index) += 1.0f;
	}
	// normalize
	probptr = probs;
	for (ssi_size_t i = 0; i < _n_classes; i++) {
		*probptr++ /= _options.k;
	}

	ssi_max(n_probs, 1, probs, &confidence);

	return true;
}

bool TorchKNN::load (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	release ();

	// read parameters
	file->read (&_options.k, sizeof (_options.k), 1);
	file->read (&_n_features, sizeof (_n_features), 1);
	file->read (&_n_classes, sizeof (_n_classes), 1);

	// load model 
	_data_set = new Torch::MemoryDataSet ();
	int n_examples;
	file->read (&n_examples, sizeof (int), 1);
	Torch::Sequence **inputs = reinterpret_cast<Torch::Sequence **> (_data_set->allocator->alloc (sizeof (Torch::Sequence *) * n_examples));
	Torch::Sequence **targets = reinterpret_cast<Torch::Sequence **> (_data_set->allocator->alloc (sizeof (Torch::Sequence *) * n_examples));
	for (int i = 0; i < n_examples; i++) {
		inputs[i] = new (_data_set->allocator) Torch::Sequence (1, _n_features);
		targets[i] = new (_data_set->allocator) Torch::Sequence (1, 1);
		file->read (inputs[i]->frames[0], sizeof (real), _n_features);
		file->read (targets[i]->frames[0], sizeof (real), 1);
	}
	_data_set->setInputs (inputs, n_examples);
	_data_set->setTargets (targets, n_examples);

	// initialize classifier
	_knn = new Torch::KNN (_data_set->n_targets, _options.k);
	Torch::NPTrainer trainer (_knn);
	trainer.train (_data_set, NULL);

	delete file;
	return true;
}

bool TorchKNN::save (const ssi_char_t *filepath) {

	if (!_knn) {
		ssi_wrn ("not trained");
		return false;
	}

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);
	
	// store parameters
	file->write (&_options.k, sizeof (_options.k), 1);
	file->write (&_n_features, sizeof (_n_features), 1);
	file->write (&_n_classes, sizeof (_n_classes), 1);

	// store model 
	file->write (&_data_set->n_examples, sizeof (int), 1);
	for (int i = 0; i < _data_set->n_examples; i++) {
		_data_set->setRealExample (i, true, true);
		file->write (_data_set->inputs->frames[0], sizeof (real), _data_set->n_inputs);
		file->write (_data_set->targets->frames[0], sizeof (real), 1);
	}

	delete file;
	return true;
}

void TorchKNN::release () {

	delete _knn;
	_knn = 0;
	delete _data_set;
	_data_set = 0;
	_n_classes = 0;
	_n_features = 0;
}

}
