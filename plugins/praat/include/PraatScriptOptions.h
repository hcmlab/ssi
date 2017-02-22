// PraatScriptOptions.h
// author: Johannes Wagner
// created: 2014/06/05
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

#ifndef SSI_PRAAT_SCRIPTOPTIONS_H
#define SSI_PRAAT_SCRIPTOPTIONS_H

#include "ioput/option/OptionList.h"

namespace ssi {

class PraatScriptOptions : public OptionList {

public:

	PraatScriptOptions() : dimensions(1), samples(1), undefined_value(std::numeric_limits<float>::quiet_NaN())
	{			
		setExe ("praatcon.exe");
		setSender ("Audio");
		setEvent ("VoiceActivity");
		setTmpWav ("~.wav");
		setScript ("");
		setScriptArgs ("");
			
		addOption ("exe", exe, SSI_MAX_CHAR, SSI_CHAR, "path to praat exe");
		addOption ("script", script, SSI_MAX_CHAR, SSI_CHAR, "path to praat script");
		addOption ("script_args", script_args, SSI_MAX_CHAR, SSI_CHAR, "arguments that are needed for the script");
		addOption ("tmpwav", tmpwav, SSI_MAX_CHAR, SSI_CHAR, "path to tmp wave file (should end on wav)");
		addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender (if consumer)");
		addOption ("ename", ename, SSI_MAX_CHAR, SSI_CHAR, "name of event (if consumer)");			
		addOption ("dimensions", &dimensions, 1, SSI_SIZE, "number of features extracted by script");
		addOption ("samples", &samples, 1, SSI_SIZE, "number of samples extracted by script");		
		addOption("undefined", &undefined_value, 1, SSI_REAL, "replace undefined fields with this value");
	};

	void setExe (const ssi_char_t *exe) {			
		if (exe) {
			ssi_strcpy (this->exe, exe);
		}
	}

	void setTmpWav (const ssi_char_t *tmpwav) {			
		if (tmpwav) {
			ssi_strcpy (this->tmpwav, tmpwav);
		}
	}

	void setScript (const ssi_char_t *script) {			
		if (script) {
			ssi_strcpy (this->script, script);
		}
	}

	void setScriptArgs (const ssi_char_t *args) {			
		if (args) {
			ssi_strcpy (this->script_args, args);
		}
	}

	void setSender (const ssi_char_t *sname) {			
		if (sname) {
			ssi_strcpy (this->sname, sname);
		}
	}

	void setEvent (const ssi_char_t *ename) {
		if (ename) {
			ssi_strcpy (this->ename, ename);
		}
	}

	ssi_char_t sname[SSI_MAX_CHAR];
	ssi_char_t ename[SSI_MAX_CHAR];	
	ssi_char_t tmpwav[SSI_MAX_CHAR];	
	ssi_char_t script[SSI_MAX_CHAR];
	ssi_char_t script_args[SSI_MAX_CHAR];
	ssi_char_t exe[SSI_MAX_CHAR];
	ssi_size_t dimensions;
	ssi_size_t samples;
	ssi_real_t undefined_value;
};

}

#endif
