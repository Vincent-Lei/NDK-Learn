//
// Created by Android on 2018/11/21.
//

#include <unistd.h>
#include "LeiAudioPlayer.h"

LeiAudioPlayer::LeiAudioPlayer(JNIEnv *env, jobject *object) {
    javaCallBack = new AudioJavaCallBack(env, object);
    audioPlayStatus = new AudioPlayStatus();
    openSLES = new LeiOpenSLES();
    pthread_mutex_init(&mutex_seek, NULL);
    sampleBuffer = static_cast<SAMPLETYPE *>(malloc(44100 * 2 * 2));
    soundTouch = new SoundTouch();
}

LeiAudioPlayer::~LeiAudioPlayer() {

}

void LeiAudioPlayer::initFFMPEG() {
    av_register_all();
    avformat_network_init();
}

int avformat_callback(void *ctx) {
    LeiAudioPlayer *play = (LeiAudioPlayer *) (ctx);
    if (play->audioPlayStatus->isExist) {
        return AVERROR_EOF;
    }
    return 0;
}

void LeiAudioPlayer::onPreparedFinished() {
    audioPlayStatus->isPrepared = true;
    javaCallBack->callJavaPrepared(CHILD_THREAD_CALL);
    if (!audioPlayStatus->isPreparedError && audioPlayStatus->isNeedStartAfterPrepared) {
        //直接播放
        start();
    }
}

void *thread_prepared(void *data) {
    LeiAudioPlayer *play = (LeiAudioPlayer *) (data);
    if (play->audioPlayStatus->isExist) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("exist before play");
        return 0;
    }
    AudioSource *audioSource = play->audioSource;
    audioSource->pFormatCtx = avformat_alloc_context();
    audioSource->pFormatCtx->interrupt_callback.callback = avformat_callback;
    audioSource->pFormatCtx->interrupt_callback.opaque = play;
    if (audioSource->pFormatCtx == NULL) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not init AVFormatContext");
        return 0;
    }
    if (avformat_open_input(&audioSource->pFormatCtx, audioSource->dataSource, NULL, NULL) != 0) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not open %s", audioSource->dataSource);
        return 0;
    }
    if (avformat_find_stream_info(audioSource->pFormatCtx, NULL) < 0) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not find stream info");
        return 0;
    }
    for (int i = 0; i < audioSource->pFormatCtx->nb_streams; ++i) {
        if (audioSource->pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioSource->streamIndex = i;
            audioSource->codecpar = audioSource->pFormatCtx->streams[i]->codecpar;
            audioSource->duration = audioSource->pFormatCtx->duration / AV_TIME_BASE;
            audioSource->time_base = audioSource->pFormatCtx->streams[i]->time_base;
            break;
        }
    }
    if (audioSource->streamIndex == AUDIO_STREAM_NO_FOUND) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not find audio stream");
        return 0;
    }
    AVCodec *avCodec = avcodec_find_decoder(audioSource->codecpar->codec_id);
    if (!avCodec) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not find avCodec for this audio stream");
        return 0;
    }
    audioSource->pCodecCtx = avcodec_alloc_context3(avCodec);
    if (audioSource->pCodecCtx == NULL) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not init AVCodecContext");
        return 0;
    }
    if (avcodec_parameters_to_context(audioSource->pCodecCtx, audioSource->codecpar) < 0) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not fill parameters to AVCodecContext");
        return 0;
    }
    if (avcodec_open2(audioSource->pCodecCtx, avCodec, NULL) != 0) {
        play->audioPlayStatus->isPreparedError = true;
        LOGE("can not open audio stream");
        return 0;
    }
    LOGD("sample_fmt = %d", audioSource->pCodecCtx->sample_fmt);
    play->onPreparedFinished();
    return 0;
};

