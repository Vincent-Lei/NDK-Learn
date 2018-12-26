package com.sdk.video;

import android.text.TextUtils;

import com.lei.ndk.util.LogUtil;


/**
 * Created by Vincent.Lei on 2018/12/24.
 * Title：
 * Note：
 */
public class VideoPlayer {
    private long mNativePtr;
    private String mDataSource;
    private VideoGLSurfaceView videoGLSurfaceView;

    public VideoPlayer() {
        mNativePtr = nativeInit();
    }

    public void prepare(String dataSource) {
        this.mDataSource = dataSource;
        if (!TextUtils.isEmpty(mDataSource)) {
            nativePrepare(mNativePtr, dataSource);
        }
    }

    public void start() {
        if (TextUtils.isEmpty(mDataSource)) {
            LogUtil.d("dataSource is null");
            return;
        }
        nativeStart(mNativePtr);
    }

    public void setVideoGLSurfaceView(VideoGLSurfaceView videoGLSurfaceView) {
        this.videoGLSurfaceView = videoGLSurfaceView;
    }

    private void nativeCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        if (videoGLSurfaceView != null)
            videoGLSurfaceView.setYUVData(width, height, y, u, v);
    }

    private native long nativeInit();

    private native void nativePrepare(long ptr, String dataSource);

    private native void nativeStart(long ptr);
}
