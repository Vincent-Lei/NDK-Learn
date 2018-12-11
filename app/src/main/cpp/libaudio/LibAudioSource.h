//
// Created by Android on 2018/12/10.
//

#ifndef NDK_LEARN_LIBAUDIOSOURCE_H
#define NDK_LEARN_LIBAUDIOSOURCE_H

#include "pthread.h"
#include "LibAudioQueue.h"
#include "../globle/native-log.h"

class LibAudioSource {
public:
    pthread_t thread_prepare;
    pthread_t thread_decode;
    pthread_t thread_start;

    const char *dataSource = NULL;
    LibAudioQueue *queue = NULL;
    AVPacket *avPacket_currentResample = NULL;

    LibAudioSource();

    ~LibAudioSource();

    void emptyQueue();

    void release();
};


#endif //NDK_LEARN_LIBAUDIOSOURCE_H
