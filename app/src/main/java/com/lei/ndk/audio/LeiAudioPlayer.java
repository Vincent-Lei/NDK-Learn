package com.lei.ndk.audio;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.text.TextUtils;

import com.lei.ndk.util.LogUtil;

import java.io.File;


/**
 * Created by Vincent.Lei on 2018/11/21.
 * Title：
 * Note：
 */
public class LeiAudioPlayer {
    public interface Mute {
        int RIGHT = 0;
        int LEFT = 1;
        int DOUBLE = 2;
    }

    private static final Handler mHandler = new Handler(Looper.getMainLooper());
    private long mNativePtr;
    private String mDataSource;
    private HandlerThread mAACHandlerThread;
    private PcmToAac mPcmToAac;
    private Handler mAACHandler;
    private boolean isOnRecording;
    private boolean isPauseRecording;

    public boolean isOnRecording() {
        return isOnRecording;
    }

    public boolean isPauseRecording() {
        return isPauseRecording;
    }

    public void setCallBack(ICallBack mCallBack) {
        this.mCallBack = mCallBack;
    }

    private ICallBack mCallBack;

    public interface ICallBack {
        void onDurationChanged(int current, int all);

        void onAmplitudeChanged(int amplitude);

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

    public void setPitch(float pitch) {
        nativeSetPitch(mNativePtr, pitch);
    }

    public void setSpeed(float speed) {
        nativeSetSpeed(mNativePtr, speed);
    }

    public void setMute(int mute) {
        if (mute == Mute.DOUBLE || mute == Mute.RIGHT || mute == Mute.LEFT)
            nativeSetMute(mNativePtr, mute);
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

    private static class AACTask implements Runnable {
        int sampleRate;
        int size;
        byte[] buff;
        PcmToAac pcmToAac;

        AACTask(int size, byte[] buff, PcmToAac pcmToAac) {
            this(0, size, buff, pcmToAac);
        }

        AACTask(int sampleRate, int size, byte[] buff, PcmToAac pcmToAac) {
            this.sampleRate = sampleRate;
            this.size = size;
            this.buff = buff;
            this.pcmToAac = pcmToAac;
        }

        @Override
        public void run() {
            if (pcmToAac == null)
                return;
            if (sampleRate > 0 && !pcmToAac.isInit())
                pcmToAac.init(sampleRate);
            if (size == 0 || buff == null) {
                pcmToAac.onDestory();
                pcmToAac = null;
            } else if (!pcmToAac.isDestory())
                pcmToAac.encodecPcmToAAc(size, buff);
        }
    }

    private synchronized void initAACThread() {
        if (mAACHandlerThread == null) {
            mAACHandlerThread = new HandlerThread("aac handler thread");
            mAACHandlerThread.start();
            mAACHandler = new Handler(mAACHandlerThread.getLooper());
        }
    }

    public void start2RecordAAC(File fileDest) {
        if (mPcmToAac != null) {
            LogUtil.e("please stopRecordAAC first");
            return;
        }
        if (mAACHandler == null)
            initAACThread();
        mPcmToAac = new PcmToAac(fileDest);
        nativePCMRecord(mNativePtr, true);
        isOnRecording = true;
        isPauseRecording = false;
    }

    public void pauseRecordAAC() {
        if (mPcmToAac == null) {
            LogUtil.e("please start2RecordAAC first");
        }
        LogUtil.d("---pauseRecordAAC---");
        isPauseRecording = true;
        nativePCMRecord(mNativePtr, false);
    }

    public void resumeRecordAAC() {
        if (mPcmToAac == null) {
            LogUtil.e("please start2RecordAAC first");
        }
        LogUtil.d("---resumeRecordAAC---");
        isPauseRecording = false;
        nativePCMRecord(mNativePtr, true);
    }

    public void stopRecordAAC() {
        LogUtil.d("---stopRecordAAC---");
        isOnRecording = false;
        isPauseRecording = false;
        nativePCMRecord(mNativePtr, false);
        if (mPcmToAac != null)
            mAACHandler.post(new AACTask(0, null, mPcmToAac));
        mPcmToAac = null;
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
        stopRecordAAC();
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null)
                    mCallBack.onPlayFinished();
            }
        });
    }

    public void onNativeCallAmplitude(final int amplitude) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onAmplitudeChanged(amplitude);
                }
            }
        });
    }

    public void onNativePCMRecord(final int sampleRate, int size, byte[] buff) {
        if (mPcmToAac != null && mAACHandler != null)
            mAACHandler.post(new AACTask(sampleRate, size, buff, mPcmToAac));
    }


    private native long nativeInit();

    private native void nativePrepare(long mNativePtr, String dataSource);

    private native void nativeStart(long mNativePtr);

    private native void nativePause(long mNativePtr);

    private native void nativeResume(long mNativePtr);

    private native void nativeSeek(long mNativePtr, int second);

    private native void nativeDestroy(long mNativePtr);

    private native void nativeSetVolume(long mNativePtr, int percent);

    private native void nativeSetMute(long mNativePtr, int mute);

    private native void nativeSetPitch(long mNativePtr, float pitch);

    private native void nativeSetSpeed(long mNativePtr, float speed);

    private native void nativePCMRecord(long mNativePtr, boolean recordPCM);
}
