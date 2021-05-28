// AzureKinect.cpp
// author: Fabian Wildgrube
// created: 2021
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

#include "AzureKinect.h"
#include "thread/Timer.h"

#include <iostream>
#include <math.h>
#include <chrono>

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//uncomment this to print the time it takes to process a single frame to the console
//#define PRINT_FRAME_TIMINGS
#ifdef PRINT_FRAME_TIMINGS
	#include <chrono>
#endif


namespace ssi {
	using namespace AK;

	ssi_char_t* AzureKinect::ssi_log_name = "azurekinect";

	AzureKinect::AzureKinect(const ssi_char_t* file)
		: m_firstFrame(true),
		m_rgb_provider(0),
		m_bgraBuffer(0),
		m_depthVisualisation_provider(0),
		m_depthVisualisationBuffer(0),
		m_pointCloud_provider(0),
		m_pointCloudBuffer(0),
		m_depthRaw_provider(0),
		m_depthRawBuffer(0),
		m_irVisualisation_provider(0),
		m_irVisualisationBuffer(0),
		m_irRaw_provider(0),
		m_irRawBuffer(0),
		m_skeleton_provider(0),
		m_nrOfSkeletons(1),
		m_skeletons(0),
		m_previousJointRotations(0),
		m_bodyTracker(0),
		m_camerasStarted(false),
		_file(0),
		m_timer(0),
		ssi_log_level(SSI_LOG_LEVEL_DEFAULT)
	{
		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	AzureKinect::~AzureKinect()
	{
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	bool AzureKinect::setProvider(const ssi_char_t* name, IProvider* provider)
	{
		bool providerWasAlreadySet = false;

		std::string providerNameStr = std::string("Provider ") + name;

		if (strcmp(name, SSI_AZUREKINECT_RGBIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_rgb_provider, m_rgb_channel, getRGBImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_DEPTHVISUALISATIONIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_depthVisualisation_provider, m_depthVisualisation_channel, getDepthVisualisationImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_DEPTHRAWIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_depthRaw_provider, m_depthRaw_channel, getDepthRawImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_IRVISUALISATIONIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_irVisualisation_provider, m_irVisualisation_channel, getIRVisualisationImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_IRRAWIMAGE_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setImageProvider(provider, m_irRaw_provider, m_irRaw_channel,getIRRawImageParams());
		}
		else if (strcmp(name, SSI_AZUREKINECT_SKELETON_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setSkeletonProvider(provider);
		}
		else if (strcmp(name, SSI_AZUREKINECT_POINTCLOUD_PROVIDER_NAME) == 0) {
			providerWasAlreadySet = setPointCloudProvider(provider);
		}
		else {
			ssi_wrn((providerNameStr + " does not exist on this sensor").c_str());

			return false;
		}

		if (providerWasAlreadySet) {
			ssi_wrn((providerNameStr + " was already set before.").c_str());
		}

		ssi_msg(SSI_LOG_LEVEL_DETAIL,(providerNameStr + " set.").c_str());

		return true;
	}

	bool AzureKinect::setImageProvider(IProvider* providerIn, IProvider* &internalProvider, IChannel &internalChannel, ssi_video_params_t params) {
		bool providerWasDefined = false;
		if (internalProvider) {
			providerWasDefined = true;
		}

		internalProvider = providerIn;

		if (internalProvider) {
			internalProvider->setMetaData(sizeof(params), &params);

			internalChannel.getStreamPtr()->sr = _options.sr;
			internalChannel.getStreamPtr()->byte = params.widthInPixels * params.heightInPixels * params.numOfChannels * params.depthInBitsPerChannel / 8;

			internalProvider->init(&internalChannel);
		}

		return providerWasDefined;
	}

	bool AzureKinect::setSkeletonProvider(IProvider* skeleton_provider) {
		bool providerWasDefined = false;
		if (m_skeleton_provider) {
			providerWasDefined = true;
		}

		m_skeleton_provider = skeleton_provider;

		if (m_skeleton_provider) {
			//at least one body, filled with dummy values if necessary otherwise this stream would have no dimensions when body tracking is disabled
			ssi_size_t minBodies = 1;
			m_nrOfSkeletons = (std::max)(minBodies, _options.nrOfBodiesToTrack);

			SSI_SKELETON_META m = ssi_skeleton_meta(SSI_SKELETON_TYPE::AZURE_KINECT, m_nrOfSkeletons);
			m_skeleton_provider->setMetaData(sizeof(SSI_SKELETON_META), &m);
			m_skeleton_channel.stream.dim = m_nrOfSkeletons * SKELETON_JOINT::NUM * SKELETON_JOINT_VALUE::NUM;
			m_skeleton_channel.stream.sr = _options.sr;
			m_skeleton_provider->init(&m_skeleton_channel);
		}

		return providerWasDefined;
	}
	
	bool AzureKinect::setPointCloudProvider(IProvider* pointcloud_provider) {
		bool providerWasDefined = false;
		if (m_pointCloud_provider) {
			providerWasDefined = true;
		}

		m_pointCloud_provider = pointcloud_provider;

		if (m_pointCloud_provider) {
			m_pointCloud_channel.stream.dim = _options.depthVideoWidth * _options.depthVideoHeight * 6; //6 bytes per pointcloud voxel
			m_pointCloud_channel.stream.sr = _options.sr;
			m_pointCloud_provider->init(&m_pointCloud_channel);
		}

		return providerWasDefined;
	}

	bool AzureKinect::connect()
	{
		auto count = k4a::device::get_installed_count();

		if (count > 0) {

			if (count > 1) {
				ssi_wrn("Multiple Azure Kinects found. Connecting to the first one only.");
			}

			try {
				m_azureKinectDevice = k4a::device::open(0);
			} 
			catch (const std::exception& e)
			{
				ssi_wrn("Could not connect to azure kinect");
				m_azureKinectDevice.close();
				return false;
			}

			ssi_msg(SSI_LOG_LEVEL_DETAIL, "connected");

			initBuffers();

			startCameras();
		}
		else {
			ssi_wrn("No Azure Kinect found!");
		}

		// set thread name
		Thread::setName(getName());

		return true;
	}

	bool AzureKinect::start()
	{
		return Thread::start();
	}

	bool AzureKinect::stop()
	{
		return Thread::stop();
	}

	void AzureKinect::initBuffers() {
		if (m_rgb_provider || _options.showBodyTracking) {
			m_bgraBuffer = new BgraPixel[getRGBImageParams().widthInPixels * getRGBImageParams().heightInPixels];
		}

		if (m_depthRaw_provider || m_depthVisualisation_provider || m_pointCloud_provider) {
			m_depthRawBuffer = new DepthPixel[getDepthVisualisationImageParams().widthInPixels * getDepthVisualisationImageParams().heightInPixels];
		}

		if (m_depthVisualisation_provider) {
			m_depthVisualisationBuffer = new BgrPixel[getDepthVisualisationImageParams().widthInPixels * getDepthVisualisationImageParams().heightInPixels];
			m_depthHSVConversionMat = cv::Mat(getDepthVisualisationImageParams().widthInPixels, getDepthVisualisationImageParams().heightInPixels, CV_8UC3, m_depthVisualisationBuffer);
		}

		if (m_irRaw_provider || m_irVisualisation_provider) {
			m_irRawBuffer = new DepthPixel[getIRRawImageParams().widthInPixels * getIRRawImageParams().heightInPixels];
		}

		if (m_irVisualisation_provider) {
			m_irVisualisationBuffer = new IRPixel[getIRVisualisationImageParams().widthInPixels * getIRVisualisationImageParams().heightInPixels];
		}

		if (m_skeleton_provider || _options.showBodyTracking)
		{
			m_skeletons = new SKELETON[m_nrOfSkeletons];
			for (ssi_size_t k = 0; k < m_nrOfSkeletons; k++)
			{
				for (ssi_size_t i = 0; i < SKELETON_JOINT::NUM; i++)
				{
					for (ssi_size_t j = 0; j < SKELETON_JOINT_VALUE::NUM; ++j)
					{
						m_skeletons[k][i][j] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;
					}
				}
			}
		}

		if ((m_skeleton_provider || _options.showBodyTracking) && _options.lowpassFilterJointRotations)
		{
			m_previousJointRotations = new JOINTROTATION_AVERAGES[m_nrOfSkeletons];
			for (ssi_size_t k = 0; k < m_nrOfSkeletons; k++)
			{
				for (ssi_size_t i = 0; i < SKELETON_JOINT::NUM; i++)
				{
					for (ssi_size_t j = 0; j < 4; ++j)
					{
						m_skeletons[k][i][j] = 0.0f;
					}
				}
			}
		}

		if (m_pointCloud_provider) {
			m_pointCloudBuffer = new PointCloudPixel[_options.depthVideoWidth * _options.depthVideoHeight];
		}
	}


	void AzureKinect::startCameras()
	{
		m_azureKinectConfig = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
		_options.applyToCameraConfiguration(m_azureKinectConfig);
		m_sensorCalibration = m_azureKinectDevice.get_calibration(m_azureKinectConfig.depth_mode, m_azureKinectConfig.color_resolution);
		m_transformation = k4a::transformation(m_sensorCalibration);

		try
		{
			m_azureKinectDevice.start_cameras(&m_azureKinectConfig);
			m_camerasStarted = true;
		}
		catch (const std::exception& e)
		{
			ssi_wrn("Could not start the kinect cameras");
			return;
		}

		if (m_skeleton_provider || _options.showBodyTracking)
		{
			k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
			tracker_config.processing_mode = K4ABT_TRACKER_PROCESSING_MODE_GPU_CUDA; //important, otherwise will default to WindowsML which doesn't seem to work
			m_bodyTracker = k4abt::tracker::create(m_sensorCalibration, tracker_config);
		}
	}

	void AzureKinect::stopCameras() {
		if (m_azureKinectDevice) {
			m_azureKinectDevice.stop_cameras();
			m_camerasStarted = false;
		}
	}

	void AzureKinect::run()
	{
		if (!m_timer) {
			m_timer = new Timer(1 / _options.sr);
		}

#ifdef PRINT_FRAME_TIMINGS
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
#endif
	
		process();

#ifdef PRINT_FRAME_TIMINGS
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << "Frame took: " << duration << "ms\n";
#endif

		m_timer->wait();
	}

	void AzureKinect::process()
	{
		
		if (!m_azureKinectDevice || !m_camerasStarted) return;

		getCapture();

		if (m_skeleton_provider || _options.showBodyTracking) {
			//important: if you call this, ALWAYS call processBodyTracking() later
			//otherwise internal queue of the a4k body tracker will overflow and block
			passCaptureToBodyTracker();
		}

		if (m_rgb_provider) {
			processRGBAImage();
		}

		if (m_depthRaw_provider || m_depthVisualisation_provider || m_pointCloud_provider) {
			processDepthImage();
		}

		if (m_irRaw_provider || m_irVisualisation_provider) {
			processIRImage();
		}

		releaseCapture();

		if (m_skeleton_provider || _options.showBodyTracking) {
			processBodyTracking();
			if (_options.showBodyTracking) {
				visualizeTrackedBodies();
			}
		}


		processProviders();		
	}

	void AzureKinect::getCapture()
	{
		if (m_azureKinectDevice && m_camerasStarted) {
			try
			{
				m_azureKinectDevice.get_capture(&m_capturedFrame);
			}
			catch (const std::exception& e)
			{
				releaseCapture();
				ssi_wrn("Error during frame capture");
				ssi_wrn(e.what());
				return;
			}
		}
	}

	void AzureKinect::passCaptureToBodyTracker()
	{
		if (m_capturedFrame) {
			if (!m_bodyTracker.enqueue_capture(m_capturedFrame, std::chrono::milliseconds(0))) {
				ssi_wrn("Body tracker could not enqueue capture!");
			}
		}
	}

	void AzureKinect::releaseCapture() {
		m_capturedFrame.reset();
	}

	void AzureKinect::processRGBAImage()
	{
		if (m_capturedFrame) {
			k4a::image rgbImage = m_capturedFrame.get_color_image();
			if (rgbImage) {
				std::memcpy(m_bgraBuffer, rgbImage.get_buffer(), rgbImage.get_size());
				rgbImage.reset(); //release the image after getting its data
			}
		}
	}

	void AzureKinect::processDepthImage()
	{
		if (m_capturedFrame) {
			k4a::image depthImage = m_capturedFrame.get_depth_image();
			if (depthImage) {
				std::memcpy(m_depthRawBuffer, depthImage.get_buffer(), depthImage.get_size());



				if (m_pointCloud_provider) {
					try
					{
						auto pointCloudImage = m_transformation.depth_image_to_point_cloud(depthImage, K4A_CALIBRATION_TYPE_DEPTH);
						std::memcpy(m_pointCloudBuffer, pointCloudImage.get_buffer(), pointCloudImage.get_size());
						pointCloudImage.reset();
					}
					catch (const std::exception&)
					{
						ssi_wrn("Point Cloud could not be generated");
					}
				}


				depthImage.reset(); //release the image after getting its data

				if (m_depthVisualisation_provider) {
					const int width = getDepthVisualisationImageParams().widthInPixels;
					const int height = getDepthVisualisationImageParams().heightInPixels;
					const auto valueRange = GetDepthModeRange(_options._depthMode);

					for (int h = 0; h < height; ++h)
					{
						for (int w = 0; w < width; ++w)
						{
							const size_t currentPixel = static_cast<size_t>(h * width + w);
							m_depthVisualisationBuffer[currentPixel] = ColorizeBlueToRed_HSV(m_depthRawBuffer[currentPixel], valueRange.first, valueRange.second);
						}
					}

					cv::cvtColor(m_depthHSVConversionMat, m_depthHSVConversionMat, cv::COLOR_HSV2BGR);
				}
			}
		}
	}

	void AzureKinect::processIRImage()
	{
		if (m_capturedFrame) {
			k4a::image irImage = m_capturedFrame.get_ir_image();
			if (irImage) {
				std::memcpy(m_irRawBuffer, irImage.get_buffer(), irImage.get_size());
				irImage.reset(); //release the image after getting its data

				if (m_irVisualisation_provider) {
					const int width = getIRRawImageParams().widthInPixels;
					const int height = getIRRawImageParams().heightInPixels;
					const auto valueRange = GetIrLevelsRange(_options._depthMode);

					for (int h = 0; h < height; ++h)
					{
						for (int w = 0; w < width; ++w)
						{
							const size_t currentPixel = static_cast<size_t>(h * width + w);
							m_irVisualisationBuffer[currentPixel] = ColorizeGreyscale(m_irRawBuffer[currentPixel], valueRange.first, valueRange.second);
						}
					}
				}
			}
		}
	}

	void AzureKinect::processBodyTracking()
	{
		k4abt::frame body_frame = m_bodyTracker.pop_result();
		if (body_frame)
		{
			ssi_size_t nrOfTrackedBodies = body_frame.get_num_bodies();

			size_t bodiesProcessed = 0;

			auto bodiesToProcess = (std::min)(m_nrOfSkeletons, nrOfTrackedBodies);

			//for each tracked body
			for (ssi_size_t bodyIdx = 0; bodyIdx < bodiesToProcess; bodyIdx++)
			{
				k4abt_body_t body = body_frame.get_body(bodyIdx);

				//for each joint
				for (ssi_size_t jointIdx = 0; jointIdx < SKELETON_JOINT::NUM; jointIdx++)
				{
					k4a_float3_t position = body.skeleton.joints[jointIdx].position;
					k4a_quaternion_t orientation = body.skeleton.joints[jointIdx].orientation;
					k4abt_joint_confidence_level_t confidence_level = body.skeleton.joints[jointIdx].confidence_level;
					float confidence = confidence_level / (K4ABT_JOINT_CONFIDENCE_LEVELS_COUNT * 1.0f);

					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_X] = position.xyz.x;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_Y] = position.xyz.y;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_Z] = position.xyz.z;

					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_W] = orientation.wxyz.w;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_X] = orientation.wxyz.x;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_Y] = orientation.wxyz.y;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_Z] = orientation.wxyz.z;

					if (_options.lowpassFilterJointRotations) {
						if (m_firstFrame) {
							m_previousJointRotations[bodyIdx][jointIdx][0] = orientation.wxyz.w;
							m_previousJointRotations[bodyIdx][jointIdx][1] = orientation.wxyz.x;
							m_previousJointRotations[bodyIdx][jointIdx][2] = orientation.wxyz.y;
							m_previousJointRotations[bodyIdx][jointIdx][3] = orientation.wxyz.z;
						}
						else {
							auto previous_w = m_previousJointRotations[bodyIdx][jointIdx][0];
							auto previous_x = m_previousJointRotations[bodyIdx][jointIdx][1];
							auto previous_y = m_previousJointRotations[bodyIdx][jointIdx][2];
							auto previous_z = m_previousJointRotations[bodyIdx][jointIdx][3];

							auto current_w = orientation.wxyz.w;
							auto current_x = orientation.wxyz.x;
							auto current_y = orientation.wxyz.y;
							auto current_z = orientation.wxyz.z;

							/*
							//SLERP (taken from: https://www.lix.polytechnique.fr/~nielsen/WEBvisualcomputing/programs/slerp.cpp)
							float dotproduct = previous_x * current_x + previous_y * current_y + previous_z * current_z + previous_w * current_w;
							float theta, st, sut, sout, coeff1, coeff2;

							double lambda = 0.7;

							theta = (float)acos(dotproduct);
							if (theta < 0.0) theta = -theta;

							st = (float)sin(theta);
							sut = (float)sin(lambda * theta);
							sout = (float)sin((1 - lambda) * theta);
							coeff1 = sout / st;
							coeff2 = sut / st;

							auto smoothed_w = coeff1 * previous_w + coeff2 * current_w;
							auto smoothed_x = coeff1 * previous_x + coeff2 * current_x;
							auto smoothed_y = coeff1 * previous_y + coeff2 * current_y;
							auto smoothed_z = coeff1 * previous_z + coeff2 * current_z;

							//normalize the smoothed quaternion before writing it out
							double norm = sqrt(smoothed_w * smoothed_w + smoothed_x * smoothed_x + smoothed_y * smoothed_y + smoothed_z * smoothed_z);
							smoothed_w /= norm;
							smoothed_x /= norm;
							smoothed_y /= norm;
							smoothed_z /= norm;
							*/

							
							// exponentielle Glättung (https://de.wikipedia.org/wiki/Exponentielle_Gl%C3%A4ttung)
							static const float alpha = 0.8f;
							static const float _1_alpha = 1.0f - alpha;
							auto smoothed_w = alpha * orientation.wxyz.w + _1_alpha * previous_w;
							auto smoothed_x = alpha * orientation.wxyz.x + _1_alpha * previous_x;
							auto smoothed_y = alpha * orientation.wxyz.y + _1_alpha * previous_y;
							auto smoothed_z = alpha * orientation.wxyz.z + _1_alpha * previous_z;
							

							m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_W] = smoothed_w;
							m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_X] = smoothed_x;
							m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_Y] = smoothed_y;
							m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_Z] = smoothed_z;

							m_previousJointRotations[bodyIdx][jointIdx][0] = smoothed_w;
							m_previousJointRotations[bodyIdx][jointIdx][1] = smoothed_x;
							m_previousJointRotations[bodyIdx][jointIdx][2] = smoothed_y;
							m_previousJointRotations[bodyIdx][jointIdx][3] = smoothed_z;
						}
					}

					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::CONF] = confidence;
				}

				bodiesProcessed++;
			}
			
			//fill up with invalid data if less bodies were detected than the pipeline demands of us
			for (size_t bodyIdx = bodiesProcessed; bodyIdx < m_nrOfSkeletons ; bodyIdx++)
			{
				for (ssi_size_t jointIdx = 0; jointIdx < SKELETON_JOINT::NUM; jointIdx++)
				{
					//TODO: possible optimisation: use memset
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_X] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_Y] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_Z] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;

					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_W] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_X] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_Y] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;
					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::ROT_Z] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;

					m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::CONF] = SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE;
				}
			}

			m_firstFrame = false;

			body_frame.reset();
		}
		else
		{
			ssi_err("Error! Pop body frame result time out!");
		}
	}

	/// <summary>
	///	Paints simplistic joints and bones over the rgb image
	/// Prerequisites:	- rgbaBuffer needs to exist
	///					- skeleton tracking has already happened
	/// </summary>
	void AzureKinect::visualizeTrackedBodies() {
		for (ssi_size_t bodyIdx = 0; bodyIdx < m_nrOfSkeletons; bodyIdx++)
		{
			auto color = cv::Scalar(255, 0, 0);
			for (ssi_size_t jointIdx = 0; jointIdx < SKELETON_JOINT::NUM; jointIdx++)
			{
				if (m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_X] == SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE) {
					continue;
				}

				//Joint
				auto conversion = convert3DDepthTo2DColorCoordinate(m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_X],
																	m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_Y],
																	m_skeletons[bodyIdx][jointIdx][SKELETON_JOINT_VALUE::POS_Z],
																	m_sensorCalibration);
				if (!conversion.first) {
					continue;
				}
				cv::Point jointCoordinate = conversion.second;

				if (jointCoordinate.x == 0 && jointCoordinate.y == 0) {
					//ignore values that the sensore sdk could not convert (returns 0.0f, 0.0f in that case)
					continue;
				}

				cv::Mat rgbMat(_options.rgbVideoHeight, _options.rgbVideoWidth, CV_8UC4, m_bgraBuffer);
				cv::circle(rgbMat, jointCoordinate, 3, color, cv::FILLED);

				//Bone
				try
				{
					auto parentJointIdx = AZUREKINECT_BONE_LIST.at(static_cast<SKELETON_JOINT::List>(jointIdx));
					auto parentConversion = convert3DDepthTo2DColorCoordinate(m_skeletons[bodyIdx][parentJointIdx][SKELETON_JOINT_VALUE::POS_X],
																				m_skeletons[bodyIdx][parentJointIdx][SKELETON_JOINT_VALUE::POS_Y],
																				m_skeletons[bodyIdx][parentJointIdx][SKELETON_JOINT_VALUE::POS_Z],
																				m_sensorCalibration);
					if (!parentConversion.first) {
						continue;
					}

					cv::Point parentJointCoordinate = parentConversion.second;
					cv::line(rgbMat, jointCoordinate, parentJointCoordinate, color, 2);
				}
				catch (const std::exception&)
				{
					// exception thrown here means the joint has no parent joint
				}
					
			}
		}
	}

	void AzureKinect::processProviders()
	{
		if (m_rgb_provider) {
			m_rgb_provider->provide(ssi_pcast(ssi_byte_t, m_bgraBuffer), 1);
		}

		if (m_depthRaw_provider) {
			m_depthRaw_provider->provide(ssi_pcast(ssi_byte_t, m_depthRawBuffer), 1);
		}

		if (m_depthVisualisation_provider) {
			m_depthVisualisation_provider->provide(ssi_pcast(ssi_byte_t, m_depthVisualisationBuffer), 1);
		}

		if (m_irRaw_provider) {
			m_irRaw_provider->provide(ssi_pcast(ssi_byte_t, m_irRawBuffer), 1);
		}

		if (m_irVisualisation_provider) {
			m_irVisualisation_provider->provide(ssi_pcast(ssi_byte_t, m_irVisualisationBuffer), 1);
		}

		if (m_skeleton_provider) {
			m_skeleton_provider->provide(ssi_pcast(ssi_byte_t, m_skeletons), 1);
		}

		if (m_pointCloud_provider) {
			m_pointCloud_provider->provide(ssi_pcast(ssi_byte_t, m_pointCloudBuffer), 1);
		}
	}

	bool AzureKinect::disconnect() {

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "try to disconnect sensor...");

		stopCameras();

		if (m_bodyTracker) {
			m_bodyTracker.shutdown();
		}

		if (m_capturedFrame) {
			m_capturedFrame.reset();
		}

		if (m_azureKinectDevice) {
			m_azureKinectDevice.close();
		}

		if (m_bgraBuffer) {
			delete[] m_bgraBuffer;
			m_rgb_provider = 0;
		}

		if (m_depthRaw_provider) {
			delete[] m_depthRawBuffer;
			m_depthRaw_provider = 0;
		}

		if (m_depthVisualisation_provider) {
			delete[] m_depthVisualisationBuffer;
			m_depthVisualisation_provider = 0;
		}

		if (m_irRaw_provider) {
			delete[] m_irRawBuffer;
			m_irRaw_provider = 0;
		}

		if (m_irVisualisation_provider) {
			delete[] m_irVisualisationBuffer;
			m_irVisualisation_provider = 0;
		}

		if (m_skeleton_provider)
		{
			delete[] m_skeletons;
			m_skeleton_provider = 0;
		}

		if (m_previousJointRotations) {
			delete[] m_previousJointRotations;
			m_previousJointRotations = 0;
		}

		if (m_pointCloud_provider) {
			delete[] m_pointCloudBuffer;
			m_pointCloud_provider = 0;
		}

		ssi_msg(SSI_LOG_LEVEL_DETAIL, "sensor disconnected");

		return true;
	}
}