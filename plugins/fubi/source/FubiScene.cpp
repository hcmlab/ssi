// FubiScene.h
// author: Felix Kistler <kistler@hcm-lab.de>
// created: 2012/09/13
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

#include "FubiScene.h"
#include "Fubi/Fubi.h"
#include "FubiLock.h"
#include "base/Factory.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


namespace ssi {

ssi_char_t *FubiScene::ssi_log_name = "fubiscene_";
int FubiScene::ssi_log_level = SSI_LOG_LEVEL_DEFAULT;

	FubiScene::FubiScene (const ssi_char_t *file) 
		: _file (0),
		_sceneProvider(0x0),
		_timer (1/30.0)
	{
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy (file);
		}
		
		_channels[0] = new SceneChannel ();
	}

	FubiScene::~FubiScene () {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	bool FubiScene::setProvider (const ssi_char_t *name, IProvider *provider) {
		
		if (strcmp (name, SSI_FUBI_SCENE_PROVIDER_NAME) == 0) {
			setSceneProvider (provider);
			return true;
		}

		ssi_wrn ("unkown fubi scene provider name '%s'", name);

		return false;
	}

	void FubiScene::setSceneProvider (IProvider *provider) {
		if (_sceneProvider) {
			ssi_wrn ("scene provider already set");
		}

		_sceneProvider = provider;
		if (_sceneProvider) {
			ssi_video_params_t params = getSceneParams ();
			_sceneProvider->setMetaData (sizeof (params), &params);
			_sceneProvider->init (_channels[0]);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "FubiScene provider set");
		}
	}

	bool FubiScene::connect () {

		if (!Fubi::isInitialized())
		{
			if (!Fubi::init(Fubi::SensorOptions(Fubi::StreamOptions(-1), Fubi::StreamOptions(-1), Fubi::StreamOptions(-1),
				Fubi::SensorType::NONE)))
				return false;
		}

		// set thread name		
		Thread::setName (getName ());

		return true;
	}


	bool FubiScene::disconnect () {

		Fubi::release();

		ssi_msg (SSI_LOG_LEVEL_BASIC, "disconnected");
		return true;
	}

	void FubiScene::run () {	
		// Provide scene data
		if (_sceneProvider)
		{
			memset(_sceneBuffer, 0, 640*480*4);
			ssi_video_params_t param = getSceneParams();

			FubiLock::Lock();

			Fubi::getImage((unsigned char*)_sceneBuffer, Fubi::ImageType::Blank, (Fubi::ImageNumChannels::Channel)param.numOfChannels, 
				Fubi::ImageDepth::D8, (Fubi::RenderOptions::Skeletons | Fubi::RenderOptions::LocalOrientCaptions));

			FubiLock::Unlock();

			_sceneProvider->provide(_sceneBuffer, 1);
			SSI_DBG (SSI_LOG_LEVEL_DEBUG, "provide scene data");
		}

		//// Update the context				
		_timer.wait ();				
	}
}

