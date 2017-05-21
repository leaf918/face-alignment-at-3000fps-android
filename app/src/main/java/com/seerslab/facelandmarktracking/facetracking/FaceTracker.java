package com.seerslab.facelandmarktracking.facetracking;

import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by ahnsohyun on 4/21/16.
 */
public class FaceTracker {

    private AtomicBoolean mIsAvailable = new AtomicBoolean(false);

    public FaceTracker() {

    }

    public void initialize(String trackerFilePath, String detectorXmlFilePath) {
        if (mIsAvailable.compareAndSet(false, true)) {
            nativeInitializeFaceTracker(trackerFilePath, detectorXmlFilePath);
        }
    }

    public void reset() {
        nativeReset();
    }

    public void destroy() {
        if (mIsAvailable.compareAndSet(true, false)) {
            nativeDestroyFaceTracker();
        }
    }

    public float[] trackFace(byte[] frame, int width, int height, int facing) {
        if (mIsAvailable.get() && width > 0 && height > 0) {
            return nativeTrackFace(frame, width, height, facing);
        } else {
            return null;
        }
    }

    private native void nativeInitializeFaceTracker(String trackerFilePath, String detectorXmlFilePath);
    private native void nativeReset();
    private native void nativeDestroyFaceTracker();
    private native float[] nativeTrackFace(byte[] frame, int width, int height, int facing);
}
