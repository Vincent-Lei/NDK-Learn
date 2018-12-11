//
// Created by Android on 2018/12/10.
//

#ifndef NDK_LEARN_LIBAUDIOFFMPEG_H
#define NDK_LEARN_LIBAUDIOFFMPEG_H


#include "LibAudioSource.h"
#include "LibAudioPlayState.h"
#include "../globle/native-log.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};

#define AUDIO_STREAM_NO_FOUND -1
#define READ_FRAME_OK 0
#define READ_FRAME_NOT_AUDIO 10086
#define READ_FRAME_FINISHED -1

class LibAudioFFMPEG {
public:
    LibAudioSource *audioSource = NULL;
    LibAudioPlayState *playState = NULL;
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    AVCodecParameters *codecpar = NULL;

    int streamIndex = AUDIO_STREAM_NO_FOUND;
    int duration = 0;
    AVRational time_base;
    double current_clock = 0;
    double clock = 0;
    double last_clock = 0;

    int data_nb_samples = 0;
    bool isCurrentResampleFrameFinished = true;

    LibAudioFFMPEG(LibAudioSource *audioSource, LibAudioPlayState *playState);

    ~LibAudioFFMPEG();

    int prepare();

    int readFrame(AVPacket **avPacket);

    int sendPacket(AVPacket *avPacket);

    int resampleFrame(AVFrame *avFrame, uint8_t *resampleBuff);

    void release();
};


#endif //NDK_LEARN_LIBAUDIOFFMPEG_H
