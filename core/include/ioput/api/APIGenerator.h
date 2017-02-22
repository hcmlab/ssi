// APIGenerator.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2011/04/01
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

#ifndef SSI_IOPUT_APIGENERATOR_H
#define SSI_IOPUT_APIGENERATOR_H

#include "base/IObject.h"
#include "base/String.h"
#include "base/Factory.h"
#include "ioput/xml/tinyxml.h"

namespace ssi {

class APIGenerator {

public:

	enum VERSION {
		V1 = 1,		// original format		
	};
	static APIGenerator::VERSION DEFAULT_VERSION;

public:

	static bool CreateAPI (const ssi_char_t *filepath);
	static bool CreateAPIIndex (const ssi_char_t *apidir);
	static void SaveCurrentComponentList ();
	static void ResetCurrentComponentList ();

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	};

protected:

	static bool CreateSource (TiXmlElement &body);
	static bool CreateSummary (TiXmlElement &body);
	static bool CreateDetails (TiXmlElement &body);
	static bool CreateClass (TiXmlElement &body, const ssi_char_t *create);
	static bool CreateOptions (TiXmlElement &body, IObject &object);

	static bool CreateIndex (const ssi_char_t *apidir);
	static bool CreateStart (const ssi_char_t *apidir);
	static bool CreateMenu (const ssi_char_t *apidir);

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	static ssi_size_t _n_dlls;
	static ssi_char_t **_dll_names;
	static ssi_size_t _n_objects;
	static ssi_char_t **_object_names;	

};

}

#endif
