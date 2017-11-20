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

	void OpenfaceAdditionalFeat::transform(ITransformer::info info,
		ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {
		ssi_size_t sample_dimension = stream_in.dim;
		ssi_size_t sample_number = stream_in.num;

	//	Openface::FEATURE *in = ssi_pcast(Openface::FEATURE, stream_in.ptr);

		ssi_real_t *in = ssi_pcast(ssi_real_t, stream_in.ptr);
		ssi_real_t *out = ssi_pcast(ssi_real_t, stream_out.ptr);

		float
			dist_points,
			dist_total_nose_lower_lip = 0.0f,
			dist_total_nose_chin = 0.0f,
			area = 0.0f,
			areas_total = 0.0f;
		float num = stream_in.num;
		float *distances_nose_chin = new float[num];
		float *distances_nose_lower_lip = new float[num];
		float *areas = new float[num];

		for (ssi_size_t i = 0; i < sample_number; i++)
		{

			// area of mouth
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

			// distance nose to lower lip
			if (_options.sd_dist_nose_lower_lip || _options.sum_dist_nose_lower_lip) {
				float nose = in[0 * sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_34_X];
				dist_points = calc_dist_r2(
					in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_34_X],
					in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_34_Y],
					in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_67_X],
					in[i* sample_dimension + Openface::FEATURE::FACIAL_LANDMARK_67_Y]
				);
				dist_total_nose_lower_lip += dist_points;

				distances_nose_lower_lip[i] = dist_points;
			}

			// distance nose to chin
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


		}

		float variance = 0.0;

		// nose to chin features
		if (_options.sum_dist_nose_chin) {
			*out++ = dist_total_nose_chin;
		}
		if (_options.sd_dist_nose_chin) {
			variance = calc_variance(distances_nose_chin, dist_total_nose_chin, num);
			*out++ = isinf(variance) || isnan(variance) || variance < 0 ? 0 : sqrt(variance);
		}

		// nose to lower lip features
		if (_options.sum_dist_nose_lower_lip) {
			*out++ = dist_total_nose_lower_lip;
		}
		if (_options.sd_dist_nose_lower_lip) {
			variance = calc_variance(distances_nose_lower_lip, dist_total_nose_lower_lip, num);
			*out++ = isinf(variance) || isnan(variance) || variance < 0 ? 0 : sqrt(variance);
		}

		// mouth area features
		if (_options.sum_area_mouth) {
			*out++ = areas_total;
		}
		if (_options.sd_area_mouth) {
			variance = calc_variance(areas, areas_total, num);
			*out++ = isinf(variance) || isnan(variance) || variance < 0 ? 0 : sqrt(variance);
		}

		// Free memory
		delete[] distances_nose_chin;
		delete[] distances_nose_lower_lip;

	}

	void OpenfaceAdditionalFeat::transform_flush(ssi_stream_t &stream_in,
		ssi_stream_t &stream_out,
		ssi_size_t xtra_stream_in_num,
		ssi_stream_t xtra_stream_in[]) {

	}

	/**
	* calc distance of two points in R2
	* @author: Jorrit Enzio Posor
	*/
	float OpenfaceAdditionalFeat::calc_dist_r2(float point_1_1, float point_1_2, float point_2_1, float point_2_2) {
		float dist = sqrt(pow(point_1_1 - point_2_1, 2) + pow(point_1_2 - point_2_2, 2));
		return dist;
	}

	/**
	* calc variance for given data points and their sum (sum to skip a loop)
	* @author: Jorrit Enzio Posor
	*/
	float OpenfaceAdditionalFeat::calc_variance(float data[], float sum, float size) {
		float mean = sum / size;

		float variance = 0.0f;
		for (int i = 0; i < size; i++)
		{
			variance += pow(data[i] - mean, 2);
		}

		return variance / size;
	}

	/**
	* calc area for two given edge distances
	* @author: Jorrit Enzio Posor
	*/
	float OpenfaceAdditionalFeat::calc_area(float height, float width) {
		float area = height * width;
		return area;
	}



}