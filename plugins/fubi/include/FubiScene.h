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

#pragma once

#ifndef SSI_FUBI_SCENE_H
#define SSI_FUBI_SCENE_H

#include "base/ISensor.h"
#include "ioput/option/OptionList.h"
#include "thread/Thread.h"
#include "thread/Timer.h"

#define SSI_FUBI_SCENE_PROVIDER_NAME "fubi_scene"

namespace ssi {

class FubiScene : public ISensor, public Thread {

public:

	class SceneChannel : public IChannel {
		public:
			SceneChannel () {
				ssi_stream_init (stream, 0, 1, 640*480*4, SSI_IMAGE, 30.0); 
			}
			~SceneChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_FUBI_SCENE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Scene image with a resolution of 640x480 as rgba values in range [0..255]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class Options : public OptionList {

	public:

		Options ()
		{
		};

	};

	static const ssi_char_t *GetCreateName () { return "FubiScene"; };
	static IObject *Create (const ssi_char_t *file) { return new FubiScene (file); };
	~FubiScene ();
	FubiScene::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Provider for Fubi Skeleton Scene"; };

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

	ssi_video_params_t getSceneParams() 
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, 640, 480, 30, 8, 4);

		return vParam;
	}


	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return _channels[index]; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return ssi::Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();

protected:

	FubiScene (const ssi_char_t *file = 0);
	FubiScene::Options _options;

	IChannel *_channels[1];
	void setSceneProvider (IProvider *provider);

	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	Timer _timer;

	IProvider *_sceneProvider;

	char _sceneBuffer[640*480*4];

	void generateSceneMap();
};

}

#endif
;
