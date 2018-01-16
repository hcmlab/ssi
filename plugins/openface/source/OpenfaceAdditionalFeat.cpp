// OpenfaceAdditionalFeat.cpp
// author: Jorrit Posor
// created: 04/12/2017
// Copyright (C) University of Augsburg, Lab for Human Centered Multimedia
//
// *************************************************************************************************
//
// This file is part of Social Signal Interpretation (SSI) developed at the 
// Lab for Human Centered Multimedia of the University of Augsburg
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
//*************************************************************************************************

#include "OpenfaceAdditionalFeat.h"
#include "Openface.h"

#ifdef USE_SSI_LEAK_DETECTOR
#include "SSI_LeakWatcher.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

namespace ssi {

	OpenfaceAdditionalFeat::OpenfaceAdditionalFeat(const ssi_char_t *file)
		:
		_file(0) {

		if (file) {
			if (!OptionList::LoadXML(file, &_options)) {
				OptionList::SaveXML(file, &_options);
			}
			_file = ssi_strcpy(file);
		}
	}

	OpenfaceAdditionalFeat::~OpenfaceAdditionalFeat() {
		if (_file) {
			OptionList::SaveXML(_file, &_options);
			delete[] _file;
		}
	}

	void OpenfaceAdditionalFeat::transform_enter(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
	}

	/**
	Iterates through stream_in and calculates features like distances and
	variances using facial landmarks to detect movements of lips
	*/
	void OpenfaceAdditionalFeat::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;

		ssi_real_t *in = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *out = ssi_pcast(ssi_real_t, stream_out.ptr);

		float
			dist_points,
			dist_total_nose_lower_lip = 0.0f,
			dist_total_nose_chin = 0.0f,
			dist_total_chin_pose = 0.0f,
			area = 0.0f,
			areas_total = 0.0f;

		float *distances_nose_chin = new float[sample_number];
		float *distances_chin_pose = new float[sample_number];
		float *distances_nose_lower_lip = new float[sample_number];
		float *areas = new float[sample_number];

		/**
		This 2D array is used to save magnitudes for 20 facial landmarks.
		magnitudes[row][column] is magnitudes[row*sample_number + column]
		*/
		int rowCount = 20;
		float *magnitudes = new float[sample_number*rowCount];
		float magnitudes_total[20] = {0.0};

