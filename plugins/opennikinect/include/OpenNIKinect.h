// OpenNIKinect.h
// author: Felix Kistler <kistler@hcm-lab.de>
// created: 2011/01/27
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

#ifndef SSI_OPENNI_KINECT_SENSOR_H
#define SSI_OPENNI_KINECT_SENSOR_H

#include "base/ISensor.h"
#include "base/IEvents.h"
#include "base/ITheEventBoard.h"
#include "ioput/option/OptionList.h"
#include "thread/Thread.h"
#include "thread/Event.h"
#include "thread/Timer.h"

#include <set>

#define SSI_OPENNI_KINECT_DEPTH_IMAGE_PROVIDER_NAME "depth"
#define SSI_OPENNI_KINECT_DEPTH_MAX_VALUE 10000
#define SSI_OPENNI_KINECT_SCENE_IMAGE_PROVIDER_NAME "scene"
#define SSI_OPENNI_KINECT_IR_IMAGE_PROVIDER_NAME "ir"
#define SSI_OPENNI_KINECT_IR_MAX_VALUE 2047
#define SSI_OPENNI_KINECT_RGB_IMAGE_PROVIDER_NAME "rgb"
#define SSI_OPENNI_KINECT_SKELETON_PROVIDER_NAME "skeleton"
#define SSI_OPENNI_KINECT_SELECTOR_INVALID_TRANSF_VALUE 0
#define SSI_OPENNI_KINECT_DEPTH_CHANNEL_NUM 0
#define SSI_OPENNI_KINECT_SCENE_CHANNEL_NUM 1
#define SSI_OPENNI_KINECT_IR_CHANNEL_NUM 2
#define SSI_OPENNI_KINECT_RGB_CHANNEL_NUM 3
#define SSI_OPENNI_KINECT_SCELETON_CHANNEL_NUM 4

namespace ssi {

class OpenNIKinect : public ISensor, public Thread {

public:

