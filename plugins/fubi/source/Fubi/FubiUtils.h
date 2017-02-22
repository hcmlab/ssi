// ****************************************************************************************
//
// Fubi Utility Functions
// ---------------------------------------------------------
// Copyright (C) 2010-2015 Felix Kistler
//
// This software is distributed under the terms of the Eclipse Public License v1.0.
// A copy of the license may be obtained at: http://www.eclipse.org/org/documents/epl-v10.html
//
// ****************************************************************************************
#pragma once

/** \file FubiUtils.h
 * \brief Utility functions and data structures
*/

#include <time.h>
#include <cmath>
#include <cstring>
#include <sstream>
#include <vector>
#include <deque>

// Forward declarations for OpenCV
namespace cv
{
	class Mat;
};

namespace Fubi
{
	/**
	* \brief Options for image rendering
	*/
	struct ImageType
	{
		/**
		* \brief The possible image types
		*/
		enum Type
		{
			Color = 0,
			Depth,
			IR,
			Blank
		};
	};

	/**
	* \brief The number of channels in the image
	*/
	struct ImageNumChannels
	{
		enum Channel
		{
			C1 = 1,
			C3 = 3,
			C4 = 4
		};
	};
	/**
	* \brief The depth of each channel
	*/
	struct ImageDepth
	{

		enum Depth
		{
			D8 = 8,
			D16 = 16,
			F32 = 32
		};
	};
	/**
	* \brief How the depth image should be modified for depth differences
	*		 being easier to distinguish by the human eye
	*/
	struct DepthImageModification
	{

		enum Modification
		{
			Raw = 0,
			UseHistogram,
			StretchValueRange,
			ConvertToRGB
		};
	};
	/**
	* \brief options for (de-)activating different parts of the rendered tracking information
	*/
	struct RenderOptions
	{
		/**
		* \brief The possible formats for the tracking info rendering
		*/
		enum Options
		{
			None							= 0,
			Shapes							= 1,
			Skeletons						= 2,
			UserCaptions					= 4,
			LocalOrientCaptions				= 8,
			GlobalOrientCaptions			= 16,
			LocalPosCaptions				= 32,
			GlobalPosCaptions				= 64,
			Background						= 128,
			SwapRAndB						= 256,
			FingerShapes					= 512,
			DetailedFaceShapes				= 1024,
			BodyMeasurements				= 2048,
			UseFilteredValues				= 4096
		};

		/**
		* \brief IDs for the Joints to define which of them should be rendered
		*/
		enum Joint
		{
			ALL_JOINTS		= 0xFFFFFFFF,

			HEAD			= 0x00000001,
			NECK			= 0x00000002,
			TORSO			= 0x00000004,
			WAIST			= 0x00000008,

			LEFT_SHOULDER	= 0x00000010,
			LEFT_ELBOW		= 0x00000020,
			LEFT_WRIST		= 0x00000040,
			LEFT_HAND		= 0x00000080,

			RIGHT_SHOULDER	= 0x00000100,
			RIGHT_ELBOW		= 0x00000200,
			RIGHT_WRIST		= 0x00000400,
			RIGHT_HAND		= 0x00000800,

			LEFT_HIP		= 0x00001000,
			LEFT_KNEE		= 0x00002000,
			LEFT_ANKLE		= 0x00004000,
			LEFT_FOOT		= 0x00008000,

			RIGHT_HIP		= 0x00010000,
			RIGHT_KNEE		= 0x00020000,
			RIGHT_ANKLE		= 0x00040000,
			RIGHT_FOOT		= 0x00080000,

			FACE_NOSE		= 0x00100000,
			FACE_LEFT_EAR	= 0x00200000,
			FACE_RIGHT_EAR	= 0x00400000,
			FACE_FOREHEAD	= 0x00800000,
			FACE_CHIN		= 0x01000000,

			PALM			= 0x02000000,
			THUMB			= 0x04000000,
			INDEX			= 0x08000000,
			MIDDLE			= 0x10000000,
			RING			= 0x20000000,
			PINKY			= 0x40000000
		};
	};

	/**
	* \brief IDs for all supported body tracking skeleton joints
	*/
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

	/**
	* \brief IDs for all supported finger tracking skeleton joints
	*/
	struct SkeletonHandJoint
	{
		enum Joint
		{
			PALM		= 0,
			THUMB		= 1,
			INDEX		= 2,
			MIDDLE		= 3,
			RING		= 4,
			PINKY		= 5,

			NUM_JOINTS
		};
	};

