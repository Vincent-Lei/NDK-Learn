//
// Created by Android on 2018/12/10.
//

#include <unistd.h>
#include "LibAudioPlayer.h"

LibAudioPlayer::LibAudioPlayer(JNIEnv *env, jobject *object) {
    playState = new LibAudioPlayState();
    audioSource = new LibAudioSource();
    javaCallBack = new LibJavaCallBack(env, object);
    FFMPEG = new LibAudioFFMPEG(audioSource, playState);
    sampleBuffer = static_cast<soundtouch::SAMPLETYPE *>(malloc(44100 * 2 * 2));
    soundTouch = new soundtouch::SoundTouch();
    pthread_mutex_init(&mutex_seek, NULL);
}

LibAudioPlayer::~LibAudioPlayer() {
    pthread_mutex_destroy(&mutex_seek);
}

void *audio_prepare_thread(void *data) {
    LibAudioPlayer *player = (LibAudioPlayer *) (data);
    if (player->FFMPEG->prepare() == 0) {
        //prepare success
        player->playState->isPrepared = true;
    } else
        player->playState->isErrorInPrepared = true;
    player->playState->isPrepared = true;
    player->javaCallBack->callJavaPrepared(CHILD_THREAD_CALL);
    LOGD("audio_prepare_thread finished")
    if (player->playState->isNeedStartedAfterPrepared)
        player->start();
    return 0;
}

void *audio_decode_thread(void *data) {
    LOGD("audio_decode_thread start")
    LibAudioPlayer *player = (LibAudioPlayer *) (data);
    LibAudioQueue *queue = player->audioSource->queue;
    AVPacket *avPacket = NULL;
    while (!player->playState->isExit) {
        if (player->playState->isSeek) {
            usleep(20 * 1000);
            continue;
        }
        if (queue->getQueueSize() > MAX_QUEUE_SIZE) {
            usleep(50 * 1000);
            continue;
        }
        pthread_mutex_lock(&player->mutex_seek);
        int ret = player->FFMPEG->readFrame(&avPacket);
        pthread_mutex_unlock(&player->mutex_seek);
        if (ret == 0) {
            //入队列
            queue->inQueue(avPacket);
        } else if (ret == READ_FRAME_FINISHED) {
            //解码结束
            player->playState->isDecodeFinished = true;
            break;
        }
    }
    queue->notifyQueuePushFinished();
    LOGD("audio_decode_thread finished")
    return 0;
}

void LibAudioPlayer::resetForPrepare() {
    playState->isExit = true;
    if (openSLES)
        openSLES->releasePlayer();
    if (audioSource) {
        pthread_join(audioSource->thread_prepare, NULL);
        pthread_join(audioSource->thread_decode, NULL);
        pthread_join(audioSource->thread_start, NULL);
        audioSource->release();
    }
    LOGD("size = %d",audioSource->queue->getQueueSize())
    if (FFMPEG)
        FFMPEG->release();
    playState->init();
    isSoundTouchFinished = false;
}

void LibAudioPlayer::prepare(const char *audioDataSource) {
    resetForPrepare();
    audioSource->dataSource = audioDataSource;
    pthread_create(&audioSource->thread_prepare, NULL, audio_prepare_thread, this);
}

void pcmBufferCallBack2(SLAndroidSimpleBufferQueueItf bf, void *context) {
    LibAudioPlayer *player = (LibAudioPlayer *) (context);
    int soundTouchBufferSize = player->getSoundTouchData();
    if (soundTouchBufferSize > 0) {
        player->FFMPEG->clock +=
                soundTouchBufferSize / ((double) (player->FFMPEG->codecpar->sample_rate * 2 * 2));
        if (player->FFMPEG->clock - player->FFMPEG->last_clock >= 1) {
            player->FFMPEG->last_clock = player->FFMPEG->clock;
            player->javaCallBack->callJavaDuration(CHILD_THREAD_CALL,
                                                   static_cast<int>(player->FFMPEG->clock),
                                                   player->FFMPEG->duration);
        }
        player->amplitudeCount++;
        if (player->amplitudeCount >= CALL_BACK_AMPLITUDE_COUNT) {
            player->amplitudeCount = 0;
            int db = player->getPCMAmplitude((char *) (player->sampleBuffer),
                                             soundTouchBufferSize * 4);
            player->javaCallBack->callJavaAmplitude(CHILD_THREAD_CALL, db);
        }

        if (player->playState->isRecordPCM)
            player->javaCallBack->callJavaPCMRecord(CHILD_THREAD_CALL,
                                                    player->FFMPEG->codecpar->sample_rate,
                                                    soundTouchBufferSize * 4,
                                                    player->sampleBuffer);

        (*player->openSLES->pcmBufferQueue)->Enqueue(player->openSLES->pcmBufferQueue,
                                                     (char *) player->sampleBuffer,
                                                     soundTouchBufferSize * 4);
    } else {
        LOGE("play finished")
        if (!player->playState->isExit)
            player->javaCallBack->callJavaFinished(CHILD_THREAD_CALL);
    }
}

