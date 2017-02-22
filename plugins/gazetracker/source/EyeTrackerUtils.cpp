// EyeTrackerUtils.cpp
//
// Uses an altered version of SimpleBlobDetector from the OpenCV feature2d module.
// Here is their License Agreement:
//
/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "EyeTracker.h"
#include "EyeTrackerUtils.h"

#include <ssiocv.h>

using namespace cv;
using namespace ssi;

//parameters for blob detection
//http://www.learnopencv.com/wp-content/uploads/2015/02/BlobTest.jpg

int minDistBetweenBlobs = 5;

bool filterByColor = true;
int blobColor = 255;

bool filterByArea = true;
double minArea = 25;
double maxArea = 5000;

bool filterByCircularity = true;
float minCircularity = 0.1f;
float maxCircularity = std::numeric_limits<float>::max();

bool filterByInertia = true;
float minInertiaRatio = 0.05f;
float maxInertiaRatio = std::numeric_limits<float>::max();

bool filterByConvexity = true;
float minConvexity = 0.73f;
float maxConvexity = std::numeric_limits<float>::max();




void EyeTrackerUtils::blobdetector_detect(cv::Mat& image, std::vector<EyeTrackerUtils::Blob>& blobs)
{
	blobs.clear();

	if (image.type() != CV_8UC1){
		CV_Error(CV_StsUnsupportedFormat, "Blob detector only supports 8-bit images!");
	}

	findBlobs(image, blobs);

}


void EyeTrackerUtils::findBlobs(cv::Mat &image, std::vector<EyeTrackerUtils::Blob> &blobs)
{
	std::vector < std::vector<Point> > contours;
	Mat tmpBinaryImage = image.clone(); //yes, we need a copy here
	findContours(tmpBinaryImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	for (size_t contourIdx = 0; contourIdx < contours.size(); contourIdx++)
	{

		Blob blob;
		blob.inertia = 1;
		Moments moms = moments(Mat(contours[contourIdx]));
		if (filterByArea)
		{
			double area = moms.m00;
			if (area < minArea || area >= maxArea)
				continue;
		}

		if (filterByCircularity)
		{
			double area = moms.m00;
			double perimeter = arcLength(Mat(contours[contourIdx]), true);
			double ratio = 4 * CV_PI * area / (perimeter * perimeter);
			if (ratio < minCircularity || ratio >= maxCircularity){
				continue;
			}
			else{
				blob.circularity = (float)(ratio);
			}

		}

		if (filterByInertia)
		{
			double denominator = sqrt(pow(2 * moms.mu11, 2) + pow(moms.mu20 - moms.mu02, 2));
			const double eps = 1e-2;
			double ratio;
			if (denominator > eps)
			{
				double cosmin = (moms.mu20 - moms.mu02) / denominator;
				double sinmin = 2 * moms.mu11 / denominator;
				double cosmax = -cosmin;
				double sinmax = -sinmin;

				double imin = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmin - moms.mu11 * sinmin;
				double imax = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmax - moms.mu11 * sinmax;
				ratio = imin / imax;
			}
			else
			{
				ratio = 1;
			}

			if (ratio < minInertiaRatio || ratio >= maxInertiaRatio)
				continue;

			blob.inertia = ratio * ratio;
		}

		if (filterByConvexity)
		{
			std::vector < Point > hull;
			convexHull(Mat(contours[contourIdx]), hull);
			double area = contourArea(Mat(contours[contourIdx]));
			double hullArea = contourArea(Mat(hull));
			double ratio = area / hullArea;
			if (ratio < minConvexity || ratio >= maxConvexity){
				continue;
			}
			else{
				blob.convexity = ratio;
			}

		}

		blob.x = (float)(moms.m10 / moms.m00);
		blob.y = (float)(moms.m01 / moms.m00);

		bool wrongColor = false;
		if (filterByColor)
		{
			if (image.at<uchar>(cvRound(blob.y), cvRound(blob.x)) != blobColor){
				wrongColor = true;
			}
		}

		//compute blob radius
		{
			std::vector<double> dists;
			for (size_t pointIdx = 0; pointIdx < contours[contourIdx].size(); pointIdx++)
			{
				Point2f pt = contours[contourIdx][pointIdx];
				dists.push_back(norm(Point2f(blob.x, blob.y) - pt));

				
			}
			std::sort(dists.begin(), dists.end());

			blob.averageRadius = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
			blob.width = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
			blob.height = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;

		}

		if (wrongColor){
			//might still be a pupil with glint exactly at the center.
			//Take 4 peripheral probes and check their color

			int diff = cvRound(blob.width * 0.6f);
			int probeOk = 0;

			if (image.at<uchar>(cvRound(blob.y) + diff, cvRound(blob.x)) == blobColor) probeOk += 1;
			if (image.at<uchar>(cvRound(blob.y) - diff, cvRound(blob.x)) == blobColor) probeOk += 1;
			if (image.at<uchar>(cvRound(blob.y), cvRound(blob.x) + diff) == blobColor) probeOk += 1;
			if (image.at<uchar>(cvRound(blob.y), cvRound(blob.x) - diff) == blobColor) probeOk += 1;

			if (probeOk < 3){
				continue;
			}
		}


		//fit ellipse
		if ((int)(contours[contourIdx]).size() < 6){
			continue;
		}

		cv::Mat points(contours[contourIdx]);
		cv::RotatedRect r;
		r = cv::fitEllipse(points);

		blob.width = r.size.width;
		blob.height = r.size.height;
		blob.angle = r.angle;

		blobs.push_back(blob);

	}

}

void EyeTrackerUtils::blobdetector_setMinArea(double min){
	minArea = min;
}

void EyeTrackerUtils::blobdetector_setMaxArea(double max){
	maxArea = max;
}

float EyeTrackerUtils::blobdetector_getMinConvexity(){
	return minConvexity;
}

float EyeTrackerUtils::blobdetector_getMinCircularity(){
	return minCircularity;
}

