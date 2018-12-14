package com.sdk.audio;

import android.os.Handler;
import android.os.Looper;
import android.text.TextUtils;

import com.sdk.audio.transcoding.BasePCM2Transcoder;
import com.sdk.audio.transcoding.TranscoderFactory;
import com.sdk.audio.transcoding.TranscoderTask;

import java.io.File;

/**
 * Created by Vincent.Lei on 2018/12/12.
 * Title：
 * Note：
 */
public class AudioPlayer implements TranscoderFactory.ITranscodeCallBack {

    public interface ICallBack {
        void onAudioDurationChanged(int current, int all);

        void onAudioAmplitudeChanged(int amplitude);

        void onAudioPlayFinished();

        void onAudioPrepared();

        void onAudioWaitLoadData(boolean waiting);

        void onAudioError(int code, String msg);

        void onAudioTranscodeFinished();
    }


    public interface Mute {
        int RIGHT = 0;
        int LEFT = 1;
        int DOUBLE = 2;
    }

    public static final int DEFAULT_VOLUME_PERCENT = 50;

    private static final Handler mHandler = new Handler(Looper.getMainLooper());
    private long mNativePtr;
    private String mDataSource;
    private ICallBack mCallBack;
    private BasePCM2Transcoder transcoder;
    private boolean isTranscodePause;
    private boolean isTranscoding;

    public AudioPlayer() {
        mNativePtr = nativeInit();
    }

    public boolean isTranscodePause() {
        return isTranscodePause;
    }

    public boolean isTranscoding() {
        return isTranscoding;
    }

    public void setCallBack(ICallBack callBack) {
        this.mCallBack = callBack;
    }

    public void setDataSource(String dataSource) {
        this.mDataSource = dataSource;
    }

    public void prepare() {
        if (TextUtils.isEmpty(mDataSource)) {
            throw new RuntimeException("dataSource is null");
        }
        nativePrepare(mNativePtr, mDataSource);
    }

    public void start() {
        if (TextUtils.isEmpty(mDataSource)) {
            throw new RuntimeException("dataSource is null");
        }
        nativeStart(mNativePtr);
    }

    public void destroy() {
        stopTranscode();
        if (mNativePtr > 0)
            nativeDestroy(mNativePtr);
        mNativePtr = 0;
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

    public void startTranscode(File saveDest, int type) {
        if (transcoder != null)
            throw new RuntimeException("please stopTranscode first");
        transcoder = TranscoderFactory.createPCMTranscoder(type, saveDest);
        transcoder.setCallBack(this);
        resumeTranscode();
        isTranscoding = true;
    }

    public void stopTranscode() {
        if (transcoder == null)
            return;
        nativePCMRecord(mNativePtr, false);
        transcoder.taskInQueue(TranscoderTask.createFinishTask());
        transcoder = null;
        isTranscoding = false;
        isTranscodePause = false;
    }

    @Override
    public void onTranscodeFinished() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onAudioTranscodeFinished();
                }
            }
        });
    }

    public void resumeTranscode() {
        nativePCMRecord(mNativePtr, true);
        isTranscodePause = false;
    }

    public void pauseTranscode() {
        nativePCMRecord(mNativePtr, false);
        isTranscodePause = true;
    }

    private void onNativeCallPrepared() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onAudioPrepared();
                }
            }
        });
    }

    private void onNativeCallDataOnLoad(final boolean onLoad) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onAudioWaitLoadData(onLoad);
                }
            }
        });
    }

    private void onNativeCallError(final int code, final String msg) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onAudioError(code, msg);
                }
            }
        });
    }

    public void onNativeCallDuration(final int current, final int all) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onAudioDurationChanged(current, all);
                }
            }
        });
    }

    public void onNativeCallFinished() {
        stopTranscode();
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null)
                    mCallBack.onAudioPlayFinished();
            }
        });
    }

    public void onNativeCallAmplitude(final int amplitude) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallBack != null) {
                    mCallBack.onAudioAmplitudeChanged(amplitude);
                }
            }
        });
    }

    public void onNativePCMRecord(final int sampleRate, int size, byte[] buff) {
        if (transcoder != null)
            transcoder.taskInQueue(TranscoderTask.createPCMBuffTask(sampleRate, buff, size));
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
