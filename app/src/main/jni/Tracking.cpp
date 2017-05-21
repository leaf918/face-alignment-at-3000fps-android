#include "Tracking.h"

Tracking::Tracking()
{
}

Tracking::~Tracking()
{
	if (m_AX)
		delete[] m_AX;
	if (m_AY)
		delete[] m_AY;
	if (m_BX)
		delete[] m_BX;
	if (m_BX)
		delete[] m_BY;
	if (m_Thresh)
		delete[] m_Thresh;
}

sModel Tracking::GetModel()
{
	return m_Model;
}

void Tracking::Init(const string& model_name)
{
	m_Name = model_name;
	m_AX = NULL;
	m_AY = NULL;
	m_BX = NULL;
	m_BY = NULL;
	m_Thresh = NULL;

	readModel(&m_Model);

	int max_stage = m_Model.head.num_stage;
	int num_node = m_Model.head.num_leaf + m_Model.head.num_node;
	int num_point = m_Model.head.num_point;
	int num_tree_per_point = m_Model.head.num_tree_per_point;
	int node_step = m_Model.head.node_step;

	m_AX = new Mat_<float>[max_stage];
	m_AY = new Mat_<float>[max_stage];
	m_BX = new Mat_<float>[max_stage];
	m_BY = new Mat_<float>[max_stage];
	m_Thresh = new Mat_<float>[max_stage];

	for (int i = 0; i < max_stage; i++)
	{
		m_AX[i] = Mat::zeros(num_point, num_node * num_tree_per_point, CV_32FC1);
		m_AY[i] = Mat::zeros(num_point, num_node * num_tree_per_point, CV_32FC1);
		m_BX[i] = Mat::zeros(num_point, num_node * num_tree_per_point, CV_32FC1);
		m_BY[i] = Mat::zeros(num_point, num_node * num_tree_per_point, CV_32FC1);
		m_Thresh[i] = Mat::zeros(num_point, num_node * num_tree_per_point, CV_32FC1);

		for (int j = 0; j < num_point; j++)
		{
			int count = 0;
			for (int k = 0; k < num_tree_per_point; k++)
			{
				int index = i * num_point * num_tree_per_point + j * num_tree_per_point + k;
				Mat_<float> tmp = m_Model.rf.row(index);

				int pos = count;
				for (int l = 0; l < tmp.cols; l += node_step, pos++)
				{
					m_AX[i](j, pos) = tmp(0, l);
					m_AY[i](j, pos) = tmp(0, l + 1);
					m_BX[i](j, pos) = tmp(0, l + 2);
					m_BY[i](j, pos) = tmp(0, l + 3);
					m_Thresh[i](j, pos) = tmp(0, l + 4);
				}
				count += num_node;
			}
		}
	}

	Mat_<int> tmp1 = Mat::zeros(num_node, 1, CV_32SC1);
	Mat_<int> tmp2 = Mat::zeros(num_node, 2, CV_32SC1);

	for (int i = num_node / 2; i < num_node; i++)
		tmp1(i, 0) = 1;

	int tmp3 = 1;
	for (int i = 0; i < num_node / 2; i++)
	{
		tmp2(i, 0) = tmp3++;
		tmp2(i, 1) = tmp3++;
	}

	for (int i = 0; i < num_tree_per_point; i++)
	{
		m_Isleaf.push_back(tmp1);
		m_Cnodes.push_back(tmp2);
	}

	tmp3 = m_Model.head.num_leaf - 1;
	m_Idleafnodes = Mat::zeros(m_Model.head.num_leaf, 1, CV_32SC1);
	for (int i = 0; i < m_Idleafnodes.rows; i++)
		m_Idleafnodes(i, 0) = tmp3++;
}

int Tracking::readModel(sModel* model)
{
	FILE* fp = NULL;
	fp = fopen(m_Name.c_str(), "rb");
	if (fp == NULL){
		//LOGE("Cannot read %s file!!!!!", m_Name.c_str());
		return -1;
	}

	//read head
	fread(&model->head, sizeof(model->head), 1, fp);

	//read meanface
	model->meanface = Mat::zeros(2, model->head.num_point, CV_32FC1);
	for (int i = 0; i < 2; i++){
		fread(model->meanface.ptr<float>(i), sizeof(float)*model->meanface.cols, 1, fp);
	}
	model->meanface = model->meanface.t();

	//read random forest
	model->rf = Mat::zeros(model->head.num_tree_total, model->head.dim_tree, CV_32FC1);
	for (int i = 0; i < model->rf.rows; i++)
		fread(model->rf.ptr<float>(i), sizeof(float)*model->rf.cols, 1, fp);

	//read weights
	model->w = Mat::zeros(model->head.dim_feat* model->head.num_stage, model->head.num_point * 2, CV_32FC1);
	for (int i = 0; i < model->w.rows; i++)
		fread(model->w.ptr<float>(i), sizeof(float)*model->w.cols, 1, fp);

	fclose(fp);
	return 0;
}

Mat_<float> Tracking::Reshape_alt(Mat_<float>& mean, Rect& faceBox)
{
	Mat_<double> modelShape = mean.clone();
	Mat_<double> xCoords = modelShape.colRange(0, modelShape.cols / 2);
	Mat_<double> yCoords = modelShape.colRange(modelShape.cols / 2, modelShape.cols);

	double minX, maxX, minY, maxY;
	minMaxLoc(xCoords, &minX, &maxX);
	minMaxLoc(yCoords, &minY, &maxY);
	double faceboxScaleFactor = g_params.facebox_scale_factor;
	double modelWidth = maxX - minX;
	double modelHeight = maxY - minY;

	xCoords -= minX;
	yCoords -= minY;

	// scale it:
	xCoords *= faceBox.width / modelWidth;
	yCoords *= faceBox.height / modelHeight;

	xCoords += faceBox.x;
	yCoords += faceBox.y;
	return modelShape;
}