void *thread_decoder(void *data) {
    LeiAudioPlayer *play = (LeiAudioPlayer *) (data);
    AudioSource *audioSource = play->audioSource;
    AVPacket *avPacket = NULL;
    while (!play->audioPlayStatus->isExist) {

        if (play->audioPlayStatus->isSeek) {
            continue;
        }
        if (audioSource->getQueueSize() > MAX_QUEUE_SIZE) {
            usleep(50 * 1000);
            continue;
        }

        avPacket = av_packet_alloc();

        pthread_mutex_lock(&play->mutex_seek);
        int ret = av_read_frame(audioSource->pFormatCtx, avPacket);
        pthread_mutex_unlock(&play->mutex_seek);

        if (ret == 0) {
            if (avPacket->stream_index == audioSource->streamIndex) {
                //数据进入队列
                audioSource->packetInQueue(avPacket);
            } else {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            //解码结束
            play->audioPlayStatus->isDecodeFinished = true;
            break;
        }
    }
    audioSource->noticeDecodeFinished();
    LOGD("解码结束");
    return 0;

}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    LeiAudioPlayer *player = (LeiAudioPlayer *) (context);
    int soundTouchBufferSize = player->getSoundTouchData();
    if (soundTouchBufferSize > 0) {
        player->audioSource->clock +=
                soundTouchBufferSize /
                ((double) (player->audioSource->codecpar->sample_rate * 2 * 2));
        if (player->audioSource->clock - player->audioSource->last_clock >= 0.5) {
            player->audioSource->last_clock = player->audioSource->clock;
            player->javaCallBack->callJavaDuration(CHILD_THREAD_CALL,
                                                   static_cast<int>(player->audioSource->clock),
                                                   player->audioSource->duration);
        }
        player->amplitudeCount++;
        if (player->amplitudeCount >= 5) {
            player->amplitudeCount = 0;
            int db = player->getPCMAmplitude((char *) (player->sampleBuffer),
                                             soundTouchBufferSize * 4);
            player->javaCallBack->callJavaAmplitude(CHILD_THREAD_CALL, db);
        }

        if (player->isRecordPCM)
            player->javaCallBack->callJavaPCMRecord(CHILD_THREAD_CALL,
                                                    player->audioSource->codecpar->sample_rate,
                                                    soundTouchBufferSize * 4,
                                                    player->sampleBuffer);

        (*player->openSLES->pcmBufferQueue)->Enqueue(player->openSLES->pcmBufferQueue,
                                                     (char *) player->sampleBuffer,
                                                     soundTouchBufferSize * 2 * 2);
    } else {
        LOGE("play finished")
        if (!player->audioPlayStatus->isExist)
            player->javaCallBack->callJavaFinished(CHILD_THREAD_CALL);
    }
}

