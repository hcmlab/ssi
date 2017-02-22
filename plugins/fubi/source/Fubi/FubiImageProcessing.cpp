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


// Defines for enabling/disabling OpenNI and OpenCV dependencies
#include "FubiConfig.h"

// Stringifier and concat macros
#define FUBI_STR_EXPR(X) #X
#define FUBI_STR(X) FUBI_STR_EXPR(X)
#define FUBI_CONCAT3_EXPR(X,Y,Z) X ## Y ## Z
#define FUBI_CONCAT3(X,Y,Z) FUBI_CONCAT3_EXPR(X,Y,Z)

// OpenCV includes
#ifdef FUBI_USE_OPENCV
#	include <opencv2/highgui/highgui.hpp>
#	include <opencv2/imgproc/imgproc.hpp>
#	include <opencv2/core/core.hpp>
#	ifndef FUBI_OPENCV_VERSION
#		define FUBI_OPENCV_VERSION FUBI_CONCAT3(CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION)
#		pragma message( __FILE__ "(" FUBI_STR(__LINE__) ") : message : Fubi: found OpenCV version " FUBI_STR(FUBI_OPENCV_VERSION))
#	endif
#	if (CV_MAJOR_VERSION >= 3)
#		include <opencv2/imgcodecs/imgcodecs.hpp>
#		include <opencv2/imgcodecs/imgcodecs_c.h>
#		include <opencv2/core/types_c.h>
#		include <opencv2/imgproc/imgproc_c.h>
#	endif
#	if (CV_MAJOR_VERSION < 2)
#		pragma message( __FILE__ "(" FUBI_STR(__LINE__) ") : warning : Fubi: OpenCV version " FUBI_STR(FUBI_OPENCV_VERSION) " too old, disabling OpenCV support")
#		undef FUBI_USE_OPENCV
#	else
#		ifdef _DEBUG
#			pragma comment(lib, "opencv_world" FUBI_STR(FUBI_OPENCV_VERSION) "d.lib")
#		else
#			pragma comment(lib, "opencv_world" FUBI_STR(FUBI_OPENCV_VERSION) ".lib")
#		endif
#	endif
using namespace cv;
#endif

#ifdef _OPENMP
#	pragma message( __FILE__ "(" FUBI_STR(__LINE__) ") : message : Fubi: using OpenMP for parallelizing image processing")
#endif


#include "FubiImageProcessing.h"

#include "Fubi.h"
#include "FubiCore.h"

#include <queue>

using namespace Fubi;
using namespace std;

const float FubiImageProcessing::m_colors[MaxUsers+1][3] =
{
	{1.f,1.f,1.f}, // Background
	{1.f,1.f,0}, // User 1
	{0,1.f,1.f}, // User 2
	{1.f,0,1.f}, // ...
	{1.f,0,0},
	{0,1.f,0},
	{0,0,1.f},
	{1.f,.5f,0},
	{.5f,0,1.f},
	{0,1.f,.5f},
	{1.f,0,0.5f},
	{0,.5f,1.f},
	{.5f,1.f,0},
	{1.f,.5f,1.f},
	{1.f,1.f,.5f},
	{.5f,1.f,1.f}
};

float FubiImageProcessing::m_depthHist[Fubi::MaxDepth];
unsigned short FubiImageProcessing::m_lastMaxDepth = Fubi::MaxDepth;


template<class T> inline T FubiImageProcessing::roundNonFloat(float value)
{
    return (T) Fubi::ftoi_r(value);
}

template<> inline float FubiImageProcessing::roundNonFloat(float value)
{
    return value;
}


void FubiImageProcessing::getColorForUserID(unsigned int id, float& r, float& g, float& b)
{
	unsigned int nColorID = (id == 0) ? 0 : (((id-1) % MaxUsers)+1);
	b = m_colors[nColorID][0];
	g = m_colors[nColorID][1];
	r = m_colors[nColorID][2];
}


bool FubiImageProcessing::getImage(FubiISensor* sensor, unsigned char* outputImage, ImageType::Type type, ImageNumChannels::Channel numChannels, ImageDepth::Depth depth,
	int renderOptions /*= (RenderOptions::Shapes | RenderOptions::Skeletons | RenderOptions::UserCaptions)*/,
	int jointsToRender /*= RenderOptions::ALL_JOINTS*/,
	DepthImageModification::Modification depthModifications /*= DepthImageModification::UseHistogram*/,
	unsigned int userId /*= 0*/, Fubi::SkeletonJoint::Joint jointOfInterest /*= Fubi::SkeletonJoint::NUM_JOINTS*/, bool moveCroppedToUpperLeft /*=false*/)
{
	bool success = false;
	float threshold = 0;
	int roiX, roiY, roiW, roiH;
	float roiZ;

	int width = 0, height = 0;

	// First render image
	if (type == ImageType::Color)
	{
		bool swapRAndB = (renderOptions & RenderOptions::SwapRAndB) != 0;
		success = drawColorImage(sensor, outputImage, numChannels, depth, swapRAndB);
		Fubi::getRgbResolution(width, height);

		// Color image has by default the channel order RGB
		// As the tracking info's default is BGR we have to switch the swapRAndB option for the rest of the rendering
		if (swapRAndB)
			renderOptions &= ~RenderOptions::SwapRAndB;
		else
			renderOptions |= RenderOptions::SwapRAndB;
	}
	else if (type == ImageType::Depth)
	{
		success = drawDepthImage(sensor, outputImage, numChannels, depth, depthModifications, renderOptions);
		Fubi::getDepthResolution(width, height);
		if ((userId != 0) && depthModifications != DepthImageModification::UseHistogram && depthModifications != DepthImageModification::ConvertToRGB)
		{
			if (jointOfInterest == SkeletonJoint::NUM_JOINTS || jointOfInterest == SkeletonJoint::TORSO)
				threshold = 400.0f;
			else if (jointOfInterest == SkeletonJoint::HEAD)
				threshold = 200.0f;
			else
				threshold = 75.0f;
		}
	}
	else if (type == ImageType::IR)
	{
		if (depth == ImageDepth::D16)
			success = drawIRImage(sensor, (unsigned short*) outputImage, numChannels, Math::MaxUShort16);
		else if (depth == ImageDepth::D8)
			success = drawIRImage(sensor, outputImage, numChannels, Math::MaxUChar8);
		else
			success = drawIRImage(sensor, (float*) outputImage, numChannels, 1.0f);
		Fubi::getIRResolution(width, height);
	}
	else if (type == ImageType::Blank)
	{
		success = true;
		Fubi::getDepthResolution(width, height);
		if (width <= 0 || height <= 0)
		{
			// "Fake" standard resolution
			width = 640;
			height = 480;
		}
	}

	if (width <= 0 || height <= 0)
		success = false; // No valid image created


//#ifdef FUBI_USE_OPENCV
//	Mat image(Size(width, height), CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels), outputImage);
//	imshow("raw", image);
//#endif

	// Blank image defaults to depth image type, all other can be casted directly
	CoordinateType::Type coordType = (type == ImageType::Blank) ? CoordinateType::DEPTH : (CoordinateType::Type) type;

	// Threshold and Roi
	if (success && userId != 0)
	{
		// Get roi for joint
		if (getROIForUserJoint(userId, jointOfInterest, width, height, coordType,
				roiX, roiY, roiW, roiH, roiZ, (renderOptions&RenderOptions::UseFilteredValues) != 0))
			// Apply threshold (will have no effect if threshold == 0)
			applyThreshold(outputImage, width, height, depth, (size_t)width, roiZ, threshold);
		else
			userId = 0; // Roi could not be set for this user
	}

	// Add tracking info
	if (success && (renderOptions != RenderOptions::None))
	{
		drawTrackingInfo(sensor, outputImage, width, height, numChannels, depth, coordType, renderOptions, jointsToRender);
	}


//#ifdef FUBI_USE_OPENCV
//	Mat image2(Size(width, height), CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels), outputImage);
//	imshow("augmented", image2);
//#endif


	// "Crops" image if user is set
	if (success && userId != 0)
	{
#ifdef FUBI_USE_OPENCV
		// Create mat header for whole image
		Mat image(Size(width, height), CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels), outputImage);
		// Previously calculated roi rect
		Rect roiRect(roiX, roiY, roiW, roiH);
		if (moveCroppedToUpperLeft) // Crop roi and move it to the upper left corner
		{
			// Create header for roi part of the image
			Mat roiImage = image(roiRect);
			// Change roi rect to upper left corner
			roiRect = Rect(0, 0, roiW, roiH);
			// Header for upper left corner
			Mat upperLeftRoiImage = image(roiRect);
			// Copy the roi to the upper left
			roiImage.copyTo(upperLeftRoiImage);
		}
		// Fill everything else than the roi with zeros
		// Create a single-channel mask the same size as the image filled with 1s
		Mat inverseMask(image.size(), CV_8UC1, Scalar(1));
		// Specify the ROI in the mask
		Mat inverseMaskROI = inverseMask(roiRect);
		//Fill the mask's ROI with 0s
		inverseMaskROI = Scalar(0);
		//Set the image to 0 in places where the mask is 1
		image.setTo(Scalar::all(0), inverseMask);
#endif
	}

	return success;
}

template<class T> void FubiImageProcessing::drawFingerCountImage(unsigned int userID, bool leftHand, T* outputImage, int width, int height, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, int renderOptions, const T& maxValue)
{
#ifdef FUBI_USE_OPENCV
	FubiUser* user = Fubi::getUser(userID);
	if (user)
	{
		// Check scaling
		Fubi::Vec3f ImageToDepthScale(1.0f, 1.0f, 1.0f);
		int depthWidth = 0, depthHeight = 0;
		getDepthResolution(depthWidth, depthHeight);
		if (depthWidth > 0 && depthHeight > 0)
		{
			ImageToDepthScale.x = (float)width/(float)depthWidth;
			ImageToDepthScale.y = (float)height/(float)depthHeight;
		}

		// Copy the finger count images to the wanted place
		const FingerCountImageData* fCountD = user->getFingerCountImageData(leftHand);
		if (Fubi::getCurrentTime() - fCountD->timeStamp < 0.33f && fCountD->image)
		{
			const FingerCountImageData* fCIData = fCountD;
			Mat* fCImage = fCIData->image;
			Size size = fCImage->size();

			Size scaledSize = Size(ftoi_r(ImageToDepthScale.x*size.width), ftoi_r(ImageToDepthScale.y*size.height));
			Point scaledPoint = Point(ftoi_r(ImageToDepthScale.x*fCIData->posX), ftoi_r(ImageToDepthScale.y*fCIData->posY));

			double dMaxValue = (double)maxValue;
			int convType = CV_MAKE_TYPE(IPL2CV_DEPTH(depth), 4);
			Mat fImage(scaledSize, convType);

			// Convert the finger count image to an image with correct depth and size
			// And copy the finger count image into the correct place with respect to the channels
			if (maxValue != Math::MaxUChar8)
			{
				Mat convImage(size, convType);
				fCImage->convertTo(convImage, IPL2CV_DEPTH(depth),  maxValue / (double)Math::MaxUChar8);
				resize(convImage, fImage, scaledSize);
			}
			else
				resize(*fCImage, fImage, scaledSize);

			// Now add the shapes to the image
			for (int j = 0; j < scaledSize.height; ++j)
			{
				T* pfImage;
				T* pDestImage;
				T* pLineStart = ((T*)fImage.data) + j*scaledSize.width*4;
				T* pDestLineStart = outputImage + (j+scaledPoint.y)*width*numChannels;
				for(int i = 0; i < scaledSize.width; ++i)
				{
					pfImage = pLineStart + i*4;
					pDestImage = pDestLineStart + (i+scaledPoint.x)*numChannels;

					if (pfImage[3] > 0)
					{
						float alpha = (float)(pfImage[3] / dMaxValue);
						float dAlpha = 1.0f - alpha;
						if (numChannels == 1)
						{
							T greyValue = roundNonFloat<T>(0.114f*pfImage[0] + 0.587f*pfImage[1] + 0.299f*pfImage[2]);
							pDestImage[0] = roundNonFloat<T>(alpha*greyValue + dAlpha*pDestImage[0]);
						}
						else
						{
							if (renderOptions & RenderOptions::SwapRAndB)
							{
								pDestImage[0] = roundNonFloat<T>(alpha*pfImage[2] + dAlpha*pDestImage[2]);
								pDestImage[2] = roundNonFloat<T>(alpha*pfImage[0] + dAlpha*pDestImage[0]);
							}
							else
							{
								pDestImage[0] = roundNonFloat<T>(alpha*pfImage[0] + dAlpha*pDestImage[0]);
								pDestImage[2] = roundNonFloat<T>(alpha*pfImage[2] + dAlpha*pDestImage[2]);
							}
							pDestImage[1] = roundNonFloat<T>(alpha*pfImage[1] + dAlpha*pDestImage[1]);

							if (numChannels == 4)
							{
								pDestImage[3] = max(pDestImage[3], pfImage[3]);
							}
						}
					}
				}
			}
		}
	}
#else
	static double lastWarning = -99;
	if (Fubi::currentTime() - lastWarning > 10)
	{
		Fubi_logWrn("Sorry, can't draw finger count image without FUBI_USE_OPENCV defined in the FubiConfig.h.\n");
		lastWarning = Fubi::currentTime();
	}
#endif
}

