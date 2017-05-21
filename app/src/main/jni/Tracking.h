#pragma once

#ifndef __TRACKING_H__
#define __TRACKING_H__

#include "Constrains.h"

typedef struct sHead{
	int num_stage;
	int num_point;
	int num_tree_per_point;
	int tree_depth;
	int node_step;

	int num_node;
	int num_leaf;
	int dim_tree;

	int num_tree_per_stage;
	int num_tree_total;
	int dim_feat;
}sHead;

typedef struct sModel{
	sHead head;
	Mat_<float> meanface;
	Mat_<float> rf;
	Mat_<float> w;
}sModel;


class Tracking
{
private:
	string m_Name;
	Mat_<float>* m_AX;
	Mat_<float>* m_AY;
	Mat_<float>* m_BX;
	Mat_<float>* m_BY;
	Mat_<float>* m_Thresh;
	Mat_<int> m_Isleaf;
	Mat_<int> m_Cnodes;
	Mat_<int> m_Idleafnodes;
	sModel m_Model;

	int readModel(sModel* model = NULL);
	Mat_<int> lbf_fast(const Mat_<uchar>& img, const Rect& bbox, const Mat_<float>& shape, int landmarkID, int stage);
	int getIndex(const Mat_<int>& src, int val);

public:

	Tracking();
	~Tracking();
	void Init(const string& model_name);
	sModel GetModel();
	Mat_<float> Reshape_alt(Mat_<float>& mean, Rect& faceBox);
	Mat_<int> DerivBinaryfeat(const Mat_<uchar>&img, const Rect& bbox, const Mat_<float>& shape, int stage);
	void UpDate(Mat_<int>&binary, Rect &bbox, Mat_<float>& shape, int stage);


};


#endif /* __TRAKING__H__ */
