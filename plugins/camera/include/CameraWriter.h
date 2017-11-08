// CameraWriter.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/06/26
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

#ifndef SSI_SENSOR_CAMWRITER_H
#define SSI_SENSOR_CAMWRITER_H

#include "base/IConsumer.h"
#include "ioput/option/OptionList.h"
#include "base/StringList.h"
#include "CameraCons.h"

struct IGraphBuilder;
struct ICaptureGraphBuilder2;
struct IBaseFilter;
struct IFakeCamPushControl;
struct IFakeAudioPushControl;
struct IConfigAviMux;
struct IConfigInterleaving;
struct IFileSinkFilter;
struct IMediaControl;

namespace ssi
{

#ifndef SafeReleaseFJ
#define SafeReleaseFJ(p) { if( (p) != 0 ) { (p)->Release(); (p)= 0; } }
#endif

class CameraWriter : public IConsumer {

public:

	class Options : public OptionList {

	public:

		Options ()
			: flip (true), mode (0), mirror(false), overwrite(false), keepEmpty(true) {

			path[0] = '\0';
			compression[0] = '\0';
			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path (empty for stdout)");
			addOption ("overwrite", &overwrite, 1, SSI_BOOL, "overwrite file if it already exists (otherwise a unique path will be created)");
			addOption ("keepEmpty", &keepEmpty, 1, SSI_BOOL, "store stream even if it is empty");
			addOption ("compression", compression, SSI_MAX_CHAR, SSI_CHAR, "name of compression filter ('None' for none, if empty dialog is shown)");
			addOption ("flip", &flip, 1, SSI_BOOL, "flip video image");
			addOption ("mirror", &mirror, 1, SSI_BOOL, "mirror video image");
			addOption ("mode", &mode, 1, SSI_INT, "interleaving mode (0=NONE, 1=CAPTURE, 2=FULL, 3=BUFFERED)");
		};

		void setPath (const ssi_char_t *path) {
			this->path[0] = '\0';
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}
		void setCompression (const ssi_char_t *name) {
			this->compression[0] = '\0';
			if (name) {
				ssi_strcpy (this->compression, name);
			}
		}

		ssi_char_t path[SSI_MAX_CHAR];
		ssi_char_t compression[SSI_MAX_CHAR];
		bool flip;
		bool mirror;
		int mode;
		bool overwrite, keepEmpty;
	};

public:

	static const ssi_char_t *GetCreateName () { return "CameraWriter"; };
	static IObject *Create (const ssi_char_t *file) { return new CameraWriter (file); };
	~CameraWriter ();
	CameraWriter::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Writes a video (and an optional audio) stream to an avi file"; };

	void consume_enter (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume (IConsumer::info consume_info,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_flush (ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);
	void consume_fail (ssi_time_t fail_time, 
		ssi_time_t fail_duration,
		ssi_size_t stream_in_num,
		ssi_stream_t stream_in[]);

	bool notify(INotify::COMMAND::List command, const ssi_char_t *message);

	void setVideoFormat (ssi_video_params_t	video_format) { _video_format = video_format; };
	void setMetaData (ssi_size_t size, const void *meta) {
		if (sizeof (_video_format) != size) {
			ssi_err ("invalid meta size");
			return;
		}
		memcpy (&_video_format, meta, size);
	}
	ssi_video_params_t getVideoFormat () { return _video_format; };
	WAVEFORMATEX getAudioFormat () { return *_audio_format; };

	void setLogLevel (int level);
	static void SetLogLevelStatic (int level);

protected:

	CameraWriter (const ssi_char_t *file = 0);
	CameraWriter::Options _options;
	ssi_char_t *_file;

	static HRESULT GetListOfCompressionFilterNames (StringList &list);
	static int LetUserSelectDesiredCompression(StringList &list, bool fallBackToConsole = true);
	static int LetUserSelectDesiredCompressionOnConsole(StringList &list);

	char *ssi_log_name;
	static char *ssi_log_name_static;
	int ssi_log_level;
	static int ssi_log_level_static;

	bool _first_call;
	bool _ready;
	ssi_char_t *_filepath;
	ssi_stream_t _stream;
	void open();
	void close();

	int								_comInitCountConstructor;
	int								_comInitCountEnterConsumeFlush;

	ssi_video_params_t				_video_format;
	WAVEFORMATEX					*_audio_format;

	int								_interleavingMode;
	ssi_char_t						*_fileToWrite;
	int								_numberOfAudioSamplesPerVideoFrame;

	IGraphBuilder					*_pGraph;
	ICaptureGraphBuilder2			*_pBuild;
	IBaseFilter						*_pFakeSource;
	IBaseFilter						*_pFakeAudioSource;
	IFakeCamPushControl				*_pFakeCamControl;
	IFakeAudioPushControl			*_pFakeAudioControl;
	IBaseFilter						*_pAviMux;
	IBaseFilter						*_pEncoder;
	IConfigAviMux					*_pConfigMux;
	IConfigInterleaving				*_pInterleave;
	IFileSinkFilter					*_pFileSink;
	IMediaControl					*_pControl;
};

}

#endif
