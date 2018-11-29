package com.lei.ndk.audio;

import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;

import com.lei.ndk.util.LogUtil;


/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class LeiAudioPlayer {
    private static final Handler mHandler = new Handler(Looper.getMainLooper());
    private long mNativePtr;
    private String mDataSource;

    public void setCallBack(ICallBack mCallBack) {
        this.mCallBack = mCallBack;
    }

    private ICallBack mCallBack;

    public interface ICallBack {
        void onDurationChanged(int current, int all);

        void onPlayFinished();
    }

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

    public void pause() {
        nativePause(mNativePtr);
    }

    public void resume() {
        nativeResume(mNativePtr);
    }

    public void seek(int second) {
        nativeSeek(mNativePtr, second);
    }
    public void setVolume(int percent) {
        nativeSetVolume(mNativePtr, percent);
    }
    public void destroy() {
        nativeDestroy(mNativePtr);
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

    private void onNativeCallDataOnLoad(boolean onLoad) {
        LogUtil.d("--onNativeCallDataOnLoad onLoad = " + onLoad);
    }

    private void onNativeCallError(int code, String msg) {
        LogUtil.d("--onNativeCallError--");
    }

    public void onNativeCallDuration(final int current, final int all) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onDurationChanged(current, all);
                }
            }
        });
    }

    public void onNativeCallFinished() {
        LogUtil.d("--onNativeCallFinished--");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null)
                    mCallBack.onPlayFinished();
            }
        });
    }

    private native long nativeInit();

    private native void nativePrepare(long mNativePtr, String dataSource);

    private native void nativeStart(long mNativePtr);

    private native void nativePause(long mNativePtr);

    private native void nativeResume(long mNativePtr);

    private native void nativeSeek(long mNativePtr, int second);

    private native void nativeDestroy(long mNativePtr);

    private native void nativeSetVolume(long mNativePtr, int percent);
}
