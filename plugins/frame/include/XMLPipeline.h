// XMLPipeline.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/03/29
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute itand/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any laterversion.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FORA PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along withthis library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#pragma once

#ifndef SSI_FRAME_XMLPIPELINE_H
#define SSI_FRAME_XMLPIPELINE_H

#include "base/IXMLPipeline.h"
#include "base/ITheFramework.h"
#include "base/ITheEventBoard.h"
#include "base/String.h"
#include "base/Factory.h"

namespace ssi {

class TiXmlElement;

class XMLPipeline : public IXMLPipeline {

friend class Chain;

public:

	enum VERSION {
		V1 = 1,		// original format
	};
	static XMLPipeline::VERSION DEFAULT_VERSION;
	static const ssi_char_t *DATE_VARIABLE_NAME;

public:

	~XMLPipeline ();

	IOptions *getOptions () { return 0; }
	static const ssi_char_t *GetCreateName () { return "XMLPipeline"; }
	const ssi_char_t *getName () { return GetCreateName(); }
	const ssi_char_t *getInfo () { return "Handles the interpretation and execution of a xml pipeline."; }
	static IObject *Create (const char *file) {	return new XMLPipeline (); }

	virtual void SetRegisterDllFptr (Factory::register_dll_fptr_t register_fptr) {
		_register_fptr = register_fptr;
	}
	bool parse (const ssi_char_t *filepath, ssi_size_t n_confs = 0, ssi_char_t **confpaths = 0, bool savepipe = false, bool included = false);
	IObject *parseObject (TiXmlElement *element, bool auto_free = true);

	ITransformable *getTransformable (const ssi_char_t *name);
	IConsumer *getConsumer (const ssi_char_t *name);
	void clear ();

	virtual void setExeDir (const ssi_char_t *exepath) {
		ssi_strcpy (_dir_exe, exepath);
	}

	virtual bool startEventBoard () {
		return _start_eboard;
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	};

protected:

	XMLPipeline ();

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;

	ssi_char_t *applyConfig (ssi_char_t *pipestr, const ssi_char_t *confpath);

	bool parseElement (TiXmlElement *element);
	bool parseInclude (TiXmlElement *element);
	bool parseRegister (TiXmlElement *element);
	bool parseSensor (TiXmlElement *element);
	bool parseTransformer (TiXmlElement *element);
	bool parseConsumer (TiXmlElement *element);
	bool parseOption (TiXmlElement *element);
	bool parseFramework (TiXmlElement *element);
	bool parseEventBoard (TiXmlElement *element);
	bool parseListener (TiXmlElement *element);
	bool parseRunnable(TiXmlElement *element);
	bool parseExecute(TiXmlElement *element);
	bool parseJob(TiXmlElement *element);
	bool parseGate (TiXmlElement *element);

	bool getChildSize(TiXmlElement *mama, const ssi_char_t *value, ssi_size_t &n_childs);

	typedef std::map<String, ITransformable *> transformable_map_t;
	typedef std::pair<String, ITransformable *> transformable_pair_t;
	transformable_map_t _transformable_map;

	typedef std::map<String, IConsumer *> consumer_map_t;
	typedef std::pair<String, IConsumer *> consumer_pair_t;
	consumer_map_t _consumer_map;

	ITheFramework *_frame;
	Factory::register_dll_fptr_t *_register_fptr;
	ITheEventBoard *_eboard;
	bool _start_eboard;

	ssi_size_t _n_global_confpaths;
	ssi_char_t **_global_confpaths;
	bool _savepipe;

	ssi_char_t _dir_pipe[SSI_MAX_CHAR];
	ssi_char_t _dir_exe[SSI_MAX_CHAR];
	ssi_char_t _dir_work[SSI_MAX_CHAR];

	ssi_char_t _date[ssi_time_size_friendly];

	bool compare (const ssi_char_t *s1, const ssi_char_t *s2);

};

}

#endif
