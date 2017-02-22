// Camera.h
// author: Frank Jung <frank.jung@informatik.uni-augsburg.de>
// created: 2009/04/06
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

// Provides live recording from WebCam

#pragma once

#ifndef SSI_SENSOR_CAMSENSOR_H
#define	SSI_SENSOR_CAMSENSOR_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "thread/Lock.h"
#include "thread/Timer.h"
#include "CameraCons.h"
#include "CameraOptions.h"

#ifndef SafeReleaseFJ
#define SafeReleaseFJ(p) { if( (p) != 0 ) { (p)->Release(); (p)= 0; } }
#endif

struct IGraphBuilder;
struct ICaptureGraphBuilder2;
struct IBaseFilter;
struct IMediaControl;
struct IUAProxyForceGrabber;

namespace ssi
{

class CameraList;
class VideoTypeInfoList;
class CameraDeviceName;

class Camera : public ISensor, public Thread {

public:

	class VideoChannel : public IChannel {

		friend class Camera;

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

	static const ssi_char_t *GetCreateName () { return "Camera"; };
	static IObject *Create (const ssi_char_t *file) { return new Camera (file); };
	~Camera ();
	CameraOptions *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Establishes a connection to a video capture devices such as web-cams, DV cameras or TV tuner cards."; };

	ssi_size_t getChannelSize () { return 1; };
	IChannel *getChannel (ssi_size_t index) { return &_video_channel; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();

	ssi_video_params_t getFormat () { return _options.params; };
	const void *getMetaData (ssi_size_t &size) { size = sizeof (_options.params); return &_options.params; };

	virtual HRESULT GetCurrentSampleInStream (BYTE *sampleData, int *sampleBufferSize, bool flip, bool mirror);
	const BITMAPINFOHEADER &format () { return _bmpIH; };
	virtual CameraDeviceName& getCameraDeviceName ();

	void setLogLevel (int level) { ssi_log_level = level; };
	static void SetLogLevelStatic (int level) { ssi_log_level_static = level; };

protected:

	Camera (const ssi_char_t *file = 0);
	CameraOptions _options;
	ssi_char_t *_file;

	VideoChannel _video_channel;
	void setProvider (IProvider *provider);

	static CameraList* GetVideoCaptureDevices();
	bool determineIfDesiredMediaTypeExists(IBaseFilter *pCap);
	bool setVideoOutputFormatAndConnectPin();
	bool LetUserSelectMediaType();

	Mutex						_setProviderMutex;
	IProvider					*_provider;
	int							_comInitCountConstructor;
	int							_comInitCountConnectRunDisconnect;
	bool						_useDV;
	int							_dvImageSizeDenominator;
	int							_selectedMediaTypePinIndex;
	int							_selectedMediaTypeVideoInfoIndex;

	CameraList					*_listOfVideoCameras;
	VideoTypeInfoList			*_listOfVideoTypesInTheSpecifiedFilter;
	int							_indexOfSelectedCamera;
	char						*_nameOfDesiredCam;

	IGraphBuilder				*_pGraph;
	ICaptureGraphBuilder2		*_pBuild;
	IBaseFilter					*_pCapDevice;
	IBaseFilter					*_pGrabber;
	IMediaControl				*_pControl;
	IUAProxyForceGrabber		*_pGrabInterface;
	
	Timer						*_timer;

	BYTE						*_picData;
	BYTE						*_picDataFlipBuffer;
	int							_sizeOfPicData;
	CALLBACKBMPINFO				_cbIH;
	BITMAPINFOHEADER			_bmpIH;

	static char *ssi_log_name;
	int ssi_log_level;
	static int ssi_log_level_static;

};

}

#endif
