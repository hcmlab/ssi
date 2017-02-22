#pragma once

#include "ScenePainter.h"
#include <ssiocv.h>

class ScenePainterUtils
{
public:

	static void heatmap_init(int width, int height, int fps);
	static void heatmap_update(int x, int y, cv::Mat* framePtr);
	static void heatmap_shift(int sx, int sy);

protected:

	static void heat_point(int x, int y);
	static void overlay_heatmap(cv::Mat frame);
	static void decrease_heatmap();
	static void create_kernel();

	static cv::Vec3b interpolate_hsv(const cv::Vec3b color1, const cv::Vec3b color2, const float value);
	static cv::Vec3b interpolate(const cv::Vec3b color1, const cv::Vec3b color2, const float value);
	static cv::Vec3b hsv_to_bgr(const cv::Vec3b& hsv);

	static inline double round(double val);
	static inline float round(float val);
	
};
