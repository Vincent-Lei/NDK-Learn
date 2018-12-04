package com.lei.ndk.audio;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import com.lei.ndk.util.LogUtil;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by Vincent.Lei on 2018/12/3.
 * Title：
 * Note：
 */
public class PcmToAac {
    private MediaFormat mediaFormat;
    private MediaCodec mediaCodec;
    private MediaCodec.BufferInfo bufferInfo;
    private int aacSimpleRate;
    private FileOutputStream fosAAC;
    private File outfile;
    private boolean isInit;
    private boolean isDestory;

    public boolean isInit() {
        return isInit;
    }

    public void onDestory() {
        if (isDestory)
            return;
        isDestory = true;
        stop();
    }

    public boolean isDestory() {
        return isDestory;
    }

    public PcmToAac(File outfile) {
        this.outfile = outfile;
    }

    public void init(int sampleRate) {
        if (isDestory)
            return;
        isInit = true;
        try {
            aacSimpleRate = getADTSampleRate(sampleRate);
            mediaFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, 2);
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 96000);
            mediaFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 100 * 1024);
            mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            bufferInfo = new MediaCodec.BufferInfo();
            if (mediaCodec == null) {
                LogUtil.d("create encoder failed");
                return;
            }
            mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            fosAAC = new FileOutputStream(outfile);
            mediaCodec.start();
        } catch (Exception e) {
            e.printStackTrace();
            if (fosAAC != null) {
                try {
                    fosAAC.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }
        }
    }

    public void stop() {
        if (mediaCodec != null) {
            try {
//                mediaCodec.flush();
                mediaCodec.stop();
                mediaCodec.release();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        mediaFormat = null;
        mediaCodec = null;
        bufferInfo = null;
        if (fosAAC != null) {
            try {
                fosAAC.close();
            } catch (IOException e1) {
                e1.printStackTrace();
            }
        }
    }

    public void encodecPcmToAAc(int size, byte[] buffer) {
        if (isDestory)
            return;
        if (buffer != null && mediaCodec != null) {
            int inputBufferIndex = mediaCodec.dequeueInputBuffer(0);
            if (inputBufferIndex >= 0) {
                ByteBuffer byteBuffer = mediaCodec.getInputBuffers()[inputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, 0, 0);
            }

            int index;
            int perpcmSize;
            byte[] outByteBuffer;
            while ((index = mediaCodec.dequeueOutputBuffer(bufferInfo, 0)) >= 0) {
                try {
                    perpcmSize = bufferInfo.size + 7;
                    outByteBuffer = new byte[perpcmSize];
                    ByteBuffer byteBuffer = mediaCodec.getOutputBuffers()[index];
                    byteBuffer.position(bufferInfo.offset);
                    byteBuffer.limit(bufferInfo.offset + bufferInfo.size);
                    addADtsHeader(outByteBuffer, perpcmSize, aacSimpleRate);
                    byteBuffer.get(outByteBuffer, 7, bufferInfo.size);
//                    byteBuffer.position(bufferInfo.offset);
                    fosAAC.write(outByteBuffer, 0, perpcmSize);
                    mediaCodec.releaseOutputBuffer(index, false);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private void addADtsHeader(byte[] packet, int packetLen, int samplerate) {
        int profile = 2; // AAC LC
        int freqIdx = samplerate; // samplerate
        int chanCfg = 2; // CPE

        packet[0] = (byte) 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
        packet[1] = (byte) 0xF9; // 第一个t位放F
        packet[2] = (byte) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
        packet[3] = (byte) (((chanCfg & 3) << 6) + (packetLen >> 11));
        packet[4] = (byte) ((packetLen & 0x7FF) >> 3);
        packet[5] = (byte) (((packetLen & 7) << 5) + 0x1F);
        packet[6] = (byte) 0xFC;
    }

    private int getADTSampleRate(int sampleRate) {
        int rate = 4;
        switch (sampleRate) {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }


}
