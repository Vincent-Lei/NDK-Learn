//
// Created by Android on 2018/12/24.
//

#ifndef NDK_LEARN_LIBVIDEOFFMPEG_H
#define NDK_LEARN_LIBVIDEOFFMPEG_H

#define STREAM_NO_FOUND -1
#define V_READ_FRAME_FINISHED -2
#define V_READ_FRAME_NONE -3
#define V_READ_AUDIO_STREAM 1
#define V_READ_VIDEO_STREAM 2
#define SELECTED_CODEC_TYPE_SOFTEARE 1
#define SELECTED_CODEC_TYPE_HARDEARE 2


#include "LibVideoSource.h"
#include "LibVideoPlayState.h"
#include "LibVideoJavaCallBack.h"
#include <unistd.h>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"

};

class LibVideoFFMPEG {
public:
    LibVideoSource *videoSource = NULL;
    LibVideoPlayState *playState = NULL;
    LibVideoJavaCallBack *javaCallBack = NULL;
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pAudioCodecCtx = NULL;
    AVCodecParameters *audioCodecpar = NULL;
    AVCodecContext *pVideoCodecCtx = NULL;
    AVCodecParameters *videoCodecpar = NULL;
    int audioDuration = 0;
    AVRational audioTimeBase;
    int videoDuration = 0;
    AVRational videoTimeBase;

    double audioClock = 0;
    double videoClock = 0;
    double lastVideoClock = 0;
    double delayTime = 0;
    double defaultVideoDelayTime = 0;

    int stream_index_audio = STREAM_NO_FOUND;
    int stream_index_video = STREAM_NO_FOUND;

    bool isCurrentResampleAudioFrameFinished = true;

    int selectedCodecType = SELECTED_CODEC_TYPE_SOFTEARE;
    const AVBitStreamFilter *bsFilter = NULL;
    AVBSFContext *pAvbsfContext = NULL;

    LibVideoFFMPEG(LibVideoSource *videoSource, LibVideoPlayState *playState,
                   LibVideoJavaCallBack *javaCallBack);

    ~LibVideoFFMPEG();

    int prepared();

    int readFrame(AVPacket **avPacket);

    int sendAudioPacket(AVPacket *avPacket);

    int sendBsfPacket(AVPacket *avPacket);

    int receiveBsfPacket(AVPacket *avPacket);

    int resampleAudioFrame(AVFrame *avFrame, uint8_t *resampleBuff);

    int resampleVideoFrame(AVPacket *avPacket, AVFrame **avFrame_yuv, uint8_t *buffer,
                           unsigned int *videoDelayTime);
    void resetForSeek();

    int initHardwareCodec();

    void release();

    double getVideo2AudioFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket);

    double getFrameShouldDelayTimeByDiff(double diff);
};


#endif //NDK_LEARN_LIBVIDEOFFMPEG_H