	/**
	* \brief IDs for all supported body measurements
	*/
	struct BodyMeasurement
	{
		enum Measurement
		{
			BODY_HEIGHT			= 0,
			TORSO_HEIGHT		= 1,
			SHOULDER_WIDTH		= 2,
			HIP_WIDTH			= 3,
			ARM_LENGTH			= 4,
			UPPER_ARM_LENGTH	= 5,
			LOWER_ARM_LENGTH	= 6,
			LEG_LENGTH			= 7,
			UPPER_LEG_LENGTH	= 8,
			LOWER_LEG_LENGTH	= 9,
			NUM_MEASUREMENTS
		};
	};

	/**
	* \brief Profiles for tracking specific parts of the body (not working for all sensors)
	*/
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

	/**
	* \brief All supported body tracking sensor types, need to be enabled in the FubiConfig.h
	*        Further installation instructions in the readme/doc
	*/
	struct SensorType
	{
		enum Type
		{
			/** No sensor in use **/
			NONE = 0,
			/** Sensor based on OpenNI 2.x**/
			OPENNI2 = 0x01,
			/** Sensor based on OpenNI 1.x**/
			OPENNI1 = 0x02,
			/** Sensor based on the Kinect for Windows SDK 1.x**/
			KINECTSDK = 0x04,
			/** Sensor based on the Kinect for Windows SDK 2.x**/
			KINECTSDK2 = 0x08
		};
	};

	/**
	* \brief All supported finger tracking sensor types, need to be enabled in the FubiConfig.h
	*        Further installation instructions in the readme/doc
	*/
	struct FingerSensorType
	{
		enum Type
		{
			/** No sensor in use **/
			NONE = 0,
			/** Finger tracking with LEAP **/
			LEAP = 0x01
		};
	};

	/**
	* \brief A recognizer can target hands (with finger) joints or the body skeleton (without fingers)
	*/
	struct RecognizerTarget
	{
		enum Target
		{
			/** No target, usually means combination recognizer that is not yet initialized or something went wrong **/
			INVALID = -1,
			/** A full body skeleton as provided by the Kinect **/
			BODY = 0,
			/** A finger skeleton as provided by the leap motion sensor or finger counts and targeting left hands **/
			LEFT_HAND = 1,
			/** A finger skeleton as provided by the leap motion sensor or finger counts and targeting right hands **/
			RIGHT_HAND = 2,
			/** A finger skeleton as provided by the leap motion sensor or finger counts and targeting any hand **/
			BOTH_HANDS = 3,
			/** Combination recognizer that includes recognizers that refer both types of skeletons **/
			BODY_AND_HANDS = 4
		};
	};

	/**
	* \brief A recognizer can be either user-defined or not and basic or a combination
	*/
	struct RecognizerType
	{
		enum Type
		{
			USERDEFINED_GESTURE = 0,
			USERDEFINED_COMBINATION = 1,
			PREDEFINED_GESTURE = 2,
			PREDEFINED_COMBINATION = 3,
			NUM_TYPES
		};
	};

	/**
	* \brief Result of a gesture recognition
	*/
	struct RecognitionResult
	{
		enum Result
		{
			TRACKING_ERROR = -1,
			NOT_RECOGNIZED = 0,
			RECOGNIZED = 1,
			WAITING_FOR_LAST_STATE_TO_FINISH = 2	// Only for combinations with waitUntilLastStateRecognizersStop flag
		};
	};

	/**
	* \brief Coordinate types for conversion
	*/
	struct CoordinateType
	{
		enum Type
		{
			COLOR = 0,
			DEPTH,
			IR,
			REAL_WORLD
		};
	};

	/**
	* \brief Different kinds of distance measures, e.g. used in DTW function
	*/
	struct DistanceMeasure
	{
		enum Measure
		{
			Manhattan = 0,
			Euclidean = 1,
			Malhanobis = 2,
			TurningAngleDiff = 3
		};
	};

	/**
	* \brief Different kinds of resampling techniques, e.g. used for the TemplateRecognizers
	*/
	struct ResamplingTechnique
	{
		enum Technique
		{
			None = 0,
			EquiDistant = 1,
			HermiteSpline = 2,
			PolyLine = 3
		};
	};

	/**
	* \brief Coordinate axis flags that can be combined in one paramter
	*/
	struct CoordinateAxis
	{
		enum Axis
		{
			NONE = 0x0,
			X = 0x1,
			Y = 0x2,
			Z = 0x4
		};
	};

