// FrameFusion.cpp
// author: Johannes Wagner <johannes.wagner@informatik.uni-augsburg.de>
// created: 2018/02/25
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

#include "FrameFusion.h"
#include "base/Factory.h"
#include "ssiml/include/ISUnfoldSample.h"
#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

FrameFusion::FrameFusion (const ssi_char_t *file) 
	: _model (0),
	_file (0) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
} 

FrameFusion::~FrameFusion () { 

	release ();
	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
}

void FrameFusion::release () 
{
	if (_model && _model->isTrained())
	{
		_model->release();
	}
}

bool FrameFusion::train (ISamples &samples,
	ssi_size_t stream_index) {

	if (!_model)
	{
		ssi_wrn("no model");
		return false;
	}

	SampleList copy;
	ModelTools::CopySampleList(samples, copy);

	ISUnfoldSample unfold(&copy);
	unfold.set(stream_index, _options.n_context);
	
	return _model->train(unfold, stream_index);
}

bool FrameFusion::forward (ssi_stream_t &stream,
	ssi_size_t n_probs,
	ssi_real_t *probs,
	ssi_real_t &confidence) {

	if (!_model || !isTrained()) {
		ssi_wrn ("not trained");
		return false;
	}

	if (stream.num <= _options.n_context)
	{
		return false;
	}

	confidence = 0;
	for (ssi_size_t i = 0; i < n_probs; i++)
	{
		switch (_options.method)
		{
		case METHOD::SUM:
			probs[i] = 0;
			break;
		case METHOD::PRODUCT:
			probs[i] = 1;
			break;
		case METHOD::MAX:
			probs[i] = 0;
		}
	}

	ssi_size_t n = stream.num - _options.n_context;
	ssi_stream_t tmp_stream = stream;
	tmp_stream.dim *= 1 + _options.n_context;
	tmp_stream.num = 1;
	tmp_stream.tot = tmp_stream.dim * tmp_stream.byte;
	ssi_real_t *tmp_probs = new ssi_real_t[n_probs];
	ssi_real_t tmp_confidence = 0;
	for (ssi_size_t i = 0; i < n; i++)
	{
		if (!_model->forward(tmp_stream, n_probs, tmp_probs, tmp_confidence))
		{
			delete[] tmp_probs;
			return false;
		}
		confidence += tmp_confidence;
		for (ssi_size_t j = 0; j < n_probs; j++)
		{
			switch (_options.method)
			{
			case METHOD::SUM:
				probs[j] += tmp_probs[j];
				break;
			case METHOD::PRODUCT:
				probs[j] *= tmp_probs[j];
				break;
			case METHOD::MAX:
				probs[j] = max(probs[j], tmp_probs[j]);
				break;
			}
		}

		tmp_stream.ptr += stream.dim * stream.byte;
	}

	confidence /= n;

	delete[] tmp_probs;

	return true;
}

bool FrameFusion::load (const ssi_char_t *filepath) {
	
	File *file = File::CreateAndOpen (File::BINARY, File::READ, filepath);

	release ();

	file->read (&_options.method, sizeof (_options.method), 1);
	file->read (&_options.n_context, sizeof (_options.n_context), 1);
	
	ssi_size_t n_name = 0;
	file->read(&n_name, sizeof(ssi_size_t), 1);
	ssi_char_t *name = new ssi_char_t[n_name + 1];
	file->read(name, sizeof(ssi_char_t), n_name);
	name[n_name] = '\0';
	
	FilePath fp(filepath);
	ssi_char_t path[SSI_MAX_CHAR];
	ssi_sprint(path, "%s.%s.option", fp.getPath(), name);
	_model = ssi_pcast(IModel, Factory::Create(name, path, true));
	ssi_sprint(path, "%s.%s.model", fp.getPath(), name);
	_model->load(path);

	delete file;

	return true;
}

bool FrameFusion::save (const ssi_char_t *filepath) {

	if (!_model || !isTrained()) {
		ssi_wrn ("not trained");
		return false;
	}

	File *file = File::CreateAndOpen (File::BINARY, File::WRITE, filepath);

	file->write (&_options.method, sizeof (_options.method), 1);
	file->write (&_options.n_context, sizeof (_options.n_context), 1);
	
	const ssi_char_t *name = _model->getName();
	ssi_size_t n_name = ssi_strlen(name);
	file->write(&n_name, sizeof(ssi_size_t), 1);
	file->write(name, sizeof(ssi_char_t), n_name);

	FilePath fp(filepath);
	ssi_char_t path[SSI_MAX_CHAR];
	ssi_sprint(path, "%s.%s.model", fp.getPath(), _model->getName());
	_model->save(path);
	ssi_sprint(path, "%s.%s.option", fp.getPath(), _model->getName());
	Options::SaveXML(path, _model->getOptions());

	delete file;

	return true;
}


void FrameFusion::setModel(IModel *model)
{
	_model = model;
}

}
