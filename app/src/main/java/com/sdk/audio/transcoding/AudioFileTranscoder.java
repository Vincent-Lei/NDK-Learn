package com.sdk.audio.transcoding;

import com.lei.ndk.util.LogUtil;

import java.io.File;

/**
 * Created by Vincent.Lei on 2018/12/14.
 * Title：
 * Note：
 */
public class AudioFileTranscoder extends BasePCM2Transcoder {
    private File fileInput;
    private int transcodeToType;
    private long mNativePtr;
    private boolean isCallBackToNextTranscoder;

    public static class AudioParam {
        int sampleRate;
    }

    AudioFileTranscoder(File fileInput, int transcodeToType, File fileOutDest) {
        super(fileOutDest);
        this.fileInput = fileInput;
        this.transcodeToType = transcodeToType;
        if (fileInput == null || !fileInput.exists())
            throw new IllegalArgumentException("fileInput is not exists");
    }

    @Override
    protected boolean init(int sampleRate) {
        return false;
    }

    @Override
    public void taskInQueue(TranscoderTask task) {
        //ignore-
    }

    @Override
    protected void finish() {
        destroy();
        if (!isCallBackToNextTranscoder)
            onTranscodeFinished();
    }

    @Override
    protected void handlePCMFileTranscode(TranscoderTask task) {
        File filePcmTemp = createFileByOutDestFile(".pcm");
        mNativePtr = nativeInit();
        AudioParam audioParam = nativeCreatePcm(mNativePtr, fileInput.getAbsolutePath(), filePcmTemp.getAbsolutePath());
        destroy();
        if (audioParam != null) {
            LogUtil.d("pcm create success sampleRate = " + audioParam.sampleRate);
            isCallBackToNextTranscoder = true;
            BasePCM2Transcoder transcoder = TranscoderFactory.createPCMTranscoder(transcodeToType, fileOutDest);
            transcoder.setCallBack(callBack);
            transcoder.taskInQueue(TranscoderTask.createPCMFileTask(audioParam.sampleRate, filePcmTemp, true));
            transcoder.taskInQueue(TranscoderTask.createFinishTask());
        }
    }

    public void start() {
        isCallBackToNextTranscoder = false;
        super.taskInQueue(TranscoderTask.createPCMFileTask(TranscoderTask.FLAG_ENCODE_PCM_FILE, null, true));
        super.taskInQueue(TranscoderTask.createFinishTask());
    }

    public void destroy() {
        synchronized (this) {
            if (mNativePtr != 0)
                nativeDestory(mNativePtr);
            mNativePtr = 0;
        }
    }

    private native long nativeInit();

    private native AudioParam nativeCreatePcm(long ptr, String fileInputPath, String fileOutPath);

    private native void nativeDestory(long ptr);

}