	/**
	* \brief Stochastic models to be used in template recognizers
	*/
	struct StochasticModel
	{
		enum Model
		{
			NONE = 0x0,
			/*HMM = 0x1,*/
			GMR = 0x2
		};
	};


	/**
	* \brief Additional information about what went wrong and how to correct it
	*/
	class Vec3f;
	struct RecognitionCorrectionHint
	{
		/**
		* \brief the type of what needs to be changed
		*/
		enum ChangeType
		{
			SPEED,
			POSE,
			DIRECTION,
			FORM,
			FINGERS
		};
		/**
		* \brief the direction (if any) of what needs to be changed
		*/
		enum ChangeDirection
		{
			DIFFERENT,
			MORE,
			LESS
		};

		SkeletonJoint::Joint m_joint;
		float m_dirX, m_dirY, m_dirZ, m_dist;
		bool m_isAngle;
		ChangeType m_changeType;
		ChangeDirection m_changeDirection;
		BodyMeasurement::Measurement m_measuringUnit;
		int m_failedState;
		RecognizerTarget::Target m_recTarget;
		/**
		* \brief raw or normalized input data vector as used by the template recognizers
		* needs to be initialized if you want to get it
		*/		
		std::vector<Fubi::Vec3f>* m_inputData;
		/**
		* \brief set to true if you want to get the input data with normalizations applied
		*/
		bool m_normalizeInputData;

		RecognitionCorrectionHint(SkeletonJoint::Joint joint = SkeletonJoint::NUM_JOINTS, float dirX = 0, float dirY = 0, float dirZ = 0,
			float dist = 0, bool isAngle = false, ChangeType changeType = SPEED, ChangeDirection changeDirection = DIFFERENT,
			BodyMeasurement::Measurement measuringUnit = BodyMeasurement::NUM_MEASUREMENTS, int failedState = -1,
			RecognizerTarget::Target recTarget = RecognizerTarget::BODY)
			: m_joint(joint), m_dirX(dirX), m_dirY(dirY), m_dirZ(dirZ), m_dist(dist), m_isAngle(isAngle),
			m_changeType(changeType), m_changeDirection(changeDirection), m_measuringUnit(measuringUnit),
			m_failedState(failedState), m_recTarget(recTarget), m_inputData(0x0), m_normalizeInputData(false)
		{}
		RecognitionCorrectionHint& RecognitionCorrectionHint::operator=(const RecognitionCorrectionHint& other)
		{
			m_joint = other.m_joint;
			m_dirX = other.m_dirX; m_dirY = other.m_dirY; m_dirZ = other.m_dirZ;
			m_dist = other.m_dist;
			m_isAngle = other.m_isAngle;
			m_changeType = other.m_changeType;
			m_changeDirection = other.m_changeDirection;
			m_measuringUnit = other.m_measuringUnit;
			m_failedState = other.m_failedState;
			m_recTarget = other.m_recTarget;
			m_normalizeInputData = other.m_normalizeInputData;
			if (m_inputData != 0x0 && other.m_inputData != 0x0)
			{
				*m_inputData = *other.m_inputData;
			}
			return *this;
		}
	};
	/**
	* \brief Options for configuring a stream of a sensor
	*/
	struct StreamOptions
	{
		StreamOptions(int width = 640, int height = 480, int fps = 30)
			: m_width(width), m_height(height), m_fps(fps)
		{}
		void invalidate()
		{ m_width = -1; m_height = -1; m_fps = -1; }
		bool isValid() const
		{ return m_width > 0 && m_height > 0 && m_fps > 0; }
		int m_width;
		int m_height;
		int m_fps;
	};

	static const StreamOptions DefaultStreamOptions = StreamOptions();

	/**
	* \brief Parameters for configuring the filtering of joint tracking data
	*/
	struct FilterOptions
	{
		FilterOptions(float minCutOffFrequency = 1.0f, float velocityCutOffFrequency = 1.0f, float cutOffSlope = 0.007f, float bodyMeasureFilterFac = 0.1f)
			:  m_minCutOffFrequency(minCutOffFrequency), m_velocityCutOffFrequency(velocityCutOffFrequency), 
			m_cutOffSlope(cutOffSlope), m_bodyMeasureFilterFac(bodyMeasureFilterFac)
		{}
		float m_minCutOffFrequency;
		float m_velocityCutOffFrequency;
		float m_cutOffSlope;
		float m_bodyMeasureFilterFac;
	};