void *audio_start_thread(void *data) {
    LOGD("audio_start_thread start")
    LibAudioPlayer *player = (LibAudioPlayer *) (data);
    player->soundTouch->setSampleRate(player->FFMPEG->codecpar->sample_rate);
    player->soundTouch->setChannels(2);
    player->soundTouch->setPitch(player->pitch);
    player->soundTouch->setTempo(player->speed);
    if (player->openSLES == NULL) {
        player->openSLES = new LibAudioOpenSLES();
    }
    if (player->openSLES) {
        player->openSLES->prepare(player->FFMPEG->codecpar->sample_rate, pcmBufferCallBack2,
                                  player);
        player->openSLES->play();
    }
    LOGD("audio_start_thread finished")
    return 0;
}


void LibAudioPlayer::start() {
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
    pthread_create(&audioSource->thread_decode, NULL, audio_decode_thread, this);
    pthread_create(&audioSource->thread_start, NULL, audio_start_thread, this);
}

void onQueueWaitData(void *data) {
    LibAudioPlayer *player = (LibAudioPlayer *) (data);
    if (!player->playState->isOnWaitingData) {
        player->playState->isOnWaitingData = true;
        player->javaCallBack->callJavaDataOnLoad(CHILD_THREAD_CALL, true);
    }
}

