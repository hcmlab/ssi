// ****************************************************************************************
//
// Fubi Image Processing
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************

#pragma once

// Sensor interfaces for getting stream data
#include "FubiISensor.h"
#include "FubiIFingerSensor.h"


class FubiImageProcessing
{
public:
	// Get color of a user in the enhanced depth image
	static void getColorForUserID(unsigned int id, float& r, float& g, float& b);

	// Draw an image into the given buffer, returns true if succesful
	static bool getImage(FubiISensor* sensor, unsigned char* outputImage, Fubi::ImageType::Type type, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth,
		int renderOptions = (Fubi::RenderOptions::Shapes | Fubi::RenderOptions::Skeletons | Fubi::RenderOptions::UserCaptions),
		int jointsToRender = Fubi::RenderOptions::ALL_JOINTS,
		Fubi::DepthImageModification::Modification depthModifications = Fubi::DepthImageModification::UseHistogram,
		unsigned int userId = 0, Fubi::SkeletonJoint::Joint jointOfInterest = Fubi::SkeletonJoint::NUM_JOINTS, bool moveCroppedToUpperLeft = false);

	// Save a picture of one user (or the whole scene if userId = 0)
	static bool saveImage(FubiISensor* sensor, const char* fileName, int jpegQuality,
		Fubi::ImageType::Type type, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth,
		int renderOptions = (Fubi::RenderOptions::Shapes | Fubi::RenderOptions::Skeletons | Fubi::RenderOptions::UserCaptions),
		int jointsToRender = Fubi::RenderOptions::ALL_JOINTS,
		Fubi::DepthImageModification::Modification depthModifications = Fubi::DepthImageModification::UseHistogram,
		unsigned int userId = 0, Fubi::SkeletonJoint::Joint jointOfInterest = Fubi::SkeletonJoint::NUM_JOINTS);

	// Applies a fingercount calculation for one hand of one user
	static int applyFingerCount(FubiISensor* sensor, unsigned int userID, bool leftHand = false, bool useOldConvexityDefectMethod = false, Fubi::FingerCountImageData* debugData = 0x0);

	// Releases an image previously created by the FubiImageProcessing methods
	static void releaseImage(cv::Mat* image);

	// Get a finger sensor image in the defined format
	static bool getImage(FubiIFingerSensor* sensor, unsigned char* outputImage, unsigned int imageIndex, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth);

	static void plotImage(const std::vector<Fubi::Vec3f>& dataToPlot, const std::vector<Fubi::Matrix3f>* invDataCovs /*= 0x0*/,
		unsigned char* outputImage, unsigned int width, unsigned height,
		Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, int lineThickness = 1);
	static void showPlot(const std::vector<Fubi::Vec3f>& dataToPlot, const std::vector<Fubi::Matrix3f>* invDataCovs = 0x0,
		unsigned int width = 640, unsigned height = 480, const std::string& windowName = "plot");

private:

	// Draw the color image of the sensor, returns true if succesful
	static bool drawColorImage(FubiISensor* sensor, unsigned char* outputImage, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, bool swapBandR = false);
	// Draw the ir image of the sensor, returns true if succesful
	template<class T> static bool drawIRImage(FubiISensor* sensor, T* outputImage, Fubi::ImageNumChannels::Channel numChannels, const T& maxValue);
	// Draws the depth histogram with optional tracking info to the given image buffer, returns true if succesful
	static bool drawDepthImage(FubiISensor* sensor, unsigned char* outputImage, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth,
		Fubi::DepthImageModification::Modification depthModifications, int renderOptions);
	// Converts the raw finger sensor image to the given type
	template<class T> static bool drawFingerSensorImage(FubiIFingerSensor* sensor, T* outputImage, unsigned int imageIndex,
		Fubi::ImageNumChannels::Channel numChannels, const T& maxValue);
	template<class T> static void convertDepthImage(const unsigned short* srcImage, T* dstImage, const unsigned short* userlabelImage,
		int width, int height, Fubi::ImageNumChannels::Channel numChannels, Fubi::DepthImageModification::Modification depthModifications,
		const T& maxValue, float scaleFac, float maxDepth, bool forceBackgroundRendering, bool swapRnB);

	// Adds tracking info to a image
	static void drawTrackingInfo(FubiISensor* sensor, unsigned char* outputImage, int width, int height, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth,
		Fubi::CoordinateType::Type coordinateType, int renderOptions, int jointsToRender);
	// Adds skeleton tracking info for a specific user to an image
	static void drawUserSkeletonTrackingInfo(class FubiUser* user, cv::Mat* pImage, Fubi::CoordinateType::Type coordinateType, int renderOptions, int jointsToRender);
	// Adds skeleton tracking info for a specific hand to an image
	static void drawHandSkeletonTrackingInfo(class FubiHand* hand, cv::Mat* pImage, Fubi::CoordinateType::Type coordinateType, int renderOptions, int jointsToRender);
	// overlay the last finger count image onto the output image
	template<class T> static void drawFingerCountImage(unsigned int userID, bool leftHand, T* outputImage, int width, int height, Fubi::ImageNumChannels::Channel numChannels,
		Fubi::ImageDepth::Depth depth, int renderOptions, const T& maxValue);

	// Draw a single limp of a player in a image buffer
	static void drawLimb(unsigned int userId, Fubi::SkeletonJoint::Joint eJoint1, Fubi::SkeletonJoint::Joint eJoint2, cv::Mat* pImage,
		Fubi::CoordinateType::Type coordinateType, int renderOptions);
	// Draw the label for a body measurement
	static void drawBodyMeasurement(unsigned int userId, Fubi::SkeletonJoint::Joint eJoint1, Fubi::SkeletonJoint::Joint eJoint2,
		Fubi::BodyMeasurement::Measurement bodyMeasure, cv::Mat* pImage, Fubi::CoordinateType::Type coordinateType, int renderOptions);

	// Draw a finger in the image buffer
	static void drawFinger(unsigned int handId, Fubi::SkeletonHandJoint::Joint finger, cv::Mat* pImage,
		Fubi::CoordinateType::Type coordinateType, int renderOptions);

	// Helper function for setting the image roi around the user joint (and thresholding the image) returns true if user and joint were found
	static bool getROIForUserJoint(unsigned int userId, Fubi::SkeletonJoint::Joint jointOfInterest, int width, int height, Fubi::CoordinateType::Type coordinateType,
		int& roiX, int& roiY, int& roiW, int& roiH, float& roiZ, bool useFilteredData = false);
	// Applying a two sided threshold
	static void applyThreshold(unsigned char* imageData, int width, int height, Fubi::ImageDepth::Depth depth, size_t step, float zCenter, float threshold, float replaceValue = 0);

	// Draw text to an image
	static void drawText(cv::Mat* pImage, const std::string& text, float xPos, float yPos, float colR, float colG, float colB, bool center = true);

	// Counts the number fingers in a depth image that should have set its roi to the region of one hand (and threshold applied preferrably)
	// The processing steps will be visualized into the rgbImage if given
	static int fingerCount(cv::Mat* pDepthImage, cv::Mat* pRgbaImage = 0x0, bool useContourDefectMode = false);

    // Fast round (banker's round) only applied on non-float values
    template<class T> static inline T roundNonFloat(float value);

	// Buffer for calculating the depth histogram
	static float m_depthHist[Fubi::MaxDepth];
	static unsigned short m_lastMaxDepth;

	// The different colors for each user id
	static const float m_colors[Fubi::MaxUsers+1][3];
};