	/**
	* \brief Options for a sensor, including which streams to use, tracking options and global options for all streams
	*/
	struct SensorOptions
	{
		SensorOptions(const StreamOptions& depthOptions = StreamOptions(),
			const StreamOptions& rgbOptions = StreamOptions(), const StreamOptions& irOptions = StreamOptions(-1, -1, -1),
			SensorType::Type sensorType = SensorType::OPENNI2,
			const SkeletonTrackingProfile::Profile trackingProfile = SkeletonTrackingProfile::ALL,
			bool mirrorStreams = true, bool registerStreams = true)
			: m_depthOptions(depthOptions), m_irOptions(irOptions), m_rgbOptions(rgbOptions),
			m_trackingProfile(trackingProfile), m_mirrorStreams(mirrorStreams), m_registerStreams(registerStreams),
			m_type(sensorType)
		{}
		StreamOptions m_depthOptions;
		StreamOptions m_irOptions;
		StreamOptions m_rgbOptions;

		SkeletonTrackingProfile::Profile m_trackingProfile;
		bool m_mirrorStreams;
		bool m_registerStreams;

		SensorType::Type m_type;
	};


	/**
	* \brief Processed part of the depth stream with tracking data
	*        by Fubi's build in finger tracking
	*/
	struct FingerCountImageData
	{
		FingerCountImageData() : image(0x0), timeStamp(-1) {}
		cv::Mat* image;
		double timeStamp;
		int fingerCount;
		int posX, posY;
	};

	/**
	* \brief Data used and provided by the finger tracking
	*/
	struct FingerCountData
	{
		FingerCountData() : trackingEnabled(false),
			trackingIntervall(0.1), maxHistoryLength(10), useConvexityDefectMethod(false) {}
		~FingerCountData();
		// General options
		bool trackingEnabled;
		bool useConvexityDefectMethod;
		double trackingIntervall;
		unsigned int maxHistoryLength;
		// Fingercounts
		std::deque<int> fingerCounts;
		// And corresponding timestamps
		std::deque<double> timeStamps;
		Fubi::FingerCountImageData fingerCountImage;
	};	

	/**
	* \brief Maximum depth value that can occure in the depth image
	*/
	static const int MaxDepth = 10000;
	/**
	* \brief Maximum number of tracked users
	*/
	static const int MaxUsers = 15;
	/**
	* \brief UserID for the playback of recorded skeleton data
	*/
	static const int PlaybackUserID = MaxUsers+1;
	/**
	* \brief And hands
	*/
	static const int MaxHands = 2*MaxUsers;
	/**
	* \brief HandID for the playback of recorded skeleton data
	* As the hand ids have no fixed limit we need take max int to be on the safe side...
	*/
	static const int PlaybackHandID = 0x7FFFFFFF;

	/**
	* \brief Maximum number of iterations used to calculate a GMM during the expectation maximization
	*/
	static const int MaxGMMEMIterations = 200;


	/**
	* \brief Logging functions
	*  Note: For internal use only!
	*/
	class Logging
	{
	public:
		static void logInfo(const char* msg, ...);
		static void logDbg(const char* file, int line, const char* msg, ...);
		static void logWrn(const char* file, int line, const char* msg, ...);
		static void logErr(const char* file, int line, const char* msg, ...);
	};
#ifdef _MSC_VER
#define Fubi_logInfo(msg, ...) Fubi::Logging::logInfo((msg), __VA_ARGS__)
#define Fubi_logDbg(msg, ...) Fubi::Logging::logDbg(__FILE__, __LINE__, (msg), __VA_ARGS__)
#define Fubi_logWrn(msg, ...) Fubi::Logging::logWrn(__FILE__, __LINE__, (msg), __VA_ARGS__)
#define Fubi_logErr(msg, ...) Fubi::Logging::logErr(__FILE__, __LINE__, (msg), __VA_ARGS__)
#else
#define Fubi_logInfo(msg, ...) Fubi::Logging::logInfo((msg), ##__VA_ARGS__)
#define Fubi_logDbg(msg, ...) Fubi::Logging::logDbg(__FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define Fubi_logWrn(msg, ...) Fubi::Logging::logWrn(__FILE__, __LINE__, (msg), ##__VA_ARGS__)
#define Fubi_logErr(msg, ...) Fubi::Logging::logErr(__FILE__, __LINE__, (msg), ##__VA_ARGS__)
#endif

