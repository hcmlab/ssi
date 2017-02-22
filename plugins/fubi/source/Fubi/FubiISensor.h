// ****************************************************************************************
//
// Fubi Sensor Interface
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************
#pragma once

#include "FubiMath.h"

// The Fubi Sensor interface offers depth/rgb/ir image streams and user tracking data
class FubiISensor
{
public:
	virtual ~FubiISensor() {}

	// Update should be called once per frame for the sensor to update its streams and tracking data
	virtual void update() = 0;

	// Get the ids of all currently valid users: Ids will be stored in userIDs (if not 0x0), returns the number of valid users
	virtual unsigned short getUserIDs(unsigned int* userIDs) = 0;

	// Check if the sensor has new tracking data available
	virtual bool hasNewTrackingData() = 0;

	// Check if that user with the given id is tracked by the sensor
	virtual bool isTracking(unsigned int id) = 0;

	// Get the current joint position and orientation of one user
	virtual void getSkeletonJointData(unsigned int id, Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJointPosition& position, Fubi::SkeletonJointOrientation& orientation) = 0;

	// Get the current tracked face points of a user
	virtual bool getFacePoints(unsigned int id, const std::vector<Fubi::Vec3f>** vertices, const std::vector<Fubi::Vec3f>** triangleIndices)
	{
		return false;
	}

	// Get Stream data
	virtual const unsigned short* getDepthData() = 0;
	virtual const unsigned char* getRgbData()
	{
		return 0x0;
	}
	virtual const unsigned short* getIrData()
	{
		return 0x0;
	}
	virtual const unsigned short* getUserLabelData(Fubi::CoordinateType::Type coordType)
	{
		return 0x0;
	}

	// Init with options for streams and tracking
	virtual bool initWithOptions(const Fubi::SensorOptions& options)
	{
		Fubi_logWrn("initWithOptions not supported by this sensor!\n");
		return false;
	}

	// Init with a sensor specific xml file for options
	virtual bool initFromXml(const char* xmlPath, Fubi::SkeletonTrackingProfile::Profile trackingProfile = Fubi::SkeletonTrackingProfile::ALL,
		bool mirrorStreams = true, bool registerStreams =true)
	{
		Fubi_logWrn("initFromXml not supported by this sensor!\n");
		return false;
	}

	// Resets the tracking of a users
	virtual void resetTracking(unsigned int id)
	{
		Fubi_logWrn("resetTracking not supported by this sensor!\n");
	}

	// Get the floor plane
	virtual Fubi::Plane getFloor()
	{
		Fubi_logWrn("getFloor not supported by this sensor!\n");
		return Fubi::Plane();
	}

	// Convert coordinates between real world, depth, color, or IR image
	// inputType should be != outputType
	virtual Fubi::Vec3f convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
	{
		if (inputType == outputType)
			return inputCoords;
		// Catch some cases  using default Kinect parameters
		// Should be overwritten in an actual sensor
		Fubi::Vec3f ret(0, 0, inputCoords.z);
		static const double realWorldXtoZ = tan(1.0144686707507438/2.0)*2.0;
		static const double realWorldYtoZ = tan(0.78980943449644714/2.0)*2.0;

		if (inputType == Fubi::CoordinateType::REAL_WORLD || outputType == Fubi::CoordinateType::REAL_WORLD)
		{
			// Note: different camera positions are not taken into account (e.g. color camera)
			Fubi::StreamOptions resolutionOptions = m_options.m_depthOptions;
			if (inputType == Fubi::CoordinateType::COLOR || outputType == Fubi::CoordinateType::COLOR)
				resolutionOptions = m_options.m_rgbOptions;
			else if (inputType == Fubi::CoordinateType::IR || outputType == Fubi::CoordinateType::IR)
				resolutionOptions = m_options.m_irOptions;

			if (resolutionOptions.isValid())
			{
				const float coeffX = (float) (resolutionOptions.m_width / realWorldXtoZ);
				const float coeffY = (float) (resolutionOptions.m_height / realWorldYtoZ);
				const int nHalfXres = resolutionOptions.m_width / 2;
				const int nHalfYres = resolutionOptions.m_height / 2;

				if (inputType == Fubi::CoordinateType::REAL_WORLD)
				{
					ret.x = coeffX * inputCoords.x / inputCoords.z + nHalfXres;
					ret.y = nHalfYres - coeffY * inputCoords.y / inputCoords.z;
				}
				else
				{
					ret.x = (inputCoords.x - nHalfXres) * inputCoords.z / coeffX;
					ret.y = (inputCoords.y - nHalfYres) * inputCoords.z / -coeffX;
				}
			}
		}
		else
		{
			// Only convert resolutions, no defaults for the different camera positions..
			Fubi::StreamOptions inputRes = m_options.m_depthOptions;
			if (inputType == Fubi::CoordinateType::COLOR)
				inputRes = m_options.m_rgbOptions;
			else if (inputType == Fubi::CoordinateType::IR)
				inputRes = m_options.m_irOptions;
			Fubi::StreamOptions outputRes = m_options.m_rgbOptions;
			if (inputType == Fubi::CoordinateType::DEPTH)
				inputRes = m_options.m_depthOptions;
			else if (inputType == Fubi::CoordinateType::IR)
				inputRes = m_options.m_irOptions;

			const int nHalfInXres = inputRes.m_width / 2;
			const int nHalfInYres = inputRes.m_height / 2;
			const int nHalfOutXres = outputRes.m_width / 2;
			const int nHalfOutYres = outputRes.m_height / 2;
			const float scaleW = (float) outputRes.m_width / (float) inputRes.m_width;
			const float scaleH = (float) outputRes.m_height / (float) inputRes.m_height;
			ret.x = ((inputCoords.x-nHalfInXres) * scaleW) + nHalfOutXres;
			ret.y = ((inputCoords.y-nHalfInYres) * scaleH) + nHalfOutYres;
		}

		return ret;
	}

	// Get Options
	inline const Fubi::SensorOptions& getOptions() { return m_options; }
	inline const Fubi::StreamOptions& getStreamOptions(Fubi::ImageType::Type imageType)
	{
		switch (imageType)
		{
		case Fubi::ImageType::Depth:
			return m_options.m_depthOptions;
		case Fubi::ImageType::Color:
			return m_options.m_rgbOptions;
		case Fubi::ImageType::IR:
			return m_options.m_irOptions;
		}
		return Fubi::DefaultStreamOptions;
	}
	virtual inline int getMaxRawStreamValue(Fubi::ImageType::Type imageType)
	{
		switch (imageType)
		{
		case Fubi::ImageType::IR:
			return 1024;
		case Fubi::ImageType::Depth:
			return Fubi::MaxDepth;
		}
		return 255;
	}
	Fubi::SensorType::Type getType() { return m_options.m_type; }

protected:
	Fubi::SensorOptions m_options;

};
