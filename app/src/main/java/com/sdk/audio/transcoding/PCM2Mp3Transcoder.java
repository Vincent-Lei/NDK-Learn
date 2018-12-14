package com.sdk.audio.transcoding;

import com.lei.ndk.util.LogUtil;
import com.sdk.audio.lame.SimpleLame;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by Vincent.Lei on 2018/12/12.
 * Title：
 * Note：
 */
public class PCM2Mp3Transcoder extends BasePCM2Transcoder {
    private boolean hasInit;
    private boolean isFinished;
    private FileOutputStream fosPCM;
    private File filePcmTemp;
    private int sampleRate;
    private boolean isByteBuffMode;

    public PCM2Mp3Transcoder(File fileOutDest) {
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
            this.sampleRate = sampleRate;
            if (isByteBuffMode) {
                filePcmTemp = createFileByOutDestFile(".pcm");
                try {
                    fosPCM = new FileOutputStream(filePcmTemp);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
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
            realHandlePCMFileTranscode(pcmFile, task.isDeletePcmFile());
        }
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
            LogUtil.d("start to transcode mp3 file");
            SimpleLame.pcmToMp3(sampleRate, 2, sampleRate, 192, 9, pcmFile.getAbsolutePath(), fileOutDest.getAbsolutePath());
            if (deletePcmFile)
                pcmFile.delete();
            LogUtil.d("end transcode mp3 file");
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

    @Override
    protected void finish() {
        if (isFinished)
            return;
        isFinished = true;
        if (isByteBuffMode) {
            finishWritePCM();
            realHandlePCMFileTranscode(filePcmTemp, true);
        }
        onTranscodeFinished();
    }
}
