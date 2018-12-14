package com.sdk.audio.transcoding;

import com.lei.ndk.util.LogUtil;
import com.sdk.audio.amrencoder.AmrEncoder;
import com.sdk.audio.amrencoder.SSRC;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by Vincent.Lei on 2018/12/12.
 * Title：
 * Note：
 */
public class PCM2AmrTranscoder extends BasePCM2Transcoder {
    private boolean hasInit;
    private boolean isFinished;
    private FileOutputStream fosPCM;
    private File filePcmTemp;
    private boolean isByteBuffMode;
    private int sampleRate;

    PCM2AmrTranscoder(File fileOutDest) {
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
                if (isByteBuffMode) {
                    filePcmTemp = createFileByOutDestFile(".pcm");
                    fosPCM = new FileOutputStream(filePcmTemp);
                }

            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return true;
    }


    @Override
    protected void handlePCMBuffTranscode(TranscoderTask task) {
        isByteBuffMode = true;
        init(task.getSampleRate());
        writePCM(task.getSize(), task.getBuffer());
    }

    @Override
    protected void handlePCMFileTranscode(TranscoderTask task) {
        isByteBuffMode = false;
        File pcmFile = task.getPcmFile();
        if (pcmFile.exists()) {
            init(task.getSampleRate());
            realHandlePCMFileTranscode(ssrc(pcmFile), task.isDeletePcmFile());
        }
    }

    private void writePCM(int size, byte[] buffer) {
        if (isFinished)
            return;
        if (size > 0 && buffer != null) {
            try {
                fosPCM.write(buffer, 0, size);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void finishWritePCM() {
        try {
            if (fosPCM != null) {
                fosPCM.flush();
                fosPCM.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private File ssrc(File filePcmTemp) {
        File filePCMLowRate = createFileByOutDestFile(".pcm");
        FileInputStream fileInputStream = null;
        FileOutputStream fileOutputStream = null;
        try {
            fileInputStream = new FileInputStream(filePcmTemp);
            fileOutputStream = new FileOutputStream(filePCMLowRate);
            new SSRC(fileInputStream, fileOutputStream, sampleRate, 8000, 2, 2, 1, Integer.MAX_VALUE, 0, 0, true);
        } catch (Exception e) {
            if (filePCMLowRate.exists())
                filePCMLowRate.delete();
            filePCMLowRate = null;
            e.printStackTrace();
        } finally {
            if (fileInputStream != null) {
                try {
                    fileInputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            if (fileOutputStream != null) {
                try {
                    fileOutputStream.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return filePCMLowRate;
    }

    @Override
    protected void finish() {
        if (isFinished)
            return;
        isFinished = true;
        if (isByteBuffMode) {
            finishWritePCM();
            File filePCMLowRate = null;
            if (filePcmTemp.exists()) {
                if (filePcmTemp.length() > 0)
                    filePCMLowRate = ssrc(filePcmTemp);
                filePcmTemp.delete();
            }
            realHandlePCMFileTranscode(filePCMLowRate, true);
        }
        onTranscodeFinished();
    }

    private void realHandlePCMFileTranscode(File pcmFile, boolean deletePcmFile) {
        if (pcmFile != null && pcmFile.exists()) {
            if (!fileOutDest.exists()) {
                try {
                    fileOutDest.createNewFile();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            LogUtil.d("start to transcode amr file");
            AmrEncoder.pcm2Amr(pcmFile.getAbsolutePath(), fileOutDest.getAbsolutePath());
            if (deletePcmFile)
                pcmFile.delete();
            LogUtil.d("start to transcode amr file");
        }
    }
}
