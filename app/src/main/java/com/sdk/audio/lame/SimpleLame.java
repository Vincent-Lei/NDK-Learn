package com.sdk.audio.lame;

/**
 * Created by Vincent.Lei on 2018/12/13.
 * Title：
 * Note：
 */
public class SimpleLame {
    private long mNativePtr;

    public SimpleLame() {
        this.mNativePtr = native_create();
    }

    public static int pcmToMp3(int inSampleRate, int outChannel, int outSampleRate, int outBitrate, int quality, String fileSource, String fileDest) {
        return native_pcmToMp3(inSampleRate, outChannel, outSampleRate, outBitrate, quality, fileSource, fileDest);
    }


    private native long native_create();

    private native static int native_pcmToMp3(int inSampleRate, int outChannel, int outSampleRate, int outBitrate, int quality, String fileSource, String fileDest);

    private native void native_init(long ptr, int inSampleRate, int outChannel, int outSampleRate, int outBitrate, int quality);

    private native int native_flush(long ptr, byte[] mp3buf);

    private native void native_close(long ptr);

    private native int native_encode(long ptr, short[] buffer_l, short[] buffer_r, int samples, byte[] mp3buf);


}
