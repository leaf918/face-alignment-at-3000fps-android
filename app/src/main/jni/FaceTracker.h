//
// Created by AhnSohyun on 5/11/16.
//

#ifndef FACELANDMARKTRACKING_FACETRACKER_H
#define FACELANDMARKTRACKING_FACETRACKER_H

#include "Tracking.h"

#define RESIZED_WIDTH 320.0f
#define NUM_LANDMARK_POINTS 68

class FaceTracker {
public:
    Mat gray;
    Mat scaled_image;
    vector<Rect> face_boxs;
    vector<Mat_<float> > current_shapes;

    FaceTracker(const char* tracker_path, const char* detector_xml_path);
    ~FaceTracker();

    void Reset();
    int TrackFace(unsigned char* frame, int width, int height, int facing);
    float* get_shape(); // 리턴값: 포인트 정보 (포인트개수 * 2, Mat 의 width, height 로 나눈 array 값)
    float get_roll_angle(); // 리턴값: 얼굴 회전 각도 (degree)
    float* get_face_boundary(); // 리턴값: 얼굴이 검출 영역 ([x, y, width, height], Mat 의 width, height 로 나눈 값)


private:
// TJ Start
    double scale;
    float* face_region;
    float* face_shape;
    vector<Rect> faces;
    Tracking tracker;

    bool setGlobalParams(string cascadeName);
    void Initialize(string model_path, string cascade_path);
    bool isDetected();
    bool getFaceBound();
// TJ End
    int DetectLandmark(Mat& preprocessed_mat); // 리턴값: 성공 여부 (0, 1) or 신뢰도 ([0,10]: 10에 가까울 수록 좋은 결과값)
    Mat GetPreProcessingMat(unsigned char* frame, int width, int height, int facing);


};

#endif //FACELANDMARKTRACKING_FACETRACKER_H
