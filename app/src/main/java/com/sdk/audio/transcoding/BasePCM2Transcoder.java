package com.sdk.audio.transcoding;

import android.os.Handler;
import android.os.HandlerThread;

import java.io.File;
import java.io.IOException;

/**
 * Created by Vincent.Lei on 2018/12/12.
 * Title：
 * Note：
 */
public abstract class BasePCM2Transcoder {
    private static HandlerThread workThread;
    private static Handler handler;

    static {
        workThread = new HandlerThread("PCMTranscodeThread");
        workThread.start();
        handler = new Handler(workThread.getLooper());
    }

    protected File fileOutDest;
    protected TranscoderFactory.ITranscodeCallBack callBack;

    public BasePCM2Transcoder(File fileOutDest) {
        this.fileOutDest = fileOutDest;
    }

    public void setCallBack(TranscoderFactory.ITranscodeCallBack callBack) {
        this.callBack = callBack;
    }

    public void taskInQueue(TranscoderTask task) {
        task.setTranscoder(this);
        handler.post(task);
    }

    protected abstract boolean init(int sampleRate);

    protected abstract void finish();

    final void handlePcmData(TranscoderTask task) {
        if (task.getFlag() == TranscoderTask.FLAG_FINISH)
            finish();
        else if (task.getFlag() == TranscoderTask.FLAG_ENCODE_PCM_BYTE_BUFF)
            handlePCMBuffTranscode(task);
        else if (task.getFlag() == TranscoderTask.FLAG_ENCODE_PCM_FILE)
            handlePCMFileTranscode(task);
    }


    protected void handlePCMBuffTranscode(TranscoderTask task) {
    }

    protected void handlePCMFileTranscode(TranscoderTask task) {
    }

    protected File createFileByOutDestFile(String suffix) {
        File file = new File(fileOutDest.getParent() + File.separator + System.currentTimeMillis() + suffix);
        if (!file.exists()) {
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return file;
    }

    protected void onTranscodeFinished() {
        if (callBack != null)
            callBack.onTranscodeFinished();
    }
}
