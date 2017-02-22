// MicrosoftKinect.h
// author: Johannes Wagner <wagner@hcm-lab.de>
// created: 2012/07/03
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

// Provides live recording from Alive Heart Rate Monitor via Bluetooth

#pragma once

#ifndef SSI_SENSOR_MICROSOFTKINECT_H
#define	SSI_SENSOR_MICROSOFTKINECT_H

#include "base/ISensor.h"
#include "base/IProvider.h"
#include "thread/Timer.h"
#include "thread/Thread.h"
#include "ioput/option/OptionList.h"

#define NUI_SKELETON_COUNT ( 6 )

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

struct _Matrix4;

namespace ssi {

#define SSI_MICROSOFTKINECT_DEPTHRAW_PROVIDER_NAME "depthr"
#define SSI_MICROSOFTKINECT_DEPTHIMAGE_PROVIDER_NAME "depth"
#define SSI_MICROSOFTKINECT_RGBIMAGE_PROVIDER_NAME "rgb"
#define SSI_MICROSOFTKINECT_SKELETON_PROVIDER_NAME "skeleton"
#define SSI_MICROSOFTKINECT_SKELETON2SCREEN_PROVIDER_NAME "skeleton2screen"
#define SSI_MICROSOFTKINECT_SKELETONCONFIDENCE_PROVIDER_NAME "skeletonconf"
#define	SSI_MICROSOFTKINECT_FACEPOINT_PROVIDER_NAME "face"
#define	SSI_MICROSOFTKINECT_FACEPOINT3D_PROVIDER_NAME "face3d"
#define	SSI_MICROSOFTKINECT_ACTIONUNIT_PROVIDER_NAME "au"
#define	SSI_MICROSOFTKINECT_SHAPEUNIT_PROVIDER_NAME "su"
#define	SSI_MICROSOFTKINECT_HEADPOSE_PROVIDER_NAME "head"

#define SSI_MICROSOFTKINECT_DEPTHIMAGE_MAX_VALUE 6000
#define SSI_MICROSOFTKINECT_DEPTHIMAGE_MIN_VALUE 300

#define SSI_MICROSOFTKINECT_INVALID_ACTIONUNIT_VALUE 0.0f
#define SSI_MICROSOFTKINECT_INVALID_FACEPOINT_VALUE -1.0f
#define SSI_MICROSOFTKINECT_INVALID_FACEPOINT3D_VALUE -1.0f
#define SSI_MICROSOFTKINECT_INVALID_SHAPEUNIT_VALUE -1.0f
#define SSI_MICROSOFTKINECT_INVALID_HEADPOSE_VALUE -91.0f
#define SSI_MICROSOFTKINECT_INVALID_SKELETON_JOINT_VALUE 0.0f
#define SSI_MICROSOFTKINECT_INVALID_SKELETON_CONFIDENCE_VALUE -1.0f

#define SSI_MICROSOFTKINECT_CHANNEL_NUM 11
#define SSI_MICROSOFTKINECT_DEPTHRAW_CHANNEL_NUM 0
#define SSI_MICROSOFTKINECT_DEPTHIMAGE_CHANNEL_NUM 1
#define SSI_MICROSOFTKINECT_RGBIMAGE_CHANNEL_NUM 2
#define SSI_MICROSOFTKINECT_SKELETON_CHANNEL_NUM 3
#define	SSI_MICROSOFTKINECT_FACEPOINT_CHANNEL_NUM 4
#define SSI_MICROSOFTKINECT_FACEPOINT3D_CHANNEL_NUM 5
#define	SSI_MICROSOFTKINECT_ACTIONUNIT_CHANNEL_NUM 6
#define	SSI_MICROSOFTKINECT_SHAPEUNIT_CHANNEL_NUM 7
#define	SSI_MICROSOFTKINECT_HEADPOSE_CHANNEL_NUM 8
#define	SSI_MICROSOFTKINECT_SKELETON2SCREEN_CHANNEL_NUM 9
#define	SSI_MICROSOFTKINECT_SKELETONCONFIDENCE_CHANNEL_NUM 10

#define SSI_MICROSOFTKINECT_SKELETON_MAX_USER 2

class MicrosoftKinect : public ISensor, public Thread {

public:


