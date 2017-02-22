// MicrosoftKinect.h
// author: Daniel Schork 
// created: 2015
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

#ifndef SSI_SENSOR_MICROSOFTKINECT2_H
#define	SSI_SENSOR_MICROSOFTKINECT2_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"

// OLD
struct INuiSensor;
struct IFTFaceTracker;
struct IFTResult;
struct IFTImage;
struct FT_VECTOR3D;
struct IFTModel;
struct FT_CAMERA_CONFIG;
typedef enum _NUI_IMAGE_TYPE NUI_IMAGE_TYPE;
typedef enum _NUI_IMAGE_RESOLUTION NUI_IMAGE_RESOLUTION;
typedef struct _NUI_SKELETON_FRAME NUI_SKELETON_FRAME;
typedef struct _Vector4 Vector4;


//forward
struct _Matrix4;
typedef struct tagRGBQUAD RGBQUAD;
struct IKinectSensor;
struct IMultiSourceFrameReader;
struct IMultiSourceFrame;
struct IBody;
struct ICoordinateMapper;
struct IBodyFrame;
struct IBodyFrameReference;
struct IHighDefinitionFaceFrameReader;
struct IDepthFrameReference;
struct IDepthFrame;
struct IColorFrameReference;
struct IColorFrame;
struct IInfraredFrameReference;
struct IInfraredFrame;
struct IHighDefinitionFaceFrameSource;
struct IMMDevice;
class CWASAPICapture;

namespace ssi {
	class Timer;
}

namespace ssi {

#define SSI_MICROSOFTKINECT2_DEPTHRAW_PROVIDER_NAME "depthr"
#define SSI_MICROSOFTKINECT2_DEPTHIMAGE_PROVIDER_NAME "depth"
#define SSI_MICROSOFTKINECT2_INFRAREDRAW_PROVIDER_NAME "irr"
#define SSI_MICROSOFTKINECT2_INFRAREDIMAGE_PROVIDER_NAME "ir"
#define SSI_MICROSOFTKINECT2_RGBIMAGE_PROVIDER_NAME "rgb"
#define SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME "skeleton"
#define SSI_MICROSOFTKINECT2_SKELETON2SCREEN_PROVIDER_NAME "skeleton2screen"
#define SSI_MICROSOFTKINECT2_SKELETONCONFIDENCE_PROVIDER_NAME "skeletonconf"
#define	SSI_MICROSOFTKINECT2_FACEPOINT_PROVIDER_NAME "face"
#define	SSI_MICROSOFTKINECT2_FACEPOINT3D_PROVIDER_NAME "face3d"
#define	SSI_MICROSOFTKINECT2_ACTIONUNIT_PROVIDER_NAME "au"
#define	SSI_MICROSOFTKINECT2_HEADPOSE_PROVIDER_NAME "head"
#define	SSI_MICROSOFTKINECT2_HANDPOSE_PROVIDER_NAME "hand"
#define	SSI_MICROSOFTKINECT2_AUDIO_PROVIDER_NAME "audio"

#define SSI_MICROSOFTKINECT2_DEPTHIMAGE_MAX_VALUE 4500
#define SSI_MICROSOFTKINECT2_DEPTHIMAGE_MIN_VALUE 500

#define SSI_MICROSOFTKINECT2_INFRAREDIMAGE_MAX_VALUE 60000
#define SSI_MICROSOFTKINECT2_INFRAREDIMAGE_MIN_VALUE 0

#define	SSI_MICROSOFTKINECT2_TRACKNEARESTPERSONS_INVALID 1000

#define SSI_MICROSOFTKINECT2_JOINT_COUNT 25
#define SSI_MICROSOFTKINECT2_SKELETON_MAX_USER 6

#define SSI_MICROSOFTKINECT2_INVALID_ACTIONUNIT_VALUE 0.0f
#define SSI_MICROSOFTKINECT2_INVALID_FACEPOINT_VALUE -1.0f
#define SSI_MICROSOFTKINECT2_INVALID_FACEPOINT3D_VALUE -1.0f
#define SSI_MICROSOFTKINECT2_INVALID_HEADPOSE_VALUE -91.0f
#define SSI_MICROSOFTKINECT2_INVALID_SKELETON_JOINT_VALUE 0.0f
#define SSI_MICROSOFTKINECT2_INVALID_SKELETON_CONFIDENCE_VALUE -1.0f

#define SSI_MICROSOFTKINECT2_CHANNEL_NUM 13
#define SSI_MICROSOFTKINECT2_DEPTHRAW_CHANNEL_NUM 0
#define SSI_MICROSOFTKINECT2_DEPTHIMAGE_CHANNEL_NUM 1
#define SSI_MICROSOFTKINECT2_INFRAREDRAW_CHANNEL_NUM 2
#define SSI_MICROSOFTKINECT2_INFRAREDIMAGE_CHANNEL_NUM 3
#define SSI_MICROSOFTKINECT2_RGBIMAGE_CHANNEL_NUM 4
#define SSI_MICROSOFTKINECT2_SKELETON_CHANNEL_NUM 5
#define	SSI_MICROSOFTKINECT2_FACEPOINT_CHANNEL_NUM 6
#define SSI_MICROSOFTKINECT2_FACEPOINT3D_CHANNEL_NUM 7
#define	SSI_MICROSOFTKINECT2_ACTIONUNIT_CHANNEL_NUM 8
#define	SSI_MICROSOFTKINECT2_HEADPOSE_CHANNEL_NUM 9
#define	SSI_MICROSOFTKINECT2_SKELETON2SCREEN_CHANNEL_NUM 10
#define	SSI_MICROSOFTKINECT2_SKELETONCONFIDENCE_CHANNEL_NUM 11
#define	SSI_MICROSOFTKINECT2_HANDPOSE_CHANNEL_NUM 12
#define SSI_MICROSOFTKINECT2_AUDIO_CHANNEL_NUM 13

// Number of milliseconds of acceptable lag between live sound being produced and recording operation.
#define SSI_MICROSOFTKINECT2_AUDIO_TARGETLATENCY 20


class MicrosoftKinect2 : public ISensor, public Thread {

public:

	struct SKELETON_JOINT_VALUE
	{
		enum List
		{
			POS_X = 0,
			POS_Y,
			POS_Z,
			POS_CONF,
			
			ROT_W,
			ROT_X,
			ROT_Y,
			ROT_Z,
			ROT_CONF,

			RELROT_W,
			RELROT_X,
			RELROT_Y,
			RELROT_Z,
			RELROT_CONF,
			
			NUM
		};
	};

	typedef float SKELETON[SSI_MICROSOFTKINECT2_JOINT_COUNT][SKELETON_JOINT_VALUE::NUM];
	typedef float SKELETONCONF[SSI_MICROSOFTKINECT2_JOINT_COUNT];
	typedef float HANDPOSE[2];
	
	struct FACEPOINT {
		enum List
		{
			NUM = 1347
		};
	};
	struct FACEPOINT_VALUE {
		enum List
		{
			X = 0,
			Y,

			NUM
		};
	};
	typedef float FACEPOINTS[FACEPOINT::NUM][FACEPOINT_VALUE::NUM];

	struct FACEPOINT3D {
		enum List
		{
			NUM = 1347
		};
	};
	struct FACEPOINT3D_VALUE {
		enum List
		{
			X = 0,
			Y,
			Z,

			NUM
		};
	};
	typedef float FACEPOINTS3D[FACEPOINT3D::NUM][FACEPOINT3D_VALUE::NUM];

	struct ACTIONUNIT
	{
		enum List
		{
			NUM = 17
		};
	};
	typedef float ACTIONUNITS[ACTIONUNIT::NUM];

	struct HEADPOSE_ANGLE
	{
		enum List
		{
			PITCH,
			ROLL,
			YAW,
			
			NUM
		};
	};
	typedef float HEADPOSE[HEADPOSE_ANGLE::NUM];

public:

	class DepthRawChannel : public IChannel {

		friend class MicrosoftKinect2;