void FubiImageProcessing::drawTrackingInfo(FubiISensor* sensor, unsigned char* outputImage, int width, int height, ImageNumChannels::Channel numChannels, ImageDepth::Depth depth, CoordinateType::Type coordinateType, int renderOptions, int jointsToRender)
{
	FubiUser** users;
	unsigned short numUsers = Fubi::getCurrentUsers(&users);

	if (numUsers > 0)
	{
		// Render the user shapes
		if (renderOptions & RenderOptions::Shapes)
		{
			if (sensor)
			{
				// Get user labels
				const unsigned short* pImageStart = sensor->getUserLabelData(coordinateType);
				if (pImageStart)
				{
					// Check for tracking/calibration per id
					bool trackedIDs[Fubi::MaxUsers];
					memset(trackedIDs, 0, sizeof(bool)*(Fubi::MaxUsers));
					for (unsigned short i = 0; i < numUsers; ++i)
					{
						trackedIDs[users[i]->id()] = users[i]->isTracked();
					}

					// Check scaling
					Fubi::Vec3f ImageToDepthScale(1.0f, 1.0f, 1.0f);

					int depthWidth = 0, depthHeight = 0;
					getDepthResolution(depthWidth, depthHeight);
					if (depthWidth > 0 && depthHeight > 0)
					{
						ImageToDepthScale.x = (float)(depthWidth - 1) / (float)(width - 1);
						ImageToDepthScale.y = (float)(depthHeight - 1) / (float)(height - 1);
					}

					if (depth == ImageDepth::D16)
					{
						// Now add the shapes to the image
						unsigned short* outputData = (unsigned short*)outputImage;
#pragma omp parallel for schedule(dynamic)
						for (int j = 0; j < height; ++j)
						{
							unsigned short* pDestImage = outputData + j*width*numChannels;
							unsigned int nColorID;
							const unsigned short* pLineStart = pImageStart + ftoi_r(j*ImageToDepthScale.y)*depthWidth;
							const unsigned short* pLabels;
							for (int i = 0; i < width; ++i)
							{
								pLabels = pLineStart + ftoi_r(i*ImageToDepthScale.x);
								if (*pLabels != 0)
								{
									// Add user shapes (tracked users highlighted)
									nColorID = (*pLabels) % (MaxUsers + 1);

									if (renderOptions & RenderOptions::SwapRAndB)
										pDestImage[0] = (unsigned short)(pDestImage[0] * m_colors[nColorID][2]);
									else
										pDestImage[0] = (unsigned short)(pDestImage[0] * m_colors[nColorID][0]);

									if (numChannels > 1)
									{
										pDestImage[1] = (unsigned short)(pDestImage[1] * m_colors[nColorID][1]);
										if (renderOptions & RenderOptions::SwapRAndB)
											pDestImage[2] = (unsigned short)(pDestImage[2] * m_colors[nColorID][0]);
										else
											pDestImage[2] = (unsigned short)(pDestImage[2] * m_colors[nColorID][2]);
										if (numChannels == 4)
										{
											if (trackedIDs[(*pLabels)] || (*pLabels) == 0)
												pDestImage[3] = Math::MaxUShort16;
											else
												pDestImage[3] = Math::MaxUShort16 / 2;
										}
									}
								}
								pDestImage += numChannels;
							}
						}
					}
					else if (depth == ImageDepth::F32)
					{
						// Now add the shapes to the image
						float* dstImage = (float*)outputImage;
#pragma omp parallel for schedule(dynamic)
						for (int j = 0; j < height; ++j)
						{
							float* pDestImage = dstImage + j*width*numChannels;
							unsigned int nColorID;
							const unsigned short* pLineStart = pImageStart + ftoi_r(j*ImageToDepthScale.y)*depthWidth;
							const unsigned short* pLabels;
							for (int i = 0; i < width; ++i)
							{
								pLabels = pLineStart + ftoi_r(i*ImageToDepthScale.x);
								if (*pLabels != 0)
								{
									// Add user shapes (tracked users highlighted)
									nColorID = (*pLabels) % (MaxUsers + 1);

									if (renderOptions & RenderOptions::SwapRAndB)
										pDestImage[0] = pDestImage[0] * m_colors[nColorID][2];
									else
										pDestImage[0] = pDestImage[0] * m_colors[nColorID][0];

									if (numChannels > 1)
									{
										pDestImage[1] = pDestImage[1] * m_colors[nColorID][1];
										if (renderOptions & RenderOptions::SwapRAndB)
											pDestImage[2] = pDestImage[2] * m_colors[nColorID][0];
										else
											pDestImage[2] = pDestImage[2] * m_colors[nColorID][2];
										if (numChannels == 4)
										{
											if (trackedIDs[(*pLabels)] || (*pLabels) == 0)
												pDestImage[3] = 1.0f;
											else
												pDestImage[3] = 0.5f;
										}
									}
								}
								pDestImage += numChannels;
							}
						}
					}
					else
					{
						// Now add the shapes to the image
#pragma omp parallel for schedule(dynamic)
						for (int j = 0; j < height; ++j)
						{
							unsigned char* pDestImage = (unsigned char*)outputImage + j*width*numChannels;
							unsigned int nColorID;
							const unsigned short* pLineStart = pImageStart + ftoi_r(j*ImageToDepthScale.y)*depthWidth;
							const unsigned short* pLabels;
							for (int i = 0; i < width; ++i)
							{
								pLabels = pLineStart + ftoi_r(i*ImageToDepthScale.x);
								if (*pLabels != 0)
								{
									// Add user shapes (tracked users highlighted)
									nColorID = (*pLabels) % (MaxUsers + 1);
									if (renderOptions & RenderOptions::SwapRAndB)
										pDestImage[0] = (unsigned char)(pDestImage[0] * m_colors[nColorID][2]);
									else
										pDestImage[0] = (unsigned char)(pDestImage[0] * m_colors[nColorID][0]);
									if (numChannels > 1)
									{
										pDestImage[1] = (unsigned char)(pDestImage[1] * m_colors[nColorID][1]);
										if (renderOptions & RenderOptions::SwapRAndB)
											pDestImage[2] = (unsigned char)(pDestImage[2] * m_colors[nColorID][0]);
										else
											pDestImage[2] = (unsigned char)(pDestImage[2] * m_colors[nColorID][2]);
										if (numChannels == 4)
										{
											if (trackedIDs[(*pLabels)] || (*pLabels) == 0)
												pDestImage[3] = Math::MaxUChar8;
											else
												pDestImage[3] = 128;
										}
									}
								}
								pDestImage += numChannels;
							}
						}
					}
				}
			}
		}
	}

#ifdef FUBI_USE_OPENCV
	// Additional render options require OpenCV
	// Create image header for current data
	Mat image(Size(width, height), CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels), outputImage);

	// Draw tracking info for all current users
	for (unsigned short i = 0; i < numUsers; ++i)
	{
		FubiUser* user = users[i];
		if (user->currentTrackingData()->jointPositions[SkeletonJoint::TORSO].m_confidence > 0
			&& user->currentTrackingData()->jointPositions[SkeletonJoint::TORSO].m_position.z > 100.0f)
		{
			// First render finger shapes if wanted
			if (renderOptions & RenderOptions::FingerShapes)
			{
				if (depth == ImageDepth::D16)
				{
					// For left
					drawFingerCountImage(user->id(), true, (unsigned short*)outputImage, width, height, numChannels, depth, renderOptions, Math::MaxUShort16);
					// and right hand
					drawFingerCountImage(user->id(), false, (unsigned short*)outputImage, width, height, numChannels, depth, renderOptions, Math::MaxUShort16);
				}
				else if (depth == ImageDepth::D8)
				{
					// For left
					drawFingerCountImage(user->id(), true, outputImage, width, height, numChannels, depth, renderOptions, Math::MaxUChar8);
					// and right hand
					drawFingerCountImage(user->id(), false, outputImage, width, height, numChannels, depth, renderOptions, Math::MaxUChar8);
				}
				else
				{
					// For left
					drawFingerCountImage(user->id(), true, (float*)outputImage, width, height, numChannels, depth, renderOptions, 1.0f);
					// and right hand
					drawFingerCountImage(user->id(), false, (float*)outputImage, width, height, numChannels, depth, renderOptions, 1.0f);
				}
			}

			// Now do the main rendering part
			drawUserSkeletonTrackingInfo(user, &image, coordinateType, renderOptions, jointsToRender);

			// Add detailed face shapes if requested
			if (renderOptions & RenderOptions::DetailedFaceShapes)
			{
				if (sensor && user->isTracked())
				{
					float maxValue = Math::MaxUChar8;
					if (depth == ImageDepth::D16)
						maxValue = Math::MaxUShort16;
					else if (depth == ImageDepth::F32)
						maxValue = 1.0f;

					const std::vector<Fubi::Vec3f>* vertices = 0x0;
					const std::vector<Fubi::Vec3f>* triangleIndices = 0x0;
					if (sensor->getFacePoints(user->id(), &vertices, &triangleIndices))
					{
						float r, g, b;
						getColorForUserID(user->id(), r, g, b);
						auto end = triangleIndices->end();
						for (auto iter = triangleIndices->begin(); iter != end; ++iter)
						{
							Fubi::Vec3f pos1 = Fubi::convertCoordinates((*vertices)[ftoi_r(iter->x)], CoordinateType::REAL_WORLD, coordinateType);
							Fubi::Vec3f pos2 = Fubi::convertCoordinates((*vertices)[ftoi_r(iter->y)], CoordinateType::REAL_WORLD, coordinateType);
							Fubi::Vec3f pos3 = Fubi::convertCoordinates((*vertices)[ftoi_r(iter->z)], CoordinateType::REAL_WORLD, coordinateType);
							Point point1 = Point(ftoi_r(pos1.x), ftoi_r(pos1.y));
							Point point2 = Point(ftoi_r(pos2.x), ftoi_r(pos2.y));
							Point point3 = Point(ftoi_r(pos3.x), ftoi_r(pos3.y));
							line(image, point1, point2, Scalar(maxValue*(1.0f - b), maxValue*(1.0f - g), maxValue*(1.0f - r), maxValue));
							line(image, point2, point3, Scalar(maxValue*(1.0f - b), maxValue*(1.0f - g), maxValue*(1.0f - r), maxValue));
							line(image, point3, point1, Scalar(maxValue*(1.0f - b), maxValue*(1.0f - g), maxValue*(1.0f - r), maxValue));
						}
					}
				}
			}
		}
	}

	// Tracking info for recorded user/hand
	int currentFrameID;
	if (Fubi::isPlayingSkeletonData(&currentFrameID))
	{
		FubiUser* user = Fubi::getUser(PlaybackUserID);
		if (user && user->currentTrackingData()->jointPositions[SkeletonJoint::TORSO].m_confidence > 0
			&& user->currentTrackingData()->jointPositions[SkeletonJoint::TORSO].m_position.z > 100.0f)
			drawUserSkeletonTrackingInfo(user, &image, coordinateType, renderOptions, jointsToRender);
		FubiHand* hand = Fubi::getHand(PlaybackHandID);
		if (hand)
			drawHandSkeletonTrackingInfo(hand, &image, coordinateType, renderOptions, jointsToRender);
		// Add frame ID
		stringstream ss;
		ss << "Playing Frame ID: ";
		ss.width(3);
		ss << currentFrameID;
		// print text
		float r, g, b;
		getColorForUserID(user ? PlaybackUserID : PlaybackHandID, r, g, b);
		if (renderOptions & RenderOptions::SwapRAndB)
			swap(r, b);
		// Print text in opposite color
		drawText(&image, ss.str(), 0, 13.0f, 1 - r, 1 - g, 1 - b, false);
	}

	// Draw tracking info for all hands
	unsigned short numHands = Fubi::getNumHands();
	FubiCore* core = FubiCore::getInstance();
	if (core && numHands > 0)
	{
		for (unsigned short i = 0; i < numHands; ++i)
		{
			FubiHand* hand = core->getHand(core->getHandID(i));
			if (hand)
			{
				drawHandSkeletonTrackingInfo(hand, &image, coordinateType, renderOptions, jointsToRender);
			}
		}
	}
