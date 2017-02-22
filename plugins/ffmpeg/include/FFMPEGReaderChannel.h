// FFMPEGReaderChannel.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2014/04/15
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

#ifndef SSI_FFMPEG_FFMPEGREADERCHANNEL_H
#define SSI_FFMPEG_FFMPEGREADERCHANNEL_H

#include "base/ISensor.h"

#define SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME "video"
#define SSI_FFMPEGREADER_AUDIO_PROVIDER_NAME "audio"

namespace ssi {

	class VideoChannel : public IChannel {

		friend class FFMPEGReader;

	public:

		VideoChannel () {
			ssi_stream_init (stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~VideoChannel () {
			ssi_stream_destroy (stream);
		}

		const ssi_char_t *getName () { return SSI_FFMPEGREADER_VIDEO_PROVIDER_NAME; };
		const ssi_char_t *getInfo () { return "Properties are determined from the options."; };
		ssi_stream_t getStream () { return stream; };

	protected:

		ssi_stream_t stream;
	};

	class AudioChannel : public IChannel {

		friend class FFMPEGReader;

	public:

		AudioChannel () {
			ssi_stream_init (stream, 0, 1, sizeof (ssi_real_t), SSI_FLOAT, 8000);
		}
		~AudioChannel () {
			ssi_stream_destroy (stream);
		}

		const ssi_char_t *getName () { return SSI_FFMPEGREADER_AUDIO_PROVIDER_NAME; };
		const ssi_char_t *getInfo () { return "Properties are determined from the options."; };
		ssi_stream_t getStream () { return stream; };

	protected:

		ssi_stream_t stream;
	};
};
#endif
