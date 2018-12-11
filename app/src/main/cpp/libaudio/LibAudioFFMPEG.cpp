//
// Created by Android on 2018/12/10.
//

#include "LibAudioFFMPEG.h"

LibAudioFFMPEG::LibAudioFFMPEG(LibAudioSource *audioSource, LibAudioPlayState *playState) {
    this->audioSource = audioSource;
    this->playState = playState;
    av_register_all();
    avformat_network_init();
}

LibAudioFFMPEG::~LibAudioFFMPEG() {
}

int LibAudioFFMPEG::prepare() {
    if (playState->isExit) {
        LOGE("exit before prepare");
        return -1;
    }
    if (audioSource->dataSource == NULL) {
        LOGE("dataSource can not be null");
        return -1;
    }
    pFormatCtx = avformat_alloc_context();
    if (pFormatCtx == NULL) {
        LOGE("can not init AVFormatContext");
        return -1;
    }
    if (avformat_open_input(&pFormatCtx, audioSource->dataSource, NULL, NULL) != 0) {
        LOGE("can not open %s", audioSource->dataSource);
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
            duration = pFormatCtx->duration / AV_TIME_BASE;
            time_base = pFormatCtx->streams[i]->time_base;
            break;
        }
    }
    if (streamIndex == AUDIO_STREAM_NO_FOUND) {
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

int LibAudioFFMPEG::readFrame(AVPacket **avPacket) {
    AVPacket *packet = av_packet_alloc();
    *avPacket = NULL;
    int ret = av_read_frame(pFormatCtx, packet);
    if (ret == READ_FRAME_OK) {
        if (packet->stream_index == streamIndex) {
            *avPacket = packet;
            return ret;
        } else {
            ret = READ_FRAME_NOT_AUDIO;
            av_packet_free(&packet);
            av_free(packet);
            packet = NULL;
        }
    } else {
        //解码结束
        ret = READ_FRAME_FINISHED;
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }

    return ret;
}

int LibAudioFFMPEG::sendPacket(AVPacket *avPacket) {
    if (avPacket == NULL)
        return -1;
    return avcodec_send_packet(pCodecCtx, avPacket);
}

int LibAudioFFMPEG::resampleFrame(AVFrame *avFrame, uint8_t *resampleBuff) {
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
                data_nb_samples = swr_convert(
                        swr_ctx,
                        &resampleBuff,
                        avFrame->nb_samples,
                        (const uint8_t **) avFrame->data,
                        avFrame->nb_samples);

                int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
                dataSize =
                        data_nb_samples * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                current_clock = avFrame->pts * av_q2d(time_base);
                if (current_clock < clock)
                    current_clock = clock;
                clock = current_clock;
            }
            swr_free(&swr_ctx);
            return dataSize;
        }
    } else {
        isCurrentResampleFrameFinished = true;
    }
    return dataSize;
}

void LibAudioFFMPEG::release() {
    if (pFormatCtx != NULL) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
        LOGD("free pFormatCtx")
    }
    if (pCodecCtx != NULL) {
//        avcodec_flush_buffers(pCodecCtx);
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = NULL;
        codecpar = NULL;
        LOGD("free pCodecCtx")
    }

    streamIndex = AUDIO_STREAM_NO_FOUND;
    duration = 0;
    current_clock = 0;
    clock = 0;
    last_clock = 0;

    data_nb_samples = 0;
    isCurrentResampleFrameFinished = true;

}
