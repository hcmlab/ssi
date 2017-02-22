// MicrosoftKinect2.cpp
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

#include "MicrosoftKinect2.h"
#include "Kinect2Utils.h"
#include "SafeRelease.h"
#include "WASAPICapture.h"
#include "SSI_SkeletonCons.h"
#include "thread/Timer.h"

#include <Kinect.h>
#include <Kinect.Face.h>
#include <shlobj.h>
#include <wchar.h>
#include <devicetopology.h>
#include <Functiondiscoverykeys_devpkey.h>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif




namespace ssi {

	ssi_char_t *MicrosoftKinect2::ssi_log_name = "mickinect2";

	//move to header if you know how to forward declare the typedefs
	Joint						m_joints[SSI_MICROSOFTKINECT2_SKELETON_MAX_USER][JointType_Count];
	JointOrientation			m_jointOrients[SSI_MICROSOFTKINECT2_SKELETON_MAX_USER][JointType_Count];
	HandState					leftHandState;
	HandState					rightHandState;

	struct FaceInfo
	{
		IHighDefinitionFaceFrameSource*		source;
		IHighDefinitionFaceFrameReader*		reader;
		BOOLEAN								isTracked;
		IFaceAlignment*						alignment;
		FaceAlignmentQuality				alignmentQuality;
		Vector4								orientation;
		CameraSpacePoint*					vertices;
		FaceInfo()
		{
			source = 0x0;
			reader = 0x0;
			isTracked = false;
			alignment = 0x0;
			orientation = { 0 };
			vertices = 0x0;
		};
	};
	FaceInfo m_faces[SSI_MICROSOFTKINECT2_SKELETON_MAX_USER];
	unsigned int				m_faceVertexCount;
	IFaceModel*					m_faceModel;


	MicrosoftKinect2::MicrosoftKinect2(const ssi_char_t *file)
		: m_face_provider(0),
		m_face3d_provider(0),
		m_au_provider(0),
		m_head_provider(0),
		m_depth_provider(0),
		m_depthr_provider(0),
		m_ir_provider(0),
		m_irr_provider(0),
		m_rgb_provider(0),
		m_skeleton_provider(0),
		m_skeleton_to_screen_provider(0),
		m_skeleton_confidence_provider(0),
		m_hand_provider(0),
		m_channel_order(0),
		m_n_skeletons(0),
		m_skeleton(0),
		m_skeleton_to_screen(0),
		m_skeleton_confidence(0),
		m_timer(0),
		m_irImage(0),
		m_actionunits(0),
		m_facepoints2d(0),
		m_facepoints3d(0),
		m_headpose(0),
		m_handpose(0),
		m_audio_provider(0),
		m_audio_device(0),
		m_audio_capturer(0),
		m_audio_is_capturing (false),
		_file (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {
			
		m_LastFaceTrackSucceeded = false;
		
		m_pCoordinateMapper = NULL;
		m_bodyFrame = NULL;
		m_pBodyFrameReference = NULL;

		m_pDepthFrameReference = NULL;
		m_pColorFrameReference = NULL;
		m_pIrFrameReference = NULL;
		m_depthFrame = NULL;
		m_colorFrame = NULL;
		m_irFrame = NULL;
		m_depthImage = NULL;
		m_colorImage = NULL;
		m_irImage = NULL;
		m_bgraBuffer = NULL;
		m_depthrBuffer = NULL;
		m_irrBuffer = NULL;

		for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; i++)
			m_faces[i].reader = NULL;

		if (file) {
			if (!OptionList::LoadXML (file, _options)) {
				OptionList::SaveXML (file, _options);
			}
			_file = ssi_strcpy (file);
		}

		HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
		if (!SUCCEEDED(hr))
		{
			ssi_wrn("could not initialize com");
		}
	}

	MicrosoftKinect2::~MicrosoftKinect2 () {

		if (_file) {
			OptionList::SaveXML (_file, _options);
			delete[] _file;
		}

		CoUninitialize();
	}

