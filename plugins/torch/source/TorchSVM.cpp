// TorchSVM.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/02/19
// Copyright (C) University of Augsburg

#include "TorchSVM.h"

#include "ioput/file/FileBinary.h"
#include "TorchTools.h"
#include "QCTrainer.h"
#include "DiskXFile.h"
#include "Kernel.h"
#include "SVM.h"
#include "SVMClassification.h"
#include "ClassFormatDataSet.h"
#include "MemoryDataSet.h"

namespace ssi {

TorchSVM::TorchSVM (const ssi_char_t *file) 
	: _svms (0),
	_kernel (0),
	_n_classes (0),
	_file (0) {	

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

TorchSVM::~TorchSVM () { 

	release ();

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

bool TorchSVM::train (ISamples &samples, ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
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

	if (_options.degree > 0) {
		_kernel = new Torch::PolynomialKernel (_options.degree, _options.a_cst, _options.b_cst);
	} else {
		_kernel = new Torch::GaussianKernel (1.0f / (_options.stdv * _options.stdv));
	}
	
	_svms = new Torch::SVM *[_n_classes];
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++)
	{
		_svms[nclass] = new Torch::SVMClassification (_kernel);
		_svms[nclass]->setROption("C", _options.c_cst);
		_svms[nclass]->setROption("cache size", _options.cache_size);
	}

	Torch::MemoryDataSet *data_set = TorchTools::CreateMemoryDataSet (samples, stream_index, true);

	for (ssi_size_t i = 0; i < _n_classes; i++)
    {
		Torch::QCTrainer trainer (_svms[i]);
		trainer.setROption("end accuracy", _options.accuracy);
		trainer.setIOption("iter shrink", _options.iter_shrink);

		Torch::Sequence class_labels (_n_classes, 1);
		for (ssi_size_t j = 0; j < _n_classes; j++)
		{
			if(j == i)
				class_labels.frames[j][0] =  1;
			else
				class_labels.frames[j][0] = -1;
		}
		Torch::ClassFormatDataSet data (data_set, &class_labels);

		trainer.train (&data, NULL);
		ssi_print ("%d SV with %d at bounds", _svms[i]->n_support_vectors, _svms[i]->n_support_vectors_bound);		
    }
	
	delete data_set;

	return true;
}

bool TorchSVM::forward(ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence, ssi_video_params_t &params)
{
	return forward(stream, n_probs, probs, confidence);
}

bool TorchSVM::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence) {

	if (!_svms) {
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

	for(int i = 0; i < _n_classes; i++) {
		_svms[i]->forward (sequence);
		probs[i] = _svms[i]->outputs->frames[0][0];
	}
	

/*

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
	}*/

	ssi_max(n_probs, 1, probs, &confidence);

	return true;
}

bool TorchSVM::load (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);
	Torch::DiskXFile xfile (file->getFile ());

	release ();

	// read parameters
	file->read (&_n_features, sizeof (_n_features), 1);
	file->read (&_n_classes, sizeof (_n_classes), 1);

	// load model 
	if (_options.degree > 0) {
		_kernel = new Torch::PolynomialKernel (_options.degree, _options.a_cst, _options.b_cst);
	} else {
		_kernel = new Torch::GaussianKernel (1.0f / (_options.stdv * _options.stdv));
	}
	
	_svms = new Torch::SVM *[_n_classes];
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		_svms[nclass] = new Torch::SVMClassification (_kernel);
		_svms[nclass]->loadXFile (&xfile);
	}

	delete file;
	return true;
}

bool TorchSVM::save (const ssi_char_t *filepath) {

	if (!_svms) {
		ssi_wrn ("not trained");
		return false;
	}

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);
	Torch::DiskXFile xfile (file->getFile ());
	
	// store parameters
	file->write (&_n_features, sizeof (_n_features), 1);
	file->write (&_n_classes, sizeof (_n_classes), 1);

	// store model 
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		_svms[nclass]->saveXFile (&xfile);
	}

	delete file;
	return true;
}

void TorchSVM::release () {

	delete _kernel;
	_kernel = 0;
	for (ssi_size_t nclass = 0; nclass < _n_classes; nclass++) {
		delete _svms[nclass];
	}
	delete[] _svms;
	_svms = 0;	
	_n_classes = 0;
	_n_features = 0;
}

}
