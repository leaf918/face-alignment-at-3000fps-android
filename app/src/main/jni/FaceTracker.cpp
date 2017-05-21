//
// Created by AhnSohyun on 5/11/16.
//

#include "FaceTracker.h"

FaceTracker::FaceTracker(const char* tracker_path, const char* detector_xml_path) {
    // Getter 메소드로 얻을 수 있는 변수들은 모두 이 함수내에서 값이 갱신된다
    //LOG.D("TJ Initialilze Start");
    Initialize(tracker_path, detector_xml_path);
    //LOGD("TJ Convert RGB to Gray");
}

FaceTracker::~FaceTracker() {
    delete[] face_region;
    delete[] face_shape;
}

void FaceTracker::Initialize(string modelPath, string cascadeName) {
    scale = 1.0;

    face_region = new float[4];
    face_shape = new float[NUM_LANDMARK_POINTS*2];
    for( int i=0; i<4; i++)
        face_region[i] = 0.0;
    for( int i=0; i<NUM_LANDMARK_POINTS; i++)
    {
        face_shape[i*2] = 0.0;
        face_shape[i*2+1] = 0.0;
    }

    // SetGlobalParameters
    if(!setGlobalParams(cascadeName))
    {
        //LOGE("ERROR : Could not setting the global parametere!!! \n");
    }

    // Load Model file
    tracker.Init(modelPath + "/model.bin");
}

bool FaceTracker::setGlobalParams(string cascadeName) {
    double m_max_numfeats[8] = { 1000, 1000, 1000, 500, 500, 500, 400, 400};
    double m_max_radio_radius[8] = { 0.3, 0.2, 0.2, 0.15, 0.12, 0.10, 0.08, 0.06};

    g_params.max_numstage = 4;
    g_params.max_depth = 3;
    g_params.max_nodes = (1 << g_params.max_depth) - 1;
    g_params.max_numtrees = 5;
    g_params.max_numthreshs = 500;
    g_params.bagging_overlap = 0.4;

    for (int i = 0; i < 10; i++)
    {
        g_params.max_numfeats[i] = m_max_numfeats[i];
        g_params.max_raio_radius[i] = m_max_radio_radius[i];
    }

    // Mean face parameters
    g_params.max_iters = 120;
    g_params.max_errors = 0.000001;
    g_params.facebox_scale_factor = 1.0;
    g_params.facebox_scale_const = 1.0;
    g_params.facebox_with_div = 1.95;
    g_params.facebox_height_div = 1.6;

    // load cascade file
    if(!g_params.faceCascade.load(cascadeName))
    {
        //LOGE("ERROR : Could not load classifier cascade!!! \n");
        return false;
    }
    return true;
}

void FaceTracker::Reset() {
}

int FaceTracker::TrackFace(unsigned char *frame, int width, int height, int facing) {
    Mat image = GetPreProcessingMat(frame, width, height, facing);
    int success = DetectLandmark(image);
    //if( success )
        //LOGE("TJ face detecting is success !!");
    //else
        //LOGE("TJ face is not detecting!!");

    image.release();
    gray.release();
    scaled_image.release();
    return success;
}

float *FaceTracker::get_shape() {
    return face_shape;
}

float FaceTracker::get_roll_angle() {
    return 0;
}

float *FaceTracker::get_face_boundary() {
    return face_region;
}

