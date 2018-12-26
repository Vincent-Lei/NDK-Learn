//
// Created by Android on 2018/12/24.
//

#ifndef NDK_LEARN_LIBVIDEOQUEUE_H
#define NDK_LEARN_LIBVIDEOQUEUE_H
#define MAX_VICEO_QUEUE_SIZE 60
extern "C" {
#include "libavcodec/avcodec.h"
};
#include <native-log.h>
#include "pthread.h"
#include "queue"

class LibVideoQueue {
public:
    std::queue<AVPacket *> audioPackageQueue;
    std::queue<AVPacket *> videoPackageQueue;
    pthread_mutex_t mutex_audio;
    pthread_cond_t cond_audio;
    pthread_mutex_t mutex_video;
    pthread_cond_t cond_video;
    bool isDecodeFinished = false;

    LibVideoQueue();

    ~LibVideoQueue();

    void audioInQueue(AVPacket *avPacket);

    void videoInQueue(AVPacket *avPacket);

    int getAudioQueueSize();

    int getVideoQueueSize();

    void emptyQueue();

    int audioPopQueue(AVPacket *costPacket, void (*onQueueWaitData)(void *data), void *data);

    int videoPopQueue(AVPacket **costPacket, void (*onQueueWaitData)(void *data), void *data);

    void notifyQueuePushFinished();

    void release();
};


#endif //NDK_LEARN_LIBVIDEOQUEUE_H
