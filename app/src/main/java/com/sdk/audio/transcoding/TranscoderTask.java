package com.sdk.audio.transcoding;

import java.io.File;

/**
 * Created by Vincent.Lei on 2018/12/12.
 * Title：
 * Note：
 */
public class TranscoderTask implements Runnable {

    public static final int FLAG_ENCODE_PCM_BYTE_BUFF = 0;
    public static final int FLAG_FINISH = 1;
    public static final int FLAG_ENCODE_PCM_FILE = 2;

    private int flag;
    private byte[] buffer;
    private int size;
    private int sampleRate;
    private File pcmFile;
    private boolean isDeletePcmFile;
    private BasePCM2Transcoder transcoder;

    private TranscoderTask(int sampleRate, File pcmFile, boolean isDeletePcmFile) {
        this.flag = FLAG_ENCODE_PCM_FILE;
        this.sampleRate = sampleRate;
        this.pcmFile = pcmFile;
        this.isDeletePcmFile = isDeletePcmFile;
    }

    private TranscoderTask() {
        this.flag = FLAG_FINISH;
    }

    private TranscoderTask(int sampleRate, byte[] buffer, int size) {
        this.flag = FLAG_ENCODE_PCM_BYTE_BUFF;
        this.buffer = buffer;
        this.size = size;
        this.sampleRate = sampleRate;
    }

    public static TranscoderTask createFinishTask() {
        return new TranscoderTask();
    }

    public static TranscoderTask createPCMFileTask(int sampleRate, File pcmFile, boolean isDeletePcmFile) {
        return new TranscoderTask(sampleRate, pcmFile, isDeletePcmFile);
    }

    public static TranscoderTask createPCMBuffTask(int sampleRate, byte[] buffer, int size) {
        return new TranscoderTask(sampleRate, buffer, size);
    }

    public int getFlag() {
        return flag;
    }

    public byte[] getBuffer() {
        return buffer;
    }

    public int getSize() {
        return size;
    }

    public File getPcmFile() {
        return pcmFile;
    }

    public int getSampleRate() {
        return sampleRate;
    }

    public boolean isDeletePcmFile() {
        return isDeletePcmFile;
    }

    void setTranscoder(BasePCM2Transcoder transcoder) {
        this.transcoder = transcoder;
    }

    @Override
    public void run() {
        if (transcoder == null)
            return;
        transcoder.handlePcmData(this);
    }
}
