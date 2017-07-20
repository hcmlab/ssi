// PythonConsumer.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2016/03/02
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

#include "PythonModel.h"
#include "PythonHelper.h"
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

ssi_char_t *PythonModel::ssi_log_name = "pymodel";

PythonModel::PythonModel(const ssi_char_t *file)
	: _file (0),
	_isTrained(false),
	_helper(0) 
{

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_file = ssi_strcpy (file);
	}
}

PythonModel::~PythonModel() {

	if (_file) {
		OptionList::SaveXML(_file, &_options);
		delete[] _file;
	}
	
	delete _helper;	_helper = 0;
}

void PythonModel::initHelper()
{
	ssi_char_t *workdir = 0;
	if (_file)
	{
		FilePath fp(_file);
		workdir = ssi_strcpy(fp.getDir());
	}

	_helper = new PythonHelper(_options.script, _options.optsfile, _options.optsstr, _options.syspath, workdir);

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "python is ready");

	delete[] workdir;
}


void PythonModel::release()
{

}

bool PythonModel::train(ISamples &samples,
	ssi_size_t stream_index) {
	if (!_helper)
	{
		initHelper();
	}

	_isTrained = _helper->train(samples, stream_index);

	return _isTrained;
}

bool PythonModel::forward(ssi_stream_t &stream, ssi_size_t n_probs, ssi_real_t *probs) {
	if (!_helper)
	{
		initHelper();
	}

	return _helper->forward(stream, n_probs, probs);
}

bool PythonModel::load(const ssi_char_t *filepath) {
	if (!_helper)
	{
		initHelper();
	}

	return _helper->load(filepath);
}

bool PythonModel::save(const ssi_char_t *filepath) {
	if (!_helper)
	{
		initHelper();
	}
	return _helper->save(filepath);
}
}

