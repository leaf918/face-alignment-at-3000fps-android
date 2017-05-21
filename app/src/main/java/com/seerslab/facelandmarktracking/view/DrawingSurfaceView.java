package com.seerslab.facelandmarktracking.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Created by ahnsohyun on 4/22/16.
 */
public class DrawingSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

    private DrawingThread mThread;
    public DrawingSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        getHolder().addCallback(this);
        getHolder().setFormat(PixelFormat.TRANSLUCENT);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (mThread == null || !mThread.isAlive()) {
            mThread = new DrawingThread(getHolder());
            mThread.setRunning(true);
            mThread.start();
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mThread.setRunning(false);
    }

    public void setFacePoints(float[] landmarkPoints) {
        mThread.setFacePoints(landmarkPoints);
    }

    private class DrawingThread extends Thread {

        private final SurfaceHolder mSurfaceHolder;
        private AtomicBoolean mIsRunning = new AtomicBoolean(false);
        private final Paint mPaint;
        private final Paint mRectPaint;
        private final List<Float> mLandmarkPoints = new ArrayList<>();

        public DrawingThread(SurfaceHolder surfaceHolder) {
            mSurfaceHolder = surfaceHolder;
            mPaint = new Paint();
            mPaint.setColor(Color.BLUE);
            mRectPaint = new Paint();
            mRectPaint.setColor(Color.WHITE);
            mRectPaint.setStyle(Paint.Style.STROKE);
            mRectPaint.setStrokeWidth(5.0f);
        }

        public void setRunning(boolean flag) {
            mIsRunning.set(flag);
        }

        /**
         *
         * @param landmarkPoints Normalized face points. [0, 1]
         */
        public void setFacePoints(float[] landmarkPoints) {
            synchronized (mLandmarkPoints) {
                mLandmarkPoints.clear();
                if (landmarkPoints != null) {
                    for (float point : landmarkPoints) {
                        mLandmarkPoints.add(point);
                    }
                }
            }
        }

        @Override
        public void run() {
            while (mIsRunning.get()) {
                Canvas canvas = null;
                try {
                    canvas = mSurfaceHolder.lockCanvas();
                    synchronized (mSurfaceHolder) {
                        draw(canvas);
                    }
                } finally {
                    if (canvas != null) {
                        mSurfaceHolder.unlockCanvasAndPost(canvas);
                    }
                }
            }
        }

        private void draw(Canvas canvas) {
            if (canvas != null) {
                canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);

                synchronized (mLandmarkPoints) {
                    if (mLandmarkPoints.size() > 0) {
                        for (int i = 0; i < mLandmarkPoints.size() - 4; i += 2) {
                            float x = mLandmarkPoints.get(i) * canvas.getWidth();
                            float y = mLandmarkPoints.get(i + 1) * canvas.getHeight();
                            canvas.drawCircle(x, y, 8, mPaint);
                        }

                        // FaceBound

                        int left = (int) (mLandmarkPoints.get(mLandmarkPoints.size() - 4) * canvas.getWidth());
                        int top = (int) (mLandmarkPoints.get(mLandmarkPoints.size() - 3) * canvas.getHeight());
                        int width = (int) (mLandmarkPoints.get(mLandmarkPoints.size() - 2) * canvas.getWidth());
                        int height = (int) (mLandmarkPoints.get(mLandmarkPoints.size() - 1) * canvas.getHeight());
                        canvas.drawRect(left, top, left + width, top + height, mRectPaint);
                    }
                }
            }
        }
    }
}
