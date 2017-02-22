// ScenePainterUtils.cpp
//
// Original heatmap code from here: https://github.com/wlitwin/heatmap
// *************************************************************************************************


#include "ScenePainter.h"
#include "ScenePainterUtils.h"

#include <ssiocv.h>

using namespace cv;
using namespace ssi;


int g_kernel_size = 32;
float g_fade_time = 1.5;
float g_base_intensity = 1.0;
float g_max_transparency = 0.45;
bool g_linear_kernel = false;

int frame_width;
int frame_height;

Mat g_heatmap;
Mat g_kernel;
Mat g_ones;
Mat g_zeros;
Mat g_fade_mat;

Vec3b g_heat_color1 = Vec3b(0, 255, 255); // Red in HSV
Vec3b g_heat_color2 = Vec3b(170, 255, 255); // Blue in HSV


void ScenePainterUtils::heatmap_init(int width, int height, int fps){
	
	width /= 2;
	height /= 2;
	g_kernel_size /= 2;
	
	frame_width = width;
	frame_height = height;

	g_heatmap = Mat::zeros(height, width, CV_32FC1);
	g_ones = Mat::ones(height, width, CV_32F);
	g_zeros = Mat::zeros(height, width, CV_32F);
	g_fade_mat = Mat::ones(height, width, CV_32F);

	// Determine how much to fade the heatmap values by each frame
	g_fade_mat.setTo((1.0 / fps) / g_fade_time);
	// Create heatmap kernel
	create_kernel();
}

void ScenePainterUtils::heatmap_update(int x, int y, cv::Mat* framePtr){

	decrease_heatmap();

	x /= 2;
	y /= 2;

	if (x > 0 && y > 0 && x <= frame_width && y <= frame_height){
		heat_point(x, y);
	}

	overlay_heatmap(*framePtr);

	
}

void ScenePainterUtils::heatmap_shift(int sx, int sy){

	if ((sx != 0 && abs(sx) < g_heatmap.cols) || (sy != 0 && abs(sy) < g_heatmap.rows)){
		
		cv::Mat tmp = cv::Mat::zeros(g_heatmap.size(), g_heatmap.type());

		int rw = g_heatmap.cols - abs(sx);
		int rh = g_heatmap.rows - abs(sy);

		if (sx >= 0 && sy >= 0){ //up left
			g_heatmap(cv::Rect(sx, sy, rw, rh)).copyTo(tmp(cv::Rect(0, 0, rw, rh)));
		}
		else if (sx <= 0 && sy >= 0){ //up right
			g_heatmap(cv::Rect(0, sy, rw, rh)).copyTo(tmp(cv::Rect(-sx, 0, rw, rh)));
		}
		else if (sx >= 0 && sy <= 0){ //down left
			g_heatmap(cv::Rect(sx, 0, rw, rh)).copyTo(tmp(cv::Rect(0, -sy, rw, rh)));
		}
		else if (sx <= 0 && sy <= 0){ //down right
			g_heatmap(cv::Rect(0, 0, rw, rh)).copyTo(tmp(cv::Rect(-sx, -sy, rw, rh)));
		}

		tmp.copyTo(g_heatmap);
	}
}

/* Create the heatmap kernel. This is applied when heat_point() is called.
*/
void ScenePainterUtils::create_kernel()
{
	if (g_linear_kernel)
	{
		// Linear kernel
		const float max_val = 1.0 * g_base_intensity;
		const float min_val = 0.0;
		const float interval = max_val - min_val;

		const int center = g_kernel_size / 2 + 1;
		const float radius = g_kernel_size / 2;

		g_kernel = Mat::zeros(g_kernel_size, g_kernel_size, CV_32F);
		for (int r = 0; r < g_kernel_size; ++r)
		{
			float* ptr = g_kernel.ptr<float>(r);
			for (int c = 0; c < g_kernel_size; ++c)
			{
				// Calculate the distance from the center	
				const float diff_x = static_cast<float>(abs(r - center));
				const float diff_y = static_cast<float>(abs(c - center));
				const float length = sqrt(diff_x*diff_x + diff_y*diff_y);
				if (length <= radius)
				{
					const float b = 1.0 - (length / radius);
					const float val = b*interval + min_val;
					ptr[c] = val;
				}
			}
		}
	}
	else
	{
		// Gaussian kernel
		Mat coeffs = getGaussianKernel(g_kernel_size, 0.0, CV_32F) * 150 * g_base_intensity;
		g_kernel = coeffs * coeffs.t();
	}
}



