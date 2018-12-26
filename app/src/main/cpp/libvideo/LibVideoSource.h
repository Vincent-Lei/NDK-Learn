//
// Created by Android on 2018/12/24.
//

#ifndef NDK_LEARN_LIBVIDEOSOURCE_H
#define NDK_LEARN_LIBVIDEOSOURCE_H


#include "pthread.h"
#include "../globle/native-log.h"
#include "LibVideoQueue.h"

class LibVideoSource {
public:
    const char *dataSource = NULL;
    LibVideoQueue *queue = NULL;
    AVPacket *avPacket_currentAudioResample = NULL;
    pthread_t pthread_prepare;
    pthread_t pthread_decode;
    pthread_t pthread_audio_run;
    pthread_t pthread_vedio_run;

    LibVideoSource();

    ~LibVideoSource();

    void release();
};


#endif //NDK_LEARN_LIBVIDEOSOURCE_H
