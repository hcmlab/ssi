// PythonOptions.h
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

#ifndef SSI_PYTHON_OPTIONS_H
#define SSI_PYTHON_OPTIONS_H

#include "ioput/option/OptionList.h"

namespace ssi {

class PythonOptions : public OptionList {

public:

	PythonOptions() {
		
		setSysPath(".");
		script[0] = '\0';
		optsstr[0] = '\0';
		optsfile[0] = '\0';

		addOption("syspath", syspath, SSI_MAX_CHAR, SSI_CHAR, "path to look for python script (if several seperate by ';')");
		addOption("script", script, SSI_MAX_CHAR, SSI_CHAR, "name of python script (without ending)");
		addOption("optsstr", optsstr, SSI_MAX_CHAR, SSI_CHAR, "string with option tuple <key=val> (if several seperate by ;)");
		addOption("optsfile", optsfile, SSI_MAX_CHAR, SSI_CHAR, "path to file with option tuple <key=val> (if several one per line)");
	};

	void setSysPath(const ssi_char_t *syspath) {
		if (syspath) {
			ssi_strcpy(this->syspath, syspath);
		}
	}
	void setScript(const ssi_char_t *script) {
		if (script) {
			ssi_strcpy(this->script, script);
		}
	}
	void add(const ssi_char_t *key, bool value)
	{
		add(key, value ? "True" : "False");
	}
	void add(const ssi_char_t *key, int value)
	{
		ssi_char_t tmp[15];
		ssi_sprint(tmp, "%d", value);
		add(key, tmp);
	}
	void add(const ssi_char_t *key, double value)
	{
		ssi_char_t tmp[50];
		ssi_sprint(tmp, "%lf", value);
		add(key, tmp);
	}
	void add(const ssi_char_t *key, const ssi_char_t *value)
	{
		ssi_size_t n = ssi_strlen(optsstr);
		ssi_size_t n_key = ssi_strlen(key);
		ssi_size_t n_value = ssi_strlen(value);
		if (n + n_key + n_value + 3 < SSI_MAX_CHAR) // <old>;name=value\0
		{
			if (n > 0)
			{
				n += ssi_sprint(optsstr + n, ";");
			}
			ssi_sprint(optsstr + n, "%s=%s", key, value);
		}
		else
		{
			ssi_wrn("length of option string exceeded");
		}
	}
	void setOptionFile(const ssi_char_t *optsfile) {
		if (optsfile) {
			ssi_strcpy(this->optsfile, optsfile);
		}
	}

	ssi_char_t syspath[SSI_MAX_CHAR];
	ssi_char_t script[SSI_MAX_CHAR];
	ssi_char_t optsstr[SSI_MAX_CHAR];
	ssi_char_t optsfile[SSI_MAX_CHAR];
};

}

#endif