	class DepthChannel : public IChannel {
		friend class OpenNIKinect;
		public:
			DepthChannel () {
				ssi_stream_init (stream, 0, 1, 640*480*2, SSI_IMAGE, 30.0); 
			}
			~DepthChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_OPENNI_KINECT_DEPTH_IMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Depth image with a resolution of 640*480 as unsigned short values in range [0..10000]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class SceneChannel : public IChannel {
		friend class OpenNIKinect;
		public:
			SceneChannel (bool showRGBScene = false) {
				if(showRGBScene)
					ssi_stream_init (stream, 0, 1, 640*480*3, SSI_IMAGE, 30.0); 
				else
					ssi_stream_init (stream, 0, 1, 640*480*4, SSI_IMAGE, 30.0); 
			}
			~SceneChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_OPENNI_KINECT_SCENE_IMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Scene image with a resolution of 640x480 as rgb(a) values in range [0..255]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class IrChannel : public IChannel {
		friend class OpenNIKinect;
		public:
			IrChannel () {
				ssi_stream_init (stream, 0, 1, 640*480*2, SSI_IMAGE, 30.0); 
			}
			~IrChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_OPENNI_KINECT_IR_IMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Infra-red image with a resolution of 640*480 as unsigned short values in range [0..2047]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class RGBChannel : public IChannel {
		friend class OpenNIKinect;
		public:
			RGBChannel () {
				ssi_stream_init (stream, 0, 1, 640*480*3, SSI_IMAGE, 30.0); 
			}
			~RGBChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_OPENNI_KINECT_RGB_IMAGE_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "RGB image with a resolution of 640x480 as rgb values in range [0..255]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};

	class SceletonChannel : public IChannel {
		friend class OpenNIKinect;
		public:
			SceletonChannel () {
				ssi_stream_init (stream, 0, 0, sizeof(float), SSI_FLOAT, 30.0); 
			}
			~SceletonChannel () {
				ssi_stream_destroy (stream);
			}
			const ssi_char_t *getName () { return SSI_OPENNI_KINECT_SKELETON_PROVIDER_NAME; };
			const ssi_char_t *getInfo () { return "Reports for each user the detected 3D sceleton positions in a struct [head, neck, torso, waist, left_shoulder, left_elbow, left_wrist, left_hand, right_shoulder, right_elbow, right_wrist, right_hand, left_hip, left_knee, left_ankle, left_foot, right_hip, right_knee, right_ankle, right_foot]."; };
			ssi_stream_t getStream () { return stream; };
		protected:
			ssi_stream_t stream;
	};


	struct JOINT_VALUES
	{
		enum List
		{
			POS_X = 0,
			POS_Y,
			POS_Z,
			POS_CONF,

			ROT_X,
			ROT_Y,
			ROT_Z,
			ROT_CONF,

			LOCROT_X,
			LOCROT_Y,
			LOCROT_Z,
			LOCROT_CONF,

			NUM
		};
	};

	struct SkeletonJoint
	{
		enum Joint
		{
			HEAD			= 0,
			NECK			= 1,
			TORSO			= 2,
			WAIST			= 3,

			LEFT_SHOULDER	= 4,
			LEFT_ELBOW		= 5,
			LEFT_WRIST		= 6,
			LEFT_HAND		= 7,

			RIGHT_SHOULDER	=8,
			RIGHT_ELBOW		=9,
			RIGHT_WRIST		=10,
			RIGHT_HAND		=11,

			LEFT_HIP		=12,
			LEFT_KNEE		=13,
			LEFT_ANKLE		=14,
			LEFT_FOOT		=15,

			RIGHT_HIP		=16,
			RIGHT_KNEE		=17,
			RIGHT_ANKLE		=18,
			RIGHT_FOOT		=19,
			
			FACE_NOSE		=20,
			FACE_LEFT_EAR	=21,
			FACE_RIGHT_EAR	=22,
			FACE_FOREHEAD	=23,
			FACE_CHIN		=24,

			NUM_JOINTS
		};
	};

	struct SkeletonTrackingProfile
	{
		enum Profile
		{
			/** No joints at all (not really usefull)**/
			NONE		= 1,

			/** All joints (standard) **/
			ALL			= 2,
	
			/** All the joints in the upper body (torso and upwards) **/
			UPPER_BODY	= 3,
	
			/** All the joints in the lower body (torso and downwards) **/
			LOWER_BODY	= 4,
	
			/** The head and the hands (minimal profile) **/
			HEAD_HANDS	= 5
		};
	};

	static void PrintPos3D (FILE *file, ssi_real_t *j) {
		ssi_fprint (file, "[%.2f,%.2f,%2.f]", j[JOINT_VALUES::POS_X], j[JOINT_VALUES::POS_Y], j[JOINT_VALUES::POS_Z]);
	};

	static void PrintPos3Dconf (FILE *file, ssi_real_t *j) {
		if (j[JOINT_VALUES::POS_CONF] > 0.5f) {
			ssi_fprint (file, "[%.2f,%.2f,%2.f]", j[JOINT_VALUES::POS_X], j[JOINT_VALUES::POS_Y], j[JOINT_VALUES::POS_Z]);
		} else {
			ssi_fprint (file, "[-,-,-]");
		}
	};

	typedef float SKELETON[SkeletonJoint::NUM_JOINTS][JOINT_VALUES::NUM];
	SKELETON _skeleton;

	static void PrintSkeleton (FILE *file, SKELETON skel) {		
		ssi_fprint (file, "head="); PrintPos3Dconf (file, skel[SkeletonJoint::HEAD]);
		ssi_fprint (file, "\n\tneck="); PrintPos3Dconf (file, skel[SkeletonJoint::NECK]);
		ssi_fprint (file, "\n\ttorso="); PrintPos3Dconf (file, skel[SkeletonJoint::TORSO]);
		ssi_fprint (file, "\n\twaist="); PrintPos3Dconf (file, skel[SkeletonJoint::WAIST]);
		ssi_fprint (file, "\n\tleft_shoulder="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_SHOULDER]);
		ssi_fprint (file, "\n\tleft_elbow="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_ELBOW]);
		ssi_fprint (file, "\n\tleft_wrist="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_WRIST]);
		ssi_fprint (file, "\n\tleft_hand="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_HAND]);
		ssi_fprint (file, "\n\tright_shoulder="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_SHOULDER]);
		ssi_fprint (file, "\n\tright_elbow="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_ELBOW]);
		ssi_fprint (file, "\n\tright_wrist="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_WRIST]);
		ssi_fprint (file, "\n\tright_hand="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_HAND]);
		ssi_fprint (file, "\n\tleft_hip="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_HIP]);
		ssi_fprint (file, "\n\tleft_knee="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_KNEE]);
		ssi_fprint (file, "\n\tleft_ankle="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_ANKLE]);
		ssi_fprint (file, "\n\tleft_foot="); PrintPos3Dconf (file, skel[SkeletonJoint::LEFT_FOOT]);
		ssi_fprint (file, "\n\tright_hip="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_HIP]);
		ssi_fprint (file, "\n\tright_knee="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_KNEE]);
		ssi_fprint (file, "\n\tright_ankle="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_ANKLE]);
		ssi_fprint (file, "\n\tright_foot="); PrintPos3Dconf (file, skel[SkeletonJoint::RIGHT_FOOT]);
		ssi_fprint (file, "\n");
	};

	class Options : public OptionList {

	public:

		Options ()
			: users (1), project (false), scale (false), flip (false), 
			sampleRate (30.0), hangin (0), hangout (0), showRGBScene(false),
			profile (SkeletonTrackingProfile::ALL), rendering (515), depthImageModification (1), jointsToRender(0xFFFFFFF)
		{
			setSenderName ("kinect");
			recognizer[0] = '\0';
			recognizer_xml[0] = '\0';

			addOption ("sname", sname, SSI_MAX_CHAR, SSI_CHAR, "name of sender");
			addOption ("users", &users, 1, SSI_SIZE, "max number of tracked users");
			addOption ("project", &project, 1, SSI_BOOL, "project real world skeleton to screen coordinates");		
			addOption ("scale", &scale, 1, SSI_BOOL, "scale skeleton real world positions (x and y only) to interval [0..1]");
			addOption ("flip", &flip, 1, SSI_BOOL, "flips y-axis (only works with scale=true)");
			addOption ("sampleRate", &sampleRate, 1, SSI_TIME, "sample rate");
			addOption ("recognizer_xml", recognizer_xml, SSI_MAX_CHAR, SSI_CHAR, "xml recognizer definition file (see fubi documentation), overwrites 'recognizer' option");			
			addOption ("recognizer", recognizer, SSI_MAX_CHAR, SSI_CHAR, "indices of recognizers (0=RIGHT_HAND_OVER_SHOULDER,1=LEFT_HAND_OVER_SHOULDER,2=ARMS_CROSSED,3=ARMS_NEAR_POCKETS,4=ARMS_DOWN_TOGETHER,5=RIGHT_HAND_OUT,6=LEFT_HAND_OUT,7=HANDS_FRONT_TOGETHER,8=LEFT_KNEE_UP,9=RIGHT_KNEE_UP,10=RIGHT_HAND_OVER_HEAD,11=LEFT_HAND_OVER_HEAD,12=RIGHT_HAND_LEFT_OF_SHOULDER,13=RIGHT_HAND_RIGHT_OF_SHOULDER)");			
			addOption ("hangin", &hangin, 1, SSI_SIZE, "number of successive frames to be recognized as a gestures before the begin of a gesture is verified");
			addOption ("hangout", &hangout, 1, SSI_SIZE, "number of successive frames not to be recognized as a gestures before the end of a gesture is verified");
			addOption ("showRGBScene", &showRGBScene, 1, SSI_BOOL, "show scene as rgb image instead of depth image (set only by xml options file)");
			addOption ("profile", &profile, 1, SSI_INT, "skeleton profile: 	1=NONE, 2=ALL, 3=UPPER_BODY, 4=LOWER_BODY, 5=HEAD_HANDS");
			addOption ("rendering", &rendering, 1, SSI_UINT, "rendering options (sum up to combine settings):	0=None, 1=Shapes, 2=Skeletons, 4=UserCaptions, 8=GlobalOrientCaptions, 16=GlobalOrientCaptions, 32=LocalPosCaptions, 64=GlobalPosCaptions, 128=Background, 256=SwapRAndB, 512=FingerShapes, BodyMeasurements=2048, UseFilteredValues=4096");
			addOption ("depthImageModification", &depthImageModification, 1, SSI_UINT, "depth image modification (0=Raw, 1=UseHistogram, 2=StretchValueRange, 3=ConvertToRGB");			
			addOption ("jointsToRender", &jointsToRender, 1, SSI_UINT, "define which joints to render (sum up to combine settings):	0=None, 1=Head, 2=Neck, 4=Torso, 8=Waist, 16=LeftShoulder, 32=LeftElbow, 64=LeftWrist, 128=LeftHand, 256=RightShoulder, 512=RightElbow, 1024=RightWrist, 2048=RightHand, 4096=LeftHip, 8192=LeftKnee, 16384=LeftAnkle, 32768=LeftFoot, 65536=RightHip, 131072=RightKnee, 262144=RightAnkle, 524288=RightFoot, 268435455=AllJoints");
		};

		void setSenderName (const ssi_char_t *sname) {			
			if (sname) {
				ssi_strcpy (this->sname, sname);
			}
		}

		void setRecognizerXml (const ssi_char_t *name) {
			if (name) {
				ssi_strcpy (this->recognizer_xml, name);
			}
		}
		void setRecognizer (ssi_size_t index) {
			setRecognizer (1, &index);
		}

		void setRecognizer (ssi_size_t n_inds, ssi_size_t *inds) {
			recognizer[0] = '\0';
			if (n_inds > 0) {
				ssi_char_t s[SSI_MAX_CHAR];
				ssi_sprint (s, "%u", inds[0]);
				strcat (recognizer, s);
				for (ssi_size_t i = 1; i < n_inds; i++) {
					ssi_sprint (s, ",%u", inds[i]);
					strcat (recognizer, s);
				}
			}
		}
	
		ssi_size_t users;
		bool project;
		bool scale;
		bool flip;
		bool showRGBScene;
		ssi_time_t sampleRate;
		ssi_char_t sname[SSI_MAX_CHAR];
		ssi_char_t recognizer_xml[SSI_MAX_CHAR];
		ssi_char_t recognizer[SSI_MAX_CHAR];
		ssi_size_t hangin, hangout;
		SkeletonTrackingProfile::Profile profile;
		unsigned int rendering;
		unsigned int depthImageModification;
		unsigned int jointsToRender;
	};

	static const ssi_char_t *GetCreateName () { return "OpenNIKinect"; };
	static IObject *Create (const ssi_char_t *file) { return new OpenNIKinect (file); };
	~OpenNIKinect ();
	OpenNIKinect::Options *getOptions () { return &_options; };
	const ssi_char_t *getName () { return GetCreateName (); };
	const ssi_char_t *getInfo () { return "Microsoft Kinect OpenNI Framework"; };

	static void SetLogLevel (int level) {
		ssi_log_level = level;
	}

	ssi_video_params_t getDepthParams() 
	{
		ssi_video_params_t vParam;
		ssi_video_params(vParam, 640, 480, _options.sampleRate, 16, 1);
		return vParam;
	}

	ssi_video_params_t getSceneParams() 
	{
		ssi_video_params_t vParam;

		if(_options.showRGBScene)
			ssi_video_params(vParam, 640, 480, _options.sampleRate, 8, 3);
		else
			ssi_video_params(vParam, 640, 480, _options.sampleRate, 8, 4);

		return vParam;
	}

	ssi_video_params_t getRGBParams() 
	{
		ssi_video_params_t vParam;
		ssi_video_params(vParam, 640, 480, _options.sampleRate, 8, 3);
		return vParam;
	}

	ssi_video_params_t getIRParams() 
	{
		ssi_video_params_t vParam;
		ssi_video_params(vParam, 640, 480, _options.sampleRate, 16, 1);
		return vParam;
	}

	ssi_size_t getChannelSize () { return 5; };
	IChannel *getChannel (ssi_size_t index) { return _channels[index]; };
	bool setProvider (const ssi_char_t *name, IProvider *provider);
	bool connect ();
	bool start () { return ssi::Thread::start (); };
	void run ();
	bool stop () { return Thread::stop (); };
	bool disconnect ();

	bool setEventListener (IEventListener *listener);

