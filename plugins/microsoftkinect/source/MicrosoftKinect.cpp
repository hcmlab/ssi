// MicrosoftKinect.cpp
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

#include "MicrosoftKinect.h"
#include "KinectUtils.h"
#include "SSI_SkeletonCons.h"

#include <FaceTrackLib.h>
#include <NuiApi.h>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


namespace ssi {

	ssi_char_t *MicrosoftKinect::ssi_log_name = "mickinect_";
	Vector4 tmpdata[NUI_SKELETON_COUNT][MicrosoftKinect::SKELETON_JOINT::NUM];

	MicrosoftKinect::MicrosoftKinect (const ssi_char_t *file) 
		: m_face_provider (0),
		m_face3d_provider (0),
		m_au_provider (0),
		m_shape_provider (0),
		m_head_provider (0),
		m_depth_provider (0),
		m_depthr_provider (0),
		m_rgb_provider (0),
		m_skeleton_provider (0),
		m_skeleton_to_screen_provider (0),
		m_skeleton_confidence_provider(0),
		m_channel_order (0),
		m_n_skeletons (0),
		m_skeleton (0),
		m_skeleton_to_screen (0),
		m_timer (0),
		_file (0),
		ssi_log_level (SSI_LOG_LEVEL_DEFAULT) {
			
		m_bNuiInitialized = false;
		m_pFaceTracker = 0;
		m_pFTResult = NULL;
		m_colorImage = NULL;
		m_depthImage = NULL;
		m_depthRImage = NULL;
		m_LastFaceTrackSucceeded = false;
		m_colorRes = NUI_IMAGE_RESOLUTION_INVALID;   
		
		m_ViewOffset.x = 0;
		m_ViewOffset.y = 0;
		m_pDepthStreamHandle = NULL;
		m_pVideoStreamHandle = NULL;
		m_pNuiSensor = NULL;
		m_ZoomFactor = 1.0f;
			
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy (file);
		}
	}

	MicrosoftKinect::~MicrosoftKinect () {

		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	bool MicrosoftKinect::setProvider (const ssi_char_t *name, IProvider *provider) {

		if (strcmp (name, SSI_MICROSOFTKINECT_FACEPOINT_PROVIDER_NAME) == 0) {
			setFacePointProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_FACEPOINT3D_PROVIDER_NAME) == 0) {
			setFacePoint3DProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_ACTIONUNIT_PROVIDER_NAME) == 0) {
			setActionUnitProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_SHAPEUNIT_PROVIDER_NAME) == 0) {
			setShapeUnitProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_HEADPOSE_PROVIDER_NAME) == 0) {
			setHeadPoseProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_RGBIMAGE_PROVIDER_NAME) == 0) {
			setRGBImageProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_DEPTHRAW_PROVIDER_NAME) == 0) {
			setDepthRawProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_DEPTHIMAGE_PROVIDER_NAME) == 0) {
			setDepthImageProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_SKELETON_PROVIDER_NAME) == 0) {
			setSkeletonProvider (provider);
			return true;
		} else if (strcmp (name, SSI_MICROSOFTKINECT_SKELETON2SCREEN_PROVIDER_NAME) == 0) {
			setSkeleton2ScreenProvider (provider);
			return true;
		} else if (strcmp(name, SSI_MICROSOFTKINECT_SKELETONCONFIDENCE_PROVIDER_NAME) == 0) {
			setSkeletonConfidenceProvider(provider);
			return true;
		}

		ssi_wrn ("unkown provider name '%s'", name);

		return false;
	}

	void MicrosoftKinect::setRGBImageProvider (IProvider *rgb_provider) {

		if (m_rgb_provider) {
			ssi_wrn ("rgb image provider already set");
		}

		m_rgb_provider = rgb_provider;
	
		if (m_rgb_provider) {

			ssi_video_params_t params = getRGBImageParams ();
			
			m_rgb_provider->setMetaData (sizeof (params), &params);
			m_rgb_channel.stream.sr = _options.sr;

			if(_options.rgbres == RESOLUTION::RES_1280x960)
			{
				m_rgb_channel.stream.byte = 1280*960*4;
			}
			else
			{
				m_rgb_channel.stream.byte = 640*480*4;
			}
			
			m_rgb_provider->init (&m_rgb_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "rgb provider set");
		}
	}

	void MicrosoftKinect::setDepthRawProvider (IProvider *depthr_provider) {

		if (m_depthr_provider) {
			ssi_wrn ("depth raw provider already set");
		}

		m_depthr_provider = depthr_provider;
		if (m_depthr_provider) {
			ssi_video_params_t params = getDepthRawParams ();
			m_depthr_provider->setMetaData (sizeof (params), &params);
			m_depthr_channel.stream.sr = _options.sr;
			if (_options.depthres == RESOLUTION::RES_80x60)
			{
				m_depthr_channel.stream.byte = 80*60*2;
			}
			else if(_options.depthres == RESOLUTION::RES_320x240)
			{
				m_depthr_channel.stream.byte = 320*240*2;
			}
			else
			{
				m_depthr_channel.stream.byte = 640*480*2;
			}

			m_depthr_provider->init (&m_depthr_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "depth raw provider set");
		}
	}


	void MicrosoftKinect::setDepthImageProvider (IProvider *depth_provider) {

		if (m_depth_provider) {
			ssi_wrn ("depth image provider already set");
		}

		m_depth_provider = depth_provider;
		if (m_depth_provider) {
			ssi_video_params_t params = getDepthImageParams ();
			m_depth_provider->setMetaData (sizeof (params), &params);
			m_depth_channel.stream.sr = _options.sr;
			if (_options.depthres == RESOLUTION::RES_80x60)
			{
				m_depth_channel.stream.byte = 80*60*1;
			}
			else if (_options.depthres == RESOLUTION::RES_320x240)
			{
				m_depth_channel.stream.byte = 320*240*1;
			}
			else
			{
				m_depth_channel.stream.byte = 640*480*1;
			}

			m_depth_provider->init (&m_depth_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "depth provider set");
		}
	}

	void MicrosoftKinect::setSkeletonProvider (IProvider *skeleton_provider) {

		if (m_skeleton_provider) {
			ssi_wrn ("skeleton image provider already set");
		}

		m_skeleton_provider = skeleton_provider;
		if (m_skeleton_provider) {			
			m_track_nearest_person = _options.trackNearestPerson;
			m_n_skeletons = m_track_nearest_person ? 1 : SSI_MICROSOFTKINECT_SKELETON_MAX_USER;
			SSI_SKELETON_META m = ssi_skeleton_meta(SSI_SKELETON_TYPE::MICROSOFT_KINECT, m_n_skeletons);
			m_skeleton_provider->setMetaData (sizeof (SSI_SKELETON_META), &m);
			m_skeleton_channel.stream.dim = m_n_skeletons * SKELETON_JOINT::NUM * SKELETON_JOINT_VALUE::NUM;
			m_skeleton_channel.stream.sr = _options.sr;
			m_skeleton_provider->init (&m_skeleton_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "skeleton provider set");
		}
	}

	void MicrosoftKinect::setSkeleton2ScreenProvider (IProvider *skeleton_to_screen_provider) {

		if (m_skeleton_to_screen_provider) {
			ssi_wrn ("skeleton2screen image provider already set");
		}

		m_skeleton_to_screen_provider = skeleton_to_screen_provider;
		if (m_skeleton_to_screen_provider) {
			m_track_nearest_person = _options.trackNearestPerson;
			m_n_skeletons = m_track_nearest_person ? 1 : SSI_MICROSOFTKINECT_SKELETON_MAX_USER;
			SSI_SKELETON_META m = ssi_skeleton_meta(SSI_SKELETON_TYPE::MICROSOFT_KINECT, m_n_skeletons);
			m_skeleton_to_screen_provider->setMetaData (sizeof (SSI_SKELETON_META), &m);
			m_skeleton_to_screen_channel.stream.dim = m_n_skeletons * SKELETON_JOINT::NUM * SKELETON_JOINT_VALUE::NUM;
			m_skeleton_to_screen_channel.stream.sr = _options.sr;
			m_skeleton_to_screen_provider->init (&m_skeleton_to_screen_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "skeleton2screen provider set");
		}
	}

	void MicrosoftKinect::setSkeletonConfidenceProvider(IProvider *skeleton_confidence_provider) {

		if (m_skeleton_confidence_provider) {
			ssi_wrn("skeletonconf provider already set");
		}

		m_skeleton_confidence_provider = skeleton_confidence_provider;
		if (m_skeleton_confidence_provider) {						
//			SSI_SKELETON_META m = ssi_skeleton_meta(SSI_SKELETON_TYPE::MICROSOFT_KINECT, m_n_skeletons);
//			m_skeleton_confidence_provider->setMetaData(sizeof(SSI_SKELETON_META), &m);
			m_skeleton_confidence_channel.stream.dim = m_n_skeletons * SKELETON_JOINT::NUM;
			m_skeleton_confidence_channel.stream.sr = _options.sr;
			m_skeleton_confidence_provider->init(&m_skeleton_confidence_channel);
			ssi_msg(SSI_LOG_LEVEL_DETAIL, "skeletonconf provider set");
		}
	}

	void MicrosoftKinect::setFacePointProvider (IProvider *face_provider) {

		if (m_face_provider) {
			ssi_wrn ("face point provider already set");
		}

		m_face_provider = face_provider;
		if (m_face_provider) {
			SSI_FACE_META m = ssi_face_meta(SSI_FACE_TYPE::MICROSOFT_KINECT, m_n_skeletons);
			m_face_provider->setMetaData (sizeof (SSI_FACE_META), &m);
			m_face_channel.stream.sr = _options.sr;
			m_face_provider->init (&m_face_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "face provider set");
		}
	}

	void MicrosoftKinect::setFacePoint3DProvider (IProvider *face3d_provider) {

		if (m_face3d_provider) {
			ssi_wrn ("face point provider already set");
		}

		m_face3d_provider = face3d_provider;
		if (m_face3d_provider) {
			SSI_FACE_META m = ssi_face_meta(SSI_FACE_TYPE::MICROSOFT_KINECT, m_n_skeletons);			
			m_face3d_provider->setMetaData(sizeof(SSI_FACE_META), &m);
			m_face3d_channel.stream.sr = _options.sr;
			m_face3d_provider->init (&m_face3d_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "face3d provider set");
		}
	}

	void MicrosoftKinect::setActionUnitProvider (IProvider *au_provider) {

		if (m_au_provider) {
			ssi_wrn ("action unit provider already set");
		}

		m_au_provider = au_provider;
		if (m_au_provider) {
			m_au_channel.stream.sr = _options.sr;
			m_au_provider->init (&m_au_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "action unit provider set");
		}
	}

	void MicrosoftKinect::setShapeUnitProvider (IProvider *shape_provider) {

		if (m_shape_provider) {
			ssi_wrn ("shape unit provider already set");
		}

		m_shape_provider = shape_provider;
		if (m_shape_provider) {
			m_shape_channel.stream.sr = _options.sr;
			m_shape_provider->init (&m_shape_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "shape provider set");
		}
	}

	void MicrosoftKinect::setHeadPoseProvider (IProvider *head_provider) {

		if (m_head_provider) {
			ssi_wrn ("head pose provider already set");
		}

		m_head_provider = head_provider;
		if (m_head_provider) {
			m_head_channel.stream.sr = _options.sr;
			m_head_provider->init (&m_head_channel);
			ssi_msg (SSI_LOG_LEVEL_DETAIL, "head pose provider set");
		}
	}

	bool MicrosoftKinect::connect () {

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "try to connect sensor...");
		FT_CAMERA_CONFIG* pDepthConfig = NULL;

		NUI_IMAGE_TYPE			depthType =	NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX;
		NUI_IMAGE_RESOLUTION	depthRes =	NUI_IMAGE_RESOLUTION_640x480;				
		NUI_IMAGE_TYPE			colorType = NUI_IMAGE_TYPE_COLOR;
		NUI_IMAGE_RESOLUTION	colorRes =	NUI_IMAGE_RESOLUTION_640x480;
		
		switch (_options.rgbres) {
			case RESOLUTION::RES_640x480:
				colorRes = NUI_IMAGE_RESOLUTION_640x480;
				break;
			case RESOLUTION::RES_1280x960:
				colorRes = NUI_IMAGE_RESOLUTION_1280x960;
				break;
			default:
				ssi_wrn ("invalid rgb resolution, setting default");
				depthRes = NUI_IMAGE_RESOLUTION_640x480;
				break;
		}
		m_colorRes = colorRes;

		switch (_options.depthres) {
			case RESOLUTION::RES_80x60:
				depthRes = NUI_IMAGE_RESOLUTION_80x60;
				break;				 
			case RESOLUTION::RES_320x240:
				depthRes = NUI_IMAGE_RESOLUTION_320x240;
				break;
			case RESOLUTION::RES_640x480:
				depthRes = NUI_IMAGE_RESOLUTION_640x480;
				break;
			default:
				ssi_wrn ("invalid depth resolution, setting default");
				depthRes = NUI_IMAGE_RESOLUTION_640x480;
				break;
		}

		for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
			m_SkeletonTracked[i] = false;

		//init temp data for feature calculation
		for(int i = 0; i < MicrosoftKinect::SKELETON_JOINT::NUM; i++)
		{
			for(int j = 0; j < NUI_SKELETON_COUNT; j++)
			{
				tmpdata[j][i].w = 0.0f; tmpdata[j][i].x = 0.0f; tmpdata[j][i].y = 0.0f; tmpdata[j][i].z = 0.0f;
			}
		}

		// close any open kinect connections
		NuiShutdown();

		/*
		 * Connect to the kinect sensor
		 */
		int numDevices = 0;
		HRESULT hr = NuiGetSensorCount(&numDevices);

		// Create the sensor so we can check status, if we can't create it, move on to the next
		if (_options.deviceIndex >= (ssi_size_t)numDevices) ssi_err("Invalid Device Index %u", _options.deviceIndex);
		HRESULT status ;
		INuiSensor * pNuiSensor = NULL;
		hr = NuiCreateSensorByIndex(_options.deviceIndex, &pNuiSensor);

		if (SUCCEEDED(hr))
		{
			status = pNuiSensor ? pNuiSensor->NuiStatus() : E_NUI_NOTCONNECTED;
			if (status == E_NUI_NOTCONNECTED)
				pNuiSensor->Release();

			m_pNuiSensor = pNuiSensor;     
			pNuiSensor->Release();
		}
 
		ssi_print("Kinect Instance: %d\t", m_pNuiSensor->NuiInstanceIndex());
		ssi_print("Connection ID: %ls\n", m_pNuiSensor->NuiDeviceConnectionId());
   
		// Get the status of the sensor, and if connected, then we can initialize it  
		if (NULL != m_pNuiSensor)
		{
			// Initialize the Kinect and specify that we'll be using skeleton
			hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX|NUI_INITIALIZE_FLAG_USES_COLOR|NUI_INITIALIZE_FLAG_USES_SKELETON);
			if (SUCCEEDED(hr))
			{
				// Open a skeleton stream to receive skeleton data
				hr = m_pNuiSensor->NuiSkeletonTrackingEnable(0, 0); 
			}
		}

		//do not support NUI_IMAGE_TYPE_COLOR_RAW_YUV for now
		if(colorType != NUI_IMAGE_TYPE_COLOR && colorType != NUI_IMAGE_TYPE_COLOR_YUV
			|| depthType != NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX && depthType != NUI_IMAGE_TYPE_DEPTH)
		{
			ssi_wrn("Could not initialize the Kinect sensor (NUI_IMAGE_TYPE_COLOR_RAW_YUV color not supported)");
			m_KinectSensorPresent = FALSE;
			return false;
		}
	
		DWORD dwNuiInitDepthFlag = (depthType == NUI_IMAGE_TYPE_DEPTH)? NUI_INITIALIZE_FLAG_USES_DEPTH : NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;

		hr = NuiInitialize(dwNuiInitDepthFlag | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR);
		if (FAILED(hr))
		{
			ssi_wrn("Could not initialize the Kinect sensor (NuiInitialize failed)");
			m_KinectSensorPresent = FALSE;
			return false;
		}
		m_bNuiInitialized = true;


		DWORD dwSkeletonFlags = NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE;
		
		if (_options.seatedSkeletonMode)
			dwSkeletonFlags |= NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT;

		hr = NuiSkeletonTrackingEnable( 0, dwSkeletonFlags );
		if (FAILED(hr))
		{
			ssi_wrn("Could not initialize the Kinect sensor (Cannot initialize skeleton tracking)");
			m_KinectSensorPresent = FALSE;
			return false;
		}

		/*
		 * Init RGB stream
		 */
		hr = NuiImageStreamOpen(colorType, colorRes, 0, 2, 0, &m_pVideoStreamHandle );
		if (FAILED(hr))
		{
			ssi_wrn("Could not initialize the Kinect sensor (Cannot open RGB stream)");
			m_KinectSensorPresent = FALSE;
			return false;
		}
		
		/*
		 * Init Depth stream
		 */
		hr = NuiImageStreamOpen(depthType, depthRes, (_options.nearTrackingMode)? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0, 2, 0, &m_pDepthStreamHandle );
		if (FAILED(hr))
		{
			if(_options.nearTrackingMode)
			{
				hr = NuiImageStreamOpen(depthType, depthRes, 0, 2, 0, &m_pDepthStreamHandle );
				if(FAILED(hr))
				{
					ssi_wrn("could not initialize the Kinect sensor (Cannot open Depth stream)");
					m_KinectSensorPresent = FALSE;
					return false;
				}
			}
		}

		// connection to kinect succeeded
		m_KinectSensorPresent = TRUE;
			
		DWORD width = 0, height = 0;
		
		FT_CAMERA_CONFIG videoConfig;
		NuiImageResolutionToSize(colorRes, width, height);
		KinectUtils::GetVideoConfiguration(&videoConfig, width, height);
			
		FT_CAMERA_CONFIG depthConfig;
		NuiImageResolutionToSize(depthRes, width, height);
		KinectUtils::GetDepthConfiguration(&depthConfig, width, height);

		pDepthConfig = &depthConfig;
		
		// Try to start the face tracker.
		if (m_au_provider || m_face_provider || m_face3d_provider  || m_head_provider) {

			m_pFaceTracker = FTCreateFaceTracker(0);
			if (!m_pFaceTracker)
			{
				ssi_wrn("could not create the face tracker (Face Tracker Initialization Error)");
				return false;
			}

			hr = m_pFaceTracker->Initialize(&videoConfig, pDepthConfig, NULL, NULL); 
			if (FAILED(hr))
			{
				WCHAR path[512];
				GetCurrentDirectoryW(ARRAYSIZE(path), path);
				ssi_wrn ("could not initialize face tracker (hr=0x%x)", hr);
				return false;
			}

			hr = m_pFaceTracker->CreateFTResult(&m_pFTResult);
			if (FAILED(hr) || !m_pFTResult)
			{
				ssi_wrn("could not initialize the face tracker result (Face Tracker Initialization Error)");
				return false;
			}
		}

		// Initialize the RGB image.
		m_colorImage = FTCreateImage();
		if (!m_colorImage || FAILED(hr = m_colorImage->Allocate(videoConfig.Width, videoConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8)))
		{
			return false;
		}

		// Initialize the raw depth image.
		if (pDepthConfig)
		{
			m_depthRImage = FTCreateImage();
			if (!m_depthRImage || FAILED(hr = m_depthRImage->Allocate(depthConfig.Width, depthConfig.Height, FTIMAGEFORMAT_UINT16_D13P3)))
			{
				return false;
			}
		}

		if (m_depth_provider) {
			ssi_video_params_t params = getDepthImageParams ();
			m_depthImage = new ssi_byte_t[ssi_video_size (params)];
		}
		
		initMemory();

		m_LastFaceTrackSucceeded = false;

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "connected");

		// set thread name
		Thread::setName (getName ());

		return true;
	}

	void MicrosoftKinect::run () {
		
		if (!m_timer) {
			m_timer = new Timer (1/_options.sr);
		}

		process();
		m_timer->wait ();
	}

	void MicrosoftKinect::initMemory () {

		if (m_skeleton_provider) 
		{			
			m_skeleton = new MicrosoftKinect::SKELETON[m_n_skeletons];
			for( ssi_size_t k = 0; k < m_n_skeletons; k++)
			{
				for (ssi_size_t i = 0; i<SKELETON_JOINT::NUM;i++)
				{
					for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
					{
						m_skeleton[k][i][j] = SSI_MICROSOFTKINECT_INVALID_SKELETON_JOINT_VALUE;
					}
				}
			}
		}

		if (m_skeleton_to_screen_provider) 
		{							
			m_skeleton_to_screen = new MicrosoftKinect::SKELETON[m_n_skeletons];
			for( ssi_size_t k = 0; k < m_n_skeletons; k++)
			{
				for (ssi_size_t i = 0; i<SKELETON_JOINT::NUM;i++)
				{
					for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
					{
						m_skeleton_to_screen[k][i][j] = SSI_MICROSOFTKINECT_INVALID_SKELETON_JOINT_VALUE;
					}
				}
			}
		}

		if (m_skeleton_confidence_provider)
		{
			m_skeleton_confidence = new MicrosoftKinect::SKELETONCONF[m_n_skeletons];
			for (ssi_size_t k = 0; k < m_n_skeletons; k++)
			{
				for (ssi_size_t i = 0; i<SKELETON_JOINT::NUM; i++)
				{
					m_skeleton_confidence[k][i] = SSI_MICROSOFTKINECT_INVALID_SKELETON_CONFIDENCE_VALUE;
				}
			}
		}

		if(m_face_provider){
			for (ssi_size_t i = 0; i<FACEPOINT::NUM;i++)
			{
				m_facepoints[i][FACEPOINT_VALUE::X] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT_VALUE;
				m_facepoints[i][FACEPOINT_VALUE::Y] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT_VALUE;
			}
		}

		if(m_face3d_provider){
			for (ssi_size_t i = 0; i<FACEPOINT3D::NUM;i++)
			{
				m_facepoints3d[i][FACEPOINT3D_VALUE::X] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT3D_VALUE;
				m_facepoints3d[i][FACEPOINT3D_VALUE::Y] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT3D_VALUE;
				m_facepoints3d[i][FACEPOINT3D_VALUE::Z] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT3D_VALUE;
			}
		}
	
		if(m_au_provider){
			for (ssi_size_t i = 0; i<ACTIONUNIT::NUM;i++)
			{
				m_actionunits[i] = SSI_MICROSOFTKINECT_INVALID_ACTIONUNIT_VALUE;
			}
		}

		if(m_shape_provider){
			for (ssi_size_t i = 0; i<SHAPEUNIT::NUM;i++)
			{
				m_shapeunits[i] = SSI_MICROSOFTKINECT_INVALID_SHAPEUNIT_VALUE;
			}
		}

		if(m_head_provider){
			for (ssi_size_t i = 0; i<HEADPOSE_ANGLE::NUM;i++)
			{
				m_headpose[i] = SSI_MICROSOFTKINECT_INVALID_HEADPOSE_VALUE;
			}
		}
	}
	
	void MicrosoftKinect::clearData (ssi_size_t nui_id) {

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
			for (ssi_size_t i = 0; i<SKELETON_JOINT::NUM;i++)
			{
				for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
				{
					m_skeleton[ssi_id][i][j] = SSI_MICROSOFTKINECT_INVALID_SKELETON_JOINT_VALUE;
				}
			}
		}

		if (m_skeleton_to_screen_provider) 
		{	
			for (ssi_size_t i = 0; i<SKELETON_JOINT::NUM;i++)
			{
				for(ssi_size_t j=0; j< SKELETON_JOINT_VALUE::NUM; ++j) 
				{
					m_skeleton_to_screen[ssi_id][i][j] = SSI_MICROSOFTKINECT_INVALID_SKELETON_JOINT_VALUE;
				}
			}
		}

		if (m_skeleton_confidence_provider)
		{
			for (ssi_size_t i = 0; i<SKELETON_JOINT::NUM; i++)
			{
				m_skeleton_confidence[ssi_id][i] = SSI_MICROSOFTKINECT_INVALID_SKELETON_CONFIDENCE_VALUE;
			}
		}

		// if this is the first skeleton, we also need to clear the head data
		if(ssi_id == 0)
		{
			if(m_face_provider){
				for (ssi_size_t i = 0; i<FACEPOINT::NUM;i++)
				{
					m_facepoints[i][FACEPOINT_VALUE::X] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT_VALUE;
					m_facepoints[i][FACEPOINT_VALUE::Y] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT_VALUE;
				}
			}

			if(m_face3d_provider){
				for (ssi_size_t i = 0; i<FACEPOINT3D::NUM;i++)
				{
					m_facepoints3d[i][FACEPOINT3D_VALUE::X] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT3D_VALUE;
					m_facepoints3d[i][FACEPOINT3D_VALUE::Y] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT3D_VALUE;
					m_facepoints3d[i][FACEPOINT3D_VALUE::Z] = SSI_MICROSOFTKINECT_INVALID_FACEPOINT3D_VALUE;
				}
			}
	
			if(m_au_provider){
				for (ssi_size_t i = 0; i<ACTIONUNIT::NUM;i++)
				{
					m_actionunits[i] = SSI_MICROSOFTKINECT_INVALID_ACTIONUNIT_VALUE;
				}
			}

			if(m_shape_provider){
				for (ssi_size_t i = 0; i<SHAPEUNIT::NUM;i++)
				{
					m_shapeunits[i] = SSI_MICROSOFTKINECT_INVALID_SHAPEUNIT_VALUE;
				}
			}

			if(m_head_provider){
				for (ssi_size_t i = 0; i<HEADPOSE_ANGLE::NUM;i++)
				{
					m_headpose[i] = SSI_MICROSOFTKINECT_INVALID_HEADPOSE_VALUE;
				}
			}
		}
	}

	void MicrosoftKinect::processSkeleton () {

		if (m_skeleton_provider || m_skeleton_to_screen_provider || m_skeleton_confidence_provider) 
		{
			// get skeleton from kinect
			NUI_SKELETON_FRAME skeletonFrame;
			HRESULT hr = NuiSkeletonGetNextFrame(0, &skeletonFrame);
			if(hr == S_OK)
			{
				for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
				{
					if( skeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
						NUI_SKELETON_POSITION_TRACKED == skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] &&
						NUI_SKELETON_POSITION_TRACKED == skeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER])
					{
						m_SkeletonTracked[i] = true;			
					}
					else
					{
						if(m_SkeletonTracked[i]) // if this skeleton was tracked before, we clear the old data
							clearData(i);

						m_SkeletonTracked[i] = false;
					}
				}

				if (!m_channel_order) {
					m_channel_order = new int[m_n_skeletons];				
				}
				for (ssi_size_t i = 0; i < m_n_skeletons; i++) {
					m_channel_order[i] = -1;
				}

				if(m_track_nearest_person)
				{					
					float pick = FLT_MAX;
					for(ssi_size_t j=0; j < NUI_SKELETON_COUNT; j++)
					{			
						if(m_SkeletonTracked[j])
						{
							float value = skeletonFrame.SkeletonData[j].SkeletonPositions[SKELETON_JOINT::HEAD].z;	
							if(value > 0 && value < pick) 
							{
								pick = value;
								m_channel_order[0] = j;
							}
						}
					}
				} else {
					int count = 0;
					for(ssi_size_t j=0; j < NUI_SKELETON_COUNT; j++)
					{			
						if(m_SkeletonTracked[j])
						{
							m_channel_order[count++] = j;
						}
					}
					if (count > 1) {
						if (skeletonFrame.SkeletonData[m_channel_order[0]].SkeletonPositions[SKELETON_JOINT::HEAD].x > skeletonFrame.SkeletonData[m_channel_order[1]].SkeletonPositions[SKELETON_JOINT::HEAD].x) {
							int tmp = m_channel_order[0];
							m_channel_order[0] = m_channel_order[1];
							m_channel_order[1] = tmp;
						}
					}
				}

				for(ssi_size_t j=0; j < m_n_skeletons; j++)
				{
					if (m_channel_order[j] == -1) {
						continue;
					}
					int pick = m_channel_order[j];

					const NUI_SKELETON_DATA & skeleton = skeletonFrame.SkeletonData[pick];
					NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
					NuiSkeletonCalculateBoneOrientations(&skeleton, boneOrientations);

					float absX, absY, absZ, relX, relY, relZ, conf;

					if (m_skeleton_provider) {
						for (ssi_size_t i=0; i < SKELETON_JOINT::NUM; i++)
						{
							// position
							conf = ssi_cast (float, skeletonFrame.SkeletonData[pick].eSkeletonPositionTrackingState[i]) / ssi_cast (float, NUI_SKELETON_POSITION_TRACKED);
							m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_CONF] = conf;
							m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_X] = skeletonFrame.SkeletonData[pick].SkeletonPositions[i].x;
							m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_Y] = skeletonFrame.SkeletonData[pick].SkeletonPositions[i].y;
							m_skeleton[j][i][SKELETON_JOINT_VALUE::POS_Z] = skeletonFrame.SkeletonData[pick].SkeletonPositions[i].z;
														
							// absolute rotation
							if(_options.rotQuaternion)
							{
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_W] = boneOrientations[i].absoluteRotation.rotationQuaternion.w;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_X] = boneOrientations[i].absoluteRotation.rotationQuaternion.x;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Y] = boneOrientations[i].absoluteRotation.rotationQuaternion.y;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Z] = boneOrientations[i].absoluteRotation.rotationQuaternion.z;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
							}
							else
							{
								KinectUtils::rotMatToRotation(absX, absY, absZ, boneOrientations[i].absoluteRotation.rotationMatrix);
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_W] = 0;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_X] = absX;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Y] = absY;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_Z] = absZ;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
							}
							
							//relative rotation
							if(_options.rotQuaternion)
							{
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.w;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.x;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.y;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.z;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
							}
							else
							{
								KinectUtils::rotMatToRotation(relX, relY, relZ, boneOrientations[i].hierarchicalRotation.rotationMatrix);
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = 0;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = relX;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = relY;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = relZ;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
							}


						}
					}

					if (m_skeleton_to_screen_provider) {
						float x, y;
						for (ssi_size_t i=0; i < SKELETON_JOINT::NUM; i++)
						{
							DWORD width = 0, height = 0;
							if (_options.screenWidth != 0 && _options.screenHeight != 0) {
								width = _options.screenWidth;
								height = _options.screenHeight;
							} else
								NuiImageResolutionToSize((NUI_IMAGE_RESOLUTION)m_colorRes, width, height);

							conf = ssi_cast (float, skeletonFrame.SkeletonData[pick].eSkeletonPositionTrackingState[i]) / ssi_cast (float, NUI_SKELETON_POSITION_TRACKED);
							m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_CONF] = conf;
							x = ssi_cast(float, KinectUtils::SkeletonToScreen(skeletonFrame.SkeletonData[pick].SkeletonPositions[i], width, height, 320, 240).x);
							m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_X] = _options.screenScale ? x / width : x;
							y = ssi_cast(float, KinectUtils::SkeletonToScreen(skeletonFrame.SkeletonData[pick].SkeletonPositions[i], width, height, 320, 240).y);
							m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_Y] = _options.screenScale ? y / height : y;
							m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::POS_Z] = skeletonFrame.SkeletonData[pick].SkeletonPositions[i].z;

							// absolute rotation
							if(_options.rotQuaternion)
							{
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_W] = boneOrientations[i].absoluteRotation.rotationQuaternion.w;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_X] = boneOrientations[i].absoluteRotation.rotationQuaternion.x;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_Y] = boneOrientations[i].absoluteRotation.rotationQuaternion.y;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_Z] = boneOrientations[i].absoluteRotation.rotationQuaternion.z;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
							}
							else
							{
								KinectUtils::rotMatToRotation(absX, absY, absZ, boneOrientations[i].absoluteRotation.rotationMatrix);
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_W] = 0;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_X] = absX;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_Y] = absY;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_Z] = absZ;
								m_skeleton_to_screen[j][i][SKELETON_JOINT_VALUE::ROT_CONF] = conf;
							}
							
							//relative rotation
							if(_options.rotQuaternion)
							{
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.w;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.x;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.y;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = boneOrientations[i].hierarchicalRotation.rotationQuaternion.z;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
							}
							else
							{
								KinectUtils::rotMatToRotation(relX, relY, relZ, boneOrientations[i].hierarchicalRotation.rotationMatrix);
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_W] = 0;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_X] = relX;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Y] = relY;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_Z] = relZ;
								m_skeleton[j][i][SKELETON_JOINT_VALUE::RELROT_CONF] = conf;
							}
						}
					}

					if (m_skeleton_confidence_provider) {
						for (ssi_size_t i = 0; i <SKELETON_JOINT::NUM; i++)
						{
							m_skeleton_confidence[j][i] = ssi_cast(float, skeletonFrame.SkeletonData[pick].eSkeletonPositionTrackingState[i]) / ssi_cast(float, NUI_SKELETON_POSITION_TRACKED);
						}
					}
				}
			}

			// paint skeleton on top of rgb image
			showSkeleton();
		}
	}

	void MicrosoftKinect::showSkeleton ()
	{
		for(ssi_size_t i=0; i< m_n_skeletons; ++i)
		{
			if (m_rgb_provider && _options.showBodyTracking) 
			{
				KinectUtils::paintSkeleton (m_colorImage, m_skeleton[i], true);
			}
		}
	}

	void MicrosoftKinect::processImages () {
		
		/*
		 * RGB
		 */
		if(m_rgb_provider || m_au_provider || m_face_provider || m_face3d_provider || m_head_provider)
		{
			const NUI_IMAGE_FRAME* pImageFrame = NULL;
			HRESULT hr = NuiImageStreamGetNextFrame(m_pVideoStreamHandle, 0, &pImageFrame);
			if (hr == S_OK)
			{		
				INuiFrameTexture* pTexture = pImageFrame->pFrameTexture;
				NUI_LOCKED_RECT LockedRect;
				pTexture->LockRect(0, &LockedRect, NULL, 0);
				if (LockedRect.Pitch)
				{
					memcpy(m_colorImage->GetBuffer(), PBYTE(LockedRect.pBits), min(m_colorImage->GetBufferSize(), UINT(pTexture->BufferLen())));
				}
				pTexture->UnlockRect(0);
			}
			else
				ssi_msg(SSI_LOG_LEVEL_DEBUG, "Kinect: no next frame");

			hr = NuiImageStreamReleaseFrame(m_pVideoStreamHandle, pImageFrame);
		}		

		/*
		 * Depth
		 */
		if(m_depth_provider || m_depthr_provider || m_au_provider || m_face_provider || m_face3d_provider || m_head_provider)
		{
			const NUI_IMAGE_FRAME* pImageFrame = NULL;
			HRESULT hr = NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &pImageFrame);
			if (hr == S_OK)
			{		
				INuiFrameTexture* pTexture = pImageFrame->pFrameTexture;
				NUI_LOCKED_RECT LockedRect;
				pTexture->LockRect(0, &LockedRect, NULL, 0);
				if (LockedRect.Pitch)
				{
					memcpy(m_depthRImage->GetBuffer(), PBYTE(LockedRect.pBits), min(m_depthRImage->GetBufferSize(), UINT(pTexture->BufferLen())));
				}
			}
			hr = NuiImageStreamReleaseFrame(m_pDepthStreamHandle, pImageFrame);
		}
	}

	void MicrosoftKinect::processFace () {

		if (m_au_provider || m_face_provider || m_face3d_provider  || m_head_provider) {

			// preapare data
			FT_SENSOR_DATA sensorData(m_colorImage, m_depthRImage, m_ZoomFactor, &m_ViewOffset);

			FT_VECTOR3D hint[2];

			// we will do face tracking for the nearest skeleton
			hint[0].x = m_skeleton[0][SKELETON_JOINT::SHOULDER_CENTER][SKELETON_JOINT_VALUE::POS_X];
			hint[0].y = m_skeleton[0][SKELETON_JOINT::SHOULDER_CENTER][SKELETON_JOINT_VALUE::POS_Y];
			hint[0].z = m_skeleton[0][SKELETON_JOINT::SHOULDER_CENTER][SKELETON_JOINT_VALUE::POS_Z];
			hint[1].x = m_skeleton[0][SKELETON_JOINT::HEAD][SKELETON_JOINT_VALUE::POS_X];
			hint[1].y = m_skeleton[0][SKELETON_JOINT::HEAD][SKELETON_JOINT_VALUE::POS_Y];
			hint[1].z = m_skeleton[0][SKELETON_JOINT::HEAD][SKELETON_JOINT_VALUE::POS_Z];

			// do face tracking
			if (m_LastFaceTrackSucceeded)
				m_pFaceTracker->ContinueTracking(&sensorData, hint, m_pFTResult);
			else
				m_pFaceTracker->StartTracking(&sensorData, NULL, hint, m_pFTResult);

			m_LastFaceTrackSucceeded = SUCCEEDED(m_pFTResult->GetStatus());

			// process face tracking results
			if (m_LastFaceTrackSucceeded)
			{
				FLOAT* pSU = NULL;
				UINT numSU;
				BOOL suConverged;
				m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);

				IFTModel* ftModel;
				HRESULT hr = m_pFaceTracker->GetFaceModel(&ftModel);
				bool hasfacemodel = SUCCEEDED(hr);

				if (m_au_provider && hasfacemodel) {

					FLOAT *pAUs;
					UINT auCount;
					m_pFTResult->GetAUCoefficients(&pAUs, &auCount);

					SSI_ASSERT (auCount == ACTIONUNIT::NUM);

					for(int i = 0; i<ACTIONUNIT::NUM; i++){
						m_actionunits[i] = pAUs[i];
					}
				}

				if(m_face_provider && hasfacemodel)
				{
					FT_VECTOR2D* pPts2D;
					UINT pts2DCount;
					m_pFTResult->Get2DShapePoints(&pPts2D, &pts2DCount);

					SSI_ASSERT (pts2DCount == FACEPOINT::NUM);

					for(UINT i = 0; i < pts2DCount;i++)
					{
						DWORD width = 0, height = 0;
						if (_options.screenWidth != 0 && _options.screenHeight != 0) {
							width = _options.screenWidth;
							height = _options.screenHeight;
						}
						else
							NuiImageResolutionToSize((NUI_IMAGE_RESOLUTION)m_colorRes, width, height);

						m_facepoints[i][FACEPOINT_VALUE::X] = _options.screenScale ? pPts2D[i].x / width : pPts2D[i].x;
						m_facepoints[i][FACEPOINT_VALUE::Y] = _options.screenScale ? pPts2D[i].y / height : pPts2D[i].y;
					}
				}


				if(m_face3d_provider && hasfacemodel)
				{
					FLOAT scale;
					FLOAT rotationXYZ[3];
					FLOAT translationXYZ[3];
					m_pFTResult->Get3DPose(&scale,rotationXYZ,translationXYZ);

					FLOAT* pAUCOeffs;
					UINT pAUCOunt;
					m_pFTResult->GetAUCoefficients(&pAUCOeffs, &pAUCOunt);
					
					FLOAT* pSU = NULL;
					UINT numSU;
					BOOL suConverged;
					m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);

					IFTModel* m_iftModel;  
					m_pFaceTracker->GetFaceModel(&m_iftModel); // ->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);
					
					UINT vertexCount = m_iftModel->GetVertexCount();
					FT_VECTOR3D* pPts3D = new FT_VECTOR3D[vertexCount];
					m_iftModel->Get3DShape(pSU, numSU, pAUCOeffs, pAUCOunt, scale, rotationXYZ, translationXYZ, pPts3D, vertexCount);

					SSI_ASSERT (vertexCount == FACEPOINT3D::NUM);

					for(UINT i = 0; i < vertexCount;i++)
					{
						m_facepoints3d[i][FACEPOINT3D_VALUE::X] = pPts3D[i].x;
						m_facepoints3d[i][FACEPOINT3D_VALUE::Y] = pPts3D[i].y;
						m_facepoints3d[i][FACEPOINT3D_VALUE::Z] = pPts3D[i].z;
					}

					delete[] pPts3D;
				}

				if (m_shape_provider && hasfacemodel)
				{
					FLOAT* pSU = NULL;
					UINT numSU;
					BOOL suConverged;
					m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);

					SSI_ASSERT (numSU == SHAPEUNIT::NUM);

					for(UINT i = 0; i < numSU;i++)
					{
						m_shapeunits[i] = pSU[i];
					}
				}

				if (m_headpose && hasfacemodel)
				{
					FLOAT scale;
					FLOAT rotationXYZ[3];
					FLOAT translationXYZ[3];
					m_pFTResult->Get3DPose(&scale, rotationXYZ, translationXYZ);

					for(UINT i = 0; i < 3;i++)
					{
						m_headpose[i] = rotationXYZ[i];
					}
				}

				if (m_rgb_provider && _options.showFaceTracking) 
				{
					showFace (pSU, ftModel);
				}
			
				if(hasfacemodel) {
					ftModel->Release();
				}
			}
			else
			{
				m_pFTResult->Reset();			
			}
		}
	}

	void MicrosoftKinect::showFace (FLOAT* pSU, IFTModel* ftModel) {

		if(_options.simpleFaceTracking)
		{
			KinectUtils::paintFacePoints (m_colorImage, m_facepoints, _options.screenScale, 0x00FFFF00);
		}
		else
		{
			POINT viewOffset = {0, 0};
			FT_CAMERA_CONFIG cameraConfig;			
			DWORD width = 0, height = 0;

			if (_options.screenWidth != 0 && _options.screenHeight != 0) {
				width = _options.screenWidth;
				height = _options.screenHeight;
			}
			else
				NuiImageResolutionToSize((NUI_IMAGE_RESOLUTION)m_colorRes, width, height);

			KinectUtils::GetVideoConfiguration(&cameraConfig, width, height);

			KinectUtils::paintFaceModel(m_colorImage, ftModel, &cameraConfig, pSU, 1.0, viewOffset, m_pFTResult, 0x00FFFF00);
		}
	}

	void MicrosoftKinect::processProvide () {

		if (m_rgb_provider) {
			m_rgb_provider->provide(ssi_pcast(ssi_byte_t, m_colorImage->GetBuffer()),1);
		}

		if (m_depthr_provider) {
			m_depthr_provider->provide(ssi_pcast(ssi_byte_t, m_depthRImage->GetBuffer()),1);
		}

		if (m_depth_provider) {
			KinectUtils::depthRaw2Image (ssi_pcast(ssi_byte_t, m_depthRImage->GetBuffer()), m_depthImage, getDepthImageParams(), getDepthRawParams());
			m_depth_provider->provide(m_depthImage,1);
		}

		if (m_face_provider) {
			m_face_provider->provide(ssi_pcast(ssi_byte_t, m_facepoints),1);
		}

		if (m_face3d_provider) {
			m_face3d_provider->provide(ssi_pcast(ssi_byte_t, m_facepoints3d),1);
		}

		if (m_skeleton_provider) {
			m_skeleton_provider->provide(ssi_pcast(ssi_byte_t, m_skeleton),1);
		}

		if (m_skeleton_to_screen_provider) {
			m_skeleton_to_screen_provider->provide(ssi_pcast(ssi_byte_t, m_skeleton_to_screen),1);
		}

		if (m_skeleton_confidence_provider) {
			m_skeleton_confidence_provider->provide(ssi_pcast(ssi_byte_t, m_skeleton_confidence), 1);
		}

		if (m_au_provider) {
			m_au_provider->provide(ssi_pcast(ssi_byte_t, m_actionunits),1);
		}

		if (m_shape_provider) {
			m_shape_provider->provide(ssi_pcast(ssi_byte_t, m_shapeunits),1);
		}

		if (m_head_provider) {
			m_head_provider->provide(ssi_pcast(ssi_byte_t, m_headpose),1);
		}
	}

	void MicrosoftKinect::process()
	{
		if (!m_KinectSensorPresent) {
			return;
		}

		processImages ();	
		processSkeleton ();
		processFace ();
		processProvide ();
	}

	bool MicrosoftKinect::disconnect () {

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "try to disconnect sensor...");

		if (m_pFaceTracker) {
			m_pFaceTracker->Release();
			m_pFaceTracker = NULL;
		}

		if (m_au_provider || m_face_provider || m_face3d_provider  || m_head_provider) {
			delete[] m_skeleton; m_skeleton = 0;
			delete[] m_skeleton_to_screen; m_skeleton_to_screen = 0;
			delete[] m_channel_order; m_channel_order = 0;
		}

		if(m_colorImage)
		{
			m_colorImage->Release();
			m_colorImage = NULL;
		}

		if(m_depthRImage) 
		{
			m_depthRImage->Release();
			m_depthRImage = NULL;
		}

		if (m_depthImage) {
			delete[] m_depthImage;
			m_depthImage = 0;
		}

		if(m_pFTResult)
		{
			m_pFTResult->Release();
			m_pFTResult = NULL;
		}

		if(m_bNuiInitialized)
		{
			NuiShutdown();	
		}
		m_bNuiInitialized = false;

		if (m_pNuiSensor)
		{
			m_pNuiSensor->NuiShutdown();
		}

		delete m_timer; m_timer = 0;

		ssi_msg (SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

		return true;
	}	
}

