///////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, Tadas Baltrusaitis, all rights reserved.
//
// ACADEMIC OR NON-PROFIT ORGANIZATION NONCOMMERCIAL RESEARCH USE ONLY
//
// BY USING OR DOWNLOADING THE SOFTWARE, YOU ARE AGREEING TO THE TERMS OF THIS LICENSE AGREEMENT.  
// IF YOU DO NOT AGREE WITH THESE TERMS, YOU MAY NOT USE OR DOWNLOAD THE SOFTWARE.
//
// License can be found in OpenFace-license.txt
//
//     * Any publications arising from the use of this software, including but
//       not limited to academic journal and conference publications, technical
//       reports and manuals, must cite at least one of the following works:
//
//       OpenFace 2.0: Facial Behavior Analysis Toolkit
//       Tadas Baltrušaitis, Amir Zadeh, Yao Chong Lim, and Louis-Philippe Morency
//       in IEEE International Conference on Automatic Face and Gesture Recognition, 2018  
//
//       Convolutional experts constrained local model for facial landmark detection.
//       A. Zadeh, T. Baltrušaitis, and Louis-Philippe Morency,
//       in Computer Vision and Pattern Recognition Workshops, 2017.    
//
//       Rendering of Eyes for Eye-Shape Registration and Gaze Estimation
//       Erroll Wood, Tadas Baltrušaitis, Xucong Zhang, Yusuke Sugano, Peter Robinson, and Andreas Bulling 
//       in IEEE International. Conference on Computer Vision (ICCV),  2015 
//
//       Cross-dataset learning and person-specific normalisation for automatic Action Unit detection
//       Tadas Baltrušaitis, Marwa Mahmoud, and Peter Robinson 
//       in Facial Expression Recognition and Analysis Challenge, 
//       IEEE International Conference on Automatic Face and Gesture Recognition, 2015 
//
///////////////////////////////////////////////////////////////////////////////

#ifndef FACE_DETECTOR_MTCNN_H
#define FACE_DETECTOR_MTCNN_H

// OpenCV includes
#include <opencv2/core/core.hpp>

// System includes
#include <vector>

using namespace std;

namespace LandmarkDetector
{
	class CNN
	{
	public:

		//==========================================

		// Default constructor
		CNN() { ; }

		// Copy constructor
		CNN(const CNN& other);

		// Given an image apply a CNN on it, the boolean direct controls if direct convolution is used (through matrix multiplication) or an FFT optimization
		std::vector<cv::Mat_<float> > Inference(const cv::Mat& input_img, bool direct = true, bool thread_safe = false);

		// Reading in the model
		void Read(const string& location);

		// Clearing precomputed DFTs
		void ClearPrecomp();

		size_t NumberOfLayers() { return cnn_layer_types.size(); }

	private:
		//==========================================
		// Convolutional Neural Network

		// CNN layers
		// Layer -> Weight matrix
		vector<cv::Mat_<float> > cnn_convolutional_layers_weights;

		// Keeping some pre-allocated im2col data as malloc is a significant time cost (not thread safe though)
		vector<cv::Mat_<float> > conv_layer_pre_alloc_im2col;

		// Layer -> kernel -> input maps
		vector<vector<vector<cv::Mat_<float> > > > cnn_convolutional_layers;
		vector<vector<float > > cnn_convolutional_layers_bias;
		// Layer matrix + bas
		vector<cv::Mat_<float> >  cnn_fully_connected_layers_weights;
		vector<cv::Mat_<float> > cnn_fully_connected_layers_biases;
		vector<cv::Mat_<float> >  cnn_prelu_layer_weights;
		vector<std::tuple<int, int, int, int> > cnn_max_pooling_layers;

		// Precomputations for faster convolution
		vector<vector<map<int, vector<cv::Mat_<double> > > > > cnn_convolutional_layers_dft;

		// CNN: 0 - convolutional, 1 - max pooling, 2 - fully connected, 3 - prelu, 4 - sigmoid
		vector<int > cnn_layer_types;
	};
	//===========================================================================
	//
	// Checking if landmark detection was successful using an SVR regressor
	// Using multiple validators trained add different views
	// The regressor outputs -1 for ideal alignment and 1 for worst alignment
	//===========================================================================
	class FaceDetectorMTCNN
	{

	public:

		// Default constructor
		FaceDetectorMTCNN() { ; }

		FaceDetectorMTCNN(const string& location);

		// Copy constructor
		FaceDetectorMTCNN(const FaceDetectorMTCNN& other);

		// Given an image, orientation and detected landmarks output the result of the appropriate regressor
		bool DetectFaces(vector<cv::Rect_<float> >& o_regions, const cv::Mat& input_img, std::vector<float>& o_confidences, int min_face = 60, float t1 = 0.6, float t2 = 0.7, float t3 = 0.7);

		// Reading in the model
		void Read(const string& location);

		// Indicate if the model has been read in
		bool empty() { return PNet.NumberOfLayers() == 0 || RNet.NumberOfLayers() == 0 || ONet.NumberOfLayers() == 0; };

	private:
		//==========================================
		// Components of the model

		CNN PNet;
		CNN RNet;
		CNN ONet;
		
	};

}
#endif // FACE_DETECTOR_MTCNN_H
