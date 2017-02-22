// PythonImageFeature.h
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

#pragma once

#ifndef SSI_PYTHON_IMAGEFEATURE_H
#define SSI_PYTHON_IMAGEFEATURE_H

#include "PythonFilter.h"

namespace ssi {

class PythonImageFeature : public PythonFilter {

public:

	static const ssi_char_t *GetCreateName () { return "PythonImageFeature"; };
	static IObject *Create (const ssi_char_t *file) { return new PythonImageFeature (file); };
	~PythonImageFeature ();

	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Python image feature wrapper."; };

	void setMetaData(ssi_size_t size, const void *meta);

protected:

	PythonImageFeature (const ssi_char_t *file = 0);
	static ssi_char_t *ssi_log_name;

	bool _has_meta_data;
	ssi_video_params_t _format_in;
};

}

#endif
