#pragma once

#ifndef __CONSTRAINS_H__
#define __CONSTRAINS_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <time.h>

#ifndef OPENCV_INCLUDED
#define OPENCV_INCLUDED
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif
#include<android/log.h>

using namespace std;
using namespace cv;

#define  LOG_TAG    "FaceTracker"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LANDMARK_TYPE_68 0
#define LANDMARK_TYPE_51 1
#define LANDMARK_TYPE_42 2

struct Params
{
// TJ start
	int max_numstage;
	int max_depth;					//	tree stopping criteria
	int max_nodes;
	int max_numfeats[8];			// number of pixel pairs
	int max_numtrees;				//	number of trees per forest
	int max_numthreshs;
	double bagging_overlap;
	double max_raio_radius[8];
	bool isflip;
	int landmark_ids[42];


	// mean face
	int max_iters;
	double max_errors;
	double facebox_scale_factor;
	double facebox_scale_const;
	double facebox_with_div;
	double facebox_height_div;

	int landmark_type;
	string outputModel;
	CascadeClassifier faceCascade;
};

static Params g_params;

//static int m_landmarkIDs[33] = { 17, 19, 21, 22, 24, 26, 27, 30, 31, 33,
//								 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
//								 45, 46, 47, 48, 50, 51, 52, 54, 57, 60,
//								 62, 64, 66 };

static int m_landmarkIDs[42] = { 0, 2, 4, 6, 8, 10, 12, 14, 16, 17,
								 19, 21, 22, 24, 26, 27, 30, 31, 33, 35,
								 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
								 46, 47, 48, 50, 51, 52, 54, 57, 60, 62,
								 64, 66 };

#endif /* __CONSTRAINS_H__ */