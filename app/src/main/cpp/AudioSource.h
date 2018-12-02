//
// Created by Android on 2018/11/22.
//

#ifndef NDK_LEARN_AUDIOSOURCE_H
#define NDK_LEARN_AUDIOSOURCE_H

#include <queue>
#include "jni.h"
#include "pthread.h"
#include "AudioPlayStatus.h"
#include "native-log.h"

#define AUDIO_STREAM_NO_FOUND -1
#define MAX_QUEUE_SIZE 80
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
};

class AudioSource {
public:
    const char *dataSource = NULL;
    int streamIndex = AUDIO_STREAM_NO_FOUND;
    pthread_t pthread_prepare;
    pthread_t pthread_decoder;
    pthread_t pthread_SLES;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    std::queue<AVPacket *> packerQueue;
    AudioPlayStatus *playStatus = NULL;
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodecParameters *codecpar;
    int duration = 0;
    AVRational time_base;
    double current_time = 0;
    double clock = 0;
    double last_clock = 0;


public:
    AudioSource(const char *dataSource, AudioPlayStatus *playStatus);

    ~AudioSource();

    void packetInQueue(AVPacket *avPacket);

    void noticeDecodeFinished();

    int packetPopQueue(AVPacket *costPacket);

    int getQueueSize();

    void release();

    void clearAVPackgetQueue();
};


#endif //NDK_LEARN_AUDIOSOURCE_H
