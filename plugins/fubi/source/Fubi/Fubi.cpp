// ****************************************************************************************
//
// Fubi API
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler 
// 
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html 
// 
// 
// ****************************************************************************************

#include "Fubi.h"
#include "FubiCore.h"
#include "FubiConfig.h"
#include "FubiImageProcessing.h"
#include "FubiXMLParser.h"


#include <deque>

namespace Fubi
{
	FUBI_API bool initFromXML(const char* xmlPath /*=0x0*/, Fubi::SkeletonTrackingProfile::Profile profile /*= Fubi::SkeletonTrackingProfile::ALL*/,
		float filterMinCutOffFrequency /*= 1.0f*/, float filterVelocityCutOffFrequency /*= 1.0f*/,
		float filterCutOffSlope /*= 0.007f*/, float bodyMeasureFilterFac /*= 0.1f*/,
		bool mirrorStreams /*= true*/, bool registerStreams /*=true*/)
	{
		return FubiCore::init(xmlPath, profile, filterMinCutOffFrequency, filterVelocityCutOffFrequency, 
			filterCutOffSlope, bodyMeasureFilterFac, mirrorStreams, registerStreams);
	}

	FUBI_CPP_API bool init(const SensorOptions& sensorOptions, const FilterOptions& filterOptions /*= FilterOptions()*/)
	{
		return FubiCore::init(sensorOptions, filterOptions);
	}


	FUBI_API bool init(int depthWidth, int depthHeight, int depthFPS /*= 30*/,
		int rgbWidth /*= 640*/, int rgbHeight /*= 480*/, int rgbFPS /*= 30*/,
		int irWidth /*= -1*/, int irHeight /*= -1*/, int irFPS /*= -1*/,
		Fubi::SensorType::Type sensorType /*= Fubi::SensorType::OPENNI2*/,
		Fubi::SkeletonTrackingProfile::Profile profile /*= Fubi::SkeletonTrackingProfile::ALL*/,
		float filterMinCutOffFrequency /*= 1.0f*/, float filterVelocityCutOffFrequency /*= 1.0f*/, 
		float filterCutOffSlope /*= 0.007f*/, float bodyMeasureFilterFac /*= 0.1f*/,
		bool mirrorStreams /*= true*/, bool registerStreams /*=true*/)
	{
		return init(SensorOptions(StreamOptions(depthWidth, depthHeight, depthFPS), StreamOptions(rgbWidth, rgbHeight, rgbFPS),
				StreamOptions(irWidth, irHeight, irFPS),
				sensorType,
				profile,
				mirrorStreams,
				registerStreams),
			FilterOptions(filterMinCutOffFrequency, filterVelocityCutOffFrequency, 
				filterCutOffSlope, bodyMeasureFilterFac));
	}