	struct RESOLUTION {
			enum List {
				RES_80x60 = 0,
				RES_320x240 = 1,
				RES_640x480 = 2,
				RES_1280x960 = 3,

				NUM
			};
		};


	struct SKELETON_JOINT
	{
		enum List
		{
		
			HIP_CENTER	= 0,
			SPINE,
			SHOULDER_CENTER,
			HEAD,
			SHOULDER_LEFT,
			ELBOW_LEFT,
			WRIST_LEFT,
			HAND_LEFT,
			SHOULDER_RIGHT,
			ELBOW_RIGHT,
			WRIST_RIGHT,
			HAND_RIGHT,
			HIP_LEFT,
			KNEE_LEFT,
			ANKLE_LEFT,
			FOOT_LEFT,
			HIP_RIGHT,
			KNEE_RIGHT,
			ANKLE_RIGHT,
			FOOT_RIGHT,

			NUM
		};
	};

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

	struct SKELETON_JOINT_VALUE_OLD
	{
		enum List
		{
			POS_W = 0,
			POS_X,
			POS_Y,
			POS_Z,

			ROT_W,
			ROT_X,
			ROT_Y,
			ROT_Z,

			RELROT_W,
			RELROT_X,
			RELROT_Y,
			RELROT_Z,
			
			NUM
		};
	};

	typedef float SKELETON[SKELETON_JOINT::NUM][SKELETON_JOINT_VALUE::NUM];
	typedef float SKELETON_OLD[SKELETON_JOINT::NUM][SKELETON_JOINT_VALUE_OLD::NUM];
	typedef float SKELETONCONF[SKELETON_JOINT::NUM];

	struct FACEPOINT {
		enum List
		{
			NUM = 100
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
			NUM = 121
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
			UPPER_LIP_RAISER = 0,
			JAW_LOWERER,
			LIP_STRETCHER,
			BROW_LOWERER,
			LIP_CORNER_DEPRESSOR,
			OUTER_BROW_RAISER,

			NUM
		};
	};
	typedef float ACTIONUNITS[ACTIONUNIT::NUM];

	struct SHAPEUNIT
	{
		enum List
		{
			HEAD_HEIGHT = 0,
			EYEBROWS_VERTICAL_POSITION,
			EYES_VERTICAL_POSITION,
			EYES_WIDTH,
			EYES_HEIGHT,
			EYES_SEPARATION_DISTANCE,
			NOSE_VERTICAL_POSITION,
			MOUTH_VERTICAL_POSITION,
			MOUTH_WIDTH,
			EYES_VERTICAL_DIFFERENCE,
			CHIN_WIDTH,
			
			NUM
		};
	};
	typedef float SHAPEUNITS[SHAPEUNIT::NUM];

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

		friend class MicrosoftKinect;

