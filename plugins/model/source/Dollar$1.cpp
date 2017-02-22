// Dollar$1.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
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

#include "Dollar$1.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

Dollar$1::Dollar$1 (const ssi_char_t *file)
: _n_classes (0),
	_n_features (2),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}
} 

Dollar$1::~Dollar$1 () { 

	release ();
	if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}
}

void Dollar$1::release () {

	_dollar1.templates.clear ();
	_n_classes = 0;
}

bool Dollar$1::train (ISamples &samples, 
	ssi_size_t stream_index) {

	if (samples.getSize () == 0) {
		ssi_wrn ("empty sample list");
		return false;
	}

	ssi_stream_t stream = samples.getStream (stream_index);
	
	if (stream.type != SSI_REAL) {
		ssi_wrn ("stream type not compatible");
		return false;
	}

	if (isTrained ()) {
		ssi_wrn ("already trained");
		return false;
	}

	_n_classes = samples.getClassSize ();

	samples.reset ();
	ssi_sample_t *sample = 0;
	while (sample = samples.next ()) {
		Path2D path;
		addToPath (ssi_pcast (ssi_real_t, sample->streams[stream_index]->ptr), sample->streams[stream_index]->num, sample->streams[stream_index]->dim, _options.indx, _options.indy, path);
		_dollar1.addTemplate (sample->class_id, path);
	}

	return true;
}

void Dollar$1::addToPath (ssi_real_t *ptr, 	
	ssi_size_t num,
	ssi_size_t dim,
	ssi_size_t ind_x,
	ssi_size_t ind_y,
	Path2D &path) {

	ssi_real_t x,y;
	for (ssi_size_t i = 0; i < num; i++) {
		x = *(ptr + ind_x);
		y = *(ptr + ind_y);
		path.push_back (Point2D (x,y));
		ptr += dim;
	}
}

bool Dollar$1::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs) {
	
	if (stream.type != SSI_REAL) {
		ssi_wrn ("stream type not compatible");
		return false;
	}

	Path2D path;
	addToPath (ssi_pcast (ssi_real_t, stream.ptr), stream.num, stream.dim, _options.indx, _options.indy, path);

	//RecognitionResult result = _dollar1.recognize (path);
	if (!_dollar1.recognizeBestPerClass (path, n_probs, probs)) {
		return false;
	}

	if (_options.norm) {
		ssi_real_t sum = 0;
		for (ssi_size_t i = 0; i < n_probs; i++) {
			sum += probs[i];		
		}
		for (ssi_size_t i = 0; i < n_probs; i++) {
			probs[i] /= sum;
		}
	}

	return true;
}

bool Dollar$1::load (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);
	file->read (&_n_classes, sizeof (_n_classes), 1);
	file->read (&_n_features, sizeof (_n_features), 1);
	file->read (&_options.indx, sizeof (_options.indx), 1);
	file->read (&_options.indy, sizeof (_options.indy), 1);
	_dollar1.load (file->getFile ());	
	delete file;

	return true;
}

bool Dollar$1::save (const ssi_char_t *filepath) {

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);
	file->write (&_n_classes, sizeof (_n_classes), 1);
	file->write (&_n_features, sizeof (_n_features), 1);
	file->write (&_options.indx, sizeof (_options.indx), 1);
	file->write (&_options.indy, sizeof (_options.indy), 1);
	_dollar1.save (file->getFile ());
	delete file;

	return true;
}

}