int LibAudioPlayer::resampleAudioPacket(uint8_t **out_soundTouch_buffer) {
    int dataSize = 0;
    while (!playState->isExit) {
        if (FFMPEG->isCurrentResampleFrameFinished) {
            //.ape音乐一个AvPacket包含多个AvFrame
            audioSource->avPacket_currentResample = av_packet_alloc();
            if (audioSource->queue->popQueue(audioSource->avPacket_currentResample, onQueueWaitData,
                                             this) != 0) {
                av_packet_free(&audioSource->avPacket_currentResample);
                av_free(audioSource->avPacket_currentResample);
                audioSource->avPacket_currentResample = NULL;
                playState->isCostFinished = true;
                return 0;
            }
            if (playState->isOnWaitingData) {
                playState->isOnWaitingData = false;
                javaCallBack->callJavaDataOnLoad(CHILD_THREAD_CALL, false);
            }
            if (FFMPEG->sendPacket(audioSource->avPacket_currentResample) != 0) {
                av_packet_unref(audioSource->avPacket_currentResample);
                av_packet_free(&audioSource->avPacket_currentResample);
                av_free(audioSource->avPacket_currentResample);
                audioSource->avPacket_currentResample = NULL;
                continue;
            }
        }
        if (resampleBuff == NULL)
            resampleBuff = (uint8_t *) (malloc(FFMPEG->codecpar->sample_rate * 2 * 2));
        AVFrame *avFrame = av_frame_alloc();
        dataSize = FFMPEG->resampleFrame(avFrame, resampleBuff);
        if (dataSize > 0) {
            *out_soundTouch_buffer = resampleBuff;
        } else if (FFMPEG->isCurrentResampleFrameFinished) {
            av_packet_unref(audioSource->avPacket_currentResample);
            av_packet_free(&audioSource->avPacket_currentResample);
            av_free(audioSource->avPacket_currentResample);
            audioSource->avPacket_currentResample = NULL;
            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        break;
    }
    return dataSize;
}

int LibAudioPlayer::getSoundTouchData() {
    int data_size = 0;
    while (!playState->isExit && !playState->isCostFinished) {
        out_soundTouch_buffer = NULL;
        if (isSoundTouchFinished) {
            isSoundTouchFinished = false;
            data_size = resampleAudioPacket((&out_soundTouch_buffer));
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    sampleBuffer[i] = (out_soundTouch_buffer[i * 2] |
                                       ((out_soundTouch_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleBuffer, FFMPEG->data_nb_samples);
                data_st_num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
            } else {
                soundTouch->flush();
            }
        }
        if (data_st_num == 0) {
            isSoundTouchFinished = true;
            continue;
        } else {
            if (out_soundTouch_buffer == NULL) {
                data_st_num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if (data_st_num == 0) {
                    isSoundTouchFinished = true;
                    continue;
                }
            }
            return data_st_num;
        }
    }
    return 0;
}

int LibAudioPlayer::getPCMAmplitude(char *pcmcata, size_t pcmsize) {
    int db = 0;
    short int pervalue = 0;
    double sum = 0;
    for (int i = 0; i < pcmsize; i += 2) {
        memcpy(&pervalue, pcmcata + i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (pcmsize / 2);
    if (sum > 0) {
        db = (int) 20.0 * log10(sum);
    }
    return db;
}

void LibAudioPlayer::seek(int64_t secTarget) {
    if (playState->isExit) {
        LOGE("can not seek after exist");
        return;
    }

    if (audioSource == NULL || !playState->isStarted) {
        LOGE("can not seek before start");
        return;
    }
    if (FFMPEG == NULL || FFMPEG->duration == 0) {
        LOGE("can not seek audioSource");
        return;
    }
    if (secTarget > 0 && secTarget <= FFMPEG->duration) {
        playState->isSeek = true;
        FFMPEG->clock = 0;
        FFMPEG->last_clock = 0;
        audioSource->queue->emptyQueue();
        pthread_mutex_lock(&mutex_seek);
        int64_t rel = secTarget * AV_TIME_BASE;
        avcodec_flush_buffers(FFMPEG->pCodecCtx);
        avformat_seek_file(FFMPEG->pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
        pthread_mutex_unlock(&mutex_seek);
        playState->isSeek = false;
    }
}

void LibAudioPlayer::setVolume(int percent) {
    if (openSLES != NULL)
        openSLES->setVolume(percent);
}

void LibAudioPlayer::setMute(int mute) {
    if (openSLES != NULL)
        openSLES->setMute(mute);
}

void LibAudioPlayer::setPitch(float pitch) {
    this->pitch = pitch;
    if (soundTouch != NULL) {
        soundTouch->setPitch(pitch);
    }
}

void LibAudioPlayer::setSpeed(float speed) {
    this->speed = speed;
    if (soundTouch != NULL) {
        soundTouch->setTempo(speed);
    }
}

void LibAudioPlayer::setRecordPCM(bool isRecordPCM) {
    if (this->playState)
        this->playState->isRecordPCM = isRecordPCM;
}

void LibAudioPlayer::onPause() {
    if (openSLES)
        openSLES->pause();
}

void LibAudioPlayer::onResume() {
    if (openSLES)
        openSLES->resume();
}

void LibAudioPlayer::destroy() {
    resetForPrepare();
    if (audioSource)
        delete audioSource;
    audioSource = NULL;
    LOGD("destroy--audioSource")
    if (openSLES)
        delete openSLES;
    openSLES = NULL;
    LOGD("destroy--openSLES")
    if (FFMPEG)
        delete FFMPEG;
    FFMPEG = NULL;
    LOGD("destroy--FFMPEG")
    if (playState)
        delete playState;
    playState = NULL;
    LOGD("destroy--playState")
    if (javaCallBack)
        delete javaCallBack;
    javaCallBack = NULL;
    LOGD("destroy--javaCallBack")
    if (soundTouch)
        delete (soundTouch);
    soundTouch = NULL;
    LOGD("destroy-soundTouch")
    if (resampleBuff)
        delete (resampleBuff);
    resampleBuff = NULL;
    LOGD("destroy-resampleBuff")
    if (sampleBuffer)
        delete (sampleBuffer);
    sampleBuffer = NULL;
    LOGD("destroy-sampleBuffer")

}