#endif
}

void FubiImageProcessing::drawUserSkeletonTrackingInfo(FubiUser* user, cv::Mat* pImage,
	Fubi::CoordinateType::Type coordinateType, int renderOptions, int jointsToRender)
{
#ifdef FUBI_USE_OPENCV
	if (user->isTracked())
	{
		if (renderOptions
			& (RenderOptions::Skeletons | RenderOptions::GlobalOrientCaptions | RenderOptions::LocalOrientCaptions
			| RenderOptions::GlobalPosCaptions | RenderOptions::LocalPosCaptions))
		{
			// Draw the user's skeleton
			if (jointsToRender & RenderOptions::HEAD)
				drawLimb(user->id(), SkeletonJoint::NECK, SkeletonJoint::HEAD, pImage, coordinateType, renderOptions);

			if (jointsToRender & RenderOptions::NECK)
				drawLimb(user->id(), SkeletonJoint::LEFT_SHOULDER, SkeletonJoint::NECK, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::LEFT_ELBOW)
				drawLimb(user->id(), SkeletonJoint::LEFT_SHOULDER, SkeletonJoint::LEFT_ELBOW, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::LEFT_WRIST)
				drawLimb(user->id(), SkeletonJoint::LEFT_ELBOW, SkeletonJoint::LEFT_WRIST, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::LEFT_HAND)
				drawLimb(user->id(), SkeletonJoint::LEFT_WRIST, SkeletonJoint::LEFT_HAND, pImage, coordinateType, renderOptions);

			if (jointsToRender & RenderOptions::RIGHT_SHOULDER)
				drawLimb(user->id(), (jointsToRender & RenderOptions::NECK) ? SkeletonJoint::NECK : SkeletonJoint::LEFT_SHOULDER, SkeletonJoint::RIGHT_SHOULDER, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::RIGHT_ELBOW)
				drawLimb(user->id(), SkeletonJoint::RIGHT_SHOULDER, SkeletonJoint::RIGHT_ELBOW, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::RIGHT_WRIST)
				drawLimb(user->id(), SkeletonJoint::RIGHT_ELBOW, SkeletonJoint::RIGHT_WRIST, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::RIGHT_HAND)
				drawLimb(user->id(), SkeletonJoint::RIGHT_WRIST, SkeletonJoint::RIGHT_HAND, pImage, coordinateType, renderOptions);

			if (jointsToRender & RenderOptions::LEFT_SHOULDER)
				drawLimb(user->id(), SkeletonJoint::TORSO, SkeletonJoint::LEFT_SHOULDER, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::TORSO)
				drawLimb(user->id(), SkeletonJoint::RIGHT_SHOULDER, SkeletonJoint::TORSO, pImage, coordinateType, renderOptions);

			if (jointsToRender & RenderOptions::LEFT_HIP)
				drawLimb(user->id(), SkeletonJoint::TORSO, SkeletonJoint::LEFT_HIP, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::LEFT_KNEE)
				drawLimb(user->id(), SkeletonJoint::LEFT_HIP, SkeletonJoint::LEFT_KNEE, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::LEFT_ANKLE)
				drawLimb(user->id(), SkeletonJoint::LEFT_KNEE, SkeletonJoint::LEFT_ANKLE, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::LEFT_FOOT)
				drawLimb(user->id(), SkeletonJoint::LEFT_ANKLE, SkeletonJoint::LEFT_FOOT, pImage, coordinateType, renderOptions);

			if (jointsToRender & RenderOptions::RIGHT_HIP)
				drawLimb(user->id(), SkeletonJoint::TORSO, SkeletonJoint::RIGHT_HIP, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::RIGHT_KNEE)
				drawLimb(user->id(), SkeletonJoint::RIGHT_HIP, SkeletonJoint::RIGHT_KNEE, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::RIGHT_ANKLE)
				drawLimb(user->id(), SkeletonJoint::RIGHT_KNEE, SkeletonJoint::RIGHT_ANKLE, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::RIGHT_FOOT)
				drawLimb(user->id(), SkeletonJoint::RIGHT_ANKLE, SkeletonJoint::RIGHT_FOOT, pImage, coordinateType, renderOptions);

			if (jointsToRender & RenderOptions::WAIST)
			{
				drawLimb(user->id(), SkeletonJoint::LEFT_HIP, SkeletonJoint::WAIST, pImage, coordinateType, renderOptions);
				// Never draw joint captions for this one as we already have...
				drawLimb(user->id(), SkeletonJoint::WAIST, SkeletonJoint::RIGHT_HIP, pImage, coordinateType,
					(renderOptions & ~(RenderOptions::GlobalOrientCaptions | RenderOptions::LocalOrientCaptions
					| RenderOptions::GlobalPosCaptions | RenderOptions::LocalPosCaptions)));
			}
			else if (jointsToRender & (RenderOptions::LEFT_HIP | RenderOptions::RIGHT_HIP))
			{
				// Never draw joint captions for this one as we already have...
				drawLimb(user->id(), SkeletonJoint::LEFT_HIP, SkeletonJoint::RIGHT_HIP, pImage, coordinateType,
					(renderOptions & ~(RenderOptions::GlobalOrientCaptions | RenderOptions::LocalOrientCaptions
					| RenderOptions::GlobalPosCaptions | RenderOptions::LocalPosCaptions)));
			}

			if (jointsToRender & RenderOptions::FACE_NOSE)
				drawLimb(user->id(), SkeletonJoint::HEAD, SkeletonJoint::FACE_NOSE, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::FACE_RIGHT_EAR)
				drawLimb(user->id(), SkeletonJoint::FACE_CHIN, SkeletonJoint::FACE_RIGHT_EAR, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::FACE_FOREHEAD)
				drawLimb(user->id(), SkeletonJoint::FACE_RIGHT_EAR, SkeletonJoint::FACE_FOREHEAD, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::FACE_LEFT_EAR)
				drawLimb(user->id(), SkeletonJoint::FACE_FOREHEAD, SkeletonJoint::FACE_LEFT_EAR, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::FACE_CHIN)
				drawLimb(user->id(), SkeletonJoint::FACE_LEFT_EAR, SkeletonJoint::FACE_CHIN, pImage, coordinateType, renderOptions);
		}
	}
	if (renderOptions & RenderOptions::UserCaptions)
	{
		// Create user labels
		stringstream ss;
		ss.setf(ios::fixed, ios::floatfield);
		ss.precision(0);

		const Fubi::TrackingData* data = user->currentTrackingData();
		if (renderOptions & RenderOptions::UseFilteredValues)
		{
			data = user->currentFilteredTrackingData();
		}
		Fubi::Vec3f pos = data->jointPositions[SkeletonJoint::TORSO].m_position;

		if (user->isTracked())
		{
			// Tracking
			ss << "User" << user->id() << "@(" << pos.x << "," << pos.y << "," << pos.z << ") Tracking";
		}
		else
		{
			// Not yet tracked = Calibrating
			ss << "User" << user->id() << "@(" << pos.x << "," << pos.y << "," << pos.z << ") Calibrating";
		}

		// print text
		float r, g, b;
		getColorForUserID(user->id(), r, g, b);
		if (renderOptions & RenderOptions::SwapRAndB)
			swap(r, b);
		Fubi::Vec3f posProjective = Fubi::convertCoordinates(pos, CoordinateType::REAL_WORLD, coordinateType);
		// Print text in opposite color
		drawText(pImage, ss.str(), posProjective.x, posProjective.y, 1 - r, 1 - g, 1 - b);
	}

	if (renderOptions & RenderOptions::BodyMeasurements)
	{
		drawBodyMeasurement(user->id(), SkeletonJoint::RIGHT_FOOT, SkeletonJoint::HEAD, BodyMeasurement::BODY_HEIGHT,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::WAIST, SkeletonJoint::NECK, BodyMeasurement::TORSO_HEIGHT,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::RIGHT_SHOULDER, SkeletonJoint::LEFT_SHOULDER, BodyMeasurement::SHOULDER_WIDTH,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::RIGHT_HIP, SkeletonJoint::LEFT_HIP, BodyMeasurement::HIP_WIDTH,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::RIGHT_SHOULDER, SkeletonJoint::RIGHT_HAND, BodyMeasurement::ARM_LENGTH,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::LEFT_SHOULDER, SkeletonJoint::LEFT_ELBOW, BodyMeasurement::UPPER_ARM_LENGTH,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::LEFT_ELBOW, SkeletonJoint::LEFT_HAND, BodyMeasurement::LOWER_ARM_LENGTH,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::RIGHT_FOOT, SkeletonJoint::RIGHT_HIP, BodyMeasurement::LEG_LENGTH,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::LEFT_HIP, SkeletonJoint::LEFT_KNEE, BodyMeasurement::UPPER_LEG_LENGTH,
			pImage, coordinateType, renderOptions);
		drawBodyMeasurement(user->id(), SkeletonJoint::LEFT_KNEE, SkeletonJoint::LEFT_FOOT, BodyMeasurement::LOWER_LEG_LENGTH,
			pImage, coordinateType, renderOptions);
	}
#endif
}