	FUBI_CPP_API bool switchSensor(const SensorOptions& options)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->initSensorWithOptions(options);
		return false;
	}

	FUBI_API bool switchSensor(Fubi::SensorType::Type sensorType, int depthWidth, int depthHeight, int depthFPS /*= 30*/,
		int rgbWidth /*= 640*/, int rgbHeight /*= 480*/, int rgbFPS /*= 30*/,
		int irWidth /*= -1*/, int irHeight /*= -1*/, int irFPS /*= -1*/,
		Fubi::SkeletonTrackingProfile::Profile profile /*= Fubi::SkeletonTrackingProfile::ALL*/,
		bool mirrorStream /*= true*/, bool registerStreams /*=true*/)
	{
		return switchSensor(SensorOptions(StreamOptions(depthWidth, depthHeight, depthFPS), StreamOptions(rgbWidth, rgbHeight, rgbFPS),
			StreamOptions(irWidth, irHeight, irFPS), sensorType,
			profile,
			mirrorStream, registerStreams));
	}

	FUBI_API Fubi::SensorType::Type getCurrentSensorType()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && core->getSensor())
			return core->getSensor()->getType();
		return Fubi::SensorType::NONE;
	}

	FUBI_API int getAvailableSensorTypes()
	{
		int ret = 0;
#ifdef FUBI_USE_OPENNI2
		ret |= SensorType::OPENNI2;
#endif
#ifdef FUBI_USE_OPENNI1
		ret |= SensorType::OPENNI1;
#endif
#ifdef FUBI_USE_KINECT_SDK
		ret |= SensorType::KINECTSDK;
#endif
#ifdef FUBI_USE_KINECT_SDK_2
		ret |= SensorType::KINECTSDK2;
#endif
		return ret;
	}

	FUBI_API void release()
	{
		FubiCore::release();
	}

	FUBI_API void updateSensor()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->updateSensor();
	}

	FUBI_API bool getImage(unsigned char* outputImage, ImageType::Type type, ImageNumChannels::Channel numChannels, ImageDepth::Depth depth,
		int renderOptions /*= (RenderOptions::Shapes | RenderOptions::Skeletons | RenderOptions::UserCaptions)*/,
		int jointsToRender /*= RenderOptions::ALL_JOINTS*/,
		DepthImageModification::Modification depthModifications /*= DepthImageModification::UseHistogram*/,
		unsigned int userId/* = 0*/, Fubi::SkeletonJoint::Joint jointOfInterest /*= Fubi::SkeletonJoint::NUM_JOINTS*/, bool moveCroppedToUpperLeft /*=false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return FubiImageProcessing::getImage(core->getSensor(), outputImage, type, numChannels, depth, renderOptions, jointsToRender, depthModifications, userId, jointOfInterest, moveCroppedToUpperLeft);
		}
		return false;
	}

	FUBI_API bool saveImage(const char* fileName, int jpegQuality, ImageType::Type type, ImageNumChannels::Channel numChannels, ImageDepth::Depth depth,
		int renderOptions /*= (RenderOptions::Shapes | RenderOptions::Skeletons | RenderOptions::UserCaptions)*/,
		int jointsToRender /*= RenderOptions::ALL_JOINTS*/,
		DepthImageModification::Modification depthModifications /*= DepthImageModification::UseHistogram*/,
		unsigned int userId /*= 0*/, Fubi::SkeletonJoint::Joint jointOfInterest /*= Fubi::SkeletonJoint::NUM_JOINTS*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return FubiImageProcessing::saveImage(core->getSensor(), fileName, jpegQuality, type, numChannels, depth, renderOptions, jointsToRender, depthModifications, userId, jointOfInterest);
		return false;
	}

	FUBI_API bool isInitialized()
	{
		return (FubiCore::getInstance() != 0x0);
	}

	FUBI_API Fubi::RecognitionResult::Result recognizePostureOn(Postures::Posture postureID, unsigned int userID)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->recognizeGestureOn(postureID, userID);
		return Fubi::RecognitionResult::NOT_RECOGNIZED;
	}

	FUBI_CPP_API Fubi::RecognitionResult::Result recognizeGestureOn(unsigned int recognizerIndex, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->recognizeGestureOn(recognizerIndex, userID, correctionHint);
		return Fubi::RecognitionResult::NOT_RECOGNIZED;
	}

	FUBI_API Fubi::RecognitionResult::Result recognizeGestureOn(const char* recognizerName, unsigned int userID, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && recognizerName)
			return core->recognizeGestureOn(recognizerName, userID, correctionHint);
		return Fubi::RecognitionResult::NOT_RECOGNIZED;
	}

	FUBI_API Fubi::RecognitionResult::Result getCombinationRecognitionProgressByIDOn(Combinations::Combination combinationID, unsigned int userID,
		std::vector<Fubi::TrackingData>* trackingStates /*= 0x0*/, bool restart /*= true*/, bool returnFilteredData /*= false*/,
		Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getCombinationRecognitionProgressOn(combinationID, userID, trackingStates, restart, returnFilteredData, correctionHint);
		return Fubi::RecognitionResult::NOT_RECOGNIZED;
	}

	FUBI_API Fubi::RecognitionResult::Result getCombinationRecognitionProgressOn(const char* recognizerName, unsigned int userID,
		std::vector<Fubi::TrackingData>* trackingStates /*= 0x0*/, bool restart /*= true*/, bool returnFilteredData /*= false*/,
		Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getCombinationRecognitionProgressOn(recognizerName, userID, trackingStates, restart, returnFilteredData, correctionHint);
		return Fubi::RecognitionResult::NOT_RECOGNIZED;
	}

	FUBI_API void enableCombinationRecognitionByID(Combinations::Combination combinationID, unsigned int userID, bool enable)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->enableCombinationRecognition(combinationID, userID, enable);
	}

	FUBI_API void enableCombinationRecognition(const char* combinationName, unsigned int userID, bool enable)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && combinationName)
			core->enableCombinationRecognition(combinationName, userID, enable);
	}

	FUBI_API void setAutoStartCombinationRecognition(bool enable, Combinations::Combination combinationID /*= Combinations::NUM_COMBINATIONS*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->setAutoStartCombinationRecognition(enable, combinationID);
	}

	FUBI_API bool getAutoStartCombinationRecognition(Combinations::Combination combinationID /*= Combinations::NUM_COMBINATIONS*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getAutoStartCombinationRecognition(combinationID);
		return false;
	}

	FUBI_API void getColorForUserID(unsigned int id, float& r, float& g, float& b)
	{
		FubiImageProcessing::getColorForUserID(id, r, g, b);
	}


	FUBI_API unsigned int getUserID(unsigned int index)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getUserID(index);
		return 0;
	}

	FUBI_API unsigned int addJointRelationRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
		float minX, float minY, float minZ /*= -Fubi::Math::MaxFloat,-Fubi::Math::MaxFloat, -Fubi::Math::MaxFloat*/,
		float maxX, float maxY, float maxZ /*= Fubi::Math::MaxFloat, Fubi::Math::MaxFloat, Fubi::Math::MaxFloat*/,
		float minDistance /*= 0*/,
		float maxDistance /*= Fubi::Math::MaxFloat*/,
		Fubi::SkeletonJoint::Joint midJoint /*= Fubi::SkeletonJoint::NUM_JOINTS*/,
		float midJointMinX /*= -Fubi::Math::MaxFloat*/, float midJointMinY /*= -Fubi::Math::MaxFloat*/, float midJointMinZ /*= -Fubi::Math::MaxFloat*/,
		float midJointMaxX /*= Fubi::Math::MaxFloat*/, float midJointMaxY /*= Fubi::Math::MaxFloat*/, float midJointMaxZ /*= Fubi::Math::MaxFloat*/,
		float midJointMinDistance /*= 0*/,
		float midJointMaxDistance /*= Fubi::Math::MaxFloat*/,
		bool useLocalPositions /*= false*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*= -1.0f*/,
		Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addJointRelationRecognizer(joint, relJoint, Vec3f(minX, minY, minZ), Vec3f(maxX, maxY, maxZ), minDistance, maxDistance,
			midJoint, Vec3f(midJointMinX, midJointMinY, midJointMinZ), Vec3f(midJointMaxX, midJointMaxY, midJointMaxZ), midJointMinDistance, midJointMaxDistance,
			useLocalPositions, atIndex, name, minConfidence, measuringUnit, useFilteredData);
		return -1;
	}

	FUBI_API unsigned int addJointOrientationRecognizer(Fubi::SkeletonJoint::Joint joint,
		float minX /*= -180.0f*/, float minY /*= -180.0f*/, float minZ /*= -180.0f*/,
		float maxX /*= 180.0f*/, float maxY /*= 180.0f*/, float maxZ /*= 180.0f*/,
		bool useLocalOrientations /*= true*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*=-1*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addJointOrientationRecognizer(joint, Vec3f(minX, minY, minZ), Vec3f(maxX, maxY, maxZ), useLocalOrientations, atIndex, name, minConfidence, useFilteredData);
		return -1;
	}

	FUBI_API unsigned int addJointOrientationRecognizerFromOrient(Fubi::SkeletonJoint::Joint joint,
		float orientX, float orientY, float orientZ,
		float maxAngleDifference /*= 45.0f*/,
		bool useLocalOrientations /*= true*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*=-1*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addJointOrientationRecognizer(joint, Vec3f(orientX, orientY, orientZ), maxAngleDifference, useLocalOrientations, atIndex, name, minConfidence, useFilteredData);
		return -1;
	}

	unsigned int addFingerCountRecognizer(Fubi::SkeletonJoint::Joint handJoint,
		unsigned int minFingers, unsigned int maxFingers,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float minConfidence /*=-1*/,
		bool useMedianCalculation /*= false*/,
		unsigned int medianWindowSize /*= 10*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addFingerCountRecognizer(handJoint, minFingers, maxFingers, atIndex, name, minConfidence, useMedianCalculation, medianWindowSize, useFilteredData);
		return -1;
	}

	FUBI_CPP_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel /*= Fubi::Math::MaxFloat*/,
		bool useLocalPositions /*= false*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float maxAngleDifference /*= 45.0f*/,
		bool useOnlyCorrectDirectionComponent /*= true*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addLinearMovementRecognizer(joint, relJoint, Vec3f(dirX, dirY, dirZ), minVel, maxVel, useLocalPositions, atIndex, name, -1.0f, maxAngleDifference, useOnlyCorrectDirectionComponent, useFilteredData);
		return -1;
	}
	FUBI_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel /*= Fubi::Math::MaxFloat*/,
		bool useLocalPositions /*= false*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float maxAngleDifference /*= 45.0f*/,
		bool useOnlyCorrectDirectionComponent /*= true*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addLinearMovementRecognizer(joint, Vec3f(dirX, dirY, dirZ), minVel, maxVel, useLocalPositions, atIndex, name, -1.0f, maxAngleDifference, useOnlyCorrectDirectionComponent, useFilteredData);
		return -1;
	}
	FUBI_CPP_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint, SkeletonJoint::Joint relJoint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel,
		float minLength, float maxLength /*= Fubi::Math::MaxFloat*/,
		Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
		bool useLocalPositions /*= false*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float maxAngleDifference /*= 45.0f*/,
		bool useOnlyCorrectDirectionComponent /*= true*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addLinearMovementRecognizer(joint, relJoint, Vec3f(dirX, dirY, dirZ), minVel, maxVel, minLength, maxLength, measuringUnit, useLocalPositions, atIndex, name, -1.0f, maxAngleDifference, useOnlyCorrectDirectionComponent, useFilteredData);
		return -1;
	}
	FUBI_CPP_API unsigned int addLinearMovementRecognizer(SkeletonJoint::Joint joint,
		float dirX, float dirY, float dirZ, float minVel, float maxVel,
		float minLength, float maxLength /*= Fubi::Math::MaxFloat*/,
		Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
		bool useLocalPositions /*= false*/,
		int atIndex /*= -1*/,
		const char* name /*= 0*/,
		float maxAngleDifference /*= 45.0f*/,
		bool useOnlyCorrectDirectionComponent /*= true*/,
		bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addLinearMovementRecognizer(joint, Vec3f(dirX, dirY, dirZ), minVel, maxVel, minLength, maxLength, measuringUnit, useLocalPositions, atIndex, name, -1.0f, maxAngleDifference, useOnlyCorrectDirectionComponent, useFilteredData);
		return -1;
	}

	FUBI_API unsigned int addAngularMovementRecognizer(Fubi::SkeletonJoint::Joint joint,
		float minVelX /*= -Fubi::Math::MaxFloat*/, float minVelY /*= -Fubi::Math::MaxFloat*/, float minVelZ /*= -Fubi::Math::MaxFloat*/,
		float maxVelX /*= Fubi::Math::MaxFloat*/, float maxVelY /*= Fubi::Math::MaxFloat*/, float maxVelZ /*= Fubi::Math::MaxFloat*/,
		bool useLocalOrients /*= true*/,
		int atIndex /*= -1*/, const char* name /*= 0*/,
		float minConfidence /*= -1.0f*/, bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addAngularMovementRecognizer(joint, Vec3f(minVelX, minVelY, minVelZ), Vec3f(maxVelX, maxVelY, maxVelZ), useLocalOrients, atIndex, name, minConfidence, useFilteredData);
		return -1;
	}

	FUBI_API unsigned int addTemplateRecognizer(Fubi::SkeletonJoint::Joint joint, Fubi::SkeletonJoint::Joint relJoint,
		const char* trainingDataFile, int startFrame, int endFrame,
		float maxDistance,
		Fubi::DistanceMeasure::Measure distanceMeasure /*= Fubi::DistanceMeasure::Euclidean*/,
		float maxRotation /*= 45.0f*/,
		bool aspectInvariant /*= false*/,
		unsigned int ignoreAxes /*= Fubi::CoordinateAxis::NONE*/,
		bool useOrientations /*= false*/,
		bool useDTW /*= true*/,
		float maxWarpingFactor /*= 0.5f*/,
		Fubi::ResamplingTechnique::Technique resamplingTechnique /*= Fubi::ResamplingTechnique::None*/,
		int resampleSize /*= -1*/,
		bool searchBestInputLength /*= false*/,
		Fubi::StochasticModel::Model stochasticModel /*= Fubi::StochasticModel::GMR*/,
		unsigned int numGMRStates /*= 5*/,
		Fubi::BodyMeasurement::Measurement measuringUnit /*= Fubi::BodyMeasurement::NUM_MEASUREMENTS*/,
		int atIndex /*= -1*/, const char* name /*= 0*/,
		float minConfidence /*= -1.0f*/,
		bool useLocalTransformations /*= false*/,
		bool useFilteredData /*= true*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addTemplateRecognizer(joint, relJoint, trainingDataFile, startFrame, endFrame, maxDistance,
			distanceMeasure, maxRotation, aspectInvariant, ignoreAxes, useOrientations, useDTW, maxWarpingFactor, resamplingTechnique, resampleSize,
			searchBestInputLength, stochasticModel, numGMRStates, measuringUnit,	atIndex, name, minConfidence, useLocalTransformations, useFilteredData);
		return -1;
	}

	FUBI_API unsigned int addTemplateRecognizerFromXML(const char* xmlDefinition, int atIndex /*= -1*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->addTemplateRecognizer(xmlDefinition, atIndex);
		return -1;
	}

	FUBI_API bool addCombinationRecognizer(const char* xmlDefinition)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && xmlDefinition)
			return core->addCombinationRecognizer(xmlDefinition);
		return false;
	}

	FUBI_API bool loadRecognizersFromXML(const char* fileName)
	{
		if (fileName)
			return FubiXMLParser::loadRecognizersFromXML(fileName);
		return false;
	}

	FUBI_API unsigned int getNumUserDefinedRecognizers()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getNumUserDefinedRecognizers();
		return 0;
	}

	FUBI_API const char* getUserDefinedRecognizerName(unsigned int recognizerIndex)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getUserDefinedRecognizerName(recognizerIndex).c_str();
		return "";
	}

	FUBI_API int getUserDefinedRecognizerIndex(const char* recognizerName)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getUserDefinedRecognizerIndex(recognizerName);
		return -1;
	}

	FUBI_API unsigned int getNumUserDefinedCombinationRecognizers()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getNumUserDefinedCombinationRecognizers();
		return 0;
	}

	FUBI_API const char* getUserDefinedCombinationRecognizerName(unsigned int recognizerIndex)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getUserDefinedCombinationRecognizerName(recognizerIndex).c_str();
		return "";
	}

	FUBI_API int getUserDefinedCombinationRecognizerIndex(const char* recognizerName)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getUserDefinedCombinationRecognizerIndex(recognizerName);
		return -1;
	}

	FUBI_API unsigned short getCurrentUsers(FubiUser*** pUserContainer /*= 0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getCurrentUsers(pUserContainer);

		return 0;
	}

	FUBI_API FubiUser* getUser(unsigned int id)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getUser(id);

		return 0;
	}

	FUBI_API void getDepthResolution(int& width, int& height)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->getDepthResolution(width, height);
		}
	}


	FUBI_API void getRgbResolution(int& width, int& height)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->getRgbResolution(width, height);
		}
	}

	FUBI_API void getIRResolution(int& width, int& height)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->getIRResolution(width, height);
		}
	}

	FUBI_API int getFingerCount(unsigned int userID, bool leftHand /*= false*/,
		bool getMedianOfLastFrames /*= true*/, unsigned int medianWindowSize /*= 10*/,
		bool useOldConvexityDefectMethod /*= false*/)
	{
		int numFingers = -1;

		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			numFingers = core->getFingerCount(userID, leftHand, getMedianOfLastFrames, medianWindowSize, useOldConvexityDefectMethod);
		}

		return numFingers;
	}

	FUBI_API int getHandFingerCount(const Fubi::TrackingData* trackingData)
	{
		const Fubi::FingerTrackingData* fData = dynamic_cast<const Fubi::FingerTrackingData*>(trackingData);
		if (fData)
			return fData->fingerCount;
		return 0;
	}

	FUBI_API void enableFingerTracking(unsigned int userID, bool leftHand, bool rightHand, bool useConvexityDefectMethod /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->enableFingerTracking(userID, leftHand, rightHand, useConvexityDefectMethod);
		}
	}

	FUBI_API bool isUserInScene(unsigned int userID)
	{
		FubiUser* user = getUser(userID);
		if (user)
		{
			return user->inScene();
		}
		return false;
	}

	FUBI_API bool isUserTracked(unsigned int userID)
	{
		FubiUser* user = getUser(userID);
		if (user)
		{
			return user->isTracked();
		}
		return false;
	}


	FUBI_API const Fubi::TrackingData* getCurrentTrackingData(unsigned int userId, bool filteredData /*= false*/)
	{
		FubiUser* user = getUser(userId);
		if (user)
		{
			return filteredData ? user->currentFilteredTrackingData() : user->currentTrackingData();
		}
		return 0;
	}

	FUBI_API const Fubi::TrackingData* getLastTrackingData(unsigned int userId, bool filteredData /*= false*/)
	{
		FubiUser* user = getUser(userId);
		if (user)
		{
			return  filteredData ? user->lastFilteredTrackingData() : user->lastTrackingData();
		}
		return 0;
	}


	FUBI_API void getSkeletonJointPosition(const Fubi::TrackingData* trackingData, SkeletonJoint::Joint joint,
		float& x, float& y, float& z, float& confidence, double& timeStamp, bool localPosition /*= false*/)
	{
		if (trackingData && joint < trackingData->numJoints)
		{
			SkeletonJointPosition jointPos;
			if (localPosition)
				jointPos = trackingData->localJointPositions[joint];
			else
				jointPos = trackingData->jointPositions[joint];
			x = jointPos.m_position.x;
			y = jointPos.m_position.y;
			z = jointPos.m_position.z;
			confidence = jointPos.m_confidence;
			timeStamp = trackingData->timeStamp;
		}
	}

	FUBI_API void getSkeletonJointOrientation(const Fubi::TrackingData* trackingData, SkeletonJoint::Joint joint,
		float* mat, float& confidence, double& timeStamp, bool localOrientation /*= true*/)
	{
		if (trackingData && mat && joint < trackingData->numJoints)
		{
			SkeletonJointOrientation jointOrient;
			if (localOrientation)
				jointOrient = trackingData->localJointOrientations[joint];
			else
				jointOrient = trackingData->jointOrientations[joint];
			for (int i = 0; i < 9; ++i)
			{
				mat[i] = jointOrient.m_orientation.x[i];
			}
			confidence = jointOrient.m_confidence;
			timeStamp = trackingData->timeStamp;
		}
	}


	FUBI_API void getBodyMeasurementDistance(unsigned int userId, Fubi::BodyMeasurement::Measurement measure, float& dist, float& confidence)
	{
		FubiUser* user = getUser(userId);
		if (user && measure != BodyMeasurement::NUM_MEASUREMENTS)
		{
			const BodyMeasurementDistance& md = user->bodyMeasurements()[measure];
			dist = md.m_dist;
			confidence = md.m_confidence;
		}
	}

	FUBI_API std::vector<Fubi::UserTrackingData>* createTrackingDataVector()
	{
		return new std::vector<Fubi::UserTrackingData>();
	}

	FUBI_API void releaseTrackingDataVector(std::vector<Fubi::UserTrackingData>* vec)
	{
		delete vec;
	}

	FUBI_API unsigned int getTrackingDataVectorSize(std::vector<Fubi::UserTrackingData>* vec)
	{
		if (vec)
			return (unsigned int)vec->size();
		return 0;
	}

	FUBI_API Fubi::UserTrackingData* getTrackingData(std::vector<Fubi::UserTrackingData>* vec, unsigned int index)
	{
		if (vec)
			return &((*vec)[index]);
		return 0;
	}

	FUBI_API unsigned int getClosestUserID()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getClosestUserID();
		}
		return 0;
	}

	FUBI_CPP_API std::deque<unsigned int> getClosestUserIDs(int maxNumUsers /*= -1*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getClosestUserIDs(maxNumUsers);
		}
		static std::deque<unsigned int> s_emptyDeque;
		return s_emptyDeque;
	}

	FUBI_API unsigned int getClosestUserIDs(unsigned int* userIds, int maxNumUsers /*= -1*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			std::deque<unsigned int> userDeque = core->getClosestUserIDs(maxNumUsers);
			for (unsigned int i = 0; i < userDeque.size(); ++i)
			{
				userIds[i] = userDeque[i];
			}
			return (unsigned int)userDeque.size();
		}
		return 0;
	}

	FUBI_API FubiUser* getClosestUser()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getClosestUser();
		}
		return 0x0;
	}

	FUBI_CPP_API std::deque<FubiUser*> getClosestUsers(int maxNumUsers /*= -1*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getClosestUsers(maxNumUsers);
		}
		static std::deque<FubiUser*> s_emptyDeque;
		return s_emptyDeque;
	}

	FUBI_API void clearUserDefinedRecognizers()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->clearUserDefinedRecognizers();
		}
	}

	FUBI_API void updateTrackingData(unsigned int userId, float* skeleton, double timeStamp /*= -1*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->updateTrackingData(userId, skeleton, timeStamp);
		}
	}

	FUBI_CPP_API Fubi::Vec3f convertCoordinates(const Fubi::Vec3f& inputCoords, Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->convertCoordinates(inputCoords, inputType, outputType);
		return NullVec;
	}

	FUBI_API void convertCoordinates(float inputCoordsX, float inputCoordsY, float inputCoordsZ, float& outputCoordsX, float& outputCoordsY, float& outputCoordsZ,
		Fubi::CoordinateType::Type inputType, Fubi::CoordinateType::Type outputType)
	{
		Vec3f result = convertCoordinates(Vec3f(inputCoordsX, inputCoordsY, inputCoordsZ), inputType, outputType);
		outputCoordsX = result.x;
		outputCoordsY = result.y;
		outputCoordsZ = result.z;
	}

	FUBI_API void resetTracking()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->resetTracking();
		}
	}

	FUBI_API double getCurrentTime()
	{
		return currentTime();
	}

	FUBI_API void setFilterOptions(float minCutOffFrequency /*= 1.0f*/, float velocityCutOffFrequency /*= 1.0f*/, 
		float cutOffSlope /*= 0.007f*/, float bodyMeasureFilterFac /*= 0.1f*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->setFilterOptions(minCutOffFrequency, velocityCutOffFrequency, cutOffSlope, bodyMeasureFilterFac);
		}
	}

	FUBI_API void getFilterOptions(float& filterMinCutOffFrequency, float& filterVelocityCutOffFrequency, 
		float& filterCutOffSlope, float& bodyMeasureFilterFac)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->getFilterOptions(filterMinCutOffFrequency, filterVelocityCutOffFrequency, filterCutOffSlope, bodyMeasureFilterFac);
		}
	}

	FUBI_API int getCurrentCombinationRecognitionState(const char* recognizerName, unsigned int userID, unsigned int& numStates, bool& isInterrupted, bool& isInTransition)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getCurrentCombinationRecognitionState(recognizerName, userID, numStates, isInterrupted, isInTransition);
		}
		return -2;
	}

	FUBI_API const char* getCombinationRecognitionStateMetaInfo(const char* recognizerName, unsigned int stateIndex, const char* propertyName)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getCombinationRecognitionStateMetaInfo(recognizerName, stateIndex, propertyName);
		}
		return 0x0;
	}

	FUBI_API bool initFingerSensor(Fubi::FingerSensorType::Type type, float offsetPosX /*= 0*/, float offsetPosY /*= -40.0f*/, float offsetPosZ /*= 20.0f*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->initFingerSensor(type, Vec3f(offsetPosX, offsetPosY, offsetPosZ));
		}
		return false;
	}

	FUBI_API Fubi::FingerSensorType::Type getCurrentFingerSensorType()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && core->getFingerSensor())
			return core->getFingerSensor()->getType();
		return Fubi::FingerSensorType::NONE;
	}

	FUBI_API int getAvailableFingerSensorTypes()
	{
		int ret = 0;
#ifdef FUBI_USE_LEAP
		ret |= FingerSensorType::LEAP;
#endif
		return ret;
	}

	FUBI_API unsigned short getNumHands()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getNumHands();
		}
		return 0;
	}

	FUBI_API unsigned int getHandID(unsigned int index)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getHandID(index);
		}
		return 0;
	}

	FUBI_API FubiHand* getHand(unsigned int id)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getHand(id);
		return 0x0;
	}

	FUBI_API const Fubi::TrackingData* getCurrentFingerTrackingData(unsigned int handId, bool filteredData /*= false*/)
	{
		FubiHand* hand = getHand(handId);
		if (hand)
		{
			return filteredData ? hand->currentFilteredTrackingData() : hand->currentTrackingData();
		}
		return 0;
	}

	FUBI_API const Fubi::TrackingData* getLastFingerTrackingData(unsigned int handId, bool filteredData /*= false*/)
	{
		FubiHand* hand = getHand(handId);
		if (hand)
		{
			return filteredData ? hand->lastFilteredTrackingData() : hand->lastTrackingData();
		}
		return 0;
	}

	FUBI_API void getSkeletonHandJointPosition(const Fubi::TrackingData* trackingData, SkeletonHandJoint::Joint joint,
		float& x, float& y, float& z, float& confidence, double& timeStamp, bool localPosition /*= false*/)
	{
		if (trackingData && joint < trackingData->numJoints)
		{
			SkeletonJointPosition jointPos;
			if (localPosition)
				jointPos = trackingData->localJointPositions[joint];
			else
				jointPos = trackingData->jointPositions[joint];
			x = jointPos.m_position.x;
			y = jointPos.m_position.y;
			z = jointPos.m_position.z;
			confidence = jointPos.m_confidence;
			timeStamp = trackingData->timeStamp;
		}
	}

	FUBI_API void getSkeletonHandJointOrientation(const Fubi::TrackingData* trackingData, SkeletonHandJoint::Joint joint,
		float* mat, float& confidence, double& timeStamp, bool localOrientation /*= true*/)
	{
		if (trackingData && mat && joint < trackingData->numJoints)
		{
			SkeletonJointOrientation jointOrient;
			if (localOrientation)
				jointOrient = trackingData->localJointOrientations[joint];
			else
				jointOrient = trackingData->jointOrientations[joint];
			for (int i = 0; i < 9; ++i)
			{
				mat[i] = jointOrient.m_orientation.x[i];
			}
			confidence = jointOrient.m_confidence;
			timeStamp = trackingData->timeStamp;
		}
	}

	FUBI_API FubiHand* getClosestHand()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getClosestHand();
		return 0x0;
	}

	FUBI_API unsigned int getClosestHandID()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getClosestHandID();
		return 0;
	}

	FUBI_API Fubi::RecognitionResult::Result recognizeGestureOnHand(const char* recognizerName, unsigned int handId)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && recognizerName)
			return core->recognizeGestureOnHand(recognizerName, handId);
		return Fubi::RecognitionResult::NOT_RECOGNIZED;
	}

	FUBI_API Fubi::RecognitionResult::Result getCombinationRecognitionProgressOnHand(const char* recognizerName, unsigned int handId,
		std::vector<Fubi::TrackingData>* trackingStates /*= 0x0*/, bool restart /*= true*/,
		bool returnFilteredData /*= false*/, Fubi::RecognitionCorrectionHint* correctionHint /*= 0x0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && recognizerName)
		{
			FubiHand* hand = core->getHand(handId);
			if (hand)
				return hand->getRecognitionProgress(recognizerName, trackingStates, restart, returnFilteredData, correctionHint);
		}
		return Fubi::RecognitionResult::NOT_RECOGNIZED;
	}


	FUBI_API void enableCombinationRecognitionHand(const char* combinationName, unsigned int handId, bool enable)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && combinationName)
			core->enableCombinationRecognitionHand(combinationName, handId, enable);
	}

	FUBI_API int getCurrentCombinationRecognitionStateForHand(const char* recognizerName, unsigned int handId, unsigned int& numStates, bool& isInterrupted, bool& isInTransition)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && recognizerName)
		{
			FubiHand* hand = core->getHand(handId);
			if (hand)
				return hand->getCurrentRecognitionState(recognizerName, numStates, isInterrupted, isInTransition);
		}
		return -2;
	}

	FUBI_API void getFingerSensorOffsetPosition(float& xOffset, float& yOffset, float& zOffset)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			const Vec3f& vec = core->getFingerSensorOffsetPosition();
			xOffset = vec.x;
			yOffset = vec.y;
			zOffset = vec.z;
		}
	}

	FUBI_API void setFingerSensorOffsetPosition(float xOffset, float yOffset, float zOffset)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			core->setFingerSensorOffsetPosition(Vec3f(xOffset, yOffset, zOffset));
		}
	}

	FUBI_API Fubi::RecognizerTarget::Target getCombinationRecognizerTargetSkeleton(const char* recognizerName)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getCombinationRecognizerTargetSkeleton(recognizerName);
		}
		return RecognizerTarget::INVALID;
	}

	FUBI_API Fubi::RecognizerTarget::Target getRecognizerTargetSkeleton(const char* recognizerName)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return core->getCombinationRecognizerTargetSkeleton(recognizerName);
		}
		return RecognizerTarget::INVALID;
	}

	FUBI_API void getCorrectionHint(Fubi::RecognitionCorrectionHint* res)
	{
		if (res)
		{
			res->m_joint = SkeletonJoint::TORSO;
			res->m_dirX = 42.0f;
		}
	}

	FUBI_API void getFloorPlane(float& normalX, float& normalY, float& normalZ, float& dist)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			Fubi::Plane plane = core->getFloor();
			normalX = plane.normal.x;
			normalY = plane.normal.y;
			normalZ = plane.normal.z;
			dist = plane.dist;
		}
		else
		{
			normalX = normalY = normalZ = 0;
		}
	}

	FUBI_API bool startRecordingSkeletonData(const char* fileName, unsigned int targetID, bool isHand /*= false*/, bool useRawData /*= true*/, bool useFilteredData /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->startRecordingSkeletonData(fileName, targetID, isHand, useRawData, useFilteredData);
		return false;
	}

	FUBI_API bool isRecordingSkeletonData(int* currentFrameID /*= 0x0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->isRecordingSkeletonData(currentFrameID);
		return false;
	}

	FUBI_API void stopRecordingSkeletonData()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->stopRecordingSkeletonData();
	}

	FUBI_API int loadRecordedSkeletonData(const char* fileName)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->loadRecordedSkeletonData(fileName);
		return -1;
	}

	FUBI_API void setPlaybackMarkers(int currentFrame, int startFrame /*= 0*/, int endFrame /*= -1*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->setPlaybackMarkers(currentFrame, startFrame, endFrame);
	}

	FUBI_API void getPlaybackMarkers(int& currentFrame, int& startFrame, int& endFrame)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->getPlaybackMarkers(currentFrame, startFrame, endFrame);
	}

	FUBI_API void startPlayingSkeletonData(bool loop /*= false*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->startPlayingSkeletonData(loop);
	}

	FUBI_API bool isPlayingSkeletonData(int* currentFrameID /*= 0x0*/, bool* isPaused /*= 0x0*/)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->isPlayingSkeletonData(currentFrameID, isPaused);
		return false;
	}

	FUBI_API void stopPlayingRecordedSkeletonData()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->stopPlayingRecordedSkeletonData();
	}

	FUBI_API void pausePlayingRecordedSkeletonData()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->pausePlayingRecordedSkeletonData();
	}

	FUBI_API double getPlaybackDuration()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getPlaybackDuration();
		return -1.0;
	}

	FUBI_API bool trimPlaybackFileToMarkers()
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->trimPlaybackFileToMarkers();
		return false;
	}

	FUBI_API void setRecognitionCallbacks(RecognitionCallback startCallback, RecognitionCallback endCallback)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			core->setRecognitionCallbacks(startCallback, endCallback);
	}

	FUBI_API bool getFingerSensorImage(unsigned char* outputImage, unsigned int imageIndex, Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
		{
			return FubiImageProcessing::getImage(core->getFingerSensor(), outputImage, imageIndex, numChannels, depth);
		}
		return false;
	}

	FUBI_API void getFingerSensorImageConfig(int& width, int& height, unsigned int& numStreams)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && core->getFingerSensor())
			core->getFingerSensor()->getImageConfig(width, height, numStreams);
		else
		{
			width = height = -1;
			numStreams = 0;
		}
	}

	FUBI_CPP_API const std::vector<Fubi::TemplateData>& getTemplateTrainingData(const char* recognizerName)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core && recognizerName)
			return core->getTemplateTrainingData(recognizerName);
		return Fubi::EmptyTemplateDataVec;
	}

	FUBI_CPP_API const std::vector<Fubi::TemplateData>& getTemplateTrainingData(unsigned int recognizerIndex)
	{
		FubiCore* core = FubiCore::getInstance();
		if (core)
			return core->getTemplateTrainingData(recognizerIndex);
		return Fubi::EmptyTemplateDataVec;
	}

	FUBI_CPP_API void plotImage(const std::vector<Fubi::Vec3f>& dataToPlot, unsigned char* outputImage, unsigned int width, unsigned height,
		Fubi::ImageNumChannels::Channel numChannels, Fubi::ImageDepth::Depth depth, int lineThickness /*= 1*/)
	{
		FubiImageProcessing::plotImage(dataToPlot, 0x0, outputImage, width, height, numChannels, depth, lineThickness);
	}
}
