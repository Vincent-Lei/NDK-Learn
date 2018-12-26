//
// Created by Android on 2018/12/24.
//

#include "LibVideoQueue.h"

LibVideoQueue::LibVideoQueue() {
    pthread_mutex_init(&mutex_audio, NULL);
    pthread_mutex_init(&mutex_video, NULL);
    pthread_cond_init(&cond_audio, NULL);
    pthread_cond_init(&cond_video, NULL);
}

LibVideoQueue::~LibVideoQueue() {
    pthread_mutex_destroy(&mutex_audio);
    pthread_mutex_destroy(&mutex_video);
    pthread_cond_destroy(&cond_audio);
    pthread_cond_destroy(&cond_video);
}

void LibVideoQueue::audioInQueue(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex_audio);
    audioPackageQueue.push(avPacket);
    pthread_cond_signal(&cond_audio);
    pthread_mutex_unlock(&mutex_audio);
}

void LibVideoQueue::videoInQueue(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex_video);
    videoPackageQueue.push(avPacket);
    pthread_cond_signal(&cond_video);
    pthread_mutex_unlock(&mutex_video);
}

int LibVideoQueue::getAudioQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutex_audio);
    size = audioPackageQueue.size();
    pthread_mutex_unlock(&mutex_audio);
    return size;
}

int LibVideoQueue::getVideoQueueSize() {
    int size = 0;
    pthread_mutex_lock(&mutex_video);
    size = videoPackageQueue.size();
    pthread_mutex_unlock(&mutex_video);
    return size;
}

void LibVideoQueue::emptyQueue() {
    AVPacket *avPacket = NULL;
    pthread_mutex_lock(&mutex_audio);
    while (!audioPackageQueue.empty()) {
        avPacket = audioPackageQueue.front();
        audioPackageQueue.pop();
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_mutex_unlock(&mutex_audio);

    pthread_mutex_lock(&mutex_video);
    while (!videoPackageQueue.empty()) {
        avPacket = videoPackageQueue.front();
        videoPackageQueue.pop();
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
    }
    pthread_mutex_unlock(&mutex_video);
    LOGD("--emptyQueue--");
}

int
LibVideoQueue::audioPopQueue(AVPacket *costPacket, void (*onQueueWaitData)(void *), void *data) {
    /**
       *  pthread_cond_wait前要先加锁
       pthread_cond_wait内部会解锁，然后等待条件变量被其它线程激活
       pthread_cond_wait被激活后会再自动加锁
       */
    int result;
    pthread_mutex_lock(&mutex_audio);
    while (true) {
        result = -1;
        if (audioPackageQueue.size() > 0) {
            AVPacket *avPacket = audioPackageQueue.front();
            if (av_packet_ref(costPacket, avPacket) == 0) {
                result = 0;
                audioPackageQueue.pop();
            }
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&mutex_audio);
            break;
        } else if (!isDecodeFinished) {
            if (onQueueWaitData)
                onQueueWaitData(data);
            pthread_cond_wait(&cond_audio, &mutex_audio);
            continue;
        } else {
            pthread_mutex_unlock(&mutex_audio);
            break;
        }
    }
    return result;
}

int
LibVideoQueue::videoPopQueue(AVPacket **costPacket, void (*onQueueWaitData)(void *), void *data) {
    int result;
    pthread_mutex_lock(&mutex_video);
    *costPacket = NULL;
    while (true) {
        result = -1;
        if (videoPackageQueue.size() > 0) {
//            AVPacket *avPacket = videoPackageQueue.front();
//            if (av_packet_ref(costPacket, avPacket) == 0) {
//                result = 0;
//            videoPackageQueue.pop();
//            }
//            av_packet_free(&avPacket);
//            av_free(avPacket);
//            avPacket = NULL;
            result = 0;
            *costPacket = videoPackageQueue.front();
            videoPackageQueue.pop();
            pthread_mutex_unlock(&mutex_video);
            break;
        } else if (!isDecodeFinished) {
            if (onQueueWaitData)
                onQueueWaitData(data);
            pthread_cond_wait(&cond_video, &mutex_video);
            continue;
        } else {
            pthread_mutex_unlock(&mutex_video);
            break;
        }
    }
    return result;
}

void LibVideoQueue::notifyQueuePushFinished() {
    isDecodeFinished = true;
    pthread_cond_signal(&cond_audio);
    pthread_cond_signal(&cond_video);
}

void LibVideoQueue::release() {
    isDecodeFinished = true;
    emptyQueue();
    isDecodeFinished = false;
}