void FubiImageProcessing::drawHandSkeletonTrackingInfo(class FubiHand* hand, cv::Mat* pImage,
	Fubi::CoordinateType::Type coordinateType, int renderOptions, int jointsToRender)
{
#ifdef FUBI_USE_OPENCV
	if (hand->isTracked())
	{
		if (renderOptions
			& (RenderOptions::Skeletons | RenderOptions::GlobalOrientCaptions | RenderOptions::LocalOrientCaptions
			| RenderOptions::GlobalPosCaptions | RenderOptions::LocalPosCaptions))
		{
			// Draw the fingers
			/*if (jointsToRender & RenderOptions::PALM)
			drawFinger(hand->id(), SkeletonHandJoint::PALM, outputImage, width, height, numChannels, depth, renderOptions);*/
			if (jointsToRender & RenderOptions::THUMB)
				drawFinger(hand->id(), SkeletonHandJoint::THUMB, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::INDEX)
				drawFinger(hand->id(), SkeletonHandJoint::INDEX, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::MIDDLE)
				drawFinger(hand->id(), SkeletonHandJoint::MIDDLE, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::RING)
				drawFinger(hand->id(), SkeletonHandJoint::RING, pImage, coordinateType, renderOptions);
			if (jointsToRender & RenderOptions::PINKY)
				drawFinger(hand->id(), SkeletonHandJoint::PINKY, pImage, coordinateType, renderOptions);
		}
	}

	if (renderOptions & RenderOptions::UserCaptions)
	{
		// Create user labels
		stringstream ss;
		ss.setf(ios::fixed, ios::floatfield);
		ss.precision(0);

		const Fubi::TrackingData* data = hand->currentTrackingData();
		if (renderOptions & RenderOptions::UseFilteredValues)
		{
			data = hand->currentFilteredTrackingData();
		}
		Fubi::Vec3f pos = data->jointPositions[SkeletonHandJoint::PALM].m_position;

		if (renderOptions & RenderOptions::LocalOrientCaptions)
		{
			Fubi::Vec3f rot = data->localJointOrientations[SkeletonHandJoint::PALM].m_orientation.getRot();
			ss << "Hand" << hand->id() << "@(" << rot.x << "," << rot.y << "," << rot.z;
		}
		else if (renderOptions & RenderOptions::GlobalOrientCaptions)
		{
			Fubi::Vec3f rot = data->jointOrientations[SkeletonHandJoint::PALM].m_orientation.getRot();
			ss << "Hand" << hand->id() << "@(" << rot.x << "," << rot.y << "," << rot.z;
		}
		else if (renderOptions & RenderOptions::LocalPosCaptions)
		{
			const Fubi::Vec3f& jPos = data->localJointPositions[SkeletonHandJoint::PALM].m_position;
			ss << "Hand" << hand->id() << "@(" << jPos.x << "," << jPos.y << "," << jPos.z;
		}
		else
		{
			ss << "Hand" << hand->id() << "@(" << pos.x << "," << pos.y << "," << pos.z;
		}

		if (hand->isTracked())
		{
			// Tracking
			ss << ") Tracking";
		}
		else
		{
			// Not yet tracked? Should not happen for hands
			ss << ") Calibrating";
		}

		// print text
		float r, g, b;
		getColorForUserID(hand->id(), r, g, b);
		if (renderOptions & RenderOptions::SwapRAndB)
			swap(r, b);
		Fubi::Vec3f posProjective = Fubi::convertCoordinates(pos, CoordinateType::REAL_WORLD, coordinateType);
		// Print text in opposite color
		drawText(pImage, ss.str(), posProjective.x, posProjective.y, 1 - r, 1 - g, 1 - b);
	}
#endif
}


bool FubiImageProcessing::drawDepthImage(FubiISensor* sensor, unsigned char* outputImage, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, Fubi::DepthImageModification::Modification depthModifications, int renderOptions)
{
	if (sensor)
	{
		// Get options for the resolution
		Fubi::StreamOptions options = sensor->getStreamOptions(ImageType::Depth);

		// Get depth data
		const unsigned short* pDepth = sensor->getDepthData();

		if (options.isValid() && pDepth != 0)
		{
			if (depthModifications == DepthImageModification::Raw
				&& (renderOptions == RenderOptions::None || (renderOptions & RenderOptions::Background) != 0)
				&& numChannels == ImageNumChannels::C1 && depth == ImageDepth::D16)
			{
				// Catch the special case that the source and target formats are the same
				memcpy(outputImage, pDepth, options.m_width*options.m_height*sizeof(unsigned short));
				return true;
			}

			unsigned short maxDepth = MaxDepth;

			if (depthModifications == DepthImageModification::UseHistogram)
			{
				// Calculate depth histogram
				unsigned short nIndex = 0;
				float nNumberOfPoints = 0;
				memset(m_depthHist, 0, m_lastMaxDepth*sizeof(float));
				maxDepth = 0;
				unsigned short nValue = 0;
				for (unsigned short nY=0; nY<options.m_height; ++nY)
				{
					for (unsigned short nX=0; nX<options.m_width; ++nX)
					{
						nValue = *pDepth;
						if (nValue != 0)
						{
							m_depthHist[nValue]++;
							nNumberOfPoints++;
							if (nValue > maxDepth)
								maxDepth = nValue;
						}
						pDepth++;
					}
				}
				m_lastMaxDepth = maxDepth;

				if (nNumberOfPoints > 0 && maxDepth > 0)
				{
					// Already add first value
					m_depthHist[1] += m_depthHist[0];
					// Calculate the rest
					for (nIndex=2; nIndex<maxDepth; ++nIndex)
					{
						m_depthHist[nIndex] += m_depthHist[nIndex-1];
						m_depthHist[nIndex-1] = 1.0f - (m_depthHist[nIndex-1] / nNumberOfPoints);
					}
					// And divide the last one
					m_depthHist[maxDepth] = 1.0f - (m_depthHist[maxDepth] / nNumberOfPoints);
				}
			}
			else if (depthModifications == DepthImageModification::StretchValueRange
				|| depthModifications == DepthImageModification::ConvertToRGB)
			{
				maxDepth = 0;
				// Only calculate maxDepth
				unsigned short nValue = 0;
				for (unsigned short nY=0; nY<options.m_height; ++nY)
				{
					for (unsigned short nX=0; nX<options.m_width; ++nX)
					{
						nValue = *pDepth;
						if (nValue > maxDepth)
							maxDepth = nValue;
						pDepth++;
					}
				}
			}

			// Get user labels
			if (sensor->getUserLabelData(CoordinateType::DEPTH) == 0x0)
			{
				// No users so never render shapes, but the background
				renderOptions &= ~RenderOptions::Shapes;
				renderOptions |= RenderOptions::Background;
			}


			const unsigned short * labels = 0;
			if (renderOptions & RenderOptions::Shapes)
				labels = sensor->getUserLabelData(CoordinateType::DEPTH);
			bool forceBackgroundRendering =  renderOptions == RenderOptions::None || renderOptions == RenderOptions::SwapRAndB
				|| renderOptions == RenderOptions::UseFilteredValues || (renderOptions & RenderOptions::Background) != 0;
			if (depth == ImageDepth::D16)
			{
				convertDepthImage(sensor->getDepthData(), (unsigned short*) outputImage, labels,
					options.m_width, options.m_height, numChannels, depthModifications,
					Math::MaxUShort16, -1.0f, (float)maxDepth, forceBackgroundRendering, (renderOptions & RenderOptions::SwapRAndB) != 0);
			}
			else if (depth == ImageDepth::F32)
			{
				const float scale = 1.0f / ( (depthModifications == DepthImageModification::Raw) ? MaxDepth : Math::MaxUShort16);
				convertDepthImage(sensor->getDepthData(), (float*) outputImage, labels,
					options.m_width, options.m_height, numChannels,	depthModifications,
					1.0f, scale, (float)maxDepth, forceBackgroundRendering, (renderOptions & RenderOptions::SwapRAndB) != 0);
			}
			else
			{
				const float scale = Math::MaxUChar8 / (float)( (depthModifications == DepthImageModification::Raw) ? MaxDepth : Math::MaxUShort16);
				convertDepthImage(sensor->getDepthData(), outputImage, labels,
					options.m_width, options.m_height, numChannels,	depthModifications,
					Math::MaxUChar8, scale, (float)maxDepth, forceBackgroundRendering, (renderOptions & RenderOptions::SwapRAndB) != 0);
			}
			return true;
		}
	}
	return false;
}

template<class T> void FubiImageProcessing::convertDepthImage(const unsigned short* srcImage, T* dstImage, const unsigned short* userlabelImage,
	int width, int height, Fubi::ImageNumChannels::Channel numChannels, Fubi::DepthImageModification::Modification depthModifications,
	const T& maxValue, float scaleFac, float maxDepth, bool forceBackgroundRendering, bool swapRnB)
{
	static const float fMaxUShort16 = (float) Math::MaxUShort16;
	const float stretchFac = Math::MaxUShort16 / (float) maxDepth;
	const bool multipleChannels = numChannels > 1;
	const bool fourChannels = numChannels == 4;
	const bool applyScale = scaleFac > 0;

#pragma omp parallel for schedule(dynamic)
	for (int j = 0; j < height; ++j)
	{
		unsigned short nValue, nValue1, nValue2;
		int step = j*width;
		const unsigned short* pSrc = srcImage + step;
		const unsigned short* pLabels;
		if (userlabelImage)
			pLabels = userlabelImage + step;
		T* pDest = dstImage + step*numChannels;
		for (int i = 0; i < width; ++i)
		{
			if (forceBackgroundRendering || (userlabelImage && *pLabels != 0))
			{
				nValue = *pSrc;
				if (depthModifications == DepthImageModification::UseHistogram)
				{
					nValue = (unsigned short) ftoi_r(fMaxUShort16 * m_depthHist[nValue]);
					nValue2 = nValue1 = nValue;
				}
				else if (depthModifications == DepthImageModification::ConvertToRGB)
				{
					if (nValue == 0)
					{
						nValue2 = nValue1 = 0;
					}
					else
					{
						unsigned short lb = nValue << 7;

						switch (nValue >> 9)
						{
						case 0:
							nValue = Math::MaxUShort16;
							nValue1 = Math::MaxUShort16-lb;
							nValue2 = Math::MaxUShort16-lb;
							break;
						case 1:
							nValue = Math::MaxUShort16;
							nValue1 = lb;
							nValue2 = 0;
							break;
						case 2:
							nValue = Math::MaxUShort16-lb;
							nValue1 = Math::MaxUShort16;
							nValue2 = 0;
							break;
						case 3:
							nValue = 0;
							nValue1 = Math::MaxUShort16;
							nValue2 = lb;
							break;
						case 4:
							nValue = 0;
							nValue1 = Math::MaxUShort16-lb;
							nValue2 = Math::MaxUShort16;
							break;
						case 5:
							nValue = 0;
							nValue1 = 0;
							nValue2 = Math::MaxUShort16-lb;
							break;
						default:
							nValue2 = nValue1 = nValue = (unsigned short)(nValue-3071);
							break;
						}

						if (swapRnB)
						{
							swap(nValue, nValue2);
						}
					}
				}
				else if (depthModifications == DepthImageModification::StretchValueRange)
				{
					nValue2 = nValue1 = nValue = (unsigned short) ftoi_r(stretchFac * nValue);
				}
				else
				{
					nValue2 = nValue1 = nValue;
				}

				if (applyScale)
				{
					pDest[0] = roundNonFloat<T>(nValue * scaleFac);
					if (multipleChannels)
					{
						if (depthModifications == DepthImageModification::ConvertToRGB)
						{
							// Only here, we have different values
							pDest[1] = roundNonFloat<T>(scaleFac * nValue1);
							pDest[2] = roundNonFloat<T>(scaleFac * nValue2);
						}
						else
							pDest[1] = pDest[2] = pDest[0];
						if (fourChannels)
							pDest[3] = maxValue;
					}
				}
				else
				{
					pDest[0] = (T) nValue;
					if (multipleChannels)
					{
						pDest[1] = (T) nValue1;
						pDest[2] = (T) nValue2;
						if (fourChannels)
							pDest[3] = maxValue;
					}
				}
			}
			else
			{
				// No value to put here, so make it black
				pDest[0] = 0;
				if (multipleChannels)
				{
					pDest[1] = 0;
					pDest[2] = 0;
					if (fourChannels)
						pDest[3] = 0;
				}
			}


			pSrc++;
			if (userlabelImage)
				pLabels++;
			pDest += numChannels;
		}
	}
}