/* Draws the heatmap on top of a frame. The frame must be the same size as
* the heatmap.
*/
void ScenePainterUtils::overlay_heatmap(Mat frame)
{
	// Make sure all values are capped at one
	g_heatmap = min(g_ones, g_heatmap);

	Mat temp_map;
	blur(g_heatmap, temp_map, Size(15, 15));

	Mat framePyr;
	pyrDown(frame, framePyr);

	for (int r = 0; r < framePyr.rows; ++r)
	{
		Vec3b* f_ptr = framePyr.ptr<Vec3b>(r);
		float* h_ptr = temp_map.ptr<float>(r);
		for (int c = 0; c < framePyr.cols; ++c)
		{
			const float heat_mix = h_ptr[c];
			if (heat_mix > 0.0001)
			{
				// in BGR
				const Vec3b i_color = f_ptr[c];

				const Vec3b heat_color = hsv_to_bgr(interpolate_hsv(g_heat_color2, g_heat_color1, heat_mix));

				const float heat_mix2 = std::min(heat_mix, g_max_transparency);

				const Vec3b final_color = interpolate(i_color, heat_color, heat_mix2);

				frame.at<Vec3b>(r * 2, c * 2) = final_color;
				frame.at<Vec3b>(r * 2 + 1, c * 2) = final_color;
				frame.at<Vec3b>(r * 2, c * 2 + 1) = final_color;
				frame.at<Vec3b>(r * 2 + 1, c * 2 + 1) = final_color;

			}
		}
	}
}

void ScenePainterUtils::decrease_heatmap()
{
	// Fade some of the values in the matrix	
	g_heatmap -= g_fade_mat;
	g_heatmap = cv::max(g_zeros, g_heatmap);
}

void ScenePainterUtils::heat_point(int x, int y)
{
	// Make sure the coordinates are in bounds
	if (x < 0 || y < 0 || x >= g_heatmap.cols || y >= g_heatmap.rows)
	{
		return;
	}

	// Only update a small portion of the matrix
	const int g_kernel_half = g_kernel_size / 2;
	const int fixed_x = x - g_kernel_half;
	const int fixed_y = y - g_kernel_half;
	const int roi_l = max(fixed_x, 0);
	const int roi_t = max(fixed_y, 0);
	const int roi_w = min(fixed_x + g_kernel_size, g_heatmap.cols) - roi_l;
	const int roi_h = min(fixed_y + g_kernel_size, g_heatmap.rows) - roi_t;

	Mat roi(g_heatmap(Rect(roi_l, roi_t, roi_w, roi_h)));

	const int groi_l = roi_l - fixed_x;
	const int groi_t = roi_t - fixed_y;
	const int groi_w = roi_w;
	const int groi_h = roi_h;

	Mat roi_gauss(g_kernel(Rect(groi_l, groi_t, groi_w, groi_h)));
	roi += roi_gauss;
}


inline Vec3b ScenePainterUtils::interpolate_hsv(const Vec3b color1, const Vec3b color2, const float value)
{
	if (value <= 0.0) return color1;
	if (value >= 1.0) return color2;

	uchar h = saturate_cast<uchar>(round((1.0 - value)*color1.val[0] + value*color2.val[0])) % 256;
	uchar s = round((1.0 - value)*color1.val[1] + value*color2.val[1]);
	uchar v = round((1.0 - value)*color1.val[2] + value*color2.val[2]);

	return Vec3b(h, s, v);
}


inline Vec3b ScenePainterUtils::interpolate(const Vec3b color1, const Vec3b color2, const float value)
{
	uchar b = saturate_cast<uchar>(round((1.0 - value)*color1.val[0] + value*color2.val[0]));
	uchar g = saturate_cast<uchar>(round((1.0 - value)*color1.val[1] + value*color2.val[1]));
	uchar r = saturate_cast<uchar>(round((1.0 - value)*color1.val[2] + value*color2.val[2]));

	return Vec3b(b, g, r);
}


inline Vec3b ScenePainterUtils::hsv_to_bgr(const Vec3b& hsv)
{
	const float h = ((hsv.val[0] / 255.0) * 360.0) / 60.0;
	const float s = hsv.val[1] / 255.0;
	const float v = hsv.val[2] / 255.0;

	const int i = static_cast<int>(h);
	const float ff = h - i;
	const float p = v * (1.0 - s);
	const float q = v * (1.0 - (s * ff));
	const float t = v * (1.0 - (s * (1.0 - ff)));

	float r = 0, g = 0, b = 0;
	switch (i)
	{
	case 0:
		r = v; g = t; b = p;
		break;
	case 1:
		r = q; g = v; b = p;
		break;
	case 2:
		r = p; g = v; b = t;
		break;
	case 3:
		r = p; g = q; b = v;
		break;
	case 4:
		r = t; g = p; b = v;
		break;
	case 5:
	default:
		r = v; g = p; b = q;
		break;
	}

	const int r_ = saturate_cast<uchar>(r * 255.0f);
	const int g_ = saturate_cast<uchar>(g * 255.0f);
	const int b_ = saturate_cast<uchar>(b * 255.0f);

	return Vec3b(b_, g_, r_);
}

inline double ScenePainterUtils::round(double val)
{    
    return floor(val + 0.5);
}

inline float ScenePainterUtils::round(float val)
{    
    return floor(val + 0.5);
}