		if (in[Openface::FEATURE::DETECTION_SUCCESS] && in[Openface::FEATURE::DETECTION_CERTAINTY] < 0.2) {
			// Calculate distances, areas, magnitudes andtheir sums for all provided samples in this loop
			for (ssi_size_t i = 0; i < sample_number; i++)
			{
				/**
				Calc Magnitudes of all 20 facial landmarks on the lips.
				The concerning landmarks are 49 to 68, encoded from 122 to 161 in SSI.

				122 = FACIAL_LANDMARK_49_X
				123 = FACIAL_LANDMARK_49_Y
				...
				160 = FACIAL_LANDMARK_68_X
				161 = FACIAL_LANDMARK_68_Y
				*/
				int row = 0;
				// for all facial landmarks on lips 
				for (size_t j = 122; j < 161; j = j + 2)
				{
					if (i < sample_number - 1)
					{
						if (0.0 < in[i * sample_dimension + j] < 640.0
							&& 0.0 < in[i * sample_dimension + j + 1] < 480.0
							&& 0.0 < in[(i + 1) * sample_dimension + j] < 640.0
							&& 0.0 < in[(i + 1) * sample_dimension + j + 1] < 480.0
							)
						{
							// Calc magnitude of the same facial landmark in two different samples (i and i+1)
							float magnitude = dist_points = calc_dist_r2(
								in[i * sample_dimension + j], //X
								in[i * sample_dimension + j + 1], //Y
								in[(i + 1) * sample_dimension + j], //X
								in[(i + 1) * sample_dimension + j + 1] //Y
							);

							if (0.0 < magnitude < 640.0) {
								magnitudes[row*sample_number + i] = magnitude;
								magnitudes_total[row] += magnitude;
							}
							else {
								ssi_print("%f \n\n", magnitude);
								magnitudes[row*sample_number + i] = 0.0;
							}
						}
						else {
							magnitudes[row*sample_number + i] = 0.0;
						}
						row++;
					}
				}

				// Area mouth
				if (_options.sd_area_mouth || _options.sum_area_mouth) {
					area = calc_area(
						calc_dist_r2(
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_59_X],
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_59_Y],
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_57_X],
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_57_Y]
						),
						calc_dist_r2(
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_57_X],
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_57_Y],
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_53_X],
							in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_53_Y]
						)
					);
					areas_total += area;
					areas[i] = area;
				}

				// Distance nose to lower lip
				if (_options.sd_dist_nose_lower_lip || _options.sum_dist_nose_lower_lip) {
					dist_points = calc_dist_r2(
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_34_X],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_34_Y],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_67_X],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_67_Y]
					);
					dist_total_nose_lower_lip += dist_points;
					distances_nose_lower_lip[i] = dist_points;
				}

				// Distance nose to chin
				if (_options.sd_dist_nose_chin || _options.sum_dist_nose_chin) {
					dist_points = calc_dist_r2(
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_34_X],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_34_Y],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_9_X],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_9_Y]
					);
					dist_total_nose_chin += dist_points;
					distances_nose_chin[i] = dist_points;
				}

				// Distance chin to pose
				if (_options.sd_dist_chin_pose || _options.sum_dist_chin_pose) {
					dist_points = calc_dist_r2(
						in[i* sample_dimension + Openface::FEATURE::CORRECTED_POSE_WORLD_X],
						in[i* sample_dimension + Openface::FEATURE::CORRECTED_POSE_WORLD_Y],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_9_X],
						in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_9_Y]
					);
					dist_total_chin_pose += dist_points;
					distances_chin_pose[i] = dist_points;
				}
			}
		}

		// Write out total distances and calc/write out variances 

		float variance = 0.0;
		// Nose to chin features
		if (_options.sum_dist_nose_chin) {
			*out++ = dist_total_nose_chin;
		}
		if (_options.sd_dist_nose_chin) {
			variance = calc_variance(distances_nose_chin, dist_total_nose_chin, sample_number);
			*out++ = isinf(variance) || isnan(variance) || variance < 0 ? 0 : sqrt(variance);
		}

		// Chin to pose features
		if (_options.sum_dist_chin_pose) {
			*out++ = dist_total_chin_pose;
		}
		if (_options.sd_dist_chin_pose) {
			variance = calc_variance(distances_chin_pose, dist_total_chin_pose, sample_number);
			*out++ = isinf(variance) || isnan(variance) || variance < 0 ? 0 : sqrt(variance);
		}

		// Nose to lower lip features
		if (_options.sum_dist_nose_lower_lip) {
			*out++ = dist_total_nose_lower_lip;
		}
		if (_options.sd_dist_nose_lower_lip) {
			variance = calc_variance(distances_nose_lower_lip, dist_total_nose_lower_lip, sample_number);
			*out++ = isinf(variance) || isnan(variance) || variance < 0 ? 0 : sqrt(variance);
		}

		// Mouth area features
		if (_options.sum_area_mouth) {
			*out++ = areas_total;
		}
		if (_options.sd_area_mouth) {
			variance = calc_variance(areas, areas_total, sample_number);
			*out++ = isinf(variance) || isnan(variance) || variance < 0 ? 0 : sqrt(variance);
		}

		// Magnitude features
		if (_options.magnitudes_lips) {
			for (size_t row = 0; row < rowCount; row++)
			{
				float *magnitudes_row = new float[sample_number];
				magnitudes_row = &magnitudes[row];
				*out++ = magnitudes_total[row];
			}
		}

		// Free memory
		delete[] distances_nose_chin;
		delete[] distances_nose_lower_lip;
		delete[] magnitudes;

	}

	void OpenfaceAdditionalFeat::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

	}

	/**
	Calculate the euclidean distance in two dimensional euclidean space

	@param point_1_1
	@param point_1_2
	@param point_2_1
	@param point_2_2
	@return value of the eucledian distance
	@author: Jorrit Posor
	*/
	float OpenfaceAdditionalFeat::calc_dist_r2(float point_1_1, float point_1_2, float point_2_1, float point_2_2) {
		if ((point_1_1 || point_1_1 || point_1_1 || point_1_1) <= 0.0) {
			return 0.0;
		}
		else {
			float dist = sqrt(pow(point_1_1 - point_2_1, 2) + pow(point_1_2 - point_2_2, 2));
			return dist;
		}
	}

	/**
	Calculate the variance for given data points

	@param data[] The data points to calc the variance of
	@param sum The sum of the datapoints (to skip a additional loop, since already calculated above)
	@param size The size of the data array
	@return value of the variance
	@author: Jorrit Posor
	*/
	float OpenfaceAdditionalFeat::calc_variance(float data[], float sum, float size) {

		float mean = sum / size;

		float variance = 0.0f;
		for (int i = 0; i < size; i++)
		{
			if (data[i] <= 0.0) {
				return 0.0;
			}
			else {
				variance += pow(data[i] - mean, 2);
			}
		}

		return variance / size;
	}

	/**
	Calculate area for given lengths

	@param height
	@param width
	@return value of the area
	@author: Jorrit Posor
	*/
	float OpenfaceAdditionalFeat::calc_area(float height, float width) {
		float area = height * width;
		return area;
	}

}