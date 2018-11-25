//
// Created by Android on 2018/11/21.
//

#include <unistd.h>
#include "LeiAudioPlayer.h"

LeiAudioPlayer::LeiAudioPlayer(JNIEnv *env, jobject *object) {
    javaCallBack = new AudioJavaCallBack(env, object);
    audioPlayStatus = new AudioPlayStatus();
    pthread_mutex_init(&mutex_seek, NULL);
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

void *thread_SLES(void *data) {
    LeiAudioPlayer *play = (LeiAudioPlayer *) (data);
    play->initSLES();
    return 0;
}


int LeiAudioPlayer::resampleAudioPacket() {
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
                    int nb = swr_convert(
                            swr_ctx,
                            &resampleBuff,
                            avFrame->nb_samples,
                            (const uint8_t **) avFrame->data,
                            avFrame->nb_samples);

                    int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
                    dataSize = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

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

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    LeiAudioPlayer *player = (LeiAudioPlayer *) (context);
    int bufferSize = player->resampleAudioPacket();
    if (bufferSize > 0) {
        player->audioSource->clock +=
                bufferSize / ((double) (player->audioSource->codecpar->sample_rate * 2 * 2));
        if (player->audioSource->clock - player->audioSource->last_clock >= 0.5) {
            player->audioSource->last_clock = player->audioSource->clock;
//            LOGD("%lf/%d", player->audioSource->clock, player->audioSource->duration);
            player->javaCallBack->callJavaDuration(CHILD_THREAD_CALL,
                                                   static_cast<int>(player->audioSource->clock),
                                                   player->audioSource->duration);
        }
        (*player->pcmBufferQueue)->Enqueue(player->pcmBufferQueue, (char *) player->resampleBuff,
                                           bufferSize);
    } else {
        LOGE("play finished")
        if (!player->audioPlayStatus->isExist)
            player->javaCallBack->callJavaFinished(CHILD_THREAD_CALL);
    }
}

void LeiAudioPlayer::initSLES() {
    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            static_cast<SLuint32>(getCurrentSampleRateForOpensles(
                    audioSource->codecpar->sample_rate)),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource, &audioSnk, 1,
                                       ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

//    注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE, &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
//    获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);
    audioPlayStatus->isStarted = true;
}

int LeiAudioPlayer::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void LeiAudioPlayer::onPause() {
    if (pcmPlayerPlay) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
    LOGD("native pause")
}

void LeiAudioPlayer::onResume() {
    if (pcmPlayerPlay)
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    LOGD("native resume")
}

void LeiAudioPlayer::stopSLES() {
    LOGD("prepare freeSLES")
    if (pcmPlayerPlay) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
    if (pcmPlayerObject) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
        LOGD("freeSLES--pcmPlayerObject")
    }

    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
        LOGD("freeSLES--outputMixObject")
    }

    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
        LOGD("freeSLES--engineEngine")
    }
}

void LeiAudioPlayer::onDestory() {
    stop();
    if (resampleBuff)
        delete resampleBuff;
    resampleBuff = NULL;
    if (javaCallBack)
        delete javaCallBack;
    if (audioPlayStatus)
        delete audioPlayStatus;
}

void LeiAudioPlayer::prepared(const char *dataSource) {
    stop();
    audioPlayStatus->init();
    audioSource = new AudioSource(dataSource, audioPlayStatus);
    audioPlayStatus->isCalledPrepare = true;
    pthread_create(&audioSource->pthread_prepare, NULL, thread_prepared, this);
}

void LeiAudioPlayer::stop() {
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
    stopSLES();
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


