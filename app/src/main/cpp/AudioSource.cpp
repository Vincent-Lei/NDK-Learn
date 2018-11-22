//
// Created by Android on 2018/11/22.
//

#include "AudioSource.h"

AudioSource::AudioSource(const char *dataSource, AudioPlayStatus *playStatus) {
    this->dataSource = dataSource;
    this->playStatus = playStatus;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}

AudioSource::~AudioSource() {

}

void AudioSource::packetInQueue(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);
    if (avPacket) {
        packerQueue.push(avPacket);
//        LOGD("缓存一个AVpacket，当前队列剩下 %d 个", packerQueue.size());
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

int AudioSource::packetPopQueue(AVPacket *costPacket) {
    int result = -1;
    pthread_mutex_lock(&mutex);
    while (playStatus != NULL && !playStatus->isExist) {
        if (packerQueue.size() > 0) {
            AVPacket *avPacket = packerQueue.front();
            if (av_packet_ref(costPacket, avPacket) == 0) {
                result = 0;
                packerQueue.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            LOGD("从队列里面取出一个AVpacket，还剩下 %d 个", packerQueue.size());
            break;
        } else {
            if (playStatus->isDecodeFinished) {
                playStatus->isCostFinished = true;
                break;
            }
            pthread_cond_wait(&cond, &mutex);
        }
    }
    pthread_mutex_unlock(&mutex);
    return result;
}

void AudioSource::destory() {
    if (pFormatCtx != NULL) {
        avformat_free_context(pFormatCtx);
        LOGD("free pFormatCtx")
    }
    if (pCodecCtx != NULL) {
        avcodec_free_context(&pCodecCtx);
        LOGD("free pCodecCtx")
    }
    if (dataSource != NULL) {
        delete dataSource;
        LOGD("free dataSource")
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