		public:
			DepthRawChannel () {
				ssi_stream_init (stream, 0, 1, 0, SSI_IMAGE, 0); 
			}
			~DepthRawChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_DEPTHRAW_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Depth raw image with a resolution of 512*424 as unsigned short values in range [0..4500]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class DepthImageChannel : public IChannel {

		friend class MicrosoftKinect2;

		public:
			DepthImageChannel () {
				ssi_stream_init (stream, 0, 1, 0, SSI_IMAGE, 0); 
			}
			~DepthImageChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_DEPTHIMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Depth image with a resolution of 512*424 as greyscale image in range [0..255]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class InfraredRawChannel : public IChannel {

		friend class MicrosoftKinect2;

	public:
		InfraredRawChannel() {
			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~InfraredRawChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t *getName() { return SSI_MICROSOFTKINECT2_INFRAREDRAW_PROVIDER_NAME; };
		const ssi_char_t *getInfo() { return "Infrared raw image with a resolution of 512*424 as unsigned short values in range [0..60000]."; };
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

	class InfraredImageChannel : public IChannel {

		friend class MicrosoftKinect2;

	public:
		InfraredImageChannel() {
			ssi_stream_init(stream, 0, 1, 0, SSI_IMAGE, 0);
		}
		~InfraredImageChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t *getName() { return SSI_MICROSOFTKINECT2_INFRAREDIMAGE_PROVIDER_NAME; };
		const ssi_char_t *getInfo() { return "Infrared image with a resolution of 512*424 as greyscale image in range [0..255]."; };
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

	class RGBImageChannel : public IChannel {

		friend class MicrosoftKinect2;
		public:
			RGBImageChannel () {

				ssi_stream_init (stream, 0, 1, 0, SSI_IMAGE, 0); 
			}
			~RGBImageChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_RGBIMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "RGB image with a resolution of 1920*1080 as rgb values in range [0..255]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class SkeletonChannel : public IChannel {
	
		friend class MicrosoftKinect2;
		
		public:
			SkeletonChannel () {
				ssi_stream_init (stream, 0, 0, sizeof(float), SSI_FLOAT, 0); 
			}
			~SkeletonChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 3D skeleton positions. see /infos/Kinect 1 vs 2 Skeleton.png"; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class Skeleton2ScreenChannel : public IChannel {
	
		friend class MicrosoftKinect2;
		
		public:
			Skeleton2ScreenChannel () {
				ssi_stream_init (stream, 0, 0, sizeof(float), SSI_FLOAT, 0); 
			}
			~Skeleton2ScreenChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_SKELETON2SCREEN_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 2D points for painting joints on a screen."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class SkeletonConfidenceChannel : public IChannel {

		friend class MicrosoftKinect2;

	public:
		SkeletonConfidenceChannel() {
			ssi_stream_init(stream, 0, 0, sizeof(float), SSI_FLOAT, 0);
		}
		~SkeletonConfidenceChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t *getName() { return SSI_MICROSOFTKINECT2_SKELETONCONFIDENCE_PROVIDER_NAME; };
		const ssi_char_t *getInfo() { return "Reports separate confidence values for the skeleton joints."; };
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

	class FacePointsChannel : public IChannel {

		friend class MicrosoftKinect2;

		public:

			FacePointsChannel () {
				ssi_stream_init(stream, 0, FACEPOINT::NUM * FACEPOINT_VALUE::NUM, sizeof(float), SSI_FLOAT, 0);
			}
			~FacePointsChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_FACEPOINT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 1347 face points (2d)."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};


	class FacePoints3DChannel : public IChannel {

		friend class MicrosoftKinect2;

		public:

			FacePoints3DChannel () {
				ssi_stream_init (stream, 0, FACEPOINT3D::NUM * FACEPOINT3D_VALUE::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~FacePoints3DChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_FACEPOINT3D_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 1347 face points (3d)."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

	class ActionUnitChannel : public IChannel {

		friend class MicrosoftKinect2;

		public:

			ActionUnitChannel () {
				ssi_stream_init (stream, 0, ACTIONUNIT::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~ActionUnitChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_ACTIONUNIT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 17 face shape animations (formerly called action units) (https://msdn.microsoft.com/en-us/library/microsoft.kinect.face.faceshapeanimations.aspx)."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};
	
	

	class HeadPoseChannel : public IChannel {

		friend class MicrosoftKinect2;
	
		public:
			HeadPoseChannel () {
				ssi_stream_init (stream, 0, HEADPOSE_ANGLE::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~HeadPoseChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT2_HEADPOSE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports head pose in degree (pitch, roll, yaw)."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

	class HandPoseChannel : public IChannel {

		friend class MicrosoftKinect2;

	public:
		HandPoseChannel() {
			ssi_stream_init(stream, 0, 2, sizeof(float), SSI_FLOAT, 0);
		}
		~HandPoseChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t *getName() { return SSI_MICROSOFTKINECT2_HANDPOSE_PROVIDER_NAME; };
		const ssi_char_t *getInfo() { return "Reports the handpose of left and right hand."; };
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

	class AudioChannel : public IChannel {

		friend class MicrosoftKinect2;

	public:
		AudioChannel() {
			ssi_stream_init(stream, 0, 4, sizeof(float), SSI_FLOAT, 0);
		}
		~AudioChannel() {
			ssi_stream_destroy(stream);
		}
		const ssi_char_t *getName() { return SSI_MICROSOFTKINECT2_AUDIO_PROVIDER_NAME; };
		const ssi_char_t *getInfo() { return "Captures the audio stream from the microphone array."; };
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: sr(20.0), showFaceTracking(true), showBodyTracking(true), trackNearestPersons(SSI_MICROSOFTKINECT2_TRACKNEARESTPERSONS_INVALID), screenWidth (0), screenHeight (0), rotQuaternion(false) {

			addOption ("sr", &sr, 1, SSI_TIME, "sample rate in hz");
			addOption ("showFaceTracking", &showFaceTracking, 1, SSI_BOOL, "show face tracking");
			addOption ("showBodyTracking", &showBodyTracking, 1, SSI_BOOL, "show body tracking (only paints joints, use the SkeletonPainter plugin for more options");
			addOption ("trackNearestPersons", &trackNearestPersons, 1, SSI_SIZE, "track only the n (<=6) persons closest to the kinect (front to back), otherwise 6 persons are tracked (left to right)");
			addOption ("screenWidth", &screenWidth, 1, SSI_SIZE, "screen width (if skeleton is projected). Use 0 for rgb image size.");
			addOption ("screenHeight", &screenHeight, 1, SSI_SIZE, "screen height (if skeleton is projected). Use 0 for rgb image size.");
			addOption ("rotQuaternion", &rotQuaternion, 1, SSI_BOOL, "use quaternions for rotation instead of euler");
		};		

		ssi_time_t sr;
		bool showFaceTracking;
		bool showBodyTracking;
		ssi_size_t trackNearestPersons;
		ssi_size_t screenWidth, screenHeight;
		bool rotQuaternion;
	};

public:

	static const ssi_char_t *GetCreateName () { return "MicrosoftKinect2"; };
	static IObject *Create (const ssi_char_t *file) { return new MicrosoftKinect2 (file); };
	~MicrosoftKinect2 ();
	MicrosoftKinect2::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Microsoft Kinect V2 Sensor"; };

	ssi_size_t getChannelSize () { return SSI_MICROSOFTKINECT2_CHANNEL_NUM; };
	IChannel *getChannel (ssi_size_t index) { 
		switch (index) {
			case SSI_MICROSOFTKINECT2_RGBIMAGE_CHANNEL_NUM:
				return &m_rgb_channel;
			case SSI_MICROSOFTKINECT2_DEPTHRAW_CHANNEL_NUM:
				return &m_depthr_channel;
			case SSI_MICROSOFTKINECT2_DEPTHIMAGE_CHANNEL_NUM:
				return &m_depth_channel;
			case SSI_MICROSOFTKINECT2_INFRAREDRAW_CHANNEL_NUM:
				return &m_irr_channel;
			case SSI_MICROSOFTKINECT2_INFRAREDIMAGE_CHANNEL_NUM:
				return &m_ir_channel;
			case SSI_MICROSOFTKINECT2_SKELETON_CHANNEL_NUM:
				return &m_skeleton_channel;
			case SSI_MICROSOFTKINECT2_FACEPOINT_CHANNEL_NUM:
				return &m_face_channel;
			case SSI_MICROSOFTKINECT2_FACEPOINT3D_CHANNEL_NUM:
				return &m_face3d_channel;
			case SSI_MICROSOFTKINECT2_ACTIONUNIT_CHANNEL_NUM:
				return &m_au_channel;
			case SSI_MICROSOFTKINECT2_HEADPOSE_CHANNEL_NUM:
				return &m_head_channel;
			case SSI_MICROSOFTKINECT2_SKELETON2SCREEN_CHANNEL_NUM:
				return &m_skeleton_to_screen_channel;
			case SSI_MICROSOFTKINECT2_SKELETONCONFIDENCE_CHANNEL_NUM:
				return &m_skeleton_confidence_channel;
			case SSI_MICROSOFTKINECT2_HANDPOSE_CHANNEL_NUM:
				return &m_hand_channel;
			case SSI_MICROSOFTKINECT2_AUDIO_CHANNEL_NUM:
				return &m_audio_channel;
		
		}
		return 0;
	};
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return Thread::start (); };
	bool stop () { return Thread::stop (); };
	void run ();
	bool disconnect ();

	ssi_video_params_t getDepthRawParams() 
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, 512, 424, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getDepthImageParams() 
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, 512, 424, _options.sr, SSI_VIDEO_DEPTH_8U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getInfraredRawParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, 512, 424, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getInfraredImageParams()
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, 512, 424, _options.sr, SSI_VIDEO_DEPTH_8U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getRGBImageParams() 
	{
		ssi_video_params_t vParam;

		ssi_video_params(vParam, 1920, 1080, _options.sr, SSI_VIDEO_DEPTH_8U, 4);
		vParam.flipImage = false;
		return vParam;
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	MicrosoftKinect2 (const ssi_char_t *file = 0);
	MicrosoftKinect2::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
	
	void initMemory ();
	void clearData (ssi_size_t nui_id);

	HRESULT getKinectAudioDevice(IMMDevice **ppDevice);
	void process ();
	void processMultiSourceFrame();
	void processImages();
	void processBody ();
	void processFace ();
	void processProvide();
	void showSkeleton();
	void showFace();

	bool						m_SkeletonTracked[SSI_MICROSOFTKINECT2_SKELETON_MAX_USER];
    BOOL                        m_KinectSensorPresent;
    IFTImage*                   m_colorImage;
    IFTImage*                   m_depthRImage;
	ssi_byte_t*                 m_depthImage;
	ssi_byte_t*					m_irImage;
    bool                        m_LastFaceTrackSucceeded;
	ssi_size_t					m_n_skeletons;
	SKELETON*					m_skeleton;
	HANDPOSE*					m_handpose;
	SKELETON*					m_skeleton_to_screen;
	SKELETONCONF*				m_skeleton_confidence;
	bool						m_trackNearToFar;
	int							*m_channel_order;
	ACTIONUNITS*				m_actionunits;
	FACEPOINTS*					m_facepoints2d;
	FACEPOINTS3D*				m_facepoints3d;
	HEADPOSE*					m_headpose;
	Timer*                      m_timer;
	IKinectSensor* m_kinectSensor;
	IMultiSourceFrameReader* m_multiSourceFrameReader;
	IMultiSourceFrame* m_pMultiSourceFrame;
	RGBQUAD*					m_bgraBuffer;
	UINT16*						m_depthrBuffer;
	UINT16*						m_irrBuffer;
	IBody*						m_bodies[SSI_MICROSOFTKINECT2_SKELETON_MAX_USER];
	bool						m_hasNewData;
	ICoordinateMapper* m_pCoordinateMapper;
	IBodyFrame* m_bodyFrame;
	IBodyFrameReference* m_pBodyFrameReference;
	IDepthFrameReference*		m_pDepthFrameReference;
	IDepthFrame*				m_depthFrame;
	IColorFrameReference*		m_pColorFrameReference;
	IColorFrame*				m_colorFrame;
	IInfraredFrameReference*	m_pIrFrameReference;
	IInfraredFrame*				m_irFrame;
	IMMDevice*					m_audio_device;
	CWASAPICapture*				m_audio_capturer;
	bool						m_audio_is_capturing;

	void setRGBImageProvider (IProvider *rgb_provider);
	RGBImageChannel m_rgb_channel;
	IProvider *m_rgb_provider;

	void setDepthRawProvider (IProvider *depthr_provider);
	DepthRawChannel m_depthr_channel;
	IProvider *m_depthr_provider;

	void setDepthImageProvider (IProvider *depth_provider);
	DepthImageChannel m_depth_channel;
	IProvider *m_depth_provider;

	void setInfraredRawProvider(IProvider *irr_provider);
	InfraredRawChannel m_irr_channel;
	IProvider *m_irr_provider;

	void setInfraredImageProvider(IProvider *ir_provider);
	InfraredImageChannel m_ir_channel;
	IProvider *m_ir_provider;

	void setSkeletonProvider (IProvider *skeleton_provider);
	SkeletonChannel m_skeleton_channel;
	IProvider *m_skeleton_provider;

	void setSkeleton2ScreenProvider (IProvider *skeleton_to_screen_provider);
	Skeleton2ScreenChannel m_skeleton_to_screen_channel;
	IProvider *m_skeleton_to_screen_provider;

	void setSkeletonConfidenceProvider(IProvider *skeleton_confidence_provider);
	SkeletonConfidenceChannel m_skeleton_confidence_channel;
	IProvider *m_skeleton_confidence_provider;

	void setActionUnitProvider (IProvider *au_provider);
	ActionUnitChannel m_au_channel;
	IProvider *m_au_provider;

	void setFacePointProvider (IProvider *face_provider);
	FacePointsChannel m_face_channel;
	IProvider *m_face_provider;

	void setFacePoint3DProvider (IProvider *face3d_provider);
	FacePoints3DChannel m_face3d_channel;
	IProvider *m_face3d_provider;

	void setHeadPoseProvider(IProvider *head_provider);
	HeadPoseChannel m_head_channel;
	IProvider *m_head_provider;

	void setHandPoseProvider(IProvider *hand_provider);
	HandPoseChannel m_hand_channel;
	IProvider *m_hand_provider;

	void setAudioProvider(IProvider *audio_provider);
	AudioChannel m_audio_channel;
	IProvider *m_audio_provider;

};

}

#endif