	/**
	* \brief Convert to a uniform string only containing lower case letters and no white spaces
	*
	* @param str the string to convert
	* @return the converted string without any instances of ' ', '_', '-', or '|' and all upper case letters converted to lower case
	*/
	static std::string removeWhiteSpacesAndToLower(const std::string& str)
	{
		std::string ret;
		for (unsigned int i = 0; i < str.length(); ++i)
		{
			const char& c = str[i];
			if (c != ' ' && c != '_' && c != '-' && c != '|')
			{
				if (c >= 'A' && c <= 'Z')
					ret += (c + ('a'-'A'));
				else
					ret += c;
			}
		}
		return ret;
	}

	/**
	* \brief Splits a string along defined delimiters and returns a vector of the resulting tokens
	*
	* @param string the string to split
	* @param delimiters a vector of one or more delimiters that are used to separate tokens in the string
	* @param[out] tokens this vector will be filled with the found tokens
	*/
	static void splitString(const std::string& string, const std::string& delimiters, std::vector<std::string>& tokens)
	{
		// Skip delimiters at beginning looking for the first non-delimiter
		size_t start = string.find_first_not_of(delimiters, 0);

		while (start != std::string::npos)
		{
			// Found a token start
			// Now find its end (first delimiter after the non-delimiter)
			size_t end = string.find_first_of(delimiters, start);
			// And add the token to the vector
			tokens.push_back(string.substr(start, end - start));
			// Find next non-delimiter after the token
			start = string.find_first_not_of(delimiters, end);
		}
	}

	/**
	* \brief Get directory path out of a file's path
	*
	* @param filePath the path of a file
	* @return the directory name of the file
	*/
	static std::string getDirectoryPath(const std::string& filePath)
	{
		size_t lastSlash = filePath.find_last_of("/\\");
		return (lastSlash == std::string::npos) ? "" : filePath.substr(0, lastSlash);
	}

	/**
	* \brief Get base file name out of a file's path
	*
	* @param filePath the path of a file
	* @return the base file name
	*/
	static std::string getBaseFileName(const std::string& filePath)
	{
		size_t lastSlash = filePath.find_last_of("/\\");
		return (lastSlash == std::string::npos) ? filePath : filePath.substr(lastSlash + 1);
	}

	/**
	* \brief Converts a number to string
	*
	* @param num the number to convert, should be a numeric type
	* @param precision (=2) number of decimal places to be displayed
	* @return the string representation of the give number
	*/
	template<class T> std::string numToString(T num, unsigned short precision = 2)
	{
		std::stringstream ss;
		ss.precision(precision);
		ss << std::fixed << num;
		return ss.str();
	}

	/**
	* \brief Ensures that a value is between certain bounds
	*
	* @param  value the value to be clamped
	* @param  min the minimum allowed for the value
	* @param  max the maximum allowed for the value
	* @return returns the clamped value (value if between min and max, else min or max)
	*/
	template<class T> static inline T clamp(const T& value, const T& min, const T& max)
	{
		if (value < min) return min;
		else if (value > max) return max;
		return value;
	}

	/**
	* \brief Ensures that a value is between certain bounds
	*
	* @param  value the value to be clamped
	* @param  min the minimum allowed for the value
	* @param  max the maximum allowed for the value
	* @param valueToSet this will be returned if a value exceeds the given borders
	* @return returns the clamped value (value if between min and max, else min or max)
	*/
	template<class T> static inline T clamp(const T& value, const T& min, const T& max, const T& valueToSet)
	{
		if( value < min || value > max )
			return valueToSet;
		return value;
	}

	/**
	* \brief Checks wether a value is between certain bounds
	*
	* @param  value the value to be checked
	* @param  min the minimum allowed for the value
	* @param  max the maximum allowed for the value
	* @return returns the distance to the value, i.e. (value-min) if too small, (value-max) if too large, 0 if in range
	*/
	template<class T> static inline T inRange(const T& value, const T& min, const T& max)
	{
		if (value < min)
			return value-min;
		else if (value > max)
			return value-max;
		return T();
	}

	/**
	* \brief Checks whether two values have the same sign
	*
	* @param  a first value
	* @param  b second value
	* @return true if they have the same sign
	*/
	template<class T> static inline bool sameSign( T a, T b )
	{
		return (a <= 0 && b <= 0) || (a >= 0 && b >= 0);
	}



	/**
	* \brief Number of seconds since the program start
	*  Note: For internal use only, use Fubi::getCurrentTime() instead!
	*/
	double currentTime();