	bool MicrosoftKinect2::setProvider (const ssi_char_t *name, IProvider *provider) {

		if (strcmp (name, SSI_MICROSOFTKINECT2_FACEPOINT_PROVIDER_NAME) == 0) {
			setFacePointProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT2_FACEPOINT3D_PROVIDER_NAME) == 0) {
			setFacePoint3DProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT2_ACTIONUNIT_PROVIDER_NAME) == 0) {
			setActionUnitProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT2_HEADPOSE_PROVIDER_NAME) == 0) {
			setHeadPoseProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT2_RGBIMAGE_PROVIDER_NAME) == 0) {
			setRGBImageProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT2_DEPTHRAW_PROVIDER_NAME) == 0) {
			setDepthRawProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT2_DEPTHIMAGE_PROVIDER_NAME) == 0) {
			setDepthImageProvider (provider);
			return true;
		} else if (strcmp(name, SSI_MICROSOFTKINECT2_INFRAREDRAW_PROVIDER_NAME) == 0) {
			setInfraredRawProvider(provider);
			return true;
		} else if (strcmp(name, SSI_MICROSOFTKINECT2_INFRAREDIMAGE_PROVIDER_NAME) == 0) {
			setInfraredImageProvider(provider);
			return true;
		} else if (strcmp(name, SSI_MICROSOFTKINECT2_SKELETON_PROVIDER_NAME) == 0) {
			setSkeletonProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT2_SKELETON2SCREEN_PROVIDER_NAME) == 0) {
			setSkeleton2ScreenProvider (provider);
			return true;
		} else if (strcmp(name, SSI_MICROSOFTKINECT2_SKELETONCONFIDENCE_PROVIDER_NAME) == 0) {
			setSkeletonConfidenceProvider(provider);
			return true;
		} else if (strcmp(name, SSI_MICROSOFTKINECT2_HANDPOSE_PROVIDER_NAME) == 0) {
			setHandPoseProvider(provider);
			return true;
		} else if (strcmp(name, SSI_MICROSOFTKINECT2_AUDIO_PROVIDER_NAME) == 0) {
			setAudioProvider(provider);
			return true;
		}

		ssi_wrn ("unkown provider name '%s'", name);

		return false;
	}

	void MicrosoftKinect2::setRGBImageProvider(IProvider *rgb_provider) {

		if (m_rgb_provider) {
			ssi_wrn("rgb image provider already set");
		}

		m_rgb_provider = rgb_provider;

		if (m_rgb_provider) {

			ssi_video_params_t params = getRGBImageParams();

			m_rgb_provider->setMetaData(sizeof(params), &params);
			m_rgb_channel.stream.sr = _options.sr;

			m_rgb_channel.stream.byte = 1920 * 1080 * 4;

			m_rgb_provider->init(&m_rgb_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "rgb provider set");
		}
	}

	void MicrosoftKinect2::setDepthRawProvider(IProvider *depthr_provider) {

		if (m_depthr_provider) {
			ssi_wrn("depth raw provider already set");
		}

		m_depthr_provider = depthr_provider;
		if (m_depthr_provider) {
			ssi_video_params_t params = getDepthRawParams();
			m_depthr_provider->setMetaData(sizeof(params), &params);
			m_depthr_channel.stream.sr = _options.sr;
			m_depthr_channel.stream.byte = 512 * 424 * 2;

			m_depthr_provider->init(&m_depthr_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "depth raw provider set");
		}
	}


	void MicrosoftKinect2::setDepthImageProvider(IProvider *depth_provider) {

		if (m_depth_provider) {
			ssi_wrn("depth image provider already set");
		}

		m_depth_provider = depth_provider;
		if (m_depth_provider) {
			ssi_video_params_t params = getDepthImageParams();
			m_depth_provider->setMetaData(sizeof(params), &params);
			m_depth_channel.stream.sr = _options.sr;
			m_depth_channel.stream.byte = 512 * 424 * 1;

			m_depth_provider->init(&m_depth_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "depth provider set");
		}
	}

	void MicrosoftKinect2::setInfraredRawProvider(IProvider *irr_provider) {

		if (m_irr_provider) {
			ssi_wrn("infrared raw provider already set");
		}

		m_irr_provider = irr_provider;
		if (m_irr_provider) {
			ssi_video_params_t params = getInfraredRawParams();
			m_irr_provider->setMetaData(sizeof(params), &params);
			m_irr_channel.stream.sr = _options.sr;
			m_irr_channel.stream.byte = 512 * 424 * 2;

			m_irr_provider->init(&m_irr_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "infrared raw provider set");
		}
	}

	void MicrosoftKinect2::setInfraredImageProvider(IProvider *ir_provider) {

		if (m_ir_provider) {
			ssi_wrn("infrared image provider already set");
		}

		m_ir_provider = ir_provider;
		if (m_ir_provider) {
			ssi_video_params_t params = getInfraredImageParams();
			m_ir_provider->setMetaData(sizeof(params), &params);
			m_ir_channel.stream.sr = _options.sr;
			m_ir_channel.stream.byte = 512 * 424 * 1;

			m_ir_provider->init(&m_ir_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "infrared provider set");
		}
	}

	void MicrosoftKinect2::setSkeletonProvider(IProvider *skeleton_provider) {

		if (m_skeleton_provider) {
			ssi_wrn("skeleton image provider already set");
		}

		m_skeleton_provider = skeleton_provider;
		if (m_skeleton_provider) {
			
			if (_options.trackNearestPersons > 0 && _options.trackNearestPersons <= SSI_MICROSOFTKINECT2_SKELETON_MAX_USER){
				m_n_skeletons = _options.trackNearestPersons;
				m_trackNearToFar = true;
			}
			else{
				m_n_skeletons = SSI_MICROSOFTKINECT2_SKELETON_MAX_USER;
				m_trackNearToFar = false;
			}
			
			SSI_SKELETON_META m = ssi_skeleton_meta(SSI_SKELETON_TYPE::MICROSOFT_KINECT2, m_n_skeletons);			
			m_skeleton_provider->setMetaData(sizeof(SSI_SKELETON_META), &m);
			m_skeleton_channel.stream.dim = m_n_skeletons * JointType_Count * SKELETON_JOINT_VALUE::NUM;
			m_skeleton_channel.stream.sr = _options.sr;
			m_skeleton_provider->init(&m_skeleton_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "skeleton provider set");
		}
	}

	void MicrosoftKinect2::setSkeleton2ScreenProvider(IProvider *skeleton_to_screen_provider) {

		if (m_skeleton_to_screen_provider) {
			ssi_wrn("skeleton2screen image provider already set");
		}

		m_skeleton_to_screen_provider = skeleton_to_screen_provider;
		if (m_skeleton_to_screen_provider) {
			
			if (_options.trackNearestPersons > 0 && _options.trackNearestPersons <= SSI_MICROSOFTKINECT2_SKELETON_MAX_USER){
				m_n_skeletons = _options.trackNearestPersons;
				m_trackNearToFar = true;
			}
			else{
				m_n_skeletons = SSI_MICROSOFTKINECT2_SKELETON_MAX_USER;
				m_trackNearToFar = false;
			}
			
			SSI_SKELETON_META m = ssi_skeleton_meta(SSI_SKELETON_TYPE::MICROSOFT_KINECT2, m_n_skeletons);			
			m_skeleton_to_screen_provider->setMetaData(sizeof(SSI_SKELETON_META), &m);
			m_skeleton_to_screen_channel.stream.dim = m_n_skeletons * JointType_Count * SKELETON_JOINT_VALUE::NUM;
			m_skeleton_to_screen_channel.stream.sr = _options.sr;
			m_skeleton_to_screen_provider->init(&m_skeleton_to_screen_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "skeleton2screen provider set");
		}
	}

	void MicrosoftKinect2::setSkeletonConfidenceProvider(IProvider *skeleton_confidence_provider) {

		if (m_skeleton_confidence_provider) {
			ssi_wrn("skeletonconf provider already set");
		}

		m_skeleton_confidence_provider = skeleton_confidence_provider;
		if (m_skeleton_confidence_provider) {

			if (_options.trackNearestPersons > 0 && _options.trackNearestPersons <= SSI_MICROSOFTKINECT2_SKELETON_MAX_USER){
				m_n_skeletons = _options.trackNearestPersons;
				m_trackNearToFar = true;
			}
			else{
				m_n_skeletons = SSI_MICROSOFTKINECT2_SKELETON_MAX_USER;
				m_trackNearToFar = false;
			}

			m_skeleton_confidence_channel.stream.dim = m_n_skeletons * JointType_Count;
			m_skeleton_confidence_channel.stream.sr = _options.sr;
			m_skeleton_confidence_provider->init(&m_skeleton_confidence_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "skeletonconf provider set");
		}
	}


	void MicrosoftKinect2::setHandPoseProvider(IProvider *hand_provider) {

		if (m_hand_provider) {
			ssi_wrn("hand pose provider already set");
		}
		m_hand_provider = hand_provider;
		if (m_hand_provider) {
			m_hand_channel.stream.sr = _options.sr;
			m_hand_provider->init(&m_hand_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "hand pose provider set");
		}
		
	}

	void MicrosoftKinect2::setFacePointProvider(IProvider *face_provider) {

		if (m_face_provider) {
			ssi_wrn("face point provider already set");
		}

		m_face_provider = face_provider;
		if (m_face_provider) {
			SSI_FACE_META m = ssi_face_meta(SSI_FACE_TYPE::MICROSOFT_KINECT2, m_n_skeletons);
			m_face_provider->setMetaData(sizeof(SSI_FACE_META), &m);
			m_face_channel.stream.sr = _options.sr;
			m_face_provider->init(&m_face_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "face provider set");
		}
	}

	void MicrosoftKinect2::setFacePoint3DProvider(IProvider *face3d_provider) {

		if (m_face3d_provider) {
			ssi_wrn("face point provider already set");
		}

		m_face3d_provider = face3d_provider;
		if (m_face3d_provider) {
			SSI_FACE_META m = ssi_face_meta(SSI_FACE_TYPE::MICROSOFT_KINECT2, m_n_skeletons);
			m_face3d_provider->setMetaData(sizeof(SSI_FACE_META), &m);
			m_face3d_channel.stream.sr = _options.sr;
			m_face3d_provider->init(&m_face3d_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "face3d provider set");
		}
	}

	void MicrosoftKinect2::setHeadPoseProvider(IProvider *head_provider) {

		if (m_head_provider) {
			ssi_wrn("head pose provider already set");
		}

		m_head_provider = head_provider;
		if (m_head_provider) {
			m_head_channel.stream.sr = _options.sr;
			m_head_provider->init(&m_head_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "head pose provider set");
		}
	}

	void MicrosoftKinect2::setAudioProvider(IProvider *audio_provider) {

		if (m_audio_provider) {
			ssi_wrn("audio provider already set");
		}

		m_audio_provider = audio_provider;
		if (m_audio_provider) {
			m_audio_channel.stream.sr = 16000.0;
			m_audio_provider->init(&m_audio_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "audio provider set");
		}
	}

	void MicrosoftKinect2::setActionUnitProvider(IProvider *au_provider) {

		if (m_au_provider) {
			ssi_wrn("action unit provider already set");
		}

		m_au_provider = au_provider;
		if (m_au_provider) {
			m_au_channel.stream.sr = _options.sr;
			m_au_provider->init(&m_au_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "action unit provider set");
		}
	}

	bool MicrosoftKinect2::connect () {

		if (getOptions()->sr > 30){
			ssi_wrn("Kinect 2 can only do 30 fps on every Camera, consinder reducing the sample rate")
		}

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "try to connect sensor...");
		
		DWORD frameTypes = 0;

		if (m_rgb_provider || m_au_provider || m_face_provider || m_face3d_provider || m_head_provider){
			frameTypes |= FrameSourceTypes::FrameSourceTypes_Color;
			m_bgraBuffer = new RGBQUAD[getRGBImageParams().widthInPixels * getRGBImageParams().heightInPixels];
		}

		if (m_depth_provider || m_depthr_provider || m_au_provider || m_face_provider || m_face3d_provider || m_head_provider){
			frameTypes |= FrameSourceTypes::FrameSourceTypes_Depth;
			m_depthrBuffer = new UINT16[getDepthRawParams().widthInPixels * getDepthRawParams().heightInPixels];
		}

		if (m_ir_provider || m_irr_provider) {
			frameTypes |= FrameSourceTypes::FrameSourceTypes_Infrared;
			m_irrBuffer = new UINT16[getInfraredRawParams().widthInPixels * getInfraredRawParams().heightInPixels];
		}

		if (m_depth_provider) {
			m_depthImage = new ssi_byte_t[ssi_video_size(getDepthImageParams())];
		}

		if (m_ir_provider) {
			m_irImage = new ssi_byte_t[ssi_video_size(getInfraredImageParams())];
		}

		if (m_skeleton_provider || m_skeleton_to_screen_provider || m_face_provider || m_face3d_provider){ 
			frameTypes |= FrameSourceTypes::FrameSourceTypes_Body;
			frameTypes |= FrameSourceTypes::FrameSourceTypes_BodyIndex;

			for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; ++i)
			{
				m_bodies[i] = 0;
				for (int j = 0; j < JointType_Count; ++j)
				{
					m_joints[i][j].JointType = (JointType)j;
					m_joints[i][j].Position = { 0 };
					m_joints[i][j].TrackingState = TrackingState_NotTracked;
					m_jointOrients[i][j].JointType = (JointType)j;
					m_jointOrients[i][j].Orientation = { 0 };
				}
			}

		}

		if (!m_channel_order) {
			m_channel_order = new int[m_n_skeletons];
		}
		for (ssi_size_t i = 0; i < m_n_skeletons; i++) {
			m_channel_order[i] = -1;
		}

		m_KinectSensorPresent = FALSE;

		// Initialize the Kinect 2
		if (SUCCEEDED(GetDefaultKinectSensor(&m_kinectSensor)) && m_kinectSensor)
		{
			if (SUCCEEDED(m_kinectSensor->Open()))
			{
				if (SUCCEEDED(m_kinectSensor->OpenMultiSourceFrameReader(frameTypes, &m_multiSourceFrameReader)))
				{
					m_KinectSensorPresent = TRUE;
				}
			}
		}

		if (m_skeleton_to_screen_provider || m_face_provider || _options.showBodyTracking || _options.showFaceTracking){
			m_kinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);
		}

		if (m_face_provider || m_face3d_provider){

			for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; i++)
				m_faces[i].source = NULL;
			
			unsigned int triangleCount;
			if (SUCCEEDED(GetFaceModelVertexCount(&m_faceVertexCount)) && SUCCEEDED(GetFaceModelTriangleCount(&triangleCount)))
			{
				if (m_faceVertexCount != FACEPOINT3D::NUM){
					ssi_msg(SSI_LOG_LEVEL_ERROR, "Face Vertex count is %i, but should be %i\n", m_faceVertexCount, FACEPOINT3D::NUM);
				}
				else{
					for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; i++){
						if (SUCCEEDED(CreateHighDefinitionFaceFrameSource(m_kinectSensor, &m_faces[i].source))){
							if (SUCCEEDED(m_faces[i].source->OpenReader(&m_faces[i].reader))){

								if (FAILED(CreateFaceAlignment(&m_faces[i].alignment)))
									ssi_msg(SSI_LOG_LEVEL_ERROR, "failed initinalizing face alignment!\n");
								float faceShapeDeformations[FaceShapeDeformations_Count];
								memset(faceShapeDeformations, 0, sizeof(faceShapeDeformations));
								if (FAILED(CreateFaceModel(1.0f, FaceShapeDeformations_Count, faceShapeDeformations, &m_faceModel)))
									ssi_msg(SSI_LOG_LEVEL_ERROR, "failed initinalizing face model!\n");

								m_faces[i].vertices = new CameraSpacePoint[m_faceVertexCount];
							}
						}
					}
				}
			}
		}

		initMemory();

		for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; ++i){
			m_SkeletonTracked[i] = false;
		}

		// intialize audio sensor
		if (m_audio_provider)
		{
			HRESULT hr = getKinectAudioDevice(&m_audio_device);
			if (SUCCEEDED(hr))
			{
#ifdef USE_SSI_LEAK_DETECTOR
#ifdef _DEBUG
#undef new
#endif
#endif
				m_audio_capturer = new (std::nothrow) CWASAPICapture(m_audio_device);
#ifdef USE_SSI_LEAK_DETECTOR
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
				static char THIS_FILE[] = __FILE__;
#endif
#endif
				if ((NULL != m_audio_capturer) && m_audio_capturer->Initialize(SSI_MICROSOFTKINECT2_AUDIO_TARGETLATENCY))
				{
					WAVEFORMATEX *format = m_audio_capturer->GetOutputFormat();

					m_audio_is_capturing = false;
					if (m_audio_capturer->Start(m_audio_provider))
					{
						ssi_msg(SSI_LOG_LEVEL_BASIC, "capturing audio data");
						m_audio_is_capturing = true;
					} 
					else
					{
						ssi_wrn("unable to capture audio data");						
					}
				}
				else
				{
					ssi_wrn ("unable to initialize audio capturer");
				}
			}
			else
			{
				ssi_wrn("no ready kinect found");
			}
		}
				
		ssi_msg (SSI_LOG_LEVEL_DETAIL, "connected");

		// set thread name
		Thread::setName (getName ());

		return true;
	}

	void MicrosoftKinect2::run () {
		
		if (!m_timer) {
			m_timer = new Timer (1/_options.sr);
		}

		process();
		m_timer->wait ();
	}

	void MicrosoftKinect2::initMemory () {

		if (m_skeleton_provider) 
		{			
			m_skeleton = new MicrosoftKinect2::SKELETON[m_n_skeletons];
			for( ssi_size_t k = 0; k < m_n_skeletons; k++)
			{
				for (ssi_size_t i = 0; i<JointType_Count; i++)
				{
					for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
					{
						m_skeleton[k][i][j] = SSI_MICROSOFTKINECT2_INVALID_SKELETON_JOINT_VALUE;
					}
				}
			}
		}

		if (m_skeleton_to_screen_provider) 
		{							
			m_skeleton_to_screen = new MicrosoftKinect2::SKELETON[m_n_skeletons];
			for( ssi_size_t k = 0; k < m_n_skeletons; k++)
			{
				for (ssi_size_t i = 0; i<JointType_Count; i++)
				{
					for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
					{
						m_skeleton_to_screen[k][i][j] = SSI_MICROSOFTKINECT2_INVALID_SKELETON_JOINT_VALUE;
					}
				}
			}
		}

		if (m_skeleton_confidence_provider)
		{
			m_skeleton_confidence = new MicrosoftKinect2::SKELETONCONF[m_n_skeletons];
			for (ssi_size_t k = 0; k < m_n_skeletons; k++)
			{
				for (ssi_size_t i = 0; i<JointType_Count; i++)
				{
					m_skeleton_confidence[k][i] = SSI_MICROSOFTKINECT2_INVALID_SKELETON_CONFIDENCE_VALUE;
				}
			}
		}

		if(m_face3d_provider || m_face_provider || getOptions()->showFaceTracking){
			m_facepoints3d = new MicrosoftKinect2::FACEPOINTS3D[m_n_skeletons];
			for (ssi_size_t i = 0; i < m_n_skeletons; i++)
			{
				for (ssi_size_t j = 0; j < FACEPOINT3D::NUM; j++)
				{
					m_facepoints3d[i][j][FACEPOINT3D_VALUE::X] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT3D_VALUE;
					m_facepoints3d[i][j][FACEPOINT3D_VALUE::Y] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT3D_VALUE;
					m_facepoints3d[i][j][FACEPOINT3D_VALUE::Z] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT3D_VALUE;
				}
			}
		}
	
		if (m_face_provider || getOptions()->showFaceTracking){
			m_facepoints2d = new MicrosoftKinect2::FACEPOINTS[m_n_skeletons];
			for (ssi_size_t i = 0; i < m_n_skeletons; i++)
			{
				for (ssi_size_t j = 0; j < FACEPOINT3D::NUM; j++)
				{
					m_facepoints2d[i][j][FACEPOINT_VALUE::X] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT_VALUE;
					m_facepoints2d[i][j][FACEPOINT_VALUE::Y] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT_VALUE;
				}
			}
		}

		if(m_au_provider){
			m_actionunits = new MicrosoftKinect2::ACTIONUNITS[m_n_skeletons];
			for (ssi_size_t i = 0; i < m_n_skeletons; i++)
			{
				for (ssi_size_t j = 0; j < ACTIONUNIT::NUM; j++)
				{
					m_actionunits[i][j] = SSI_MICROSOFTKINECT2_INVALID_ACTIONUNIT_VALUE;
				}
			}
		}

		if (m_head_provider || m_face3d_provider || m_face_provider){
			m_headpose = new MicrosoftKinect2::HEADPOSE[m_n_skeletons];
			for (ssi_size_t i = 0; i < m_n_skeletons; i++){
				for (ssi_size_t j = 0; j < HEADPOSE_ANGLE::NUM; j++)
				{
					m_headpose[i][j] = SSI_MICROSOFTKINECT2_INVALID_HEADPOSE_VALUE;
				}
			}
		}

		if (m_hand_provider){
			m_handpose = new HANDPOSE[m_n_skeletons];
			for (ssi_size_t i = 0; i < m_n_skeletons; i++){
				for (ssi_size_t j = 0; j < 2; j++)
				{
					m_handpose[i][j] = HandState_Unknown;
				}
			}
		}
		
	}
	
	void MicrosoftKinect2::clearData (ssi_size_t nui_id) {

		// determine which skeleton data to clear
		int ssi_id = -1;
		for(ssi_size_t l = 0; l < m_n_skeletons; ++l)
		{
			if(m_channel_order[l] == nui_id)
			{
				ssi_id = l;
				break;
			}
		}
		if(ssi_id < 0)
			return; // no data for skeleton stored

		// clear data
		if (m_skeleton_provider) 
		{			
			for (ssi_size_t i = 0; i<JointType_Count; i++)
			{
				for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
				{
					m_skeleton[ssi_id][i][j] = SSI_MICROSOFTKINECT2_INVALID_SKELETON_JOINT_VALUE;
				}
			}
		}

		if (m_skeleton_to_screen_provider) 
		{	
			for (ssi_size_t i = 0; i<JointType_Count; i++)
			{
				for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
				{
					m_skeleton_to_screen[ssi_id][i][j] = SSI_MICROSOFTKINECT2_INVALID_SKELETON_JOINT_VALUE;
				}
			}
		}

		if (m_skeleton_confidence_provider)
		{
			for (ssi_size_t i = 0; i<JointType_Count; i++)
			{
				m_skeleton_confidence[ssi_id][i] = SSI_MICROSOFTKINECT2_INVALID_SKELETON_CONFIDENCE_VALUE;
			}
		}

		if (m_hand_provider)
		{
			for (ssi_size_t i = 0; i < 2; i++)
			{
				m_handpose[ssi_id][i] = HandState_Unknown;
			}
		}



		//clear corresponding face/au/head data aswell
		if (m_face3d_provider || m_face_provider || getOptions()->showFaceTracking){
			for (ssi_size_t j = 0; j < FACEPOINT3D::NUM; j++)
			{
				m_facepoints3d[ssi_id][j][FACEPOINT3D_VALUE::X] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT3D_VALUE;
				m_facepoints3d[ssi_id][j][FACEPOINT3D_VALUE::Y] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT3D_VALUE;
				m_facepoints3d[ssi_id][j][FACEPOINT3D_VALUE::Z] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT3D_VALUE;
			}
		}

		if (m_face_provider || getOptions()->showFaceTracking){

			for (ssi_size_t j = 0; j < FACEPOINT3D::NUM; j++)
			{
				m_facepoints2d[ssi_id][j][FACEPOINT_VALUE::X] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT_VALUE;
				m_facepoints2d[ssi_id][j][FACEPOINT_VALUE::Y] = SSI_MICROSOFTKINECT2_INVALID_FACEPOINT_VALUE;
			}
		}

		if (m_au_provider){
			
			for (ssi_size_t j = 0; j < ACTIONUNIT::NUM; j++)
			{
				m_actionunits[ssi_id][j] = SSI_MICROSOFTKINECT2_INVALID_ACTIONUNIT_VALUE;
			}
			
		}

		if (m_head_provider || m_face3d_provider || m_face_provider){
				for (ssi_size_t j = 0; j < HEADPOSE_ANGLE::NUM; j++)
				{
					m_headpose[ssi_id][j] = SSI_MICROSOFTKINECT2_INVALID_HEADPOSE_VALUE;
				}
		}	
	}

	HRESULT MicrosoftKinect2::getKinectAudioDevice(IMMDevice **ppDevice)
	{
		IMMDeviceEnumerator *pDeviceEnumerator = NULL;
		IMMDeviceCollection *pDeviceCollection = NULL;
		HRESULT hr = S_OK;

		*ppDevice = NULL;

		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDeviceEnumerator));
		if (SUCCEEDED(hr))
		{
			hr = pDeviceEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pDeviceCollection);
			if (SUCCEEDED(hr))
			{
				UINT deviceCount;
				hr = pDeviceCollection->GetCount(&deviceCount);
				if (SUCCEEDED(hr))
				{
					// Iterate through all active audio capture devices looking for one that matches
					// the specified Kinect sensor.
					for (UINT i = 0; i < deviceCount; ++i)
					{
						IMMDevice *pDevice = NULL;
						bool deviceFound = false;
						hr = pDeviceCollection->Item(i, &pDevice);

						{ // Identify by friendly name
							IPropertyStore* pPropertyStore = NULL;
							PROPVARIANT varName;
							int sensorIndex = 0;

							hr = pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
							PropVariantInit(&varName);
							hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &varName);

							if (0 == lstrcmpW(varName.pwszVal, L"Microphone Array (Xbox NUI Sensor)") ||
								1 == swscanf_s(varName.pwszVal, L"Microphone Array (%d- Xbox NUI Sensor)", &sensorIndex))
							{
								*ppDevice = pDevice;
								deviceFound = true;
							}

							PropVariantClear(&varName);
							SafeRelease(pPropertyStore);

							if (true == deviceFound)
							{
								break;
							}
						}

						SafeRelease(pDevice);
					}
				}

				SafeRelease(pDeviceCollection);
			}

			SafeRelease(pDeviceEnumerator);
		}

		if (SUCCEEDED(hr) && (NULL == *ppDevice))
		{
			// If nothing went wrong but we haven't found a device, return failure
			hr = E_FAIL;
		}

		return hr;
	}


	void MicrosoftKinect2::processMultiSourceFrame(){

		if (m_multiSourceFrameReader)
		{
			if (FAILED(m_multiSourceFrameReader->AcquireLatestFrame(&m_pMultiSourceFrame))) {
				m_pMultiSourceFrame = NULL;
			}
		}
	}

	void MicrosoftKinect2::processImages() {

		if (m_pMultiSourceFrame)
		{

			/*
			* Depth
			*/
			if (m_depth_provider || m_depthr_provider || m_au_provider || m_face_provider || m_face3d_provider || m_head_provider)
			{
				if (SUCCEEDED(m_pMultiSourceFrame->get_DepthFrameReference(&m_pDepthFrameReference)))
				{
					if (SUCCEEDED(m_pDepthFrameReference->AcquireFrame(&m_depthFrame)))
					{
						UINT nBufferSize;
						UINT16 *pBuffer = NULL;

						if (SUCCEEDED(m_depthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer)))
						{
							if (FAILED(m_depthFrame->CopyFrameDataToArray(nBufferSize, reinterpret_cast<UINT16*>(m_depthrBuffer)))){
								ssi_print("failed to get depth frame\n");
							}
						}
					}
				}

			}

			SafeRelease(m_depthFrame);
			SafeRelease(m_pDepthFrameReference);

			/*
			* Infrared
			*/
			if (m_ir_provider || m_irr_provider){

				if (SUCCEEDED(m_pMultiSourceFrame->get_InfraredFrameReference(&m_pIrFrameReference)))
				{
					if (SUCCEEDED(m_pIrFrameReference->AcquireFrame(&m_irFrame))){

						UINT nBufferSize;
						UINT16 *pBuffer = NULL;

						if (SUCCEEDED(m_irFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer)))
						{
							if (FAILED(m_irFrame->CopyFrameDataToArray(nBufferSize, reinterpret_cast<UINT16*>(m_irrBuffer)))){
								ssi_print("failed to get IR frame\n");
							}
						}
					}
				}
			}

			SafeRelease(m_irFrame);
			SafeRelease(m_pIrFrameReference);

			/*
			* RGB
			*/
			if (m_rgb_provider || m_au_provider || m_face_provider || m_face3d_provider || m_head_provider)
			{
				if (SUCCEEDED(m_pMultiSourceFrame->get_ColorFrameReference(&m_pColorFrameReference)))
				{
					if (SUCCEEDED(m_pColorFrameReference->AcquireFrame(&m_colorFrame)))
					{
						UINT nBufferSize;
						BYTE *pBuffer = NULL;
						ColorImageFormat imageFormat = ColorImageFormat_None;
						if (SUCCEEDED(m_colorFrame->get_RawColorImageFormat(&imageFormat)) && SUCCEEDED(m_colorFrame->AccessRawUnderlyingBuffer(&nBufferSize, &pBuffer)))
						{

							if (imageFormat == ColorImageFormat_Bgra)
							{
								m_bgraBuffer = (RGBQUAD*)pBuffer;
							}
							else // Convert (from YUV2) to BGRA
							{
								nBufferSize = getRGBImageParams().widthInPixels * getRGBImageParams().heightInPixels * sizeof(RGBQUAD);
								if (SUCCEEDED(m_colorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(m_bgraBuffer), ColorImageFormat_Bgra))){
									showSkeleton();
									showFace();
								}
							}
						}
					}
				}

				SafeRelease(m_colorFrame);
				SafeRelease(m_pColorFrameReference);
			}
			
		}

	}

	void MicrosoftKinect2::processBody() {
		
		if (m_pMultiSourceFrame)
		{
			if (m_skeleton_provider || m_skeleton_to_screen_provider || m_skeleton_confidence_provider || m_face_provider || m_face3d_provider){
				m_pBodyFrameReference = NULL;
				if (SUCCEEDED(m_pMultiSourceFrame->get_BodyFrameReference(&m_pBodyFrameReference)))
				{
					
					m_bodyFrame = NULL;
					if (SUCCEEDED(m_pBodyFrameReference->AcquireFrame(&m_bodyFrame)))
					{
						if (SUCCEEDED(m_bodyFrame->GetAndRefreshBodyData(SSI_MICROSOFTKINECT2_SKELETON_MAX_USER, m_bodies)))
						{
							m_hasNewData = true;
							for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; ++i)
							{
								bool gotJoints = false;
								IBody* pBody = m_bodies[i];
								BOOLEAN bTracked = false;
								m_SkeletonTracked[i] = false;

								if (pBody)
								{
									if (SUCCEEDED(pBody->get_IsTracked(&bTracked)))
									{
										if (bTracked)
										{
											if (m_face_provider || m_face3d_provider)
											{
												// fix face tracking by enforcing trackingIDs from body
												UINT64 bodyTId;
												if (SUCCEEDED(pBody->get_TrackingId(&bodyTId)))
												{
													m_faces[i].source->put_TrackingId(bodyTId);													
												}
											}

											if (m_hand_provider)
											{
												//Todo: Fit this to match the correct skeleton (works for 1 user only for now)
												pBody->get_HandLeftState(&leftHandState);
												m_handpose[0][0] = ssi_cast(float, leftHandState);
												pBody->get_HandRightState(&rightHandState);
												m_handpose[0][1] = ssi_cast(float, rightHandState);
											}

											if (SUCCEEDED(pBody->GetJoints(JointType_Count, m_joints[i]))
												&& SUCCEEDED(pBody->GetJointOrientations(JointType_Count, m_jointOrients[i]))){
												m_SkeletonTracked[i] = true;

											}


										}

									}
								}

								if (!m_SkeletonTracked[i])
									clearData(i);
							}

							for (ssi_size_t i = 0; i < m_n_skeletons; i++) {
								m_channel_order[i] = -1;
							}


							int count = 0;
							for (ssi_size_t i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; i++)
							{
								if (m_SkeletonTracked[i])
								{
									m_channel_order[count++] = i;
								}
							}

							if (count > 1) {

								//bubble sort
								bool orderChanged = true;
								while (orderChanged){
									orderChanged = false;
									for (int i = 0; i < (count - 1); i++){

										bool swap = false;
										if (m_trackNearToFar){
											swap = (m_joints[m_channel_order[i]][JointType_Head].Position.Z > m_joints[m_channel_order[i + 1]][JointType_Head].Position.Z);
										}
										else
										{
											swap = (m_joints[m_channel_order[i]][JointType_Head].Position.X > m_joints[m_channel_order[i + 1]][JointType_Head].Position.X);
										}

										if (swap) {
											int tmp = m_channel_order[i];
											m_channel_order[i] = m_channel_order[i + 1];
											m_channel_order[i + 1] = tmp;
											orderChanged = true;
										}
									}
								}
							}

							for (ssi_size_t j = 0; j < m_n_skeletons; j++)
							{
								if (m_channel_order[j] == -1) {
									continue;
								}
								int pick = m_channel_order[j];
											
								float absX, absY, absZ, conf;

								if (m_skeleton_provider) {
									for (ssi_size_t i = 0; i < _JointType::JointType_Count; i++)
									{
										// position
										conf = ssi_cast(float, m_joints[pick][i].TrackingState) / ssi_cast(float, TrackingState_Tracked);
										m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_CONF] = conf;
										m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_X] = m_joints[pick][i].Position.X;
										m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_Y] = m_joints[pick][i].Position.Y;
										m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_Z] = m_joints[pick][i].Position.Z;

										//https://social.msdn.microsoft.com/Forums/en-US/3f193efe-3a9c-46f6-ae5d-21231e3ba082/kinect-sdk-20-joint-orientation?forum=kinectv2sdk

										// absolute rotation
										if (_options.rotQuaternion)
										{
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_W] = m_jointOrients[pick][i].Orientation.w;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_X] = m_jointOrients[pick][i].Orientation.x;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Y] = m_jointOrients[pick][i].Orientation.y;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Z] = m_jointOrients[pick][i].Orientation.z;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
										}
										else
										{
											Kinect2Utils::quaternionToEuler(absX, absY, absZ, m_jointOrients[pick][i].Orientation);
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_W] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_X] = absX;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Y] = absY;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Z] = absZ;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
										}

										//TODO Nobody seems to use this, so provide zeros until someone needs it
										//and can tell me which skeleton-hierarchy they want here. -Daniel
										if (_options.rotQuaternion)
										{
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
										}
										else
										{
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
										}

									}
								}

								if (m_skeleton_to_screen_provider) {
									float x, y = 0.0f;
									ColorSpacePoint colorSpacePoint = { 0 };
									for (ssi_size_t i = 0; i <JointType::JointType_Count; i++)
									{
										//map to RGB camera resolution (1920*1080)
										if (SUCCEEDED(m_pCoordinateMapper->MapCameraPointToColorSpace(m_joints[j][i].Position, &colorSpacePoint))){
											x = colorSpacePoint.X;
											y = colorSpacePoint.Y;

											//map from 1920*1080 to custom screen resolution, if set
											if (_options.screenWidth != 0 && _options.screenHeight != 0){
												x *= (float)_options.screenWidth / (float)getRGBImageParams().widthInPixels;
												y *= (float)_options.screenHeight / (float)getRGBImageParams().heightInPixels;
											}
										}

										conf = ssi_cast(float, m_joints[pick][i].TrackingState) / ssi_cast(float, TrackingState_Tracked);
										m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_CONF] = conf;
										m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_X] = x;
										m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_Y] = y;
										m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_Z] = m_joints[pick][i].Position.Z;

										// absolute rotation
										if (_options.rotQuaternion)
										{
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_W] = m_jointOrients[pick][i].Orientation.w;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_X] = m_jointOrients[pick][i].Orientation.x;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Y] = m_jointOrients[pick][i].Orientation.y;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Z] = m_jointOrients[pick][i].Orientation.z;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
										}
										else
										{
											Kinect2Utils::quaternionToEuler(absX, absY, absZ, m_jointOrients[pick][i].Orientation);
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_W] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_X] = absX;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Y] = absY;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Z] = absZ;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
										}

										//TODO Nobody seems to use this, so provide zeros until someone needs it
										//and can tell me which skeleton-hierarchy they want here. -Daniel
										if (_options.rotQuaternion)
										{
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
										}
										else
										{
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = 0;
											m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
										}
									}
								}

								if (m_skeleton_confidence_provider) {
									for (ssi_size_t i = 0; i <JointType::JointType_Count; i++)
									{
										m_skeleton_confidence[j][i] = ssi_cast(float, m_joints[pick][i].TrackingState) / ssi_cast(float, TrackingState_Tracked);
									}
								}
							}
						}
					}
				}

				SafeRelease(m_bodyFrame);
				SafeRelease(m_pBodyFrameReference);
			}
		}
	}

	void MicrosoftKinect2::processFace(){

		for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; i++){
			
			if (m_face_provider || m_face3d_provider || m_head_provider || m_au_provider){

				IHighDefinitionFaceFrame* pHDFaceFrame = NULL;

					m_faces[i].isTracked = false;
					if (m_faces[i].reader){

						if (SUCCEEDED(m_faces[i].reader->AcquireLatestFrame(&pHDFaceFrame))){
							if (SUCCEEDED(pHDFaceFrame->get_IsFaceTracked(&m_faces[i].isTracked) && m_faces[i].isTracked)){
								if (SUCCEEDED(pHDFaceFrame->GetAndRefreshFaceAlignmentResult(m_faces[i].alignment))){

									int correspondingBodyPos = -1;
									for (ssi_size_t b = 0; b < m_n_skeletons; b++){
										if (m_channel_order[b] == i){
											correspondingBodyPos = b;
										}
									}

									if (correspondingBodyPos != -1){ //only do face if a suitable body was found

										if (m_faces[i].alignment != NULL && m_faceModel != NULL){
											if (SUCCEEDED(m_faceModel->CalculateVerticesForAlignment(m_faces[i].alignment, m_faceVertexCount, m_faces[i].vertices))){
										
												if (m_face3d_provider || m_face_provider){
													ColorSpacePoint	colorSpacePoint = { 0 };
													for (ssi_size_t j = 0; j < m_faceVertexCount; j++){

														m_facepoints3d[correspondingBodyPos][j][FACEPOINT3D_VALUE::X] = m_faces[i].vertices[j].X;
														m_facepoints3d[correspondingBodyPos][j][FACEPOINT3D_VALUE::Y] = m_faces[i].vertices[j].Y;
														m_facepoints3d[correspondingBodyPos][j][FACEPOINT3D_VALUE::Z] = m_faces[i].vertices[j].Z;

														if (m_face_provider){
															if (SUCCEEDED(m_pCoordinateMapper->MapCameraPointToColorSpace(m_faces[i].vertices[j], &colorSpacePoint))){
																m_facepoints2d[correspondingBodyPos][j][FACEPOINT_VALUE::X] = colorSpacePoint.X;
																m_facepoints2d[correspondingBodyPos][j][FACEPOINT_VALUE::Y] = colorSpacePoint.Y;
															}
														}
													}
												}
												
											}

											if (m_head_provider){
												if (SUCCEEDED(m_faces[i].alignment->get_FaceOrientation(&m_faces[i].orientation))){

													float foX, foY, foZ;
													// Head orientation coordinate system already rotated
													Kinect2Utils::quaternionToEuler(foX, foY, foZ, m_faces[i].orientation, false);
													m_headpose[correspondingBodyPos][HEADPOSE_ANGLE::PITCH] = foX;
													m_headpose[correspondingBodyPos][HEADPOSE_ANGLE::ROLL] = foY;
													m_headpose[correspondingBodyPos][HEADPOSE_ANGLE::YAW] = foZ;

												}
											}

											if (m_au_provider){
												float* pAnimationUnits = new float[FaceShapeAnimations_Count];

												if (SUCCEEDED(m_faces[i].alignment->GetAnimationUnits(FaceShapeAnimations_Count, pAnimationUnits))){
													for (int j = 0; j < ACTIONUNIT::NUM; j++)
													{
														m_actionunits[correspondingBodyPos][j] = pAnimationUnits[j];
													}
												}

												delete[] pAnimationUnits;
											}

											
										}
									}
								}
							}
						}
					}

				SafeRelease(pHDFaceFrame);
			}
		}
	}


	void MicrosoftKinect2::showSkeleton()
	{
		
		if (_options.showBodyTracking)
		{
			for (ssi_size_t i = 0; i < m_n_skeletons; ++i)
			{
				ColorSpacePoint colorSpacePoint = { 0 };
				int pick = m_channel_order[i];
				if (pick != -1){
					for (int j = 0; j < JointType_Count; j++){

						if (m_joints[pick][j].TrackingState != TrackingState_NotTracked){

							if (SUCCEEDED(m_pCoordinateMapper->MapCameraPointToColorSpace(m_joints[pick][j].Position, &colorSpacePoint))){
								int x = static_cast<int>(colorSpacePoint.X);
								int y = static_cast<int>(colorSpacePoint.Y);
								int c = (m_joints[pick][j].TrackingState == TrackingState_Inferred) ? 100 : 255;
								int r, b, g;

								r = (i == 0 || i == 3 || i == 4          ) ? c : 0;
								g = (i == 1 || i == 3 ||           i == 5) ? c : 0;
								b = (i == 2			  || i == 4 || i == 5) ? c : 0; 
								
								Kinect2Utils::paint_point(ssi_pcast(BYTE, m_bgraBuffer), getRGBImageParams(), x, y, 6, r, g, b);

								int p = Kinect2Utils::getParentJoint(j);
								if (p >= 0){

									if (SUCCEEDED(m_pCoordinateMapper->MapCameraPointToColorSpace(m_joints[pick][p].Position, &colorSpacePoint))){

										int xp = static_cast<int>(colorSpacePoint.X);
										int yp = static_cast<int>(colorSpacePoint.Y);
										Kinect2Utils::paint_line(ssi_pcast(BYTE, m_bgraBuffer), getRGBImageParams(), x, xp, y, yp, 3, r, g, b);
									}
									
								}
							}
						}
					}
				}
			}
		}
	}

	void MicrosoftKinect2::showFace()
	{

		if (_options.showFaceTracking)
		{
			for (ssi_size_t i = 0; i < m_n_skeletons; ++i)
			{
				int pick = i; // m_channel_order[i];
				if (pick != -1){
					for (unsigned int j = 0; j < m_faceVertexCount; j++){

						if (m_facepoints2d[pick][j][FACEPOINT_VALUE::X] > 0 && m_facepoints2d[pick][j][FACEPOINT_VALUE::Y] > 0){

							int x = static_cast<int>(m_facepoints2d[pick][j][FACEPOINT_VALUE::X]);
							int y = static_cast<int>(m_facepoints2d[pick][j][FACEPOINT_VALUE::Y]);
							int c = 180;
							int r, b, g;

							r = (i == 0 || i == 3 || i == 4          ) ? c : 0;
							g = (i == 1 || i == 3 ||           i == 5) ? c : 0;
							b = (i == 2 ||           i == 4 || i == 5) ? c : 0;

							if (x > 0 && y > 0 && x <= getRGBImageParams().widthInPixels && getRGBImageParams().heightInPixels){
								Kinect2Utils::paint_point(ssi_pcast(BYTE, m_bgraBuffer), getRGBImageParams(), x, y, 1, r, g, b);
							}

						}
					}
				}
			}
		}

	}
	
	void MicrosoftKinect2::processProvide () {
				
		if (m_rgb_provider) {
			m_rgb_provider->provide(ssi_pcast(ssi_byte_t, m_bgraBuffer), 1);
		}

		if (m_depthr_provider) {
			m_depthr_provider->provide(ssi_pcast(ssi_byte_t, m_depthrBuffer),1);
		}

		if (m_depth_provider) {
			Kinect2Utils::depthRaw2Image(ssi_pcast(ssi_byte_t, m_depthrBuffer), m_depthImage, getDepthImageParams(), getDepthRawParams());
			m_depth_provider->provide(m_depthImage, 1);
		}

		if (m_irr_provider) {
			m_irr_provider->provide(ssi_pcast(ssi_byte_t, m_irrBuffer), 1);
		}

		if (m_ir_provider) {
			Kinect2Utils::infraredRaw2Image(ssi_pcast(ssi_byte_t, m_irrBuffer), m_irImage, getInfraredImageParams(), getInfraredRawParams());
			m_ir_provider->provide(m_irImage, 1);
		}
			
		if (m_skeleton_provider) {
			m_skeleton_provider->provide(ssi_pcast(ssi_byte_t, m_skeleton), 1);
		}

		if (m_skeleton_to_screen_provider) {
			m_skeleton_to_screen_provider->provide(ssi_pcast(ssi_byte_t, m_skeleton_to_screen), 1);
		}

		if (m_skeleton_confidence_provider) {
			m_skeleton_confidence_provider->provide(ssi_pcast(ssi_byte_t, m_skeleton_confidence), 1);
		}
			
		if (m_face_provider) {
			m_face_provider->provide(ssi_pcast(ssi_byte_t, m_facepoints2d),1);
		}

		if (m_face3d_provider) {
			m_face3d_provider->provide(ssi_pcast(ssi_byte_t, m_facepoints3d),1);
		}
		
		if (m_head_provider) {
			m_head_provider->provide(ssi_pcast(ssi_byte_t, m_headpose), 1);
		}

		if (m_au_provider) {
			m_au_provider->provide(ssi_pcast(ssi_byte_t, m_actionunits),1);
		}

		if (m_hand_provider) {
			m_hand_provider->provide(ssi_pcast(ssi_byte_t, m_handpose), 1);
		}
		
	}

	void MicrosoftKinect2::process()
	{
		if (!m_KinectSensorPresent) {
			return;
		}
		
		processMultiSourceFrame ();

		processBody ();
		processFace ();
		processImages ();
		
		SafeRelease(m_pMultiSourceFrame);
		
		processProvide ();
	}

	bool MicrosoftKinect2::disconnect () {

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "try to disconnect sensor...");

		if (m_skeleton){
			delete[] m_skeleton; m_skeleton = 0;
		}

		if (m_skeleton_to_screen){
			delete[] m_skeleton_to_screen; m_skeleton_to_screen = 0;
		}

		if (m_skeleton_confidence){
			delete[] m_skeleton_confidence; m_skeleton_confidence = 0;
		}
		
		if (m_facepoints2d){
			delete[] m_facepoints2d; m_facepoints2d = 0;
		}

		if (m_facepoints3d){
			delete[] m_facepoints3d; m_facepoints3d = 0;
		}

		if (m_actionunits){
			delete[] m_actionunits; m_actionunits = 0;
		}

		if (m_channel_order){
			delete[] m_channel_order; m_channel_order = 0;
		}

		if (m_headpose){
			delete[] m_headpose; m_headpose = 0;
		}
		
		if (m_handpose){
			delete[] m_handpose; m_handpose = 0;
		}

		if (m_skeleton_confidence_provider){
			delete[] m_skeleton_confidence; m_skeleton_confidence = 0;
		}

		if(m_bgraBuffer)
		{
			delete[] m_bgraBuffer;
			m_bgraBuffer = 0;
		}

		if(m_depthrBuffer) 
		{
			delete[] m_depthrBuffer;
			m_depthrBuffer = 0;
		}

		if (m_depthImage) {
			delete[] m_depthImage;
			m_depthImage = 0;
		}

		if (m_irrBuffer)
		{
			delete[] m_irrBuffer;
			m_irrBuffer = 0;
		}

		if (m_irImage) {
			delete[] m_irImage;
			m_irImage = 0;
		}

		if (m_faces){
			for (int i = 0; i < SSI_MICROSOFTKINECT2_SKELETON_MAX_USER; i++){
				if (m_faces[i].vertices){
					delete m_faces[i].vertices;
					m_faces[i].vertices = 0;
				}
			}
		}

		if (m_audio_provider)
		{
			if (m_audio_capturer)
			{
				if (m_audio_is_capturing)
				{
					m_audio_capturer->Stop();
					m_audio_is_capturing = false;
				}
				delete m_audio_capturer;
			}
			if (m_audio_device)
			{
				SafeRelease(m_audio_device);
			}
		}
		
		delete m_timer; m_timer = 0;
		m_kinectSensor->Close();
		m_kinectSensor->Release();

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

		return true;
	}


}

