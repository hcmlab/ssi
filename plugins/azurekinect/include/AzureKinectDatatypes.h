#pragma once
#ifndef SSI_AZUREKINECT_DATATYPES

#include <array>
#include <unordered_map>

namespace ssi {
	namespace AK {
		struct BgraPixel
		{
			uint8_t Blue;
			uint8_t Green;
			uint8_t Red;
			uint8_t Alpha;
		};

		struct BgrPixel
		{
			uint8_t Blue;
			uint8_t Green;
			uint8_t Red;
		};

		struct PointCloudPixel
		{
			int16_t x;
			int16_t y;
			int16_t z;
		};

		using HSVPixel = BgrPixel;

		using DepthPixel = uint16_t;

		using IRPixel = unsigned char;

		enum class RGB_VIDEO_RESOLUTION
		{
			p_3840x2160 = 0,
			p_2560x1440,
			p_1920x1080,
			p_1280x720,
			p_4096x3072, //only supports up to 15fps!
			p_2048x1536,
			OFF,

			NUM
		};

		enum class DEPTH_MODE
		{
			NFOV_UNBINNED = 0,
			NFOV_2x2_BINNED,
			WFOV_UNBINNED, //only supports up to 15fps!
			WFOV_2x2_BINNED,
			PASSIVE_IR,
			OFF,

			NUM
		};

		struct SKELETON_JOINT
		{
			enum List
			{
				PELVIS = 0,
				SPINE_NAVEL = 1,
				SPINE_CHEST = 2,
				NECK = 3,
				CLAVICLE_LEFT = 4,
				SHOULDER_LEFT = 5,
				ELBOW_LEFT = 6,
				WRIST_LEFT = 7,
				HAND_LEFT = 8,
				HANDTIP_LEFT = 9,
				THUMB_LEFT = 10,
				CLAVICLE_RIGHT = 11,
				SHOULDER_RIGHT = 12,
				ELBOW_RIGHT = 13,
				WRIST_RIGHT = 14,
				HAND_RIGHT = 15,
				HANDTIP_RIGHT = 16,
				THUMB_RIGHT = 17,
				HIP_LEFT = 18,
				KNEE_LEFT = 19,
				ANKLE_LEFT = 20,
				FOOT_LEFT = 21,
				HIP_RIGHT = 22,
				KNEE_RIGHT = 23,
				ANKLE_RIGHT = 24,
				FOOT_RIGHT = 25,
				HEAD = 26,
				NOSE = 27,
				EYE_LEFT = 28,
				EAR_LEFT = 29,
				EYE_RIGHT = 30,
				EAR_RIGHT = 31,

				NUM
			};
		};

		// Define the bone list based on the documentation
		// tuples of the form (child, parent)
		const std::unordered_map<SKELETON_JOINT::List, SKELETON_JOINT::List> AZUREKINECT_BONE_LIST =
		{
			std::make_pair(SKELETON_JOINT::SPINE_NAVEL, SKELETON_JOINT::PELVIS),
			std::make_pair(SKELETON_JOINT::SPINE_CHEST, SKELETON_JOINT::SPINE_NAVEL),
			std::make_pair(SKELETON_JOINT::NECK, SKELETON_JOINT::SPINE_CHEST),

			std::make_pair(SKELETON_JOINT::CLAVICLE_LEFT, SKELETON_JOINT::SPINE_CHEST),
			std::make_pair(SKELETON_JOINT::SHOULDER_LEFT, SKELETON_JOINT::CLAVICLE_LEFT),
			std::make_pair(SKELETON_JOINT::ELBOW_LEFT, SKELETON_JOINT::SHOULDER_LEFT),
			std::make_pair(SKELETON_JOINT::WRIST_LEFT, SKELETON_JOINT::ELBOW_LEFT),
			std::make_pair(SKELETON_JOINT::HAND_LEFT, SKELETON_JOINT::WRIST_LEFT),
			std::make_pair(SKELETON_JOINT::HANDTIP_LEFT, SKELETON_JOINT::HAND_LEFT),
			std::make_pair(SKELETON_JOINT::THUMB_LEFT, SKELETON_JOINT::WRIST_LEFT),

			std::make_pair(SKELETON_JOINT::CLAVICLE_RIGHT, SKELETON_JOINT::SPINE_CHEST),
			std::make_pair(SKELETON_JOINT::SHOULDER_RIGHT, SKELETON_JOINT::CLAVICLE_RIGHT),
			std::make_pair(SKELETON_JOINT::ELBOW_RIGHT, SKELETON_JOINT::SHOULDER_RIGHT),
			std::make_pair(SKELETON_JOINT::WRIST_RIGHT, SKELETON_JOINT::ELBOW_RIGHT),
			std::make_pair(SKELETON_JOINT::HAND_RIGHT, SKELETON_JOINT::WRIST_RIGHT),
			std::make_pair(SKELETON_JOINT::HANDTIP_RIGHT, SKELETON_JOINT::HAND_RIGHT),
			std::make_pair(SKELETON_JOINT::THUMB_RIGHT, SKELETON_JOINT::WRIST_RIGHT),

			std::make_pair(SKELETON_JOINT::HIP_LEFT, SKELETON_JOINT::PELVIS),
			std::make_pair(SKELETON_JOINT::KNEE_LEFT, SKELETON_JOINT::HIP_LEFT),
			std::make_pair(SKELETON_JOINT::ANKLE_LEFT, SKELETON_JOINT::KNEE_LEFT),
			std::make_pair(SKELETON_JOINT::FOOT_LEFT, SKELETON_JOINT::ANKLE_LEFT),

			std::make_pair(SKELETON_JOINT::HIP_RIGHT, SKELETON_JOINT::PELVIS),
			std::make_pair(SKELETON_JOINT::KNEE_RIGHT, SKELETON_JOINT::HIP_RIGHT),
			std::make_pair(SKELETON_JOINT::ANKLE_RIGHT, SKELETON_JOINT::KNEE_RIGHT),
			std::make_pair(SKELETON_JOINT::FOOT_RIGHT, SKELETON_JOINT::ANKLE_RIGHT),

			std::make_pair(SKELETON_JOINT::HEAD, SKELETON_JOINT::NECK),
			std::make_pair(SKELETON_JOINT::NOSE, SKELETON_JOINT::HEAD),
			std::make_pair(SKELETON_JOINT::EYE_LEFT, SKELETON_JOINT::HEAD),
			std::make_pair(SKELETON_JOINT::EAR_LEFT, SKELETON_JOINT::HEAD),
			std::make_pair(SKELETON_JOINT::EYE_RIGHT, SKELETON_JOINT::HEAD),
			std::make_pair(SKELETON_JOINT::EAR_RIGHT, SKELETON_JOINT::HEAD)
		};

		struct SKELETON_JOINT_VALUE
		{
			enum List
			{
				POS_X = 0,
				POS_Y,
				POS_Z,

				ROT_W,
				ROT_X,
				ROT_Y,
				ROT_Z,

				CONF,

				NUM
			};
		};

		#define  SSI_AZUREKINECT_INVALID_SKELETON_JOINT_VALUE 0.0f

		typedef float SKELETON[SKELETON_JOINT::NUM][SKELETON_JOINT_VALUE::NUM];
	}
	
}


#endif // !SSI_AZUREKINECT_DATATYPES
