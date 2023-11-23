// AzureKinectDatatypes.h
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
#pragma once
#ifndef SSI_AZUREKINECT_DATATYPES

#include <array>
#include <unordered_map>

#define PI 3.141592653589793238463

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

		struct PointCloudPosMask {
			int16_t maxX;
			int16_t minX;
			int16_t maxY;
			int16_t minY;
			int16_t maxZ;
			int16_t minZ;

			bool pointWithin(const int16_t* point)
			{
				if (maxX >= point[0] && minX <= point[0]) {
					if (maxY >= point[1] && minY <= point[1]) {
						if (maxZ >= point[2] && minZ <= point[2]) {
							return true;
						}
					}
				}

				return false;
			}
		};

		struct PointCloudMask {
			bool active;
			PointCloudPosMask* posMask;
			PointCloudPosMask* bpMask;
			double transformation[12];
			// double inverseTransformation[12];
			// uint8_t* cornerPoints;

			PointCloudMask(int16_t* data) {
				active = (data[6] > 0) && (data[7] > 0) && (data[8] > 0);
				if (!active) {
					return;
				}

				int16_t x = data[0];
				int16_t y = data[1];
				int16_t z = data[2];

				double toRadians = 2 * PI / std::numeric_limits<uint16_t>::max();
				double rotationX = data[3] * toRadians;
				double rotationY = data[4] * toRadians;
				double rotationZ = data[5] * toRadians;

				uint16_t scaleX = data[6];
				uint16_t scaleY = data[7];
				uint16_t scaleZ = data[8];

				uint16_t bpScaleX = data[9];
				uint16_t bpScaleY = data[10];
				uint16_t bpScaleZ = data[11];

				posMask = new PointCloudPosMask();
				posMask->maxX = std::min(x + scaleX, SHRT_MAX);
				posMask->maxY = std::min(y + scaleY, SHRT_MAX);
				posMask->maxZ = std::min(z + scaleZ, SHRT_MAX);

				posMask->minX = std::max(x - scaleX, SHRT_MIN);
				posMask->minY = std::max(y - scaleY, SHRT_MIN);
				posMask->minZ = std::max(z - scaleZ, SHRT_MIN);

				bpMask = new PointCloudPosMask();
				bpMask->maxX = std::min(x + bpScaleX, SHRT_MAX);
				bpMask->maxY = std::min(y + bpScaleY, SHRT_MAX);
				bpMask->maxZ = std::min(z + bpScaleZ, SHRT_MAX);

				bpMask->minX = std::max(x - bpScaleX, SHRT_MIN);
				bpMask->minY = std::max(y - bpScaleY, SHRT_MIN);
				bpMask->minZ = std::max(z - bpScaleZ, SHRT_MIN);

				double sinX = std::sin(-rotationX);
				double sinY = std::sin(-rotationY);
				double sinZ = std::sin(-rotationZ);

				double cosX = std::cos(-rotationX);
				double cosY = std::cos(-rotationY);
				double cosZ = std::cos(-rotationZ);

				// this matrix first translates by (-x,-y,-z), then rotates by rz, then by rx, then by ry, and finally translates back by (x,y,z)
				transformation[0] = cosY * cosZ - sinX * sinY * sinZ;
				transformation[1] = -cosX * sinZ;
				transformation[2] = sinY * cosZ + sinX * cosY * sinZ;
				transformation[3] = x - x * (cosY * cosZ - sinX * sinY * sinZ) + y * cosX * sinZ - z * (sinY * cosZ + sinX * cosY * sinZ);

				transformation[4] = cosY * sinZ + sinX * sinY * cosZ;
				transformation[5] = cosX * cosZ;
				transformation[6] = sinY * sinZ - sinX * cosY * cosZ;
				transformation[7] = y - x * (cosY * sinZ + sinX * sinY * cosZ) - y * cosX * cosZ - z * (sinY * sinZ - sinX * cosY * cosZ);

				transformation[8] = -cosX * sinY;
				transformation[9] = sinX;
				transformation[10] = cosX * cosY;
				transformation[11] = z + x * cosX * sinY - y * sinX - z * cosY;

				/*
				sinX = std::sin(rotationX);
				sinY = std::sin(rotationY);
				sinZ = std::sin(rotationZ);

				cosX = std::cos(rotationX);
				cosY = std::cos(rotationY);
				cosZ = std::cos(rotationZ);

				inverseTransformation[0] = sinX * sinY * sinZ + cosY * cosZ;
				inverseTransformation[1] = sinX * sinY * cosZ - cosY * sinZ;
				inverseTransformation[2] = cosX * sinY;
				inverseTransformation[3] = x - x * (sinX * sinY * sinZ + cosY * cosZ) - y * (sinX * sinY * cosZ - cosY * sinZ) - z * cosX * sinY;

				inverseTransformation[4] = cosX * sinZ;
				inverseTransformation[5] = cosX * cosZ;
				inverseTransformation[6] = -sinX;
				inverseTransformation[7] = y - x * cosX * sinZ - y * cosX * cosZ + z * sinX;

				inverseTransformation[8] = sinX * cosY * sinZ - sinY * cosZ;
				inverseTransformation[9] = sinX * cosY * cosZ + sinY * sinZ;
				inverseTransformation[10] = cosX * cosY;
				inverseTransformation[11] = z - x * (sinX * cosY * sinZ - sinY * cosZ) - y * (sinX * cosY * cosZ + sinY * sinZ) - z * cosX * cosY;

				uint32_t cornerColor = 255 << 16;
				int16_t* point = new int16_t[3];
				int16_t* rotatedPoint = new int16_t[3];
				cornerPoints = new uint8_t[8*pointCloudPointSizeInBytes];

				uint8_t* cornerPointsOffset = cornerPoints;
				point[0] = mask->maxX;
				point[1] = mask->maxY;
				point[2] = mask->maxZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);

				cornerPointsOffset += pointCloudPointSizeInBytes;
				point[0] = mask->maxX;
				point[1] = mask->maxY;
				point[2] = mask->minZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);

				cornerPointsOffset += pointCloudPointSizeInBytes;
				point[0] = mask->maxX;
				point[1] = mask->minY;
				point[2] = mask->maxZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);

				cornerPointsOffset += pointCloudPointSizeInBytes;
				point[0] = mask->maxX;
				point[1] = mask->minY;
				point[2] = mask->minZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);

				cornerPointsOffset += pointCloudPointSizeInBytes;
				point[0] = mask->minX;
				point[1] = mask->maxY;
				point[2] = mask->maxZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);

				cornerPointsOffset += pointCloudPointSizeInBytes;
				point[0] = mask->minX;
				point[1] = mask->maxY;
				point[2] = mask->minZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);

				cornerPointsOffset += pointCloudPointSizeInBytes;
				point[0] = mask->minX;
				point[1] = mask->minY;
				point[2] = mask->maxZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);

				cornerPointsOffset += pointCloudPointSizeInBytes;
				point[0] = mask->minX;
				point[1] = mask->minY;
				point[2] = mask->minZ;
				transform(rotatedPoint, point, maskInverseTransformation);
				std::memcpy(cornerPointsOffset, rotatedPoint, SSI_AZUREKINECT_BYTESPERDEPTHCLOUDVOXEL);
				std::memcpy(cornerPointsOffset + 6, &cornerColor, bytesPerColor);
				*/
			}
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

		enum class POINTCLOUD_FRAGMENT_MODE
		{
			LEGACY = 0,
			FRAME,
			UDP_OPTIMIZED,
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
