// SoundPlayer.cpp
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

#include "SoundPlayer.h"
#include "ioput/file/FilePath.h"

#ifdef USE_SSI_LEAK_DETECTOR
	#include "SSI_LeakWatcher.h"
	#ifdef _DEBUG
		#define new DEBUG_NEW
		#undef THIS_FILE
		static char THIS_FILE[] = __FILE__;
	#endif
#endif

namespace ssi {

ssi_char_t *SoundPlayer::ssi_log_name = "soundplay_";

SoundPlayer::SoundPlayer (const ssi_char_t *file)
	:  _file (0) {

	/*if (file) {
		if (!OptionList::LoadXML (file, _options)) {
			OptionList::SaveXML (file, _options);
		}
		_file = ssi_strcpy (file);
	}*/
}

SoundPlayer::~SoundPlayer () {

	/*if (_file) {
		OptionList::SaveXML (_file, _options);
		delete[] _file;
	}*/
}

bool SoundPlayer::update(IEvents &events, ssi_size_t n_new_events, ssi_size_t time_ms) {
	
	if (n_new_events > 0) {

		if (n_new_events > 1) {
			ssi_wrn("received %u events, only latest will be processed", n_new_events)
		}

		ssi_event_t *e = events.next();

		if (e->type == SSI_ETYPE_STRING) {
			ssi_char_t *path = ssi_pcast(ssi_char_t, e->ptr);
			ssi_char_t *wavpath = 0;

			FilePath fp(path);
			if (ssi_strcmp(fp.getExtension(), ".wav")) {
				wavpath = ssi_strcpy(path);
			} else {
				wavpath = ssi_strcat(fp.getPath(), ".wav");
			}

			if (ssi_exists(wavpath)) {
				ssi_msg(SSI_LOG_LEVEL_BASIC, "play '%s'", wavpath);
				::PlaySound(NULL, 0, 0);
				::PlaySound(wavpath, 0, SND_ASYNC);
			}

			delete[] wavpath;
		}
	}

	return true;
}

}
