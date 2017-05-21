package com.seerslab.facelandmarktracking.camera;

import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.Size;
import android.support.annotation.NonNull;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.seerslab.facelandmarktracking.facetracking.FaceTracker;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

@SuppressWarnings("deprecation")
public class CameraHandler implements SurfaceHolder.Callback {

    public static final float CAMERA_RATIO = 0.75f;
	private final String LOG_TAG = "CameraHandler";

    private Camera mCamera;
    private byte[] mPreviewData = null;
    private CameraStateListener mListener = null;

    private SurfaceView mSurfaceView;
    private SurfaceHolder mHolder;

    /** 카메라 파라미터 관련 */
    private final List<Integer> mFrontCameraIds = new ArrayList<>();
    private final List<Integer> mRearCameraIds = new ArrayList<>();
    private int mCameraFacing = Camera.CameraInfo.CAMERA_FACING_FRONT;
    private final int[] mCameraIdIndices = new int[2];
    private int mCameraOrientation = 0;
    private Size mPreviewSize;
    private int mNumCameras = 0;

    /** 기기 파라미터 관련 */
    private int mSensorOrientation = 3;
    private final int mDeviceRotation;

    private final AtomicBoolean mIsStarted;

    /** 얼굴 인식 파라미터 */
    private final FaceTracker mFaceTracker;

    public CameraHandler(SurfaceView surfaceView, int deviceRotation, FaceTracker faceTracker) {
        mSurfaceView = surfaceView;
        mHolder = surfaceView.getHolder();
        mHolder.addCallback(this);

        mDeviceRotation = deviceRotation;
        mIsStarted = new AtomicBoolean(false);
        mFaceTracker = faceTracker;

        mNumCameras = Camera.getNumberOfCameras();
        initializeCameraIds();
    }

    public void setCameraStateListener(CameraStateListener listener) {
        mListener = listener;
    }

    private int getCameraId(int facing, int cameraIdIndex) {
        int cameraId = -1;
        if (facing == Camera.CameraInfo.CAMERA_FACING_FRONT
                && mFrontCameraIds.size() > cameraIdIndex) {
            cameraId = mFrontCameraIds.get(cameraIdIndex);
        } else if (facing == Camera.CameraInfo.CAMERA_FACING_BACK
                && mRearCameraIds.size() > cameraIdIndex) {
            cameraId = mRearCameraIds.get(cameraIdIndex);
        }
        return cameraId;
    }

    public void startCamera() {
        startCamera(mCameraFacing, mCameraIdIndices[mCameraFacing]);
    }

    private void startCamera(final int facing, final int cameraIdIndex) {
        if (mIsStarted.compareAndSet(false, true)) {
            if (mFrontCameraIds.size() == 0 && mRearCameraIds.size() == 0) {
                mIsStarted.set(false);
                mListener.onReadyCamera(false, 0, 0);
            }

            int cameraId = getCameraId(facing, cameraIdIndex);
            if (cameraId < 0) {
                mIsStarted.set(false);
                mListener.onReadyCamera(false, 0, 0);
                return;
            }

            try {
                mCamera = openCamera(cameraId);
                initCamera(mCamera, facing);

                Parameters camParams = mCamera.getParameters();
                initCameraParameter(camParams);
                mCamera.setErrorCallback(mErrorCb);
                mCamera.setParameters(camParams);
                mCamera.setPreviewCallbackWithBuffer(mPreviewCb);
                mCamera.addCallbackBuffer(mPreviewData);
                mCamera.startPreview();
                if (mListener != null) mListener.onReadyCamera(true, mPreviewSize.width, mPreviewSize.height);

            } catch (NullPointerException e) {
                Log.e(LOG_TAG, "Error opening camera - NullPointerException: ", e);
                mIsStarted.set(false);
                if (mListener != null) mListener.onReadyCamera(false, 0, 0);

            } catch (IOException e) {
                Log.e(LOG_TAG, "SetPreviewTexture failed.", e);
                if (mCamera != null) {
                    releaseCamera(mCamera);
                    mCamera = null;
                }
                mIsStarted.set(false);
                if (mListener != null) mListener.onReadyCamera(false, 0, 0);

            } catch (RuntimeException e) {
                Log.e(LOG_TAG, "Error opening camera - RuntimeException: ", e);
                if (mCamera != null) {
                    releaseCamera(mCamera);
                    mCamera = null;
                }
                mIsStarted.set(false);
                if (mListener != null) mListener.onReadyCamera(false, 0, 0);
            }
        }
    }
	
	public void stopCamera() {
        if (mIsStarted.compareAndSet(true, false)) {
            try {
                mCamera.stopPreview();
                releaseCamera(mCamera);
                mCamera = null;
            } catch (NullPointerException e) {
                Log.e(LOG_TAG, "Error Stopping camera - NullPointerException: ", e);
            } catch (RuntimeException e) {
                Log.e(LOG_TAG, "Error Stopping camera - RuntimeException: ", e);
            }
        }
	}

