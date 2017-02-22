// WekaWrapper.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/03/04
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

#pragma once

#ifndef SSI_WEKARAPPER_H
#define SSI_WEKARAPPER_H

#include "base/IModel.h"
#include "model/SampleList.h"
#include "model/ModelTools.h"
#include "ioput/file/FileBinary.h"
#include "ioput/option/OptionList.h"
#include "ioput/socket/ip/UdpSocket.h"

namespace ssi {
	
class WekaServer;

class WekaWrapper : public IModel {

public:

	enum DISTANCE_MEASURE_FUNCTION {
		EUCLIDIAN = 0
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: use_jni (false), local_port (121212), server_port (121213), n_classes (0), n_features (0) {

			setModel ("");
			setClassPath ("-Djava.class.path=.;C:\\Program Files\\Weka-3-6\\weka.jar");
			setLibPath ("-Djava.library.path=C:\\Program Files (x86)\\Java\\jre\\lib");

			addOption ("use_jni", &use_jni, 1, SSI_BOOL, "use jni to start java process");
			addOption ("local_port", &local_port, 1, SSI_INT, "local socket port (to receive messages from weka)");
			addOption ("server_port", &server_port, 1, SSI_INT, "server socket port (to send messages to weka)");			
			addOption ("n_classes", &n_classes, 1, SSI_SIZE, "number of classes");
			addOption ("n_features", &n_features, 1, SSI_SIZE, "number of features");
			addOption ("model", model, SSI_MAX_CHAR, SSI_CHAR, "path to weka model");
			addOption ("class_path", class_path, SSI_MAX_CHAR, SSI_CHAR, "path to weka model");
			addOption ("lib_path", lib_path, SSI_MAX_CHAR, SSI_CHAR, "path to java library folder");
		};

		void setModel (const ssi_char_t *path) {
			ssi_strcpy (model, path);
		}
		void setClassPath (const ssi_char_t *path) {
			ssi_strcpy (class_path, path);
		}
		void setLibPath (const ssi_char_t *path) {
			ssi_strcpy (lib_path, path);
		}

		int local_port;
		int server_port;
		ssi_char_t model[SSI_MAX_CHAR];
		ssi_char_t class_path[SSI_MAX_CHAR];
		ssi_char_t lib_path[SSI_MAX_CHAR];
		ssi_size_t n_classes;
		ssi_size_t n_features;
		bool use_jni;
	};

public:

	static const ssi_char_t *GetCreateName () { return "WekaWrapper"; };
	static IObject *Create (const ssi_char_t *file) { return new WekaWrapper (file); };
	
	WekaWrapper::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Wrapper for the famous machine learning library Weka."; };

	bool train (ISamples &samples,
		ssi_size_t stream_index);
	bool isTrained () { 
		return ssi_exists (_options.model) && _options.n_features > 0 && _options.n_classes > 0; 
	};
	bool forward (ssi_stream_t &stream,
		ssi_size_t n_probs,
		ssi_real_t *probs);	
	void release ();
	bool save (const ssi_char_t *filepath);
	bool load (const ssi_char_t *filepath);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	ssi_size_t getClassSize () { return _options.n_classes; };
	ssi_size_t getStreamDim () { return _options.n_features; };
	ssi_size_t getStreamByte () { return sizeof (ssi_real_t); };
	ssi_type_t getStreamType () { return SSI_REAL; };

protected:	

	WekaWrapper (const ssi_char_t *file = 0);
	virtual ~WekaWrapper ();
	WekaWrapper::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	WekaServer *_server;
	UdpSocket *_socket;
	IpEndpointName *_server_ip;
	IpEndpointName *_local_ip;


};

}

#endif
