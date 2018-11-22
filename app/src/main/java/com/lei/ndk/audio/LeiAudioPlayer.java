package com.lei.ndk.audio;

import android.text.TextUtils;

import com.lei.ndk.util.LogUtil;

/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class LeiAudioPlayer {
    private long mNativePtr;
    private String mDataSource;

    public LeiAudioPlayer() {
        mNativePtr = nativeInit();
    }

    public void setDataSource(String dataSource) {
        mDataSource = dataSource;
    }

    public void prepared() {
        if (TextUtils.isEmpty(mDataSource)) {
            LogUtil.e("dataSource is null");
            return;
        }
        nativePrepare(mNativePtr, mDataSource);
    }

    public void start() {
        if (TextUtils.isEmpty(mDataSource)) {
            LogUtil.e("dataSource is null");
            return;
        }
        nativeStart(mNativePtr);
    }

    private void onNativeCallPrepared() {
        LogUtil.d("--onNativeCallPrepared--");
    }

    private void onNativeCallError(int code, String msg) {
        LogUtil.d("--onNativeCallError--");

    }

    private native long nativeInit();

    private native void nativePrepare(long mNativePtr, String dataSource);

    private native void nativeStart(long mNativePtr);
}