    private void initializeCameraIds() {
        try {
            Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
            for (int i = 0; i < mNumCameras; i++) {
                Camera.getCameraInfo(i, cameraInfo);
                if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    mFrontCameraIds.add(i);
                } else if (cameraInfo.facing == Camera.CameraInfo.CAMERA_FACING_BACK) {
                    mRearCameraIds.add(i);
                }
            }
            mCameraIdIndices[0] = 0;
            mCameraIdIndices[1] = 0;
        } catch (RuntimeException e) {
            Log.e(LOG_TAG, "Getting camera IDs failed.", e);
            mFrontCameraIds.clear();
            mRearCameraIds.clear();
            mCameraIdIndices[0] = 0;
            mCameraIdIndices[1] = 0;
        }
    }

    private Camera openCamera(int cameraId) throws RuntimeException {
        Camera ret = null;
        if (cameraId >= 0 && cameraId < mNumCameras) {
            ret = Camera.open(cameraId);
        }
        return ret;
    }

	private void initCamera(@NonNull Camera camera, int facing) throws IOException {
        camera.setPreviewDisplay(mHolder);
        mCameraOrientation = setCameraDisplayOrientation(facing, mDeviceRotation);
        camera.setDisplayOrientation(mCameraOrientation);
	}
	
	private void initCameraParameter(@NonNull Parameters camParams) {
		Size previewSize = getFittedPreviewSize(camParams.getSupportedPreviewSizes(), 0.75f);
		if (previewSize != null) {
			camParams.setPreviewSize(previewSize.width, previewSize.height);
		} else {
            previewSize = camParams.getPreviewSize();
        }
        mPreviewSize = previewSize;

        int size = previewSize.width * previewSize.height * 3 / 2 + 4084;
        mPreviewData = new byte[size];
	}
	
	private void releaseCamera(@NonNull Camera camera) {
		try {
            camera.setErrorCallback(null);
			camera.setPreviewTexture(null);
            camera.setPreviewCallbackWithBuffer(null);
            camera.addCallbackBuffer(null);
			camera.release();
		} catch (IOException | RuntimeException e) {
			Log.e(LOG_TAG, "Release camera error.", e);
		}
    }

    /** 카메라 파라미터 변경 메소드 */
	private Size getFittedPreviewSize(List<Size> supportedPreviewSize, float previewRatio) {
		Size retSize = null;
		for (Size size : supportedPreviewSize) {
			float ratio = size.height / (float) size.width;
            float EPSILON = (float) 1e-4;
            if (Math.abs(ratio - previewRatio) < EPSILON && Math.max(size.width, size.height) <= 1024 ) {
				if (retSize == null || retSize.width < size.width) {
					retSize = size;
				}
			}
		}
		return retSize;
	}

    private int setCameraDisplayOrientation(int cameraId, int deviceRotation) {
        Camera.CameraInfo info = new Camera.CameraInfo();
        Camera.getCameraInfo(cameraId, info);

        int degrees = 0;
        switch (deviceRotation) {
            case Surface.ROTATION_0: degrees = 0; break;
            case Surface.ROTATION_90: degrees = 90; break;
            case Surface.ROTATION_180: degrees = 180; break;
            case Surface.ROTATION_270: degrees = 270; break;
        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;  // compensate the mirror
        } else {  // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }

        return result;
    }

    private final Camera.ErrorCallback mErrorCb = new Camera.ErrorCallback() {

        @Override
        public void onError(int error, Camera camera) {
            Log.e(LOG_TAG, "camera error - " + error);
            stopCamera();
            startCamera(mCameraFacing, mCameraIdIndices[mCameraFacing]);
        }
    };

    private final Camera.PreviewCallback mPreviewCb = new Camera.PreviewCallback() {
        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            if (mPreviewSize != null) {
                float[] faces = mFaceTracker.trackFace(data, mPreviewSize.width, mPreviewSize.height, mCameraFacing);
                if (faces != null) {
                    if (faces.length == 0) {
                        faces = null;
                    }
                }
                if (mListener != null) {
                    mListener.onFoundFace(faces);
                }
            }
            camera.addCallbackBuffer(data);
        }
    };

    /** FaceTracking methods */
    public void startFaceTracking(String trackerFilePath, String detectorXmlFilePath) {
        if (mCamera != null) {
            mCamera.addCallbackBuffer(mPreviewData);
        }
        mFaceTracker.initialize(trackerFilePath, detectorXmlFilePath);
    }

    public void stopFaceTracking() {
        mFaceTracker.destroy();
    }

    /** SurfaceHolder.Callback */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // The Surface has been created, now tell the camera where to draw the preview.

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // If your preview can change or rotate, take care of those events here.
        // Make sure to stop the preview before resizing or reformatting it.

        if (mHolder.getSurface() == null) {
            // preview surface does not exist
            return;
        }

        // stop preview before making changes
        try {
            stopCamera();
            //releaseCamera(mCamera);
            //mCamera.stopPreview();
        } catch (Exception e){
            // ignore: tried to stop a non-existent preview
        }

        // set preview size and make any resize, rotate or
        // reformatting changes here

        // start preview with new settings
        try {
            startCamera();
            //mCamera.setPreviewDisplay(mHolder);
            //mCamera.startPreview();

        } catch (Exception e){
            Log.d(LOG_TAG, "Error starting camera preview: " + e.getMessage());
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public interface CameraStateListener {
        void onReadyCamera(boolean success, int width, int height);
        void onFoundFace(float[] faces);
        void onUpdateFaceFoundState(boolean foundFace);
    }
}
