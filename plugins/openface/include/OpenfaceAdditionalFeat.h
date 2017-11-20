
#ifndef SSI_OPENFACEADDITIONALFEAT_H
#define SSI_OPENFACEADDITIONALFEAT_H

#include "base/IFeature.h"
#include "ioput/option/OptionList.h"

namespace ssi {

	class OpenfaceAdditionalFeat : public IFeature {

	public:

		class Options : public OptionList {

		public:

			Options() :
				sd_area_mouth(true),
				sum_area_mouth(true),
				sd_dist_nose_lower_lip(true),
				sum_dist_nose_lower_lip(true),
				sd_dist_nose_chin(true),
				sum_dist_nose_chin(true)

			{

				addOption("SD_Area_Mouth", &sd_area_mouth, 1, SSI_BOOL, "Calculates standard deviation of the area over mouth");
				addOption("Sum_Area_Mouth", &sum_area_mouth, 1, SSI_BOOL, "Calculates sum of the area over mouth");
				addOption("SD_Dist_Lips", &sd_dist_nose_lower_lip, 1, SSI_BOOL, "Calculates standard deviation of the distance between lips");
				addOption("Sum_Dist_Lips", &sum_dist_nose_lower_lip, 1, SSI_BOOL, "Calculates sum of the distance between lips");
				addOption("SD_Dist_Nose_Chin", &sd_dist_nose_chin, 1, SSI_BOOL, "Calculates standard deviation of the distance nose to chin");
				addOption("Sum_Dist_Nose_Chin", &sum_dist_nose_chin, 1, SSI_BOOL, "Calculates sum of the distance nose to chin");

			};

			bool
				sd_area_mouth,
				sum_area_mouth,
				sd_dist_nose_lower_lip,
				sum_dist_nose_lower_lip,
				sd_dist_nose_chin,
				sum_dist_nose_chin;


		};


	public:

		static const ssi_char_t *GetCreateName() { return "OpenfaceAdditionalFeat"; };
		static IObject *Create(const ssi_char_t *file) { return new OpenfaceAdditionalFeat(file); };
		OpenfaceAdditionalFeat::Options *getOptions() { return &_options; };
		~OpenfaceAdditionalFeat();
		const ssi_char_t *getName() { return GetCreateName(); };
		const ssi_char_t *getInfo() { return "Openface additional features"; };

		ssi_size_t getSampleDimensionOut(ssi_size_t sample_dimension_in) { 
			int dimensions = 0;
			dimensions += _options.sd_area_mouth ? 1 : 0;
			dimensions += _options.sum_area_mouth ? 1 : 0;
			dimensions += _options.sd_dist_nose_lower_lip ? 1 : 0;
			dimensions += _options.sum_dist_nose_lower_lip ? 1 : 0;
			dimensions += _options.sd_dist_nose_chin ? 1 : 0;
			dimensions += _options.sum_dist_nose_chin ? 1 : 0;
			return dimensions; 
		};
		ssi_size_t getSampleBytesOut(ssi_size_t sample_bytes_in) { return sample_bytes_in; };
		ssi_type_t getSampleTypeOut(ssi_type_t sample_type_in) { return sample_type_in; };

		void transform_enter(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform(ITransformer::info info,
			ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);
		void transform_flush(ssi_stream_t &stream_in,
			ssi_stream_t &stream_out,
			ssi_size_t xtra_stream_in_num = 0,
			ssi_stream_t xtra_stream_in[] = 0);

		float calc_dist_r2(float point_1_1, float point_1_2, float point_2_1, float point_2_2);

		float calc_variance(float data[], float sum, float size);

		float calc_area(float height, float width);

	protected:

		OpenfaceAdditionalFeat(const ssi_char_t *file = 0);
		OpenfaceAdditionalFeat::Options _options;
		ssi_char_t *_file;

	};

}

#endif