int FaceTracker::DetectLandmark(Mat &preprocessed_mat) {
    // TODO
    cvtColor(preprocessed_mat, gray, CV_BGR2GRAY);
    scaled_image.create(cvRound(gray.rows / scale), cvRound(gray.cols / scale), CV_8UC1);
    resize(gray, scaled_image, scaled_image.size(), 0, 0, INTER_LINEAR);
    equalizeHist(scaled_image, scaled_image);

    float width = (float) preprocessed_mat.cols;
    float height = (float) preprocessed_mat.rows;
    if (isDetected())
    {
        rectangle(preprocessed_mat, cvPoint(faces[0].x, faces[0].y), cvPoint(faces[0].x + faces[0].width, faces[0].y+ faces[0].height), Scalar(0, 255, 0), 1, 8, 0);

        getFaceBound();
        float* face_bound = get_face_boundary();
        float* shape = get_shape();

        face_bound[0] = face_boxs[0].x / width;
        face_bound[1] = face_boxs[0].y / height;
        face_bound[2] = face_boxs[0].width / width;
        face_bound[3] = face_boxs[0].height / height;

        for( int i=0; i<NUM_LANDMARK_POINTS; i++)
        {
            shape[i*2] = current_shapes[0](i, 0) / width;
            shape[i*2+1] = current_shapes[0](i, 1) / height;
            //LOGE("TJ Shape : x : %f, y%f", shape[i*2], shape[i*2+1]);
        }
        return 1;
    }
    else
        return 0;
}

Mat FaceTracker::GetPreProcessingMat(unsigned char *frame, int width, int height, int facing) {
    // 0. Make mat.
    Mat yuv(height + height/2, width, CV_8UC1, frame);

    // 1. Resize image.
    int resized_height = (int) ((float) height * RESIZED_WIDTH / (float) width);
    Mat resized_yuv(resized_height + resized_height / 2, (int) RESIZED_WIDTH, CV_8UC1);
    resize(yuv, resized_yuv, resized_yuv.size());

    // Histogram equalization
    Mat y_roi(resized_yuv, Rect(0, 0, (int) RESIZED_WIDTH, resized_height));
    equalizeHist(y_roi, y_roi);

    // 2. Convert YUV420 -> BGR
    Mat rgb(resized_height, (int) RESIZED_WIDTH, CV_8UC3);
    cvtColor(resized_yuv, rgb, CV_YUV420sp2BGR);

    // 3. Rotate image.
    Mat tmp, rotated_image;
    transpose(rgb, tmp);
    if (facing == 1) {
        flip(tmp, rotated_image, -1); //transpose+flip(1)=CW
    } else {
        flip(tmp, rotated_image, 1);
    }

    yuv.release();
    resized_yuv.release();
    rgb.release();
    tmp.release();
    return rotated_image;
}

bool FaceTracker::isDetected() {
    if( !faces.empty() )
        faces.clear();
    clock_t t = clock();
    g_params.faceCascade.detectMultiScale(scaled_image, faces, 1.1, 2, 0 | CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_DO_ROUGH_SEARCH | CV_HAAR_SCALE_IMAGE, Size(30, 30));
    //LOGE("detection time = %f sec", (double)(clock()-t) / CLOCKS_PER_SEC);

    if (faces.empty())
        return false;
    else
        return true;
}

bool FaceTracker::getFaceBound() {
    sModel model = tracker.GetModel();
    int stages = model.head.num_stage;
    Mat_<float> meanface = model.meanface;
    current_shapes.clear();
    face_boxs.clear();
    for (int i = 0; i < faces.size(); i++)
    {
        clock_t t = clock();
        Rect detected_region;
        Mat_<float> shape = tracker.Reshape_alt(meanface, faces[i]);
        for (int j = 0; j < stages; j++){
            Mat_<int> binary = tracker.DerivBinaryfeat(scaled_image, faces[i], shape, j);
            tracker.UpDate(binary, faces[i], shape, j);
        }

        detected_region.x = faces[i].x * scale;
        detected_region.y = faces[i].y * scale;
        detected_region.width = faces[i].width * scale;
        detected_region.height = faces[i].height * scale;

        current_shapes.push_back(shape);
        face_boxs.push_back(detected_region);
        //LOGI("Alignment runtime : %f sed", (double)(clock()-t) / CLOCKS_PER_SEC);
    }

    if (current_shapes.empty())
        return false;
    else
        return true;
}