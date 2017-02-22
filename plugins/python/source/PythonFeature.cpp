// PythonFeature.cpp
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

#include "PythonFeature.h"
#include "PythonHelper.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

ssi_char_t *PythonFeature::ssi_log_name = "pyfeature_";

PythonFeature::PythonFeature(const ssi_char_t *file)
	: _file(0),
	_helper(0)
{

	if (file) {
		if (!OptionList::LoadXML(file, _options)) {
			OptionList::SaveXML(file, _options);
		}
		_file = ssi_strcpy(file);
	}
}

PythonFeature::~PythonFeature() {

	if (_file) {
		OptionList::SaveXML(_file, _options);
		delete[] _file;
	}

	delete _helper;	_helper = 0;
}

void PythonFeature::initHelper()
{
	_helper = new PythonHelper(_options.script, _options.optsfile, _options.optsstr, _options.syspath);

	ssi_msg(SSI_LOG_LEVEL_DETAIL, "python is ready");
}

void PythonFeature::transform_enter(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_helper->transform_enter(stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
}

void PythonFeature::transform_flush(ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_helper->transform_flush(stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
}

void PythonFeature::transform(ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_helper->transform(&info, stream_in, stream_out, xtra_stream_in_num, xtra_stream_in);
}

ssi_size_t PythonFeature::getSampleDimensionOut(ssi_size_t sample_dimension_in) {

	if (!_helper)
	{
		initHelper();
	}

	return _helper->getSampleDimensionOut(sample_dimension_in);
}

ssi_size_t PythonFeature::getSampleBytesOut(ssi_size_t sample_bytes_in) {

	if (!_helper)
	{
		initHelper();
	}

	return _helper->getSampleBytesOut(sample_bytes_in);
}

ssi_type_t PythonFeature::getSampleTypeOut(ssi_type_t sample_type_in) {

	if (!_helper)
	{
		initHelper();
	}

	return _helper->getSampleTypeOut(sample_type_in);
}

bool PythonFeature::setEventListener(IEventListener *listener)
{
	if (!_helper)
	{
		initHelper();
	}

	return _helper->setEventListener(listener);
}

const ssi_char_t *PythonFeature::getEventAddress()
{
	if (!_helper)
	{
		initHelper();
	}

	return _helper->getEventAddress();
}

void PythonFeature::send_enter()
{
	if (!_helper)
	{
		initHelper();
	}

	_helper->send_enter();
}

void PythonFeature::send_flush()
{
	if (!_helper)
	{
		initHelper();
	}

	_helper->send_flush();
}

void PythonFeature::listen_enter()
{
	if (!_helper)
	{
		initHelper();
	}

	_helper->listen_enter();
}

bool PythonFeature::update(ssi_event_t &e)
{
	if (!_helper)
	{
		initHelper();
	}

	return _helper->update(e);
}

bool PythonFeature::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms)
{
	if (!_helper)
	{
		initHelper();
	}

	return _helper->update(events, n_new_events, time_ms);
}

void PythonFeature::listen_flush()
{
	if (!_helper)
	{
		initHelper();
	}

	_helper->listen_flush();
}

}