bool FubiImageProcessing::saveImage(FubiISensor* sensor, const char* fileName, int jpegQuality,
	Fubi::ImageType::Type type, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth,
	int renderOptions /*= (Fubi::RenderOptions::Shapes | Fubi::RenderOptions::Skeletons | Fubi::RenderOptions::UserCaptions)*/,
	int jointsToRender /*= RenderOptions::ALL_JOINTS*/,
	Fubi::DepthImageModification::Modification depthModifications /*= Fubi::DepthImageModification::UseHistogram*/,
	unsigned int userId /*= 0*/, SkeletonJoint::Joint jointOfInterest /*= SkeletonJoint::NUM_JOINTS*/)
{
	bool success = false;
#ifdef FUBI_USE_OPENCV
	if (sensor)
	{
		Mat image;
		float threshold = 0;
		Fubi::StreamOptions options;

		if (type == ImageType::Color)
		{
			options = sensor->getStreamOptions(ImageType::Color);
			if (options.isValid())
			{
				image.create(Size(options.m_width, options.m_height), CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels));
				// Color image has by default the channel order RGB
				// As OpenCV wants BGR as the default, we use the SwapRAndB option in the opposite way
				success = drawColorImage(sensor, image.data, numChannels, depth, (renderOptions & RenderOptions::SwapRAndB) == 0);
			}
		}
		else if (type == ImageType::IR)
		{
			options = sensor->getStreamOptions(ImageType::IR);
			if (options.isValid())
			{
				image.create(Size(options.m_width, options.m_height), CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels));
				if (depth == ImageDepth::D16)
					success = drawIRImage(sensor, (unsigned short*)image.data, numChannels, Math::MaxUShort16);
				else if (depth == ImageDepth::D8)
					success = drawIRImage(sensor, (unsigned char*)image.data, numChannels, Math::MaxUChar8);
				else
					success = drawIRImage(sensor, (float*)image.data, numChannels, 1.0f);
			}
		}
		else
		{
			options = sensor->getStreamOptions(ImageType::Depth);
			if (options.isValid())
			{
				image.create(Size(options.m_width, options.m_height), CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels));
				if ((userId != 0) && depthModifications == DepthImageModification::Raw)
				{
					if (jointOfInterest == SkeletonJoint::NUM_JOINTS || jointOfInterest == SkeletonJoint::TORSO)
						threshold = 400;
					else if (jointOfInterest == SkeletonJoint::HEAD)
						threshold = 200;
					else
						threshold = 75;
				}
				success = drawDepthImage(sensor, (unsigned char*)(image.data), numChannels, depth, depthModifications, renderOptions);
			}
		}

		// Blank image defaults to depth image type, all other can be casted directly
		CoordinateType::Type coordType = (type == ImageType::Blank) ? CoordinateType::DEPTH : (CoordinateType::Type) type;

		if (success && (renderOptions != RenderOptions::None))
		{
			drawTrackingInfo(sensor, image.data, options.m_width, options.m_height, numChannels, depth, coordType, renderOptions, jointsToRender);
		}

		if (success && userId != 0)
		{
			Rect roi;
			float roiZ;
			success = getROIForUserJoint(userId, jointOfInterest, options.m_width, options.m_height, coordType, roi.x, roi.y, roi.width, roi.height, roiZ, (renderOptions&RenderOptions::UseFilteredValues) != 0);
			if (success)
			{
				image = image(roi);
				if (threshold > Math::Epsilon)
					applyThreshold(image.data, roi.width, roi.height, depth, image.step[0], roiZ, threshold);
			}
		}

		if (success)
		{
			// Save to file with given jpg quality or maximum png compression
			std::vector<int> params;
			params.push_back(CV_IMWRITE_JPEG_QUALITY);
			params.push_back(jpegQuality);
			params.push_back(CV_IMWRITE_PNG_COMPRESSION);
			params.push_back(9);
			success = imwrite(fileName, image, params) != 0;
		}
	}
#else
	static double lastWarning = -99;
	if (Fubi::currentTime() - lastWarning > 10)
	{
		Fubi_logWrn("Sorry, can't save picture without FUBI_USE_OPENCV defined in the FubiConfig.h.\n");
		lastWarning = Fubi::currentTime();
	}
#endif

	return success;
}


bool FubiImageProcessing::drawColorImage(FubiISensor* sensor, unsigned char* outputImage, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, bool swapBandR /*= false*/)
{
	if (sensor)
	{
		// Get options for the resolution
		Fubi::StreamOptions options = sensor->getStreamOptions(ImageType::Color);
		// Get image
		const unsigned char* data = sensor->getRgbData();
		if (options.isValid() && data != 0x0)
		{
			if (numChannels == ImageNumChannels::C3 && depth == ImageDepth::D8)
			{
				// Directly copy image data
				memcpy(outputImage, data, options.m_width*options.m_height*sizeof(unsigned char)*3);

#ifdef FUBI_USE_OPENCV
				if (swapBandR)
				{
					Mat image(Size(options.m_width, options.m_height), CV_8UC3, outputImage);
					cvtColor(image, image, CV_BGR2RGB);
				}
			}
			else
			{
				int dstType = CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels);
				Mat image(Size(options.m_width, options.m_height), CV_8UC3, (void*)data);
				Mat dstImage(Size(options.m_width, options.m_height), dstType, outputImage);

				Mat corrChannelImage;
				if (numChannels == ImageNumChannels::C1)
				{
					// Convert to grayscale
					corrChannelImage.create(Size(options.m_width, options.m_height), dstType);
					cvtColor(image, corrChannelImage, CV_BGR2GRAY);
				}
				else if (numChannels == ImageNumChannels::C4)
				{
					// Add alpha channel
					corrChannelImage.create(Size(options.m_width, options.m_height), dstType);
					if (swapBandR)
						cvtColor(image, corrChannelImage, CV_BGR2RGBA);
					else
						cvtColor(image, corrChannelImage, CV_BGR2BGRA);
				}

				double rescale = 1.0;
				if (depth == ImageDepth::D16)
					rescale = Math::MaxUShort16 / (double)Math::MaxUChar8;
				else if (depth == ImageDepth::F32)
					rescale = 1.0 / (double)Math::MaxUChar8;
				if (!corrChannelImage.empty())
					corrChannelImage.convertTo(dstImage, dstType, rescale);
				else
					image.convertTo(dstImage, dstType, rescale);
			}
#else
			}

			static double lastWarning = -99;
			if (Fubi::currentTime() - lastWarning > 10)
			{
				Fubi_logWrn("Sorry, can't convert picture without FUBI_USE_OPENCV defined in the FubiConfig.h.\n");
				lastWarning = Fubi::currentTime();
			}
#endif

			return true;
		}
	}
	return false;
}

template<class T> bool FubiImageProcessing::drawIRImage(FubiISensor* sensor, T* outputImage, Fubi::ImageNumChannels::Channel numChannels, const T& maxValue)
{
	if (sensor)
	{
		// Get options for the resolution
		Fubi::StreamOptions options = sensor->getStreamOptions(ImageType::IR);

		const float stretchFac = maxValue / (float) sensor->getMaxRawStreamValue(Fubi::ImageType::IR);

		// Get image
		if (options.isValid() && sensor->getIrData())
		{
#pragma omp parallel for schedule(dynamic)
			for (int j = 0; j < options.m_height; ++j)
			{
				T* pDestImage = outputImage + j*options.m_width*numChannels;
				const unsigned short* pIr = sensor->getIrData() + j*options.m_width;
				for(int i = 0; i < options.m_width; ++i)
				{
					pDestImage[0] = roundNonFloat<T>(*pIr * stretchFac);
					if (numChannels > 1)
					{
						pDestImage[1] = pDestImage[2] = pDestImage[0];
						if (numChannels == 4)
							pDestImage[3] = maxValue;
					}

					pIr++;
					pDestImage+=numChannels;
				}
			}

			return true;
		}
	}
	return false;
}


void FubiImageProcessing::drawLimb(unsigned int userID, Fubi::SkeletonJoint::Joint eJoint1, Fubi::SkeletonJoint::Joint eJoint2, cv::Mat* pImage,
	Fubi::CoordinateType::Type coordinateType, int renderOptions)
{
#ifdef FUBI_USE_OPENCV
	FubiUser* user = Fubi::getUser(userID);
	if (user && user->isTracked())
	{
		const Fubi::TrackingData* data = user->currentTrackingData();
		if (renderOptions & RenderOptions::UseFilteredValues)
		{
			data = user->currentFilteredTrackingData();
		}
		// Get positions
		const SkeletonJointPosition& joint1 = data->jointPositions[eJoint1];
		const SkeletonJointPosition& joint2 = data->jointPositions[eJoint2];

		// Check confidence
		if (joint2.m_position.z > 100.0f)
		{
			float maxValue = (float)Math::MaxUChar8;
			if (pImage->depth() == CV_16U)
				maxValue = (float)Math::MaxUShort16;
			else if (pImage->depth() == CV_32F)
				maxValue = 1.0f;

			// Convert to projective (screen coordinates)
			Fubi::Vec3f pos1 = Fubi::convertCoordinates(joint1.m_position, CoordinateType::REAL_WORLD, coordinateType);
			Fubi::Vec3f pos2 = Fubi::convertCoordinates(joint2.m_position, CoordinateType::REAL_WORLD, coordinateType);

			if (pos2.x < 0 && pos2.y < 0)
			{
				return; // Conversion failed
			}

			// Get color
			float r, g, b;
			double a;
			if (joint2.m_confidence < 0.25f)
			{
				r = g = b = 0.5f;
				a = maxValue / 4;
			}
			else
			{
				getColorForUserID(userID, r, g, b);
				if (renderOptions & RenderOptions::SwapRAndB)
					swap(r, b);
				if (joint2.m_confidence < 0.75f)
					a = maxValue / 2;
				else
					a = maxValue;
			}

			double scale = 0.45;
			int    thickness = 1;
			if (renderOptions & RenderOptions::Skeletons)
			{
				// Draw limb
				circle(*pImage, Point(ftoi_r(pos2.x), ftoi_r(pos2.y)), 10000 / ftoi_r(pos2.z), Scalar(a*(1 - b), a*(1 - g), a*(1 - r), a), -1);
				if (pos1.z > 100.0f && joint1.m_confidence > 0.25f)
				{
					double lineA = a;
					if (joint1.m_confidence < 0.75f && joint2.m_confidence >= 0.75f)
						lineA = maxValue / 2; // special case: lower confidence for joint1, but joint2 has a better one -> take the lower one
					line(*pImage, Point(ftoi_r(pos1.x), ftoi_r(pos1.y)), Point(ftoi_r(pos2.x), ftoi_r(pos2.y)), Scalar(a*(1 - b), a*(1 - g), a*(1 - r), a), thickness);
				}
			}

			if (joint2.m_confidence > 0.25f)
			{
				// Draw info for joint2
				stringstream ss;
				ss.setf(ios::fixed,ios::floatfield);
				ss.precision(0);

				if (renderOptions & RenderOptions::LocalOrientCaptions)
				{
					Fubi::Vec3f jRot = data->localJointOrientations[eJoint2].m_orientation.getRot();
					ss << Fubi::getJointName(eJoint2) << ":" << jRot.x << "/" << jRot.y << "/" << jRot.z;
				}
				else if (renderOptions & RenderOptions::GlobalOrientCaptions)
				{
					Fubi::Vec3f jRot = data->jointOrientations[eJoint2].m_orientation.getRot();
					ss << Fubi::getJointName(eJoint2) << ":" << jRot.x << "/" << jRot.y << "/" << jRot.z;
				}
				else if (renderOptions & RenderOptions::LocalPosCaptions)
				{
					const Fubi::Vec3f& jPos = data->localJointPositions[eJoint2].m_position;
					ss << Fubi::getJointName(eJoint2) << ":" << jPos.x << "/" << jPos.y << "/" << jPos.z;
				}
				else if (renderOptions & RenderOptions::GlobalPosCaptions)
				{
					const Fubi::Vec3f& jPos = data->jointPositions[eJoint2].m_position;
					ss << Fubi::getJointName(eJoint2) << ":" << jPos.x << "/" << jPos.y << "/" << jPos.z;
				}

				if (ss.str().length() > 0)
				{
					putText(*pImage, ss.str(), Point(ftoi_r(pos2.x) + 10, ftoi_r(pos2.y) + 10), CV_FONT_HERSHEY_DUPLEX, scale, Scalar((1 - b)*maxValue, (1 - g)*maxValue, (1 - r)*maxValue, a), thickness);
				}
			}
		}
	}
#endif
}