	/**
	* \brief Get a string representing the joint name for a specific joint id
	*/
	static const char* getJointName(SkeletonJoint::Joint id)
	{
		switch(id)
		{
		case SkeletonJoint::HEAD:
			return "head";
		case SkeletonJoint::NECK:
			return "neck";
		case SkeletonJoint::TORSO:
			return "torso";
		case SkeletonJoint::WAIST:
			return "waist";

		case SkeletonJoint::LEFT_SHOULDER:
			return "leftShoulder";
		case SkeletonJoint::LEFT_ELBOW:
			return "leftElbow";
		case SkeletonJoint::LEFT_WRIST:
			return "leftWrist";
		case SkeletonJoint::LEFT_HAND:
			return "leftHand";

		case SkeletonJoint::RIGHT_SHOULDER:
			return "rightShoulder";
		case SkeletonJoint::RIGHT_ELBOW:
			return "rightElbow";
		case SkeletonJoint::RIGHT_WRIST:
			return "rightWrist";
		case SkeletonJoint::RIGHT_HAND:
			return "rightHand";

		case SkeletonJoint::LEFT_HIP:
			return "leftHip";
		case SkeletonJoint::LEFT_KNEE:
			return "leftKnee";
		case SkeletonJoint::LEFT_ANKLE:
			return "leftAnkle";
		case SkeletonJoint::LEFT_FOOT:
			return "leftFoot";

		case SkeletonJoint::RIGHT_HIP:
			return "rightHip";
		case SkeletonJoint::RIGHT_KNEE:
			return "rightKnee";
		case SkeletonJoint::RIGHT_ANKLE:
			return "rightAnkle";
		case SkeletonJoint::RIGHT_FOOT:
			return "rightFoot";

		case SkeletonJoint::FACE_CHIN:
			return "faceChin";
		case SkeletonJoint::FACE_FOREHEAD:
			return "faceForeHead";
		case SkeletonJoint::FACE_LEFT_EAR:
			return "faceLeftEar";
		case SkeletonJoint::FACE_NOSE:
			return "faceNose";
		case SkeletonJoint::FACE_RIGHT_EAR:
			return "faceRightEar";
		default:
			return "";
		}

		return "";
	}

	/**
	* \brief Get the joint id out of the given joint name as a string
	*/
	static SkeletonJoint::Joint getJointID(const char* name)
	{
		if (name)
		{
			std::string lowerName = removeWhiteSpacesAndToLower(name);
			if (lowerName == "head")
				return SkeletonJoint::HEAD;
			if (lowerName == "neck")
				return SkeletonJoint::NECK;
			if (lowerName == "torso")
				return SkeletonJoint::TORSO;
			if (lowerName == "waist")
				return SkeletonJoint::WAIST;

			if (lowerName == "leftshoulder")
				return SkeletonJoint::LEFT_SHOULDER;
			if (lowerName == "leftelbow")
				return SkeletonJoint::LEFT_ELBOW;
			if (lowerName == "leftwrist")
				return SkeletonJoint::LEFT_WRIST;
			if (lowerName == "lefthand")
				return SkeletonJoint::LEFT_HAND;


			if (lowerName == "rightshoulder")
				return SkeletonJoint::RIGHT_SHOULDER;
			if (lowerName == "rightelbow")
				return SkeletonJoint::RIGHT_ELBOW;
			if (lowerName == "rightwrist")
				return SkeletonJoint::RIGHT_WRIST;
			if (lowerName == "righthand")
				return SkeletonJoint::RIGHT_HAND;

			if (lowerName == "lefthip")
				return SkeletonJoint::LEFT_HIP;
			if (lowerName == "leftknee")
				return SkeletonJoint::LEFT_KNEE;
			if (lowerName == "leftankle")
				return SkeletonJoint::LEFT_ANKLE;
			if (lowerName == "leftfoot")
				return SkeletonJoint::LEFT_FOOT;

			if (lowerName == "righthip")
				return SkeletonJoint::RIGHT_HIP;
			if (lowerName == "rightknee")
				return SkeletonJoint::RIGHT_KNEE;
			if (lowerName == "rightankle")
				return SkeletonJoint::RIGHT_ANKLE;
			if (lowerName == "rightfoot")
				return SkeletonJoint::RIGHT_FOOT;

			if (lowerName == "facechin")
				return SkeletonJoint::FACE_CHIN;
			if (lowerName == "faceforehead")
				return SkeletonJoint::FACE_FOREHEAD;
			if (lowerName == "faceleftear")
				return SkeletonJoint::FACE_LEFT_EAR;
			if (lowerName == "facenose")
				return SkeletonJoint::FACE_NOSE;
			if (lowerName == "facerightear")
				return SkeletonJoint::FACE_RIGHT_EAR;
		}

		return SkeletonJoint::NUM_JOINTS;
	}

