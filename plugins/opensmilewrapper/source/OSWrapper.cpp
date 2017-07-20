// OSWrapper.cpp
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2008/01/18
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

#include <core/configManager.hpp>
#include <core/componentManager.hpp>
#include <core/smileCommon.hpp>

#include "ssiSink.hpp"
#include "ssiSource.hpp"

#include "OSWrapper.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *OSWrapper::ssi_log_name = "oswrapper_";

OSWrapper::OSWrapper (const ssi_char_t *file)
	: _configManager (0),
	_componentManager (0),
	_sourceConfig (0),
	_sink (0),
	_source (0),
	_sample_num_old (0),
	_openSmileReady (false),
	_scale (false),
	_option_file(0),
	ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {

	if (file) {
		if (!OptionList::LoadXML(file, &_options)) {
			OptionList::SaveXML(file, &_options);
		}
		_option_file = ssi_strcpy (file);
	}
}

OSWrapper::~OSWrapper () {

	if (_option_file) {
		OptionList::SaveXML (_option_file, &_options);
		delete[] _option_file;
	}
}

void OSWrapper::transform_enter (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_scale = stream_in.type == SSI_SHORT;
	if (_scale) {
		ssi_stream_init (_scaled_stream, 0, stream_in.dim, sizeof (ssi_real_t), SSI_REAL, stream_in.sr);
	}
}

void OSWrapper::transform (ITransformer::info info,
	ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	ssi_size_t sample_dimension = stream_in.dim;
	ssi_size_t sample_number = stream_in.num;

	ssi_real_t *srcptr;
	ssi_real_t *dstptr = ssi_pcast (ssi_real_t, stream_out.ptr);
	if (_scale) {

		ssi_stream_adjust (_scaled_stream, stream_in.num);

		short *in = ssi_pcast (short, stream_in.ptr);
		ssi_real_t *out = ssi_pcast (ssi_real_t, _scaled_stream.ptr);

		for (ssi_size_t i = 0; i < stream_in.num * stream_in.dim; i++) {
			*out++ = ssi_cast (ssi_real_t, *in++) / 32768.0f;
		}

		srcptr = ssi_pcast (ssi_real_t, _scaled_stream.ptr);
	} else {
		srcptr = ssi_pcast (ssi_real_t, stream_in.ptr);
	}

	// Read config if started for the first time
	if(!_openSmileReady) {
		initOpenSmileConfig();
	}

	try {

		static bool first_call = true;

		ssi_tic ();

		if (!_options.continuous || first_call) {

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "prepare openSmile..");	

			// if sample_num changed set the config value of the source new
			if (_sample_num_old != sample_number) {
				_sourceConfig = _configManager->getInstance(_options.sourceName);
				if(_sourceConfig) {
					_sourceConfig->setInt ("numberOfSamples", sample_number);
					_sourceConfig->setInt ("sampleRate", ssi_cast (int, stream_in.sr));
				}
			} 

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "reset openSmile (runtime: %ums)", ssi_toc ());	
			ssi_tic ();
			_componentManager->resetInstances ();

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "create openSmile (runtime: %ums)", ssi_toc ());	
			ssi_tic ();
			_componentManager->createInstances(0); // 0 = do not read config

			ssi_msg (SSI_LOG_LEVEL_DETAIL, "get source and sink and set data (runtime: %ums)", ssi_toc ());		
			ssi_tic ();
		
			first_call = false;

			// get source and sink to set the input/output pointers
			_source = (ssiSource *) _componentManager->getComponentInstance (_options.sourceName);
			_sink = (ssiSink *) _componentManager->getComponentInstance (_options.sinkName);
		}
		else
		{
			//_componentManager->
		}

		if (_source) {
			_source->setDataPointer(stream_in.num, srcptr);
		}
		if (_sink) {
			_sink->setDataPointer (stream_out.num * stream_out.dim, dstptr);
		}

		//printf ("smileWrapper datapointer %i\n", srcptr);
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "ready to run openSmile (runtime: %ums)", ssi_toc ());	
		ssi_tic ();
		long long nTicks = _componentManager->runMultiThreaded();
		//long long nTicks = _componentManager->runSingleThreaded();
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "stopped openSmile (runtime: %ums, ticks: %u)", ssi_toc (), ssi_cast (ssi_size_t, nTicks));		

	} catch (cSMILException *c) { 
		ssi_err ("%s", c->getText ());
	} 

	// remember last sample number
	_sample_num_old = sample_number;
}

void OSWrapper::transform_flush (ssi_stream_t &stream_in,
	ssi_stream_t &stream_out,
	ssi_size_t xtra_stream_in_num,
	ssi_stream_t xtra_stream_in[]) {

	_openSmileReady = false;
	_sample_num_old = 0;
	_scale = false;

	if(_configManager) {
		delete _configManager;
		_configManager = 0;
	}
	if(_componentManager) {		
		delete _componentManager;
		_componentManager = 0;
	}

	if (_scale) {
		ssi_stream_destroy (_scaled_stream);
	}
}

ssi_size_t OSWrapper::getSampleDimensionOut (ssi_size_t sample_dimension_in) 
{
	int num = 0;

	// if started for the first time read config 
	if(!_openSmileReady) {
		initOpenSmileConfig();
	}

	try {

		// get config instance
		_sourceConfig = _configManager->getInstance(_options.sourceName);

		if(_sourceConfig != NULL)
		{
			// set the size of the input data size (number of samples) to a dummy value 1
			// to save memory, this should not the influence the output number of features
			_sourceConfig->setInt("dataSize", 1);

			// reset old instances, to get rid of old data
			_componentManager->resetInstances ();
			_componentManager->createInstances(0); // 0 = do not read config (we already did that above..)

			// get the sink and fetch the number of output values
			_sink = (ssiSink *) _componentManager->getComponentInstance(_options.sinkName);
			if(_sink != NULL)
			{
				num = _sink->getNumberOfOutputValues();
			}
		}
	} catch(cSMILException *c) { 
		ssi_err("%s", c->getText());
	} 

	return num;
}

void OSWrapper::initOpenSmileConfig()
{
	ssi_msg(SSI_LOG_LEVEL_BASIC, "load config '%s' [v%s]", _options.configFile, APPVERSION);

	if (!ssi_exists(_options.configFile)) {
		ssi_err("config file '%s' not found", _options.configFile);
	}


	try {

		// set up the smile logger
		LOGGER.setLogLevel(_options.logLevel);
		if (_options.enableConsole) {
			LOGGER.enableConsoleOutput();
		}
		if (_options.logFile[0] != '\0') {
			LOGGER.setLogFile(_options.logFile);
		}

		// create _configManager:
		_configManager = new cConfigManager();
		_componentManager = new cComponentManager(_configManager,componentlist);
		_configManager->addReader(new cFileConfigReader(_options.configFile));
		_configManager->readConfig();

		_openSmileReady = true;

	} catch(cSMILException *c) { 
		ssi_err("%s", c->getText());
	} 
}

}