Mat_<int> Tracking::DerivBinaryfeat(const Mat_<uchar>&img, const Rect& bbox, const Mat_<float>& shape, int stage)
{
	int cols = 0;
	int num_point = m_Model.head.num_point;
	int num_tree_per_point = m_Model.head.num_tree_per_point;
	int num_leaf = m_Model.head.num_leaf;
	int low = 0, high = 0;

	Mat_<int> binary = Mat::zeros(1, num_point * num_leaf * num_tree_per_point, CV_32SC1);
	for (int i = 0; i < num_point; i++)
	{
		Mat_<int> tmp = lbf_fast(img, bbox, shape, i, stage);
		high = low + tmp.cols;
		tmp.copyTo(binary.colRange(low, high));
		low = high;
	}
	return binary;
}

Mat_<int> Tracking::lbf_fast(const Mat_<uchar>&img, const Rect& bbox, const Mat_<float>& shape, int landmarkID, int stage)
{
	int max_stage = m_Model.head.num_stage;
	int num_node = m_Model.head.num_leaf + m_Model.head.num_node;
	int num_point = m_Model.head.num_point;
	int num_tree_per_point = m_Model.head.num_tree_per_point;
	int num_leaf = m_Model.head.num_leaf;

	m_AX[stage].row(landmarkID) *= bbox.width;
	m_AY[stage].row(landmarkID) *= bbox.height;
	m_BX[stage].row(landmarkID) *= bbox.width;
	m_BY[stage].row(landmarkID) *= bbox.height;

	m_AX[stage].row(landmarkID) += shape(landmarkID, 0);
	m_AY[stage].row(landmarkID) += shape(landmarkID, 1);
	m_BX[stage].row(landmarkID) += shape(landmarkID, 0);
	m_BY[stage].row(landmarkID) += shape(landmarkID, 1);

	Mat_<int> cind = Mat::ones(m_AX[stage].cols, 1, CV_32SC1);
	Mat_<float> AX = m_AX[stage].row(landmarkID);
	Mat_<float> AY = m_AY[stage].row(landmarkID);
	Mat_<float> BX = m_BX[stage].row(landmarkID);
	Mat_<float> BY = m_BY[stage].row(landmarkID);
	Mat_<float> Thresh = m_Thresh[stage].row(landmarkID);


	int width = img.cols;
	int height = img.rows;

	for (int j = 0; j < AX.cols; j += num_node)
	{
		for (int index = 0; index < m_Model.head.num_node; index++)
		{
			int pos = j + index;
			int a_x = (int)(AX(0, pos) + 0.5);
			int a_y = (int)(AY(0, pos) + 0.5);
			int b_x = (int)(BX(0, pos) + 0.5);
			int b_y = (int)(BY(0, pos) + 0.5);

			a_x = MAX(0, MIN(a_x, width - 1));
			a_y = MAX(0, MIN(a_y, height - 1));
			b_x = MAX(0, MIN(b_x, width - 1));
			b_y = MAX(0, MIN(b_y, height - 1));

			float pixel_v_a = (float)img(Point(a_x, a_y));
			float pixel_v_b = (float)img(Point(b_x, b_y));
			float val = pixel_v_a - pixel_v_b;

			if (val < (float)Thresh(0, pos))
				cind(pos, 0) = 0;
		}
	}

	Mat_<int> binfeature = Mat::zeros(1, (int)sum(m_Isleaf).val[0], CV_32SC1);

	int cumnum_nodes = 0;
	int cumnum_leafnodes = 0;

	for (int t = 0; t < num_tree_per_point; t++)
	{
		int id_cnode = 0;
		while (1)
		{
			if (m_Isleaf(id_cnode + cumnum_nodes))
			{
				binfeature(0, cumnum_leafnodes + getIndex(m_Idleafnodes, id_cnode)) = 1;
				cumnum_nodes = cumnum_nodes + num_node;
				cumnum_leafnodes = cumnum_leafnodes + num_leaf;
				break;
			}
			id_cnode = m_Cnodes(cumnum_nodes + id_cnode, cind(cumnum_nodes + id_cnode, 0));
		}
	}
	return binfeature;
}

int Tracking::getIndex(const Mat_<int>&src, int val)
{
	for (int i = 0; i < src.rows; i++)
	{
		if (src(i, 0) == val)
			return i;
	}
	return -1;
}

void Tracking::UpDate(Mat_<int>&binary, Rect &bbox, Mat_<float>& shape, int stage)
{
	int num_point = m_Model.head.num_point;
	int w_cols = 2 * num_point;
	int w_rows = binary.cols;

	Mat_<float> deltashapes = Mat::zeros(1, w_cols, CV_32FC1);

	for (int i = 0; i < w_rows; i++)
	{
		if (binary(0, i) == 1)
			deltashapes += m_Model.w.row(stage * w_rows + i);
	}

	Mat_<float> deltax = deltashapes.colRange(0, num_point).t();
	deltax *= bbox.width;
	Mat_<float> deltay = deltashapes.colRange(num_point, w_cols).t();
	deltay *= bbox.height;

	shape.col(0) += deltax;
	shape.col(1) += deltay;
}