void FubiImageProcessing::drawFinger(unsigned int handId, Fubi::SkeletonHandJoint::Joint finger, cv::Mat* pImage,
	Fubi::CoordinateType::Type coordinateType, int renderOptions)
{
#ifdef FUBI_USE_OPENCV
	FubiCore* core = FubiCore::getInstance();
	if (core)
	{
		FubiHand* hand = core->getHand(handId);
		if (hand && hand->isTracked())
		{
			const Fubi::TrackingData* data = hand->currentTrackingData();
			if (renderOptions & RenderOptions::UseFilteredValues)
			{
				data = hand->currentFilteredTrackingData();
			}
			// Get positions
			const SkeletonJointPosition& fingerPos = data->jointPositions[finger];
			const SkeletonJointPosition& palmPos = data->jointPositions[SkeletonHandJoint::PALM];

			if (fingerPos.m_confidence > 0)
			{
				float maxValue = (float)Math::MaxUChar8;
				if (pImage->depth() == CV_16U)
					maxValue = (float)Math::MaxUShort16;
				else if (pImage->depth() == CV_32F)
					maxValue = 1.0f;

				// Convert to projective (screen coordinates)
				Fubi::Vec3f pos2 = Fubi::convertCoordinates(fingerPos.m_position, CoordinateType::REAL_WORLD, coordinateType);
				Fubi::Vec3f pos1 = Fubi::convertCoordinates(palmPos.m_position, CoordinateType::REAL_WORLD, coordinateType);

				if (pos2.x >= 0 && pos2.x < pImage->cols && pos2.y >= 0 && pos2.y < pImage->rows)
				{
					// Get color
					float r, g, b;
					double a;
					if (fingerPos.m_confidence < 0.25f)
					{
						r = g = b = 0.5f;
						a = maxValue / 4;
					}
					else
					{
						getColorForUserID(handId, r, g, b);
						if (renderOptions & RenderOptions::SwapRAndB)
							swap(r, b);
						if (fingerPos.m_confidence < 0.75f)
							a = maxValue / 2;
						else
							a = maxValue;
					}

					double scale=0.4;
					int    thickness=1;
					if (renderOptions & RenderOptions::Skeletons)
					{
						// Draw finger
						circle(*pImage, Point(ftoi_r(pos2.x), ftoi_r(pos2.y)), 5000 / ftoi_r(clamp(pos2.z, 1.0f, Math::MaxFloat)), Scalar(a*(1 - b), a*(1 - g), a*(1 - r), a), -1);
						if (finger != SkeletonHandJoint::PALM && palmPos.m_confidence > 0.25f && pos1.x >= 0 && pos1.x < pImage->cols && pos1.y >= 0 && pos1.y < pImage->rows)
						{
							double lineA = a;
							if (palmPos.m_confidence < 0.75f && fingerPos.m_confidence >= 0.75f)
								lineA = maxValue / 2; // special case: lower confidence for palm, but finger has a better one -> take the lower one
							line(*pImage, Point(ftoi_r(pos1.x), ftoi_r(pos1.y)), Point(ftoi_r(pos2.x), ftoi_r(pos2.y)), Scalar(a*(1 - b), a*(1 - g), a*(1 - r), a));
						}
					}

					if (fingerPos.m_confidence > 0.25f)
					{
						// Draw info for finger
						stringstream ss;
						ss.setf(ios::fixed,ios::floatfield);
						ss.precision(0);

						if (renderOptions & RenderOptions::LocalOrientCaptions)
						{
							Fubi::Vec3f jRot = data->localJointOrientations[finger].m_orientation.getRot();
							ss << Fubi::getHandJointName(finger) << ":" << jRot.x << "/" << jRot.y << "/" << jRot.z;
						}
						else if (renderOptions & RenderOptions::GlobalOrientCaptions)
						{
							Fubi::Vec3f jRot = data->jointOrientations[finger].m_orientation.getRot();
							ss << Fubi::getHandJointName(finger) << ":" << jRot.x << "/" << jRot.y << "/" << jRot.z;
						}
						else if (renderOptions & RenderOptions::LocalPosCaptions)
						{
							const Fubi::Vec3f& jPos = data->localJointPositions[finger].m_position;
							ss << Fubi::getHandJointName(finger) << ":" << jPos.x << "/" << jPos.y << "/" << jPos.z;
						}
						else if (renderOptions & RenderOptions::GlobalPosCaptions)
						{
							ss << Fubi::getHandJointName(finger) << ":" << fingerPos.m_position.x << "/" << fingerPos.m_position.y << "/" << fingerPos.m_position.z;
						}

						if (ss.str().length() > 0)
						{
							putText(*pImage, ss.str(), Point(ftoi_r(pos2.x) + 10, ftoi_r(pos2.y) + 10), CV_FONT_HERSHEY_DUPLEX, scale, Scalar((1 - b)*maxValue, (1 - g)*maxValue, (1 - r)*maxValue, a), thickness);
						}
					}
				}
			}
		}
	}
#endif
}

void FubiImageProcessing::drawBodyMeasurement(unsigned int player, Fubi::SkeletonJoint::Joint eJoint1, Fubi::SkeletonJoint::Joint eJoint2, Fubi::BodyMeasurement::Measurement bodyMeasure,
	cv::Mat* pImage, Fubi::CoordinateType::Type coordinateType, int renderOptions)
{
#ifdef FUBI_USE_OPENCV
	FubiUser* user = Fubi::getUser(player);
	if (user && user->isTracked())
	{
		// Get positions
		const Fubi::TrackingData* data = user->currentTrackingData();
		if (renderOptions & RenderOptions::UseFilteredValues)
		{
			data = user->currentFilteredTrackingData();
		}
		const SkeletonJointPosition joint1 = data->jointPositions[eJoint1];
		const SkeletonJointPosition joint2 = data->jointPositions[eJoint2];
		// And the measurement
		const BodyMeasurementDistance bm = user->bodyMeasurements()[bodyMeasure];

		// Check confidence
		if (joint2.m_position.z > 100.0f)
		{
			float maxValue = (float)Math::MaxUChar8;
			if (pImage->depth() == CV_16U)
				maxValue = (float)Math::MaxUShort16;
			else if (pImage->depth() == CV_32F)
				maxValue = 1.0f;

			// Convert to projective (screen coordinates)
			Fubi::Vec3f pos1 = Fubi::convertCoordinates(joint1.m_position, CoordinateType::REAL_WORLD, coordinateType);
			Fubi::Vec3f pos2 = Fubi::convertCoordinates(joint2.m_position, CoordinateType::REAL_WORLD, coordinateType);
			Fubi::Vec3f center = pos1 + (pos2 - pos1)*0.5f;

			// Get color
			float r, g, b;
			double a;
			if (bm.m_confidence < 0.25f)
			{
				r = g = b = 0.5f;
				a = maxValue / 4;
			}
			else
			{
				getColorForUserID(player, r, g, b);
				if (renderOptions & RenderOptions::SwapRAndB)
					swap(r, b);
				if (bm.m_confidence < 0.75f)
					a = maxValue / 2;
				else
					a = maxValue;
			}

			stringstream ss;
			ss.setf(ios::fixed,ios::floatfield);
			ss.precision(0);
			ss << Fubi::getBodyMeasureName(bodyMeasure) << ":" << bm.m_dist;

			// Print text in opposite color
			drawText(pImage, ss.str(), center.x, center.y, 1 - r, 1 - g, 1 - b);
		}
	}
#endif
}


bool FubiImageProcessing::getROIForUserJoint(unsigned int userId, Fubi::SkeletonJoint::Joint jointOfInterest, int imageWidth, int imageHeight, Fubi::CoordinateType::Type coordinateType, int& roiX, int& roiY, int& roiW, int& roiH, float& roiZ, bool useFilteredData /*= false*/)
{
	bool foundRoi = false;

	// Check for the user
	FubiUser* user = getUser(userId);
	if (user)
	{
		// Cut out a shape roughly around the joint of interest
		// First get the region of interest
		int width = imageWidth;
		int height = imageHeight;
		int x = width/2, y = height/2;
		const Fubi::TrackingData* data = user->currentTrackingData();
		if (useFilteredData)
			data = user->currentFilteredTrackingData();
		if (jointOfInterest == SkeletonJoint::NUM_JOINTS) // Cut out whole user
		{
			Fubi::Vec3f pos = data->jointPositions[SkeletonJoint::TORSO].m_position;
			roiZ = pos.z;
			pos = Fubi::convertCoordinates(pos, CoordinateType::REAL_WORLD, coordinateType);
			x = ftoi_r(pos.x);
			y = ftoi_r(pos.y);
			// clamp a rectangle about 90 x 200 cm
			width = ftoi_r(0.7f * imageWidth);
			height = ftoi_r(1.75f * imageHeight);
			foundRoi = true;
		}
		else if (user->isTracked())	// Standard case
		{
			// Try to get the joint pos
			SkeletonJointPosition jPos = data->jointPositions[jointOfInterest];
			if (jPos.m_confidence > 0.5f)
			{
				Fubi::Vec3f pos = jPos.m_position;
				roiZ = pos.z;
				pos = Fubi::convertCoordinates(pos, CoordinateType::REAL_WORLD, coordinateType);
				x = ftoi_r(pos.x);
				y = ftoi_r(pos.y);
				// clamp a rectangle about 30 x 50 cm
				width = ftoi_r(0.234f * imageWidth);
				height = ftoi_r(0.4375f * imageHeight);
				foundRoi = true;
			}
		}

		if (foundRoi)
		{
			// Clamp z from 30 cm to 5 m, convert to meter, and invert it
			float zFac = 1000.0f / clamp(roiZ, 300.0f, 5000.0f);
			// Apply z-factor
			width = ftoi_r(width*zFac);
			height = ftoi_r(height*zFac);
			// Set x and y from center to upper left corner and clamp it
			roiX = clamp(x-(width/2), 0, imageWidth-1);
			roiY = clamp(y-(height/2), 0, imageHeight-1);

			// Clamp size
			roiW = clamp(width, 1, imageWidth-roiX);
			roiH = clamp(height, 1, imageHeight-roiY);
		}
	}

	return foundRoi;
}

