// OSWrapper.h
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

/**

Provides OSWrapper analysis.

*/

#pragma once

#ifndef SSI_TRANSFORMER_OSWRAPPER_H
#define SSI_TRANSFORMER_OSWRAPPER_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

class cConfigManager;
class cComponentManager;
class ConfigInstance;
class ssiSink;
class ssiSource;

namespace ssi {

class OSWrapper : public IFeature {

public:

	class Options : public OptionList {

	public:

		Options ()
			: enableConsole(false), logLevel (0) {

			configFile[0] = '\0';
			setSourceName ("ssiSource");
			setSinkName ("ssiSink");
			logFile[0] = '\0';
			continuous = false;

			addOption ("configFile", configFile, SSI_MAX_CHAR, SSI_CHAR, "the openSmile config file path");
			addOption ("sourceName", sourceName, SSI_MAX_CHAR, SSI_CHAR, "the name of the cSSISource component in the openSMILE config file");
			addOption ("sinkName", sinkName, SSI_MAX_CHAR, SSI_CHAR, "the name of the cSSISink component in the openSMILE config file");
			addOption ("continuous", &continuous, 1, SSI_BOOL, "enable continuous processing");
			addOption ("enableConsole", &enableConsole, 1, SSI_BOOL, "enable openSMILE console messages");
			addOption ("logLevel", &logLevel, 1, SSI_INT, "openSMILE log level (0-4)");
			addOption ("logFile", logFile, SSI_MAX_CHAR, SSI_CHAR, "openSMILE log file");
		};

		void setConfigPath (const ssi_char_t *string) {
			this->configFile[0] = '\0';
			if (string) {
				ssi_strcpy (this->configFile, string);
			}
		}

		void setSourceName (const ssi_char_t *string) {
			this->sourceName[0] = '\0';
			if (string) {
				ssi_strcpy (this->sourceName, string);
			}
		}

		void setSinkName (const ssi_char_t *string) {
			this->sinkName[0] = '\0';
			if (string) {
				ssi_strcpy (this->sinkName, string);
			}
		}

		void setLogFile (const ssi_char_t *string) {

			this->logFile[0] = '\0';
			if (string) {
				ssi_strcpy(this->logFile, string);
			}
		}
		ssi_char_t configFile[SSI_MAX_CHAR];
		ssi_char_t sourceName[SSI_MAX_CHAR];
		ssi_char_t sinkName[SSI_MAX_CHAR];
		bool continuous;
		bool enableConsole;
		int logLevel;
		ssi_char_t logFile[SSI_MAX_CHAR];
	};

	static const ssi_char_t *GetCreateName () { return "OSWrapper"; };
	static IObject *Create (const ssi_char_t *file) { return new OSWrapper (file); };
	
	~OSWrapper ();

	OSWrapper::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "A wrapper to extract features with openSMILE (expects a single feature vector per chunk)"; };

	void transform_enter (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform (ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);
	void transform_flush (ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num = 0,
		ssi_stream_t xtra_stream_in[] = 0);

	ssi_size_t getSampleDimensionOut (ssi_size_t sample_dimension_in);
	ssi_size_t getSampleBytesOut (ssi_size_t sample_bytes_in) {
		return sizeof (ssi_real_t);
	};
	ssi_type_t getSampleTypeOut (ssi_type_t sample_type_in) {
		if (sample_type_in != SSI_REAL && sample_type_in != SSI_SHORT) {
			ssi_err ("type %s not supported", SSI_TYPE_NAMES[sample_type_in]);
			return SSI_UNDEF;
		}
		return SSI_REAL;
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	int ssi_log_level;
	static ssi_char_t *ssi_log_name;

	// init config and component manager with the provided configuration from
	// the opensmile config file
	void initOpenSmileConfig();

	OSWrapper (const ssi_char_t *file = 0);
	ssi_char_t *_option_file;
	OSWrapper::Options _options;

	// signals if the config file was parsed and config and component manager
	// have been initalized
	bool _openSmileReady;
	int _sample_num_old;

	// open Smile config and component manager
	cConfigManager *_configManager;
	cComponentManager *_componentManager;
	ConfigInstance *_sourceConfig;
	ssiSink *_sink;
	ssiSource *_source;

	bool _scale;
	ssi_stream_t _scaled_stream;

};

}

#endif
