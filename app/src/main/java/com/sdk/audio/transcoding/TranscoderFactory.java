package com.sdk.audio.transcoding;

import java.io.File;

/**
 * Created by Vincent.Lei on 2018/12/12.
 * Title：
 * Note：
 */
public class TranscoderFactory {
    public static final int TYPE_AAC = 1;
    public static final int TYPE_MP3 = 2;
    public static final int TYPE_AMR = 3;


    public interface ITranscodeCallBack {
        void onTranscodeFinished();
    }

    public static BasePCM2Transcoder createPCMTranscoder(int type, File fileOutDest) {
        BasePCM2Transcoder transcoder = null;
        switch (type) {
            case TYPE_AAC:
                transcoder = new PCM2AacTranscoder(fileOutDest);
                break;
            case TYPE_AMR:
                transcoder = new PCM2AmrTranscoder(fileOutDest);
                break;
            case TYPE_MP3:
                transcoder = new PCM2Mp3Transcoder(fileOutDest);
                break;
        }
        return transcoder;
    }

    public static AudioFileTranscoder createAudioFileTranscoder(int type, File fileInput, File fileOutDest) {
        return new AudioFileTranscoder(fileInput, type, fileOutDest);
    }
}