		public:
			DepthRawChannel () {
				ssi_stream_init (stream, 0, 1, 0, SSI_IMAGE, 0); 
			}
			~DepthRawChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_DEPTHRAW_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Depth raw image with a resolution of 640*480 as unsigned short values in range [0..1000]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class DepthImageChannel : public IChannel {

		friend class MicrosoftKinect;

		public:
			DepthImageChannel () {
				ssi_stream_init (stream, 0, 1, 0, SSI_IMAGE, 0); 
			}
			~DepthImageChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_DEPTHIMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Depth image with a resolution of 640*480 as gray image in range [0..255]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class RGBImageChannel : public IChannel {

		friend class MicrosoftKinect;
	
		public:
			RGBImageChannel () {

				ssi_stream_init (stream, 0, 1, 0, SSI_IMAGE, 0); 
			}
			~RGBImageChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_RGBIMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "RGB image with a resolution of 640x480 as rgb values in range [0..255]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class SkeletonChannel : public IChannel {
	
		friend class MicrosoftKinect;
		
		public:
			SkeletonChannel () {
				ssi_stream_init (stream, 0, 0, sizeof(float), SSI_FLOAT, 0); 
			}
			~SkeletonChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_SKELETON_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 3D skeleton positions [head, neck, torso, waist, left_collar, left_shoulder, left_elbow, left_wrist, left_hand, left_fingertip, right_collar, right_shoulder, right_elbow, right_wrist, right_hand, right_fingertip, left_hip, left_knee, left_ankle, left_foot, right_hip, right_knee, right_ankle, right_foot]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class Skeleton2ScreenChannel : public IChannel {
	
		friend class MicrosoftKinect;
		
		public:
			Skeleton2ScreenChannel () {
				ssi_stream_init (stream, 0, 0, sizeof(float), SSI_FLOAT, 0); 
			}
			~Skeleton2ScreenChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_SKELETON2SCREEN_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 3D skeleton positions [head, neck, torso, waist, left_collar, left_shoulder, left_elbow, left_wrist, left_hand, left_fingertip, right_collar, right_shoulder, right_elbow, right_wrist, right_hand, right_fingertip, left_hip, left_knee, left_ankle, left_foot, right_hip, right_knee, right_ankle, right_foot]."; };
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
		const ssi_char_t *getName() { return SSI_MICROSOFTKINECT_SKELETONCONFIDENCE_PROVIDER_NAME; };
		const ssi_char_t *getInfo() { return "Reports separate confidence values for the skeleton joints."; };
		ssi_stream_t getStream() { return stream; };
	protected:
		ssi_stream_t stream;
	};

	class FacePointsChannel : public IChannel {

		friend class MicrosoftKinect;

		public:

			FacePointsChannel () {
				ssi_stream_init (stream, 0, FACEPOINT::NUM * FACEPOINT_VALUE::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~FacePointsChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_FACEPOINT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 100 face points."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};


	class FacePoints3DChannel : public IChannel {

		friend class MicrosoftKinect;

		public:

			FacePoints3DChannel () {
				ssi_stream_init (stream, 0, FACEPOINT3D::NUM * FACEPOINT3D_VALUE::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~FacePoints3DChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_FACEPOINT3D_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 100 face points."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

	class ActionUnitChannel : public IChannel {

		friend class MicrosoftKinect;

		public:

			ActionUnitChannel () {
				ssi_stream_init (stream, 0, ACTIONUNIT::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~ActionUnitChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_ACTIONUNIT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 6 action units (upper_lip_raiser, jaw_lowerer, lip_stretcher, brow_lowerer, lip_corner_depressor, outer_brow_raiser)."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};
	
	class ShapeUnitChannel : public IChannel {

		friend class MicrosoftKinect;
	
		public:
			ShapeUnitChannel () {
				ssi_stream_init (stream, 0, SHAPEUNIT::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~ShapeUnitChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_SHAPEUNIT_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports 11 shape units (head_height, eyebrows_vertical_position, eyes_vertical_position, eyes_width, eyes_height, eyes_separation_distance, nose_vertical_position, mouth_vertical_position, mouth_width, eyes_vertical_difference, chin_width)."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

	class HeadPoseChannel : public IChannel {

		friend class MicrosoftKinect;
	
		public:
			HeadPoseChannel () {
				ssi_stream_init (stream, 0, HEADPOSE_ANGLE::NUM, sizeof (float), SSI_FLOAT, 0);
			}
			~HeadPoseChannel () {
				ssi_stream_destroy (stream);
			}

			const ssi_char_t *getName () { return SSI_MICROSOFTKINECT_HEADPOSE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports head pose in degree (pitch, roll, yaw)."; };
			ssi_stream_t getStream () { return stream; };

		protected:
			ssi_stream_t stream;
	};

public:

	class Options : public OptionList {

	public:

		Options ()
			: sr (30.0), showFaceTracking (true), showBodyTracking (true), trackNearestPerson (true), seatedSkeletonMode (false), 
			simpleFaceTracking (true), nearTrackingMode (true), screenWidth (0), screenHeight (0), screenDepthMultiplier (2000.0f), screenScale (true), rotQuaternion(false), rgbres(RESOLUTION::RES_640x480), depthres(RESOLUTION::RES_640x480),
			deviceIndex (0) {

			addOption ("sr", &sr, 1, SSI_TIME, "sample rate in hz");
			addOption ("showFaceTracking", &showFaceTracking, 1, SSI_BOOL, "show face tracking");
			addOption ("simpleFaceTracking", &simpleFaceTracking, 1, SSI_BOOL, "shows simple Face instead of all points");
			addOption ("showBodyTracking", &showBodyTracking, 1, SSI_BOOL, "show body tracking");
			addOption ("trackNearestPerson", &trackNearestPerson, 1, SSI_BOOL, "track only person closest to the kinect, otherwise two persons are tracked (left to right)");
			addOption ("seatedSkeletonMode", &seatedSkeletonMode, 1, SSI_BOOL, "turns seated skeleton mode on/off");
			addOption ("nearTrackingMode", &nearTrackingMode, 1, SSI_BOOL, "turns near tracking mode on/off");
			addOption ("screenWidth", &screenWidth, 1, SSI_SIZE, "screen width (if skeleton is projected). Use 0 for rgb image size.");
			addOption ("screenHeight", &screenHeight, 1, SSI_SIZE, "screen height (if skeleton is projected). Use 0 for rgb image size.");
			addOption ("screenDepthMultiplier", &screenDepthMultiplier, 1, SSI_REAL, "screen depth miltplier (if skeleton is projected)");
			addOption ("screenScale", &screenScale, 1, SSI_BOOL, "scale screen coordinates (width and height) to [0..1]");			
			addOption ("rotQuaternion", &rotQuaternion, 1, SSI_BOOL, "use quaternions fro rotation instead of euler");
			addOption ("deviceIndex", &deviceIndex, 1, SSI_SIZE, "index of device");
			addOption ("rgbres", &rgbres, 1, SSI_SIZE, "rgb resolution (2=640x480, 3=1280x960 , else default 640x480)");
			addOption ("depthres", &depthres, 1, SSI_SIZE, "depth image resolution (0=80x60, 1=320x240, 2=640x480, else default 640x480)");
		};		

		ssi_time_t sr;
		bool showFaceTracking;
		bool showBodyTracking;
		bool trackNearestPerson;
		bool seatedSkeletonMode;
		bool simpleFaceTracking;
		bool nearTrackingMode;
		ssi_size_t screenWidth, screenHeight;
		ssi_real_t screenDepthMultiplier;
		bool screenScale;
		ssi_size_t deviceIndex;
		bool rotQuaternion;
		ssi_size_t rgbres;
		ssi_size_t depthres;
	};

public:

	static const ssi_char_t *GetCreateName () { return "MicrosoftKinect"; };
	static IObject *Create (const ssi_char_t *file) { return new MicrosoftKinect (file); };
	~MicrosoftKinect ();
	MicrosoftKinect::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Microsoft Kinect Sensor"; };

	ssi_size_t getChannelSize () { return SSI_MICROSOFTKINECT_CHANNEL_NUM; };
	IChannel *getChannel (ssi_size_t index) { 
		switch (index) {
			case SSI_MICROSOFTKINECT_RGBIMAGE_CHANNEL_NUM:
				return &m_rgb_channel;
			case SSI_MICROSOFTKINECT_DEPTHRAW_CHANNEL_NUM:
				return &m_depthr_channel;
			case SSI_MICROSOFTKINECT_DEPTHIMAGE_CHANNEL_NUM:
				return &m_depth_channel;
			case SSI_MICROSOFTKINECT_SKELETON_CHANNEL_NUM:
				return &m_skeleton_channel;
			case SSI_MICROSOFTKINECT_FACEPOINT_CHANNEL_NUM:
				return &m_face_channel;
			case SSI_MICROSOFTKINECT_FACEPOINT3D_CHANNEL_NUM:
				return &m_face3d_channel;
			case SSI_MICROSOFTKINECT_ACTIONUNIT_CHANNEL_NUM:
				return &m_au_channel;
			case SSI_MICROSOFTKINECT_SHAPEUNIT_CHANNEL_NUM:
				return &m_shape_channel;
			case SSI_MICROSOFTKINECT_HEADPOSE_CHANNEL_NUM:
				return &m_head_channel;
			case SSI_MICROSOFTKINECT_SKELETON2SCREEN_CHANNEL_NUM:
				return &m_skeleton_to_screen_channel;
			case SSI_MICROSOFTKINECT_SKELETONCONFIDENCE_CHANNEL_NUM:
				return &m_skeleton_confidence_channel;
		
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

		if(_options.depthres == RESOLUTION::RES_80x60)
			{
				ssi_video_params(vParam, 80, 60, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
			}
		else if(_options.depthres == RESOLUTION::RES_320x240)
			{
				ssi_video_params(vParam, 320, 240, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
			}

		else ssi_video_params(vParam, 640, 480, _options.sr, SSI_VIDEO_DEPTH_16U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getDepthImageParams() 
	{
		ssi_video_params_t vParam;
		if(_options.depthres == RESOLUTION::RES_80x60)
			{
				ssi_video_params(vParam, 80, 60, _options.sr, SSI_VIDEO_DEPTH_8U, 1);
			}
		else if(_options.depthres == RESOLUTION::RES_320x240)
			{
				ssi_video_params(vParam, 320, 240, _options.sr, SSI_VIDEO_DEPTH_8U, 1);
			}

		else ssi_video_params(vParam, 640, 480, _options.sr, SSI_VIDEO_DEPTH_8U, 1);
		vParam.flipImage = false;
		return vParam;
	}

	ssi_video_params_t getRGBImageParams() 
	{
		ssi_video_params_t vParam;

		if(_options.rgbres == RESOLUTION::RES_1280x960)
			{
				ssi_video_params(vParam, 1280, 960, _options.sr, SSI_VIDEO_DEPTH_8U, 4);
			}
		else ssi_video_params(vParam, 640, 480, _options.sr, SSI_VIDEO_DEPTH_8U, 4);
		vParam.flipImage = false;
		return vParam;
	}

	void setLogLevel (int level) {
		ssi_log_level = level;
	}

protected:

	MicrosoftKinect (const ssi_char_t *file = 0);
	MicrosoftKinect::Options _options;
	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	int ssi_log_level;
	
	void initMemory ();
	void clearData (ssi_size_t nui_id);

	void process ();
	void processSkeleton ();			
	void processImages ();
	void processFace ();
	void processProvide ();
	
	void showSkeleton ();
	void showFace (FLOAT* pSU, IFTModel* ftModel);

    bool						m_bNuiInitialized; 
	INuiSensor*					m_pNuiSensor;
	HANDLE						m_pDepthStreamHandle;
    HANDLE						m_pVideoStreamHandle;
	
    FLOAT						m_ZoomFactor;   // video frame zoom factor (it is 1.0f if there is no zoom)
    POINT						m_ViewOffset;   // Offset of the view from the top left corner.

    bool						m_SkeletonTracked[NUI_SKELETON_COUNT];

    BOOL                        m_KinectSensorPresent;
    IFTFaceTracker*             m_pFaceTracker;
    IFTResult*                  m_pFTResult;
    IFTImage*                   m_colorImage;
    IFTImage*                   m_depthRImage;
	ssi_byte_t*                 m_depthImage;
    bool                        m_LastFaceTrackSucceeded;
    int							m_colorRes;
	ssi_size_t					m_n_skeletons;
	SKELETON*					m_skeleton;
	SKELETON*					m_skeleton_to_screen;
	SKELETONCONF*				m_skeleton_confidence;
	bool						m_track_nearest_person;
	int							*m_channel_order;
	ACTIONUNITS					m_actionunits;
	FACEPOINTS					m_facepoints;
	FACEPOINTS3D				m_facepoints3d;
	SHAPEUNITS					m_shapeunits;
	HEADPOSE					m_headpose;
	Timer*                      m_timer;

	void setRGBImageProvider (IProvider *rgb_provider);
	RGBImageChannel m_rgb_channel;
	IProvider *m_rgb_provider;

	void setDepthRawProvider (IProvider *depthr_provider);
	DepthRawChannel m_depthr_channel;
	IProvider *m_depthr_provider;

	void setDepthImageProvider (IProvider *depth_provider);
	DepthImageChannel m_depth_channel;
	IProvider *m_depth_provider;

	void setSkeletonProvider (IProvider *skeleton_provider);
	SkeletonChannel m_skeleton_channel;
	IProvider *m_skeleton_provider;

	void setSkeleton2ScreenProvider (IProvider *skeleton_to_screen_provider);
	SkeletonChannel m_skeleton_to_screen_channel;
	IProvider *m_skeleton_to_screen_provider;

	void setSkeletonConfidenceProvider(IProvider *skeleton_confidence_provider);
	SkeletonChannel m_skeleton_confidence_channel;
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

	void setShapeUnitProvider (IProvider *shape_provider);
	ShapeUnitChannel m_shape_channel;
	IProvider *m_shape_provider;

	void setHeadPoseProvider (IProvider *head_provider);
	HeadPoseChannel m_head_channel;
	IProvider *m_head_provider;

};

}

#endif

