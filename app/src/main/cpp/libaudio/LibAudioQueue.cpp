//
// Created by Android on 2018/12/10.
//

#include <unistd.h>
#include "LibAudioQueue.h"

LibAudioQueue::LibAudioQueue() {
    pthread_mutex_init(&mutex_queue, NULL);
    pthread_cond_init(&cond_queue, NULL);
}

LibAudioQueue::~LibAudioQueue() {
    pthread_mutex_destroy(&mutex_queue);
    pthread_cond_destroy(&cond_queue);
}

int LibAudioQueue::getQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutex_queue);
    size = packageQueue.size();
    pthread_mutex_unlock(&mutex_queue);
    return size;
}

void LibAudioQueue::notifyQueueWaitter() {
    pthread_cond_signal(&cond_queue);
}

void LibAudioQueue::notifyQueuePushFinished() {
    isDecodeFinished = true;
    pthread_cond_signal(&cond_queue);
}

void LibAudioQueue::inQueue(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex_queue);
    packageQueue.push(avPacket);
    pthread_cond_signal(&cond_queue);
    pthread_mutex_unlock(&mutex_queue);

}

int LibAudioQueue::popQueue(AVPacket *costPacket, void (*onQueueWaitData)(void *data), void *data) {
    /**
     *  pthread_cond_wait前要先加锁
     pthread_cond_wait内部会解锁，然后等待条件变量被其它线程激活
     pthread_cond_wait被激活后会再自动加锁
     */
    int result;
    pthread_mutex_lock(&mutex_queue);
    while (true) {
        result = -1;
        if (packageQueue.size() > 0) {
            AVPacket *avPacket = packageQueue.front();
            if (av_packet_ref(costPacket, avPacket) == 0) {
                result = 0;
                packageQueue.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&mutex_queue);
            break;
        } else if (!isDecodeFinished) {
            if (onQueueWaitData != NULL)
                onQueueWaitData(data);
            pthread_cond_wait(&cond_queue, &mutex_queue);
            continue;
        } else {
            pthread_mutex_unlock(&mutex_queue);
            break;
        }
    }
    return result;
}

void LibAudioQueue::release() {
    isDecodeFinished = false;
    emptyQueue();
}


void LibAudioQueue::emptyQueue() {
    pthread_mutex_lock(&mutex_queue);
    AVPacket *avPacket = NULL;
    while (!packageQueue.empty()) {
        avPacket = packageQueue.front();
        packageQueue.pop();
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_mutex_unlock(&mutex_queue);
    LOGD("--emptyQueue--")
}

