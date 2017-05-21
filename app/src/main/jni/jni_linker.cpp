#include <jni.h>
#include "FaceTracker.h"

FaceTracker* faceTracker = NULL;

extern "C" {

JNIEXPORT void JNICALL
Java_com_seerslab_facelandmarktracking_facetracking_FaceTracker_nativeInitializeFaceTracker( JNIEnv *env,
                                                                                             jobject instance,
                                                                                             jstring trackerFilePath_,
                                                                                             jstring detectorXmlFilePath_) {
    const char *trackerFilePath = env->GetStringUTFChars(trackerFilePath_, 0);
    const char *detectorXmlFilePath = env->GetStringUTFChars(detectorXmlFilePath_, 0);

    if (faceTracker == NULL) {
        faceTracker = new FaceTracker(trackerFilePath, detectorXmlFilePath);
    }

    env->ReleaseStringUTFChars(trackerFilePath_, trackerFilePath);
    env->ReleaseStringUTFChars(detectorXmlFilePath_, detectorXmlFilePath);
}

JNIEXPORT void JNICALL
Java_com_seerslab_facelandmarktracking_facetracking_FaceTracker_nativeDestroyFaceTracker(
        JNIEnv *env, jobject instance) {
    if (faceTracker != NULL) {
        delete faceTracker;
        faceTracker = NULL;
    }
}

JNIEXPORT jfloatArray JNICALL
Java_com_seerslab_facelandmarktracking_facetracking_FaceTracker_nativeTrackFace(JNIEnv *env,
                                                                                jobject instance,
                                                                                jbyteArray frame_,
                                                                                jint width,
                                                                                jint height,
                                                                                jint facing) {
    jbyte *frame = env->GetByteArrayElements(frame_, NULL);
    jfloatArray retArray = NULL;
    if (faceTracker != NULL) {
        int success = faceTracker->TrackFace((unsigned char*) frame, width, height, facing);
        if (success > 0) {
            float* landmark_points = faceTracker->get_shape();
            float* face_bound = faceTracker->get_face_boundary();
//            float angle = faceTracker->get_roll_angle();

            retArray = env->NewFloatArray(NUM_LANDMARK_POINTS * 2 + 4);
            env->SetFloatArrayRegion(retArray, 0, NUM_LANDMARK_POINTS * 2, landmark_points);
            env->SetFloatArrayRegion(retArray, NUM_LANDMARK_POINTS * 2, 4, face_bound);
        }
    }

    env->ReleaseByteArrayElements(frame_, frame, 0);
    return retArray;
}

JNIEXPORT void JNICALL
Java_com_seerslab_facelandmarktracking_facetracking_FaceTracker_nativeReset(JNIEnv *env,
                                                                            jobject instance) {
    if (faceTracker != NULL) {
        faceTracker->Reset();
    }
}

}