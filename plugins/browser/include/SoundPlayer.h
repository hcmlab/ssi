// SoundPlayer.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2009/08/19
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

#ifndef SSI_BROWSER_SOUNDPLAYER_H
#define SSI_BROWSER_SOUNDPLAYER_H

#include "base/IObject.h"

namespace ssi {

class SoundPlayer : public IObject {

public:

	static const ssi_char_t *GetCreateName () { return "SoundPlayer"; };
	static IObject *Create (const ssi_char_t *file) { return new SoundPlayer (file); };
	~SoundPlayer ();

	IOptions *getOptions () { return 0; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Plays back wav files (path is parsed from string events)."; };

	bool update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms);

protected:

	SoundPlayer (const ssi_char_t *file = 0);
	ssi_char_t *_file;
	static ssi_char_t *ssi_log_name;

};

}

#endif
