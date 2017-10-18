// CameraReader.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2008/12/03
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

// Provides video file streaming as if captured from a WebCam

#pragma once

#ifndef SSI_SENSOR_CAMREADER_H
#define	SSI_SENSOR_CAMREADER_H

#include "base/ISensor.h"
#include "CameraCons.h"
#include "thread/Thread.h"
#include "thread/Timer.h"
#include "thread/Event.h"
#include "CameraOptions.h"

struct IGraphBuilder;
struct IMediaControl;
struct IUAProxyForceGrabber;
struct IBaseFilter;
struct IBaseFilter;
struct IBasicVideo;
struct IBaseFilter;
struct IMediaEvent;
struct IMediaSeeking;
struct IFileSourceFilter;
struct IPin;

namespace ssi {

#ifndef SafeReleaseFJ
#define SafeReleaseFJ(p) { if( (p) != 0 ) { (p)->Release(); (p)= 0; } }
#endif

class CameraReader : public IWaitableSensor, public Thread {

	class VideoChannel : public IChannel {

		friend class CameraReader;

		public:

			VideoChannel () {
				ssi_stream_init (stream, 0, 0, 0, SSI_IMAGE, 0);
			}
			~VideoChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_CAMERA_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Delivers a stream of uncompressed images in the Device Independent Bitmap (DIB) file format of the desired sub type."; };
			ssi_stream_t getStream () { return stream; };

		protected:

			ssi_stream_t stream;
	};

public:

	class Options : public CameraOptions {

	public:

		Options () : forcefps (0), best_effort_delivery (false) {

			path[0] = '\0';

			addOption ("path", path, SSI_MAX_CHAR, SSI_CHAR, "file path");
			addOption ("forcefps", &forcefps, 1, SSI_DOUBLE, "frames per second");
			addOption ("run", &best_effort_delivery, 1, SSI_BOOL, "best effort delivery");						
		};

		void setPath (const ssi_char_t *path) {
			this->path[0] = '\0';
			if (path) {
				ssi_strcpy (this->path, path);
			}
		}

		ssi_char_t path[SSI_MAX_CHAR];
		ssi_time_t forcefps;
		bool best_effort_delivery;
	};

public:

	static const ssi_char_t *GetCreateName () { return "CameraReader"; };
	static IObject *Create (const ssi_char_t *file) { return new CameraReader (file); };
	~CameraReader ();
	CameraReader::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Grabs video frames from an avi file and provides it as a video stream."; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_video_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();

	bool wait();
	bool cancel();

	ssi_video_params_t getFormat () { return _options.params; };
	const void *getMetaData (ssi_size_t &size) { size = sizeof (_options.params); return &_options.params; };
	virtual ssi_video_params_t getFormat (const ssi_char_t *filepath);

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

	static void SetLogLevelStatic (int level) {
		ssi_log_level_static = level;
	}	

protected:

	CameraReader (const ssi_char_t *file = 0);
	CameraReader::Options _options;
	ssi_char_t *_file;	

	VideoChannel _video_channel;
	void setProvider (IProvider *provider);

	static char *ssi_log_name;
	int ssi_log_level;
	static int ssi_log_level_static;

	HRESULT RunGraph();
	void InitGraph();
	void Flip();
	void Mirror();

	bool						_first_call;
	bool						_isComInitialized;
	int							_comInitCount;
	IGraphBuilder				*_pGraph;
	IMediaControl				*_pControl;
	IUAProxyForceGrabber		*_pGrabInterface;
	IProvider					*_provider;
	bool						_is_providing;
	IBaseFilter					*_pFileDevice;
	IBaseFilter					*_pFileLoadFilter;
	IBasicVideo					*_pBasicVideo;
	IBaseFilter					*_pGrabber;
    IMediaEvent					*_pEvent;
	IMediaSeeking				*_pMediaSeek;
	IFileSourceFilter			*_pFileSourceFilter;
	IPin						*_pPin;
	Timer						*_timer;
	BYTE						*_picData;
	BYTE						*_picDataTmp;
	int							_sizeOfPicData;
	CALLBACKBMPINFO				_cbIH;
	BITMAPINFOHEADER			_bmpIH;

	bool						_is_running;
	Event						_wait_event;
	bool						_interrupted;

};


}

#endif
