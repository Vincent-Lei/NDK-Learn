package com.sdk.audio.transcoding;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;

import com.lei.ndk.util.LogUtil;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * Created by Vincent.Lei on 2018/12/12.
 * Title：
 * Note：
 */
public class PCM2AacTranscoder extends BasePCM2Transcoder {
    private boolean hasInit;
    private boolean isFinished;
    private MediaFormat mediaFormat;
    private MediaCodec mediaCodec;
    private MediaCodec.BufferInfo bufferInfo;
    private FileOutputStream fosOutDest;
    private int sampleRate;
    private int adtSimpleRateIndex;


    PCM2AacTranscoder(File fileOutDest) {
        super(fileOutDest);
    }

    @Override
    protected boolean init(int sampleRate) {
        if (hasInit)
            return false;
        synchronized (this) {
            if (hasInit)
                return false;
            hasInit = true;
            try {
                this.sampleRate = sampleRate;
                adtSimpleRateIndex = getADTSampleRate(sampleRate);
                mediaFormat = MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC, sampleRate, 2);
                mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, sampleRate);
                mediaFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
                mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 100 * 1024);
                mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
                bufferInfo = new MediaCodec.BufferInfo();
                if (mediaCodec == null) {
                    LogUtil.e("create encoder failed");
                }
                mediaCodec.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
                fosOutDest = new FileOutputStream(fileOutDest);
                mediaCodec.start();
            } catch (Exception e) {
                e.printStackTrace();
                if (fosOutDest != null) {
                    try {
                        fosOutDest.close();
                    } catch (IOException e1) {
                        e1.printStackTrace();
                    }
                }
            }
        }
        return true;
    }

    @Override
    protected void handlePCMBuffTranscode(TranscoderTask task) {
        init(task.getSampleRate());
        encodePcmData(task.getSize(), task.getBuffer());
    }

    @Override
    protected void handlePCMFileTranscode(TranscoderTask task) {
        File pcmFile = task.getPcmFile();
        if (pcmFile != null && pcmFile.exists()) {
            init(task.getSampleRate());
            FileInputStream fis = null;
            try {
                fis = new FileInputStream(pcmFile);
                byte[] buff = new byte[1024];
                int len;
                while ((len = fis.read(buff)) > 0)
                    encodePcmData(len, buff);
            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                if (task.isDeletePcmFile())
                    pcmFile.delete();
                if (fis != null) {
                    try {
                        fis.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

    }

    private void encodePcmData(int size, byte[] buffer) {
        if (isFinished)
            return;
        if (size > 0 && buffer != null && mediaCodec != null) {
            int inputBufferIndex = mediaCodec.dequeueInputBuffer(0);
            if (inputBufferIndex >= 0) {
                ByteBuffer byteBuffer = mediaCodec.getInputBuffers()[inputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, 0, 0);
            }
            int perpcmSize;
            byte[] outByteBuffer;
            int index;
            do {
                index = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
//                if (index < 0) {
//                    mediaCodec.flush();
//                    mediaCodec.start();
////                    index = mediaCodec.dequeueOutputBuffer(bufferInfo, 0);
//                }
                if (index >= 0) {
                    try {
                        perpcmSize = bufferInfo.size + 7;
                        outByteBuffer = new byte[perpcmSize];
                        ByteBuffer byteBuffer = mediaCodec.getOutputBuffers()[index];
                        byteBuffer.position(bufferInfo.offset);
                        byteBuffer.limit(bufferInfo.offset + bufferInfo.size);
                        addADtsHeader(outByteBuffer, perpcmSize, adtSimpleRateIndex);
                        byteBuffer.get(outByteBuffer, 7, bufferInfo.size);
                        fosOutDest.write(outByteBuffer, 0, perpcmSize);
                        mediaCodec.releaseOutputBuffer(index, false);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            } while (index >= 0);
        }
    }

    @Override
    protected void finish() {
        isFinished = true;
        if (mediaCodec != null) {
            try {
                mediaCodec.stop();
                mediaCodec.release();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        mediaFormat = null;
        mediaCodec = null;
        bufferInfo = null;
        if (fosOutDest != null) {
            try {
                fosOutDest.close();
            } catch (IOException e1) {
                e1.printStackTrace();
            }
        }
        onTranscodeFinished();
    }

    private void addADtsHeader(byte[] packet, int packetLen, int freqIdx) {
        int profile = 2; // AAC LC
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