void FubiImageProcessing::applyThreshold(unsigned char* imageData, int width, int height, Fubi::ImageDepth::Depth depth, size_t step, float zCenter, float threshold, float replaceValue /*= 0*/)
{
	if (threshold < Math::Epsilon)
		return;

	// Clamp depth values according to hand depth
	float convertedZ = zCenter;
	float maxValue = (float) MaxDepth;
	if (depth == ImageDepth::D8)
	{
		convertedZ = zCenter * ((float)Math::MaxUChar8 / MaxDepth);
		threshold *= ((float)Math::MaxUChar8 / MaxDepth);
		maxValue = Math::MaxUChar8;
	}
	else if (depth == ImageDepth::F32)
	{
		convertedZ = zCenter / MaxDepth;
		threshold /= MaxDepth;
		maxValue = 1.0f;
	}

	float min = clamp(convertedZ - threshold, 0.0f, maxValue);
	float max = clamp(convertedZ + threshold, 0.0f, maxValue);

	if (depth == ImageDepth::D16)
	{
		unsigned short min16 = (unsigned short)ftoi_r(min), max16 = (unsigned short)ftoi_r(max), replace16 = (unsigned short)ftoi_r(replaceValue);

#pragma omp parallel for schedule(dynamic)
		for (int y = 0; y < height; ++y)
		{
			unsigned short* pData = (unsigned short*)(imageData + y*step);
			for (int x = 0; x < width; ++x, ++pData)
			{
				*pData = clamp(*pData, min16, max16, replace16);
			}
		}
	}
	else if (depth == ImageDepth::F32)
	{

#pragma omp parallel for schedule(dynamic)
		for (int y = 0; y < height; ++y)
		{
			float* pData = (float*)(imageData + y*step);
			for (int x = 0; x < width; ++x, ++pData)
			{
				*pData = (float)clamp<double>(*pData, min, max, replaceValue);
			}
		}
	}
	else
	{
		unsigned char min8 = (unsigned char)ftoi_r(min), max8 = (unsigned char)ftoi_r(max), replace8 = (unsigned char)ftoi_r(replaceValue);

#pragma omp parallel for schedule(dynamic)
		for (int y = 0; y < height; ++y)
		{
			unsigned char* pData = imageData + y*step;
			for (int x = 0; x < width; ++x, ++pData)
			{
				*pData = clamp(*pData, min8, max8, replace8);
			}
		}
	}
}