	/**
	* \brief Get the body measurement id out of the given measure name
	*/
	static BodyMeasurement::Measurement getBodyMeasureID(const char* name)
	{
		if (name)
		{
			std::string lowerName = removeWhiteSpacesAndToLower(name);
			if (lowerName =="bodyheight")
				return BodyMeasurement::BODY_HEIGHT;
			if (lowerName =="torsoheight")
				return BodyMeasurement::TORSO_HEIGHT;
			if (lowerName =="shoulderwidth")
				return BodyMeasurement::SHOULDER_WIDTH;
			if (lowerName =="hipwidth")
				return BodyMeasurement::HIP_WIDTH;
			if (lowerName =="armlength")
				return BodyMeasurement::ARM_LENGTH;
			if (lowerName =="upperarmlength")
				return BodyMeasurement::UPPER_ARM_LENGTH;
			if (lowerName =="lowerarmlength")
				return BodyMeasurement::LOWER_ARM_LENGTH;
			if (lowerName =="leglength")
				return BodyMeasurement::LEG_LENGTH;
			if (lowerName =="upperleglength")
				return BodyMeasurement::UPPER_LEG_LENGTH;
			if (lowerName =="lowerleglength")
				return BodyMeasurement::LOWER_LEG_LENGTH;
		}

		return BodyMeasurement::NUM_MEASUREMENTS;
	}

	/**
	* \brief Get the name string representation of a body measurement id
	*/
	static const char* getBodyMeasureName(BodyMeasurement::Measurement id)
	{
		switch(id)
		{
		case BodyMeasurement::BODY_HEIGHT:
			return "bodyHeight";
		case BodyMeasurement::TORSO_HEIGHT:
			return "torsoHeight";
		case BodyMeasurement::SHOULDER_WIDTH:
			return "shoulderWidth";
		case BodyMeasurement::HIP_WIDTH:
			return "hipWidth";
		case BodyMeasurement::ARM_LENGTH:
			return "armLength";
		case BodyMeasurement::UPPER_ARM_LENGTH:
			return "upperArmLength";
		case BodyMeasurement::LOWER_ARM_LENGTH:
			return "lowerArmLength";
		case BodyMeasurement::LEG_LENGTH:
			return "legLength";
		case BodyMeasurement::UPPER_LEG_LENGTH:
			return "upperLegLength";
		case BodyMeasurement::LOWER_LEG_LENGTH:
			return "lowerLegLength";
		default:
			return "";
		}
		return "";
	}

	/**
	* \brief Get a string representing the hand joint name for a specific hand joint id
	*/
	static const char* getHandJointName(SkeletonHandJoint::Joint id)
	{
		switch(id)
		{
		case SkeletonHandJoint::PALM:
			return "palm";
		case SkeletonHandJoint::THUMB:
			return "thumb";
		case SkeletonHandJoint::INDEX:
			return "index";
		case SkeletonHandJoint::MIDDLE:
			return "middle";
		case SkeletonHandJoint::RING:
			return "ring";
		case SkeletonHandJoint::PINKY:
			return "pinky";
		default:
			return "";
		}

		return "";
	}

	/**
	* \brief Get the hand joint id out of the given hand joint name as a string
	*/
	static SkeletonHandJoint::Joint getHandJointID(const char* name)
	{
		if (name)
		{
			std::string lowerName = removeWhiteSpacesAndToLower(name);
			if (lowerName == "palm")
				return SkeletonHandJoint::PALM;
			if (lowerName == "thumb")
				return SkeletonHandJoint::THUMB;
			if (lowerName == "index")
				return SkeletonHandJoint::INDEX;
			if (lowerName == "middle")
				return SkeletonHandJoint::MIDDLE;
			if (lowerName == "ring")
				return SkeletonHandJoint::RING;
			if (lowerName == "pinky")
				return SkeletonHandJoint::PINKY;
		}

		return SkeletonHandJoint::NUM_JOINTS;
	}

	/**
	* \brief Get a string representing the joint name for a specific hand or body joint id
	*/
	static const char* getJointName(SkeletonJoint::Joint id, RecognizerTarget::Target targetSkeleton)
	{
		if (targetSkeleton != RecognizerTarget::BODY && targetSkeleton != RecognizerTarget::BODY_AND_HANDS
			&& id < SkeletonHandJoint::NUM_JOINTS)
			return getHandJointName((SkeletonHandJoint::Joint) id);
		return getJointName(id);
	}

