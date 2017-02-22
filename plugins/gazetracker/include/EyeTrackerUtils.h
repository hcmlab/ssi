#pragma once

#include "EyeTracker.h"
#include <ssiocv.h>

class EyeTrackerUtils
{
public:
	
	class Blob{
	public:
		float x;
		float y;
		float width;
		float height;
		float angle;
		float averageRadius; //average distance from center to border
		
		//http://www.learnopencv.com/wp-content/uploads/2015/02/BlobTest.jpg
		float inertia;
		float circularity;
		float convexity;

	};

	static void blobdetector_setMinArea(double min);
	static void blobdetector_setMaxArea(double max);

	static float blobdetector_getMinConvexity();
	static float blobdetector_getMinCircularity();

	static void blobdetector_detect(cv::Mat& image, std::vector<EyeTrackerUtils::Blob>& blobs);

protected:

	static void findBlobs(cv::Mat &image, std::vector<EyeTrackerUtils::Blob> &blobs);
	
};
