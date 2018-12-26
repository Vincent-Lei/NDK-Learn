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
#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_NOSYNC_THRESHOLD 10.0

#include "LibVideoSource.h"
#include "LibVideoPlayState.h"
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
    double delayTime = 0;
    double defaultVideoDelayTime = 0;

    int stream_index_audio = STREAM_NO_FOUND;
    int stream_index_video = STREAM_NO_FOUND;

    bool isCurrentResampleAudioFrameFinished = true;

    LibVideoFFMPEG(LibVideoSource *videoSource, LibVideoPlayState *playState);

    ~LibVideoFFMPEG();

    int prepared();

    int readFrame(AVPacket **avPacket);

    int sendAudioPacket(AVPacket *avPacket);

    int resampleAudioFrame(AVFrame *avFrame, uint8_t *resampleBuff);

    int resampleVideoFrame(AVPacket *avPacket,AVFrame **avFrame_yuv,uint8_t *buffer,long *videoDelayTime);

    void release();

    double getVideo2AudioFrameDiffTime(AVFrame *avFrame);

    double getFrameShouldDelayTimeByDiff(double diff);
};


#endif //NDK_LEARN_LIBVIDEOFFMPEG_H
