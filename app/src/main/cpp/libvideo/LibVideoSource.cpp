//
// Created by Android on 2018/12/24.
//

#include "LibVideoSource.h"

LibVideoSource::LibVideoSource() {
    queue = new LibVideoQueue();
}

LibVideoSource::~LibVideoSource() {
    if (queue)
        delete queue;
    queue = NULL;
}

void LibVideoSource::release() {
    if (dataSource != NULL) {
        delete dataSource;
        LOGD("free dataSource");
        dataSource = NULL;
    }
    if (queue)
        queue->release();

    if (avPacket_currentAudioResample) {
        av_packet_unref(avPacket_currentAudioResample);
        av_packet_free(&avPacket_currentAudioResample);
        av_free(avPacket_currentAudioResample);
        avPacket_currentAudioResample = NULL;
    }
}