int FubiImageProcessing::fingerCount(cv::Mat* depthImage, cv::Mat* rgbaImage /*= 0x0*/, bool useContourDefectMode /*= false*/)
{
	int numFingers = -1;

#ifdef FUBI_USE_OPENCV
	// Get image (or roi) size  and pos
	Size rect = depthImage->size();

	if (useContourDefectMode)
	{
		// Find contours
		vector<vector<Point>> contours;

		findContours(*depthImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

		if(contours.size() > 0)
		{
			// Take the contour with the biggest bounding box = hand
			int maxSize = 0;
			unsigned int contourIndex = 0;
			Rect boundbox;
			for(unsigned int i = 0; i < contours.size(); ++i)
			{
				Rect box = boundingRect(contours[i]);
				int size = box.width * box.height;
				if(size > maxSize)
				{
					maxSize = size;
					contourIndex = i;
					boundbox = box;
				}
			}
			vector<Point> handContour = contours[contourIndex];

			// Only take big enough pictures, it wont make any sense else
			if (maxSize > 250)
			{
				if (rgbaImage)
				{
					// Draw the bounding box
					rectangle(*rgbaImage, Point(boundbox.x, boundbox.y), Point(boundbox.x+boundbox.width, boundbox.y+boundbox.height), Scalar(0, Math::MaxUChar8, Math::MaxUChar8, Math::MaxUChar8));
					// Draw contours in the rgba image
					drawContours(*rgbaImage, contours, contourIndex, Scalar(Math::MaxUChar8,0,0,Math::MaxUChar8), CV_FILLED, 4);
					drawContours(*rgbaImage, contours, contourIndex, Scalar(Math::MaxUChar8,0,Math::MaxUChar8,Math::MaxUChar8), 1, 4);
				}

				// Get convex hull
				std::vector<int> hull;
				convexHull(handContour, hull, false, false);

				// Calculate convexity defects
				std::vector<Vec4i> defects;
				convexityDefects(handContour, hull, defects);
				// Convert to array
				unsigned int numSmallAngles = 0;
				unsigned int numLargeCenteredDefects = 0;
				for (unsigned int i = 0; i < defects.size(); ++i)
				{
					bool defectCounted = false;

					Point start = handContour[defects[i][0]];
					Point end = handContour[defects[i][1]];
					Point depth_point = handContour[defects[i][2]];
					float depth = defects[i][3] / 256.0f;

					float defectRelY = float(depth_point.y - boundbox.y) / boundbox.height;
					float defectDToW = depth / boundbox.width;
					float defectDToH = depth / boundbox.height;

					// Filter out defects with wrong size or y position (in contour bounding box as well as in the whole image)
					int maxY = rect.height * 9 / 10; // 90 % of the whole image size
					if (defectDToH > 0.1f && defectDToW > 0.2f && defectDToH < 0.75f &&  defectRelY > 0.1f && defectRelY < 0.6f &&
						start.y <= maxY && depth_point.y <= maxY && end.y <= maxY)
					{
						numLargeCenteredDefects++;

						// Filter out defects with too large angles (only searching for the spaces between stretched fingers)
						Point vec1 = Point(start.x - depth_point.x, start.y - depth_point.y);
						Point vec2 = Point(end.x - depth_point.x,end.y - depth_point.y);
						float angle = abs(atan2f((float)vec2.y, (float)vec2.x) - atan2f((float)vec1.y, (float)vec1.x));
						if (angle > Math::Pi)
							angle = Math::TwoPi - angle;
						float a = 0.3f;
						if (angle < 90.0f * (Math::Pi / 180.0f)) // Angle smaller than 90 degrees
						{
							defectCounted = true;
							numSmallAngles++;
							a = 1.0f;
						}

						if (rgbaImage)
						{
							// Render defect edges
							line(*rgbaImage, start, end, Scalar(0, a*150, 0, a*Math::MaxUChar8));
							line(*rgbaImage, start, depth_point, Scalar(0, a*Math::MaxUChar8, a*Math::MaxUChar8, a*Math::MaxUChar8));
							line(*rgbaImage, depth_point, end, Scalar(a*Math::MaxUChar8, a*Math::MaxUChar8, 0, a*Math::MaxUChar8));
							// And vertices
							circle(*rgbaImage, end, 5, Scalar(0,a*Math::MaxUChar8,0, a*Math::MaxUChar8), -1);
							circle(*rgbaImage, start, 5, Scalar(a*Math::MaxUChar8,0,0, a*Math::MaxUChar8), -1);
							circle(*rgbaImage, depth_point, 5, Scalar(a*Math::MaxUChar8,a*Math::MaxUChar8,0, a*Math::MaxUChar8), -1);
						}
					}

					/*if (defectCounted)
					{
					Fubi_logInfo("defectDToH %.3f\n", defectDToH);
					Fubi_logInfo("defectDToW %.3f\n", defectDToW);
					Fubi_logInfo("defectRelY %.3f\n", defectRelY);
					}
					else if ((defectDToH > 0.1f && defectDToH < 0.75f) || defectDToW > 0.2f)
					{
					Fubi_logInfo("---defectDToH %.3f\n", defectDToH);
					Fubi_logInfo("---defectDToW %.3f\n", defectDToW);
					Fubi_logInfo("---defectRelY %.3f\n", defectRelY);
					Fubi_logInfo("---startY %d\n", defectArray[i].start->y);
					Fubi_logInfo("---depthY %d\n", defectArray[i].depth_point->y);
					Fubi_logInfo("---endY %d\n", defectArray[i].end->y);
					}*/
				}

				if (numSmallAngles > 0)
					numFingers = numSmallAngles + 1; // Defects with small angles = space between fingers
				else if (numLargeCenteredDefects > 0)
					numFingers = 1; // Only defects with large angles = one finger up
				else //no defects = no fingers
					numFingers = 0;
			}
		}
	}
	else
	{
		medianBlur(*depthImage, *depthImage, 7);

		vector<vector<Point> > tempContours;
		findContours(*depthImage, tempContours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
		if (tempContours.size() > 0)
		{
			numFingers = 0;
			double bigArea = 0, area = 0;
			unsigned int contourIndex = 0;
			for (unsigned int i = 0; i < tempContours.size(); ++i)
			{
				area = abs(contourArea(tempContours[i]));
				if (area > bigArea)
				{
					bigArea = area;
					contourIndex = i;
				}
			}

			drawContours(*depthImage, tempContours, contourIndex, Scalar(95), CV_FILLED);
			inRange(*depthImage, Scalar(90), Scalar(96), *depthImage);

			medianBlur(*depthImage, *depthImage, 7);

			erode(*depthImage, *depthImage, Mat(), Point(-1, -1), 1);

			static Mat e = getStructuringElement(CV_SHAPE_ELLIPSE, Size(5,5), Point(3,3));

			static Mat morphImage, subImage;
			morphologyEx(*depthImage, morphImage, CV_MOP_OPEN, e, Point(-1, -1), 4);

			subtract(*depthImage, morphImage, subImage);
			threshold(subImage, subImage, 100, Math::MaxUChar8, CV_THRESH_BINARY);

			Point2f centre;
			Moments moment = moments(*depthImage, true);
			centre.x = float(moment.m10 /moment.m00);
			centre.y = float(moment.m01 /moment.m00);

			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			findContours( subImage, contours, hierarchy, CV_RETR_LIST, CHAIN_APPROX_SIMPLE);

			if (rgbaImage)
			{
				line(*rgbaImage, Point(ftoi_r(centre.x - 20.0f), ftoi_r(centre.y)), Point(ftoi_r(centre.x + 20.0f), ftoi_r(centre.y)), Scalar(208, 224, 64, Math::MaxUChar8), 2, 8);//light blue
				line(*rgbaImage, Point(ftoi_r(centre.x), ftoi_r(centre.y - 20.0f)), Point(ftoi_r(centre.x), ftoi_r(centre.y + 20.0f)), Scalar(208, 224, 64, Math::MaxUChar8), 2, 8);//light blue
			}

			if (hierarchy.size() > 0)
			{
				Point2f u,v;
				static Mat p;
				float yD1, yD2, xD1, xD2, distance1, distance2, finger_area;
				for(int idx = 0, i = 0; idx >= 0 && hierarchy.size() > (unsigned)idx; idx = hierarchy[idx][0], ++i)
				{
					if(numFingers < 5 )
					{
						finger_area = (float)contourArea(contours[i]);
						if (finger_area > 40)
						{
							numFingers++;
							if (rgbaImage)
							{
								drawContours(*rgbaImage, contours, idx, Scalar(238, 178, 0, Math::MaxUChar8), CV_FILLED, 8, hierarchy);
								Mat(contours[i]).convertTo(p, CV_32F);
								RotatedRect box = fitEllipse(p);
								ellipse(*rgbaImage, box.center, box.size * 0.5f, box.angle, 0, 360, Scalar(64, 64, Math::MaxUChar8, Math::MaxUChar8), 2); //melon
								Point2f vtx[4];
								box.points(vtx);

								u.x = (vtx[1].x + vtx[2].x) / 2;
								u.y = (vtx[1].y + vtx[2].y) / 2;

								v.x = (vtx[0].x + vtx[3].x) / 2;
								v.y = (vtx[0].y + vtx[3].y) / 2;

								xD1 = u.x - centre.x;
								yD1 = u.y - centre.y;
								distance1 = sqrt(pow(xD1, 2) + pow(yD1, 2));

								xD2 = v.x - centre.x;
								yD2 = v.y - centre.y;
								distance2 = sqrt(pow(xD2, 2) + pow(yD2, 2));

								if (distance1 > distance2)
								{
									circle(*rgbaImage, u, 6, Scalar(180, 110, Math::MaxUChar8, Math::MaxUChar8), -1);//pink
								}
								else
								{
									circle(*rgbaImage, v, 6, Scalar(180, 110, Math::MaxUChar8, Math::MaxUChar8), -1);//pink
								}
							}
						}
					}
					else
						break;
				}
			}
		}
	}

#else
	static double lastWarning = -99;
	if (Fubi::currentTime() - lastWarning > 10)
	{
		Fubi_logWrn("Sorry, can't apply finger detection on image without FUBI_USE_OPENCV defined in the FubiConfig.h.\n");
		lastWarning = Fubi::currentTime();
	}
#endif

	return numFingers;
}

int FubiImageProcessing::applyFingerCount(FubiISensor* sensor, unsigned int userID, bool leftHand /*= false*/, bool useOldConvexityDefectMethod /*= false*/, FingerCountImageData* debugData /*= 0x0*/)
{
	int numFingers = -1;

#ifdef FUBI_USE_OPENCV
	if (sensor)
	{
		Fubi::StreamOptions options = sensor->getStreamOptions(ImageType::Depth);

		// Retrieve the depth image
		static Mat image;
		image.create(Size(options.m_width, options.m_height), CV_16UC1);
		drawDepthImage(sensor, image.data, ImageNumChannels::C1, ImageDepth::D16, DepthImageModification::Raw, RenderOptions::None);

		// Set Region of interest to the observed hand and apply a depth threshold
		Rect roi;
		float roiZ;
		if (getROIForUserJoint(userID, leftHand ? SkeletonJoint::LEFT_HAND : SkeletonJoint::RIGHT_HAND, options.m_width, options.m_height, CoordinateType::DEPTH, roi.x, roi.y, roi.width, roi.height, roiZ, true))
		{
			Mat roiImage = image(roi);
			// Apply Threshold
			applyThreshold(roiImage.data, roi.width, roi.height, ImageDepth::D16, roiImage.step[0], roiZ, 75.0f);
			// Convert the image back to 8 bit (easier to handle in the rest)
			// And by they way convert it to a binary image
			static Mat depthImage;
			depthImage.create(Size(roi.width, roi.height), CV_8UC1);
			roiImage.convertTo(depthImage, CV_8U, (double)Math::MaxUChar8 / 2.0 /*/ (double)Math::MaxUShort16*/);

			/*imshow("temp", depthImage);
			waitKey(1);*/

			Mat* rgbaImage = 0x0;
			if (debugData)
			{
				if (debugData->image == 0)
					debugData->image = new Mat();
				rgbaImage = debugData->image;
				rgbaImage->create(Size(roi.width, roi.height), CV_8UC4);
				*rgbaImage = Scalar::all(0);
				debugData->posX = roi.x;
				debugData->posY = roi.y;
			}

			// Try to detect the number of fingers and render the contours in the image
			numFingers = fingerCount(&depthImage, rgbaImage, useOldConvexityDefectMethod);

			if (debugData)
			{
				debugData->timeStamp = Fubi::getCurrentTime();
				debugData->fingerCount = numFingers;
			}

		}
	}
#else
	static double lastWarning = -99;
	if (Fubi::currentTime() - lastWarning > 10)
	{
		Fubi_logWrn("Can't apply finger detection on image without FUBI_USE_OPENCV defined in the FubiConfig.h.\n");
		lastWarning = Fubi::currentTime();
	}
#endif
	return numFingers;
}

void FubiImageProcessing::releaseImage(cv::Mat* image)
{
#ifdef FUBI_USE_OPENCV
	if (image)
	{
		delete image;
	}
#endif
	// Nothing to release in the other case
}

void FubiImageProcessing::drawText(cv::Mat* pImage, const std::string& text, float xPos, float yPos, float colR, float colG, float colB, bool center /*= true*/)
{
#ifdef FUBI_USE_OPENCV
	// Get maximum value for converting the color to
	double maxValue = Math::MaxUChar8;
	if (pImage->depth() == CV_16U)
		maxValue = Math::MaxUShort16;
	else if (pImage->depth() == CV_32F)
		maxValue = 1.0f;

	if (xPos >= 0 && xPos < pImage->cols && yPos >= 0 && yPos < pImage->rows)
	{
		// Offset = 3.5*text.length(), 5.0f if centering
		float offsetX = center ? (3.5f * text.length()) : 0;
		float offsetY = center ? 5.0f : 0;
		// Print the text with a scale of 0.5, thickness of 1 and the HERSHEY_DUPLEX font
		double scale = 0.5;
		int    thickness = 1;
		putText(*pImage, text, Point(ftoi_r(xPos - offsetX), ftoi_r(yPos - offsetY)), CV_FONT_HERSHEY_DUPLEX, scale,
			Scalar(colB*maxValue, colG*maxValue, colR*maxValue, maxValue), thickness);
	}
#endif
}

bool FubiImageProcessing::getImage(FubiIFingerSensor* sensor, unsigned char* outputImage, unsigned int imageIndex, ImageNumChannels::Channel numChannels, ImageDepth::Depth depth)
{
	if (depth == ImageDepth::D16)
		return drawFingerSensorImage(sensor, (unsigned short*)outputImage, imageIndex, numChannels, Math::MaxUShort16);
	else if (depth == ImageDepth::D8)
		return  drawFingerSensorImage(sensor, outputImage, imageIndex, numChannels, Math::MaxUChar8);
	// ImageDepth::F32
	return drawFingerSensorImage(sensor, (float*)outputImage, imageIndex, numChannels, 1.0f);
}

template<class T> bool FubiImageProcessing::drawFingerSensorImage(FubiIFingerSensor* sensor, T* outputImage, unsigned int imageIndex, Fubi::ImageNumChannels::Channel numChannels, const T& maxValue)
{
	if (sensor)
	{
		const unsigned char* rawData = sensor->getImageData(imageIndex);
		if (rawData)
		{
			int width, height;
			unsigned int numStreams;
			sensor->getImageConfig(width, height, numStreams);

			if (numChannels == ImageNumChannels::C1 && maxValue == Math::MaxUChar8)
			{
				memcpy(outputImage, rawData, width*height);
			}
			else
			{
				const float stretchFac = maxValue / (float)Math::MaxUChar8;
#pragma omp parallel for schedule(dynamic)
				for (int j = 0; j < height; ++j)
				{
					T* pDestImage = outputImage + j*width*numChannels;
					const unsigned char* pData = rawData + j*width;
					for (int i = 0; i < width; ++i)
					{
						if (*pData != 0)
						{
							pDestImage[0] = roundNonFloat<T>(*pData * stretchFac);
							if (numChannels > 1)
							{
								pDestImage[1] = pDestImage[2] = pDestImage[0];
								if (numChannels == 4)
									pDestImage[3] = maxValue;
							}
						}

						pData++;
						pDestImage += numChannels;
					}
				}
			}
			return true;
		}
	}
	return false;
}

void FubiImageProcessing::showPlot(const std::vector<Fubi::Vec3f>& dataToPlot, const std::vector<Fubi::Matrix3f>* invDataCovs /*= 0x0*/,
	unsigned int width /*= 640*/, unsigned height /*= 480*/, const std::string& windowName /*= "plot"*/)
{
#ifdef FUBI_USE_OPENCV
	// Create image
	cv::Mat img(height, width, CV_32FC3);

	// Draw points into to image (Start with centroid)
	plotImage(dataToPlot, invDataCovs, img.data, width, height, ImageNumChannels::C3, ImageDepth::F32);

	// Get min and max of the unscaled data
	Fubi::Vec3f min(Math::NO_INIT), max(Math::NO_INIT);
	Fubi::calculateAABB(dataToPlot, min, max);

	// Add labels (note: y flipped!)
	drawText(&img, string("MinX: ") + numToString(min.x), 5.0f, height/2.0f-5.0f, 1.0f, 1.0f, 1.0f, false);
	drawText(&img, string("MaxX: ") + numToString(max.x), width - 70.0f, height / 2.0f, 1.0f, 1.0f, 1.0f);
	drawText(&img, string("MinY: ") + numToString(min.y), width / 2.0f, height - 5.0f, 1.0f, 1.0f, 1.0f);
	drawText(&img, string("MaxY: ") + numToString(max.y), width / 2.0f, 25.0f, 1.0f, 1.0f, 1.0f);
	drawText(&img, string("MinZ: ") + numToString(min.z), 5.0f, 25.0f, 1.0f, 1.0f, 1.0f, false);
	drawText(&img, string("MaxZ: ") + numToString(max.z), width - 70.0f, height - 5.0f, 1.0f, 1.0f, 1.0f);

	// Display image
	imshow(windowName, img);
	waitKey(1);
#endif
}

void FubiImageProcessing::plotImage(const std::vector<Fubi::Vec3f>& dataToPlot, const std::vector<Fubi::Matrix3f>* invDataCovs /*= 0x0*/,
	unsigned char* outputImage, unsigned int width, unsigned height,
	Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, int lineThickness /*=1*/)
{
#ifdef FUBI_USE_OPENCV
	if (dataToPlot.size() < 1)
		return;

	double maxValue = Math::MaxUChar8;
	if (depth == ImageDepth::D16)
		maxValue = Math::MaxUShort16;
	else if (depth == ImageDepth::F32)
		maxValue = 1.0;


	std::vector<Fubi::Vec3f> scaledData = dataToPlot;
	Fubi::translate(scaledData, -Fubi::centroid(scaledData));
	Fubi::Vec3f appliedScale;
	Fubi::scale(scaledData, Fubi::Vec3f((float)width, (float)height, 0.95f*(float)maxValue),
		0, false, false, true, &appliedScale);
	Fubi::Vec3f centroid = Fubi::centroid(scaledData);

	// Create image
	cv::Mat img(height, width, CV_MAKE_TYPE(IPL2CV_DEPTH(depth), numChannels), outputImage);

	// First draw covariance values (always ignores the z axis!)
	if (invDataCovs)
	{
		const float  chisquare_val = 2.447651936f; // 95 % confidence interval for two DoF
		cv::Scalar covColor = cv::Scalar(maxValue, 0, maxValue, maxValue);
		for (size_t i = 0; i < invDataCovs->size(); ++i)
		{
			if (i >= scaledData.size())
				break;
			// First get the mean
			cv::Point2f mean(scaledData[i].x, height - scaledData[i].y);
			// Only take the x,y parts of the matrix
			Matrix3f cov = invDataCovs->at(i).inverted();
			cv::Mat covmat = (cv::Mat_<float>(2, 2) 
				<< appliedScale.x*appliedScale.x*cov.c[0][0],
				appliedScale.x*appliedScale.y*cov.c[0][1], 
				appliedScale.x*appliedScale.y*cov.c[1][0], 
				appliedScale.y*appliedScale.y*cov.c[1][1]);
			// Get the eigenvalues and eigenvectors
			cv::Mat eigenvalues, eigenvectors;
#if (CV_MAJOR_VERSION >= 3)
			cv::eigen(covmat, eigenvalues, eigenvectors);
#else	
			cv::eigen(covmat, true, eigenvalues, eigenvectors);
#endif
			// Calculate the angle between the largest eigenvector and the x-axis
			float angle = atan2f(eigenvectors.at<float>(0, 1), eigenvectors.at<float>(0, 0));
			// Shift the angle to [0, 2pi] instead of [-pi, pi]
			if (angle < 0)
				angle += Math::TwoPi;
			// Convert to degrees
			angle = radToDeg(angle);
			// Calculate the size of the minor and major axes
			float halfmajoraxissize = chisquare_val*sqrtf(fabsf(eigenvalues.at<float>(0)));
			float halfminoraxissize = chisquare_val*sqrtf(fabsf(eigenvalues.at<float>(1)));
			//The -angle is used because OpenCV defines the angle clockwise instead of anti-clockwise
			cv::RotatedRect ellipse = cv::RotatedRect(mean, cv::Size2f(halfmajoraxissize, halfminoraxissize), -angle);
			//Show the result
			cv::ellipse(img, ellipse, covColor);
		}
	}

	// Draw points into image (Start with centroid)
	Point currPoint, prevPoint = Point(ftoi_r(centroid.x), height - ftoi_r(centroid.y));
	Scalar color(0, maxValue, maxValue - centroid.z, maxValue);
	for (auto iter = scaledData.begin(); iter != scaledData.end(); ++iter)
	{
		currPoint.x = ftoi_r(iter->x);
		// Flip y for window coords
		currPoint.y = height - ftoi_r(iter->y);
		line(img, prevPoint, currPoint, color, lineThickness);
		prevPoint = currPoint;
		color = Scalar::all(maxValue - iter->z);
	}
#endif
}