	/**
	 * \brief Creates a string message out of a RecognitionCorrectionHint
	*/
	static std::string createCorrectionHintMsg(const RecognitionCorrectionHint& hint)
	{
		std::stringstream msg;
		msg << std::fixed;

		if (hint.m_failedState > -1)
		{
			msg << "State " << hint.m_failedState << " - ";
		}

		if (hint.m_changeType == RecognitionCorrectionHint::FINGERS)
		{
			msg.precision(0);
			msg << "Please show " << hint.m_dist << ((hint.m_dist > 0) ? " more fingers\n" : " less fingers\n");
		}
		else if (hint.m_changeType == RecognitionCorrectionHint::FORM)
		{
			msg.precision(3);
			msg << "Please perform more accurately, current distance to template: " << hint.m_dist << "\n";
		}
		else if (hint.m_changeType == RecognitionCorrectionHint::DIRECTION)
		{
			msg.precision(2);
			std::string action = hint.m_isAngle ? "turn" : "move";
			std::string direction = "";
			if (hint.m_dirX > 0.000001f)
				direction += hint.m_isAngle ? "up " : "right ";
			else if (hint.m_dirX < -0.000001f)
				direction += hint.m_isAngle ? "down " : "left ";
			if (hint.m_dirY > 0.000001f)
				direction += hint.m_isAngle ? "left " : "up ";
			else if (hint.m_dirY < -0.000001f)
				direction += hint.m_isAngle ? "right " : "down ";
			if (hint.m_dirZ > 0.000001f)
				direction += hint.m_isAngle ? "roll left " : "backward ";
			else if (hint.m_dirZ < -0.000001f)
				direction += hint.m_isAngle ? "roll right " : "forward ";
			msg << "Please " << action << " " << getJointName(hint.m_joint, hint.m_recTarget) << " "
				<< direction << ":" << hint.m_dirX << "/" << hint.m_dirY << "/" << hint.m_dirZ << "\n";
		}
		else // SPEED or POSE
		{
			msg.precision(2);
			for (int dirI = 0; dirI < 3; ++dirI)
			{
				float value = hint.m_dirX;
				std::string direction = (value < 0) ? "left" : "right";
				std::string action = hint.m_isAngle ? "pitch" : "move";
				if (hint.m_isAngle)
					direction = (value < 0) ? "down" : "up";
				if (dirI == 1)
				{
					value = hint.m_dirY;
					if (hint.m_isAngle)
					{
						direction = (value < 0) ? "right" : "left";
						action = "turn";
					}
					else
						direction = (value < 0) ? "down" : "up";
				}
				else if (dirI == 2)
				{
					value = hint.m_dirZ;
					if (hint.m_isAngle)
					{
						direction = (value < 0) ? "right" : "left";
						action = "roll";
					}
					else
						direction = (value < 0) ? "forward" : "backward";
				}
				if (fabsf(value) > 0.000001f)
				{
					std::string mod;
					std::string measure;
					if (hint.m_changeType == RecognitionCorrectionHint::POSE)
					{
						if (hint.m_changeDirection == RecognitionCorrectionHint::MORE)
							mod = "more ";
						else if (hint.m_changeDirection == RecognitionCorrectionHint::LESS)
							mod = "less ";
						if (hint.m_measuringUnit == BodyMeasurement::NUM_MEASUREMENTS)
							measure = hint.m_isAngle ? "deg" : "mm";
						else
							measure = getBodyMeasureName(hint.m_measuringUnit);
					}
					else if (hint.m_changeType == RecognitionCorrectionHint::SPEED)
					{
						if (hint.m_changeDirection == RecognitionCorrectionHint::MORE)
							mod = "faster ";
						else if (hint.m_changeDirection == RecognitionCorrectionHint::LESS)
							mod = "slower ";
						measure = hint.m_isAngle ? "deg/s" : "mm/s";
					}
					msg << "Please " << action << " " << getJointName(hint.m_joint, hint.m_recTarget) << " "
						<< mod << direction << ": " << value << " " << measure << "\n";
				}
			}
			if (fabsf(hint.m_dist) > 0.000001f)
			{
				std::string direction = (hint.m_dist < 0) ? "closer" : "further";
				std::string measure = hint.m_isAngle ? "deg" : "mm";
				if (hint.m_measuringUnit != BodyMeasurement::NUM_MEASUREMENTS)
					measure = getBodyMeasureName(hint.m_measuringUnit);
				msg << "Please move " << getJointName(hint.m_joint, hint.m_recTarget) << " "
					<< direction << ": " << hint.m_dist << " " << measure << "\n";
			}
		}
		return msg.str();
	}
};