int LeiAudioPlayer::getSoundTouchData() {
    int data_size = 0;
    while (!audioPlayStatus->isExist && !audioPlayStatus->isCostFinished) {
        out_soundTouch_buffer = NULL;
        if (isSoundTouchFinished) {
            isSoundTouchFinished = false;
            data_size = resampleAudioPacket((&out_soundTouch_buffer));
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1; i++) {
                    sampleBuffer[i] = (out_soundTouch_buffer[i * 2] |
                                       ((out_soundTouch_buffer[i * 2 + 1]) << 8));
                }
                soundTouch->putSamples(sampleBuffer, data_nb_samples);
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


int LeiAudioPlayer::resampleAudioPacket(uint8_t **out_buffer) {
    int dataSize = 0;
    AVPacket *avPacket = av_packet_alloc();
    while (!audioPlayStatus->isExist && !audioPlayStatus->isCostFinished) {
        if (audioSource->packetPopQueue(avPacket) != 0) {
            if (!audioPlayStatus->isDataOnLoad) {
                LOGE("native load data")
                audioPlayStatus->isDataOnLoad = true;
                //call java
                javaCallBack->callJavaDataOnLoad(CHILD_THREAD_CALL, true);
            } else {
                if (audioPlayStatus->isDataOnLoad) {
                    LOGE("native play data")
                    audioPlayStatus->isDataOnLoad = false;
                    //call java
                    javaCallBack->callJavaDataOnLoad(CHILD_THREAD_CALL, false);
                }
            }
            continue;
        }
        if (avcodec_send_packet(audioSource->pCodecCtx, avPacket) != 0) {
            av_packet_unref(avPacket);
            av_packet_free(&avPacket);
            continue;
        }
        //处理数据
        if (resampleBuff == NULL)
            resampleBuff = (uint8_t *) (malloc(audioSource->codecpar->sample_rate * 2 * 2));
        AVFrame *avFrame = av_frame_alloc();
        if (avcodec_receive_frame(audioSource->pCodecCtx, avFrame) == 0) {
            if (avFrame->channels && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }
            SwrContext *swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    NULL, NULL
            );
            if (swr_ctx) {
                if (swr_init(swr_ctx) >= 0) {
                    data_nb_samples = swr_convert(
                            swr_ctx,
                            &resampleBuff,
                            avFrame->nb_samples,
                            (const uint8_t **) avFrame->data,
                            avFrame->nb_samples);

                    int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
                    dataSize = data_nb_samples * out_channels *
                               av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                    *out_buffer = resampleBuff;
                    audioSource->current_time = avFrame->pts * av_q2d(audioSource->time_base);
                    if (audioSource->current_time < audioSource->clock)
                        audioSource->current_time = audioSource->clock;
                    audioSource->clock = audioSource->current_time;


                }
                swr_free(&swr_ctx);
            }
        }
        av_frame_free(&avFrame);
        av_free(avFrame);
        av_packet_unref(avPacket);
        break;
    }
    av_packet_free(&avPacket);
    av_free(avPacket);
    return dataSize;
}

void *thread_SLES(void *data) {
    LeiAudioPlayer *play = (LeiAudioPlayer *) (data);
    play->soundTouch->setSampleRate(play->audioSource->codecpar->sample_rate);
    play->soundTouch->setChannels(2);
    play->soundTouch->setPitch(play->pitch);
    play->soundTouch->setTempo(play->speed);
    if (play->openSLES) {
        play->openSLES->prepare(play->audioSource->codecpar->sample_rate, pcmBufferCallBack, play);
        play->openSLES->play();
    }
    return 0;
}


void LeiAudioPlayer::onPause() {
    if (openSLES)
        openSLES->pause();
}

void LeiAudioPlayer::onResume() {
    if (openSLES)
        openSLES->resume();
}


void LeiAudioPlayer::destroy() {
    resetToInit();
    if (openSLES)
        openSLES->releaseSLES();
    delete (openSLES);
    if (resampleBuff)
        delete resampleBuff;
    resampleBuff = NULL;
    if (javaCallBack)
        delete javaCallBack;
    if (audioPlayStatus)
        delete audioPlayStatus;
    if (soundTouch)
        delete soundTouch;
    if (sampleBuffer)
        delete (sampleBuffer);
    pthread_mutex_destroy(&mutex_seek);
}

void LeiAudioPlayer::prepared(const char *dataSource) {
    resetToInit();
    if (openSLES)
        openSLES->releasePlayer();
    audioPlayStatus->init();
    audioSource = new AudioSource(dataSource, audioPlayStatus);
    audioPlayStatus->isCalledPrepare = true;
    pthread_create(&audioSource->pthread_prepare, NULL, thread_prepared, this);
}

void LeiAudioPlayer::resetToInit() {
    if (audioSource != NULL) {
        audioPlayStatus->isExist = true;
        audioPlayStatus->isNeedStartAfterPrepared = false;
        pthread_join(audioSource->pthread_prepare, NULL);
        pthread_join(audioSource->pthread_decoder, NULL);
        pthread_join(audioSource->pthread_SLES, NULL);
        LOGD("all thread stop")
        audioSource->release();
        delete audioSource;
        LOGD("delete audioSource")
    }
}


void LeiAudioPlayer::start() {
    if (audioPlayStatus->isExist) {
        LOGE("can not start after exist");
        return;
    }
    if (!audioPlayStatus->isCalledPrepare) {
        LOGE("please call prepare first");
        return;
    }
    if (audioPlayStatus->isStarted) {
        LOGE("already started");
        return;
    }

    if (!audioPlayStatus->isPrepared) {
        LOGE("wait Prepared and will start after Prepared");
        audioPlayStatus->isNeedStartAfterPrepared = true;
        return;
    }
    audioPlayStatus->isStarted = true;
    pthread_create(&audioSource->pthread_decoder, NULL, thread_decoder, this);
    pthread_create(&audioSource->pthread_SLES, NULL, thread_SLES, this);
}

void LeiAudioPlayer::seek(int64_t secTarget) {
    if (audioPlayStatus->isExist) {
        LOGE("can not seek after exist");
        return;
    }

    if (audioSource == NULL || !audioPlayStatus->isStarted) {
        LOGE("can not seek before start");
        return;
    }
    if (audioSource->duration == 0) {
        LOGE("can not seek audioSource");
        return;
    }
    if (secTarget > 0 && secTarget <= audioSource->duration) {
        audioPlayStatus->isSeek = true;
        audioSource->clock = 0;
        audioSource->last_clock = 0;
        audioSource->clearAVPackgetQueue();
        pthread_mutex_lock(&mutex_seek);
        int64_t rel = secTarget * AV_TIME_BASE;
        avformat_seek_file(audioSource->pFormatCtx, -1, INT64_MIN, rel, INT64_MAX, 0);
        pthread_mutex_unlock(&mutex_seek);
        audioPlayStatus->isSeek = false;
    }

}

void LeiAudioPlayer::setVolume(int percent) {
    if (openSLES != NULL)
        openSLES->setVolume(percent);
}

void LeiAudioPlayer::setMute(int mute) {
    if (openSLES != NULL)
        openSLES->setMute(mute);
}

void LeiAudioPlayer::setPitch(float pitch) {
    this->pitch = pitch;
    if (soundTouch != NULL) {
        soundTouch->setPitch(pitch);
    }
}

void LeiAudioPlayer::setSpeed(float speed) {
    this->speed = speed;
    if (soundTouch != NULL) {
        soundTouch->setTempo(speed);
    }
}

int LeiAudioPlayer::getPCMAmplitude(char *pcmcata, size_t pcmsize) {
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

void LeiAudioPlayer::setRecordPCM(bool isRecordPCM) {
    this->isRecordPCM = isRecordPCM;
}



