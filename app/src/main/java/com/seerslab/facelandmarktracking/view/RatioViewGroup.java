package com.seerslab.facelandmarktracking.view;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.FrameLayout;

public class RatioViewGroup extends FrameLayout {

	private static float sRatio = 0.75f;
	public RatioViewGroup(Context context) {
		super(context);
	}
	
	public RatioViewGroup(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public RatioViewGroup(Context context, AttributeSet attrs, int defStyleAttr) {
		super(context, attrs, defStyleAttr);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		final int width = resolveSize(getSuggestedMinimumWidth(), widthMeasureSpec);
        final int height = resolveSize(getSuggestedMinimumHeight(), heightMeasureSpec);
    	if ((float)width/height > sRatio) {
//        	setMeasuredDimension((int) (FacePreview.CAMERA_RATIO * height), height);
    		widthMeasureSpec = MeasureSpec.makeMeasureSpec((int) (sRatio * height), MeasureSpec.EXACTLY);
        } else {
//        	setMeasuredDimension(width, (int) (width / FacePreview.CAMERA_RATIO));
        	heightMeasureSpec = MeasureSpec.makeMeasureSpec((int) (width / sRatio), MeasureSpec.EXACTLY);
        }
    	super.onMeasure(widthMeasureSpec, heightMeasureSpec);
	}
}