protected:

	OpenNIKinect (const ssi_char_t *file = 0);
	OpenNIKinect::Options _options;

	IChannel *_channels[5];
	void setDepthProvider (IProvider *provider);
	void setSceneProvider (IProvider *provider);
	void setIRProvider (IProvider *provider);
	void setImageProvider (IProvider *provider);
	void setSkeletonProvider (IProvider *provider);

	ssi_char_t *_file;

	static ssi_char_t *ssi_log_name;
	static int ssi_log_level;

	IProvider *_depthProvider;
	IProvider *_sceneProvider;
	IProvider *_irProvider;
	IProvider *_imageProvider;
	IProvider *_skeletonProvider;

	unsigned int _currentCandidate;
	std::set<unsigned int> _currentUsers;
	unsigned short _nUsers;

	char _sceneBuffer[640*480*4];	// 8 bit four channels
	char _depthBuffer[640*480*2];	// 16 bit single channel
	char _rgbBuffer[640*480*3];		// 8 bit three channels
	char _irBuffer[640*480*2];	// 16 bit single channels
	SKELETON *_skeletonBuffer;

	void generateSceneMap();
	static void EmptySkeleton (SKELETON &s);
	static void SetSkeletonPos (SKELETON &s, SkeletonJoint::Joint joint, float x, float y, float z, float conf);
	static void SetSkeletonRot (SKELETON &s, SkeletonJoint::Joint joint, float rx, float ry, float rz, float conf);
	static void SetSkeletonLocRot (SKELETON &s, SkeletonJoint::Joint joint, float rx, float ry, float rz, float conf);

	Timer *_timer;

	void checkPostures(unsigned int userID);
	bool *_recognizerActive;
	IEventListener *_listener;
	ssi_size_t _event_sender_id;
	ssi_event_t *_recognizerEvents;
	ssi_size_t *_recognizerIds;
	ssi_size_t *_recognizerStart;
	ssi_size_t *_recognizerEnd;
	ssi_size_t *_hangin_counter;
	ssi_size_t *_hangout_counter;
	ssi_size_t _hangin;
	ssi_size_t _hangout;
	ssi_size_t _n_recognizer;
	ssi_size_t _n_recognizer_postures;
	ssi_size_t _n_recognizer_combinations;
	int *_recognizer;

	enum FubiRecognizerType 
	{
		NO_RECOGNIZER = 0,
		XML,
		INDICES
	} _recognizer_type;

	bool _is_fubi_connected;
	bool init_fubi ();
};

}

#endif
;
