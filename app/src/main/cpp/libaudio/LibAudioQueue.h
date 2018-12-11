//
// Created by Android on 2018/12/10.
//

#ifndef NDK_LEARN_LIBAUDIOQUEUE_H
#define NDK_LEARN_LIBAUDIOQUEUE_H

#include "queue"
#include "pthread.h"
#include "../globle/native-log.h"

extern "C" {
#include "libavcodec/avcodec.h"
};
#define MAX_QUEUE_SIZE 60

class LibAudioQueue {
public:
    std::queue<AVPacket *> packageQueue;
    pthread_mutex_t mutex_queue;
    pthread_cond_t cond_queue;
    bool isDecodeFinished = false;

    LibAudioQueue();

    ~ LibAudioQueue();

    int getQueueSize();

    void notifyQueueWaitter();

    void notifyQueuePushFinished();

    void inQueue(AVPacket *avPacket);

    int popQueue(AVPacket *avPacket, void (*onQueueWaitData)(void *data), void *data);

    void release();

    void emptyQueue();
};


#endif //NDK_LEARN_LIBAUDIOQUEUE_H
