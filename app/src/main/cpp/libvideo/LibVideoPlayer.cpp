//
// Created by Android on 2018/12/24.
//

#include "LibVideoPlayer.h"

LibVideoPlayer::LibVideoPlayer(JNIEnv *env, jobject *object) {
    videoSource = new LibVideoSource();
    playState = new LibVideoPlayState();
    javaCallBack = new LibVideoJavaCallBack(env, object);
    FFMPEG = new LibVideoFFMPEG(videoSource, playState);
    pthread_mutex_init(&mutex_seek, NULL);
}

LibVideoPlayer::~LibVideoPlayer() {
    pthread_mutex_destroy(&mutex_seek);
}

void pcmAudioBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    LibVideoPlayer *player = (LibVideoPlayer *) (context);
    int bufferSize = player->resampleAudioPacket();
    if (bufferSize > 0) {
//        player->FFMPEG->audioClock +=
//                bufferSize / ((double) (player->FFMPEG->audioCodecpar->sample_rate * 2 * 2));
        (*player->openSLES->pcmBufferQueue)->Enqueue(player->openSLES->pcmBufferQueue,
                                                     player->audioResampleBuff,
                                                     bufferSize);
    }

}

void *thread_prepare(void *data) {
    LibVideoPlayer *player = (LibVideoPlayer *) (data);
    if (player->FFMPEG->prepared() != 0)
        player->playState->isErrorInPrepared = true;
    player->playState->isPrepared = true;
    if (player->playState->isNeedStartedAfterPrepared)
        player->start();
    LOGD("thread_prepare finished")
    return 0;
}

void *thread_decode(void *data) {
    LibVideoPlayer *player = (LibVideoPlayer *) (data);
    LibVideoQueue *queue = player->videoSource->queue;
    AVPacket *avPacket = NULL;
    while (!player->playState->isExit) {
        if (player->playState->isSeek) {
            usleep(20 * 1000);
            continue;
        }
        if (queue->getAudioQueueSize() > MAX_VICEO_QUEUE_SIZE) {
            usleep(50 * 1000);
            continue;
        }
        pthread_mutex_lock(&player->mutex_seek);
        int ret = player->FFMPEG->readFrame(&avPacket);
        pthread_mutex_unlock(&player->mutex_seek);
        if (ret == V_READ_FRAME_FINISHED) {
            //解码结束
            break;
        }
        if (ret == V_READ_FRAME_NONE)
            continue;
        if (ret == V_READ_AUDIO_STREAM)
            queue->audioInQueue(avPacket);
        else if (ret == V_READ_VIDEO_STREAM)
            queue->videoInQueue(avPacket);
    }
    queue->notifyQueuePushFinished();
    LOGD("thread_decode finished")
    return 0;
}

void *thread_audio_run(void *data) {
    LibVideoPlayer *player = (LibVideoPlayer *) (data);
    if (player->FFMPEG->stream_index_audio == STREAM_NO_FOUND)
        return 0;
    if (player->openSLES == NULL) {
        player->openSLES = new LibAudioOpenSLES();
    }
    if (player->openSLES) {
        player->openSLES->prepare(player->FFMPEG->audioCodecpar->sample_rate,
                                  pcmAudioBufferCallBack,
                                  player);
        player->openSLES->play();
    }
    LOGD("audio_start_thread finished")
    return 0;
}

void onQueueWaitVideoData(void *data) {
    LibVideoPlayer *player = (LibVideoPlayer *) (data);
    if (!player->playState->isOnWaitingData) {
        player->playState->isOnWaitingData = true;
//        player->javaCallBack->callJavaDataOnLoad(CHILD_THREAD_CALL, true);
    }
}

