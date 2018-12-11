//
// Created by Android on 2018/12/10.
//

#include "LibAudioSource.h"

LibAudioSource::LibAudioSource() {
    queue = new LibAudioQueue();
}

LibAudioSource::~LibAudioSource() {
    if (queue) {
        delete queue;
        queue = NULL;
    }
}

void LibAudioSource::release() {
    if (dataSource != NULL) {
        delete dataSource;
        LOGD("free dataSource");
        dataSource = NULL;
    }
    if (queue)
        queue->release();
}
