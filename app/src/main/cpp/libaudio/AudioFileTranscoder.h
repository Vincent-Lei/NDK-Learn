//
// Created by Android on 2018/12/14.
//

#ifndef NDK_LEARN_AUDIOFILETRANSCODER_H
#define NDK_LEARN_AUDIOFILETRANSCODER_H


#include <jni.h>
#include "native-log.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};

class AudioFileTranscoder {
public:
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    AVCodecParameters *codecpar = NULL;
    bool isJobFinished = true;
    bool isCalledRelease = false;
    bool isCurrentResampleFrameFinished = true;
    uint8_t *resampleBuff = NULL;
    int streamIndex = -1;
    JNIEnv *env;

    AudioFileTranscoder(JNIEnv *env);

    ~AudioFileTranscoder();

    int readFrame(AVPacket **avPacket);

    int resampleFrame(AVFrame *avFrame, uint8_t *resampleBuff);

    int prepare(const char *fileInputPath);

    void release();

    jobject createPcm(const char *fileInputPath, const char *fileOutPath);
};


#endif //NDK_LEARN_AUDIOFILETRANSCODER_H