void *thread_video_run(void *data) {
    LibVideoPlayer *player = (LibVideoPlayer *) (data);
    LibVideoQueue *queue = player->videoSource->queue;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame_yuv420P = NULL;
    long *videoDelayTime = new long;
    *videoDelayTime = 0;
    int num = av_image_get_buffer_size(
            AV_PIX_FMT_YUV420P,
            player->FFMPEG->pVideoCodecCtx->width,
            player->FFMPEG->pVideoCodecCtx->height,
            1);
    uint8_t *buffer = (uint8_t *) (av_malloc(num * sizeof(uint8_t)));
    while (!player->playState->isExit) {
        if (player->playState->isSeek) {
            usleep(20 * 1000);
            continue;
        }
        if (queue->videoPopQueue(&avPacket, onQueueWaitVideoData, player) != 0) {
            //视频结束
            LOGD("--视频结束--")
            break;
        }
        if (player->playState->isOnWaitingData) {
            player->playState->isOnWaitingData = false;
//            javaCallBack->callJavaDataOnLoad(CHILD_THREAD_CALL, false);
        }
        avFrame_yuv420P = NULL;
        int ret = player->FFMPEG->resampleVideoFrame(avPacket, &avFrame_yuv420P, buffer,
                                                     videoDelayTime);
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket = NULL;
        if (ret == 0) {
            //渲染
//            player->javaCallBack->onCallRenderYUV(
//                    player->FFMPEG->pVideoCodecCtx->width,
//                    player->FFMPEG->pVideoCodecCtx->height,
//                    avFrame_yuv420P->data[0],
//                    avFrame_yuv420P->data[1],
//                    avFrame_yuv420P->data[2]);

            av_frame_free(&avFrame_yuv420P);
            av_free(avFrame_yuv420P);
            avFrame_yuv420P = NULL;
        }

    }

    delete (videoDelayTime);
    delete (buffer);
    LOGD("thread_video_run finished")
    return 0;

}


void LibVideoPlayer::resetForPrepare() {
    playState->isExit = true;
    if (videoSource) {
        pthread_join(videoSource->pthread_prepare, NULL);
        pthread_join(videoSource->pthread_decode, NULL);
        pthread_join(videoSource->pthread_audio_run, NULL);
        pthread_join(videoSource->pthread_vedio_run, NULL);
        videoSource->release();
    }
    if (FFMPEG)
        FFMPEG->release();
    if (openSLES)
        openSLES->releasePlayer();
    playState->init();
}

void LibVideoPlayer::prepare(const char *sourceData) {
    resetForPrepare();
    videoSource->dataSource = sourceData;
    pthread_create(&videoSource->pthread_prepare, NULL, thread_prepare, this);
}

void LibVideoPlayer::start() {
    if (playState->isExit) {
        LOGE("start after exit")
        return;
    }
    if (playState->isStarted) {
        LOGE("already started")
        return;
    }
    if (!playState->isPrepared) {
        LOGE("should prepare before start")
        playState->isNeedStartedAfterPrepared = true;
        return;
    }
    if (playState->isErrorInPrepared) {
        LOGE("prepare failed and  can not start")
        return;
    }
    LOGD("--start--")
    playState->isStarted = true;

    pthread_create(&videoSource->pthread_decode, NULL, thread_decode, this);
    pthread_create(&videoSource->pthread_vedio_run, NULL, thread_video_run, this);
    pthread_create(&videoSource->pthread_audio_run, NULL, thread_audio_run, this);
}

int LibVideoPlayer::resampleAudioPacket() {
    int dataSize = 0;
    while (!playState->isExit) {
        if (FFMPEG->isCurrentResampleAudioFrameFinished) {
            //.ape音乐一个AvPacket包含多个AvFrame
            videoSource->avPacket_currentAudioResample = av_packet_alloc();
            if (videoSource->queue->audioPopQueue(videoSource->avPacket_currentAudioResample, NULL,
                                                  this) != 0) {
                av_packet_free(&videoSource->avPacket_currentAudioResample);
                av_free(videoSource->avPacket_currentAudioResample);
                videoSource->avPacket_currentAudioResample = NULL;
                LOGD("--音频结束--")
                return 0;
            }
            if (FFMPEG->sendAudioPacket(videoSource->avPacket_currentAudioResample) != 0) {
                av_packet_unref(videoSource->avPacket_currentAudioResample);
                av_packet_free(&videoSource->avPacket_currentAudioResample);
                av_free(videoSource->avPacket_currentAudioResample);
                videoSource->avPacket_currentAudioResample = NULL;
                continue;
            }
        }
        if (audioResampleBuff == NULL)
            audioResampleBuff = (uint8_t *) (malloc(FFMPEG->audioCodecpar->sample_rate * 2 * 2));
        AVFrame *avFrame = av_frame_alloc();
        dataSize = FFMPEG->resampleAudioFrame(avFrame, audioResampleBuff);
        if (dataSize > 0) {
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
        } else if (FFMPEG->isCurrentResampleAudioFrameFinished) {
            av_packet_unref(videoSource->avPacket_currentAudioResample);
            av_packet_free(&videoSource->avPacket_currentAudioResample);
            av_free(videoSource->avPacket_currentAudioResample);
            videoSource->avPacket_currentAudioResample = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
        break;
    }
    return dataSize;
}

