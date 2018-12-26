//
// Created by Android on 2018/12/14.
//

#include "AudioFileTranscoder.h"

AudioFileTranscoder::AudioFileTranscoder(JNIEnv *env) {
    this->env = env;
    av_register_all();
}

AudioFileTranscoder::~AudioFileTranscoder() {
    if (pFormatCtx != NULL) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
        LOGD("free pFormatCtx")
    }
    if (pCodecCtx != NULL) {
        avcodec_flush_buffers(pCodecCtx);
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = NULL;
        codecpar = NULL;
        LOGD("free pCodecCtx")
    }
    if (resampleBuff)
        delete resampleBuff;
}

jobject AudioFileTranscoder::createPcm(const char *fileInputPath, const char *fileOutPath) {
    if (prepare(fileInputPath) != 0)
        return nullptr;
    AVPacket *packet = NULL;
    int ret;
    int dataSize;
    FILE *pcm = fopen(fileOutPath, "wb");
    if (!pcm)
        return nullptr;
    if (resampleBuff == NULL)
        resampleBuff = (uint8_t *) (malloc(codecpar->sample_rate * 2 * 2));
    isJobFinished = false;
    while (!isCalledRelease) {
        ret = readFrame(&packet);
        if (ret == -1)
            break;
        if (ret == -2)
            continue;
        if (ret == 0) {
            if (avcodec_send_packet(pCodecCtx, packet) != 0) {
                av_packet_unref(packet);
                av_packet_free(&packet);
                av_free(packet);
                continue;
            }
            isCurrentResampleFrameFinished = false;
            while (!isCurrentResampleFrameFinished) {
                AVFrame *avFrame = av_frame_alloc();
                dataSize = resampleFrame(avFrame, resampleBuff);
                if (dataSize > 0) {
                    //写入
                    fwrite(resampleBuff, sizeof(uint8_t), dataSize, pcm);
                }
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
            }
            av_packet_unref(packet);
            av_packet_free(&packet);
            av_free(packet);
        }
    }
    fclose(pcm);
    if (isCalledRelease) {
        isJobFinished = true;
        release();
        return nullptr;
    }
    jclass javaclass = env->FindClass("com/sdk/audio/transcoding/AudioFileTranscoder$AudioParam");
    jfieldID javafiled = env->GetFieldID(javaclass, "sampleRate", "I");
    jmethodID ctorID = env->GetMethodID(javaclass, "<init>", "()V");
    jobject param = env->NewObject(javaclass, ctorID);
    env->SetIntField(param, javafiled, codecpar->sample_rate);
    isJobFinished = true;
    if (isCalledRelease)
        release();
    return param;
}

void AudioFileTranscoder::release() {
    LOGD("--AudioFileTranscoder::release--")
    isCalledRelease = true;
    if (isJobFinished)
        delete this;
}

int avformat_callback2(void *ctx) {
    AudioFileTranscoder *transcoder = (AudioFileTranscoder *) (ctx);
    if (transcoder->isCalledRelease) {
        return AVERROR_EOF;
    }
    return 0;
}

int AudioFileTranscoder::prepare(const char *fileInputPath) {
    if (fileInputPath == NULL) {
        LOGE("dataSource can not be null");
        return -1;
    }
    pFormatCtx = avformat_alloc_context();
    if (pFormatCtx == NULL) {
        LOGE("can not init AVFormatContext");
        return -1;
    }
    pFormatCtx->interrupt_callback.callback = avformat_callback2;
    pFormatCtx->interrupt_callback.opaque = this;
    if (avformat_open_input(&pFormatCtx, fileInputPath, NULL, NULL) != 0) {
        LOGE("can not open %s", fileInputPath);
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("can not find stream info");
        return -1;
    }

    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            streamIndex = i;
            codecpar = pFormatCtx->streams[i]->codecpar;
            break;
        }
    }
    if (streamIndex == -1) {
        LOGE("can not find audio stream");
        return -1;
    }
    AVCodec *avCodec = avcodec_find_decoder(codecpar->codec_id);
    if (!avCodec) {
        LOGE("can not find avCodec for this audio stream");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(avCodec);
    if (pCodecCtx == NULL) {
        LOGE("can not init AVCodecContext");
        return -1;
    }
    if (avcodec_parameters_to_context(pCodecCtx, codecpar) < 0) {
        LOGE("can not fill parameters to AVCodecContext");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, avCodec, NULL) != 0) {
        LOGE("can not open audio stream");
        return -1;
    }
    LOGD("prepare success")
    return 0;
}

int AudioFileTranscoder::readFrame(AVPacket **avPacket) {
    AVPacket *packet = av_packet_alloc();
    *avPacket = NULL;
    int ret = av_read_frame(pFormatCtx, packet);
    if (ret == 0) {
        if (packet->stream_index == streamIndex) {
            *avPacket = packet;
            return ret;
        } else {
            ret = -2;
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
        }
    } else {
        //解码结束
        ret = -1;
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }

    return ret;
}

int AudioFileTranscoder::resampleFrame(AVFrame *avFrame, uint8_t *resampleBuff) {
    int dataSize = 0;
    if (avcodec_receive_frame(pCodecCtx, avFrame) == 0) {
        isCurrentResampleFrameFinished = false;
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
                int data_nb_samples = swr_convert(
                        swr_ctx,
                        &resampleBuff,
                        avFrame->nb_samples,
                        (const uint8_t **) avFrame->data,
                        avFrame->nb_samples);

                int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
                dataSize =
                        data_nb_samples * out_channels *
                        av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
            }
            swr_free(&swr_ctx);
            return dataSize;
        }
    } else {
        isCurrentResampleFrameFinished = true;
    }
    return dataSize;
}
