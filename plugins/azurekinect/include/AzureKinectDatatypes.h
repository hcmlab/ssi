#pragma once
#ifndef SSI_AZUREKINECT_DATATYPES

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
	}
	
}


#endif // !SSI_AZUREKINECT_DATATYPES
