//
// Created by Android on 2018/12/24.
//

#include "LibVideoFFMPEG.h"

LibVideoFFMPEG::LibVideoFFMPEG(LibVideoSource *videoSource, LibVideoPlayState *playState) {
    this->videoSource = videoSource;
    this->playState = playState;

    av_register_all();
    avformat_network_init();
}

LibVideoFFMPEG::~LibVideoFFMPEG() {

}

int avformat_video_callback(void *ctx) {
    LibVideoFFMPEG *FFMPEG = (LibVideoFFMPEG *) (ctx);
    if (FFMPEG->playState->isExit) {
        return AVERROR_EOF;
    }
    return 0;
}

int getCodecCtx(AVCodecParameters *codecpar, AVCodecContext **pCodecCtx) {
    AVCodec *avCodec = avcodec_find_decoder(codecpar->codec_id);
    if (!avCodec) {
        return -1;
    }
    *pCodecCtx = avcodec_alloc_context3(avCodec);
    if (*pCodecCtx == NULL) {
        return -1;
    }
    if (avcodec_parameters_to_context(*pCodecCtx, codecpar) < 0) {
        return -1;
    }
    if (avcodec_open2(*pCodecCtx, avCodec, NULL) != 0) {
        return -1;
    }

    return 0;
}

int LibVideoFFMPEG::prepared() {
    if (playState->isExit) {
        LOGE("exit before prepare");
        return -1;
    }
    if (videoSource->dataSource == NULL) {
        LOGE("dataSource can not be null");
        return -1;
    }
    pFormatCtx = avformat_alloc_context();
    if (pFormatCtx == NULL) {
        LOGE("can not init AVFormatContext");
        return -1;
    }
    pFormatCtx->interrupt_callback.callback = avformat_video_callback;
    pFormatCtx->interrupt_callback.opaque = this;
    if (avformat_open_input(&pFormatCtx, videoSource->dataSource, NULL, NULL) != 0) {
        LOGE("can not open %s", videoSource->dataSource);
        return -1;
    }
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("can not find stream info");
        return -1;
    }

    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_index_audio = i;
            audioCodecpar = pFormatCtx->streams[i]->codecpar;
            audioDuration = pFormatCtx->duration / AV_TIME_BASE;
            audioTimeBase = pFormatCtx->streams[i]->time_base;
        } else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream_index_video = i;
            videoCodecpar = pFormatCtx->streams[i]->codecpar;
            videoDuration = pFormatCtx->duration / AV_TIME_BASE;
            videoTimeBase = pFormatCtx->streams[i]->time_base;

            int num = pFormatCtx->streams[i]->avg_frame_rate.num;
            int den = pFormatCtx->streams[i]->avg_frame_rate.den;
            if (num != 0 && den != 0) {
                int fps = num / den;//[25 / 1]
                //每一帧时间
                defaultVideoDelayTime = 1.0 / fps;
            }

        }
    }
    if (stream_index_video == STREAM_NO_FOUND) {
        LOGE("can not find video stream");
        return -1;
    }


    AVCodec *pCodec = NULL;
    switch (videoCodecpar->codec_id) {
        case AV_CODEC_ID_H264:
            pCodec = avcodec_find_decoder_by_name("h264_mediacodec");//硬解码264
            break;
        case AV_CODEC_ID_MPEG4:
            pCodec = avcodec_find_decoder_by_name("mpeg4_mediacodec");//硬解码mpeg4
            break;
        case AV_CODEC_ID_HEVC:
            pCodec = avcodec_find_decoder_by_name("hevc_mediacodec");//硬解码265
            break;
    }

    if (pCodec != NULL) {
        LOGD("video 硬解码")
        pVideoCodecCtx = avcodec_alloc_context3(pCodec);
    }else{
        LOGD("video 软解码")
        if (getCodecCtx(videoCodecpar, &pVideoCodecCtx) != 0) {
            LOGE("can not find video CodecCtx");
            return -1;
        }
    }

    LOGD("videoWidth = %d", pVideoCodecCtx->width)
    LOGD("videoHeight = %d", pVideoCodecCtx->height)
    if (stream_index_audio != STREAM_NO_FOUND) {
        getCodecCtx(audioCodecpar, &pAudioCodecCtx);
    } else
        LOGE("can not find audio stream");
    LOGD("prepare success")
    return 0;
}

int LibVideoFFMPEG::readFrame(AVPacket **avPacket) {
    AVPacket *packet = av_packet_alloc();
    *avPacket = NULL;
    int ret = av_read_frame(pFormatCtx, packet);
    if (ret == 0) {
        if (packet->stream_index == stream_index_audio) {
            *avPacket = packet;
            return V_READ_AUDIO_STREAM;
        }
        if (packet->stream_index == stream_index_video) {
            *avPacket = packet;
            return V_READ_VIDEO_STREAM;
        }
        ret = V_READ_FRAME_NONE;
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    } else {
        //解码结束
        ret = V_READ_FRAME_FINISHED;
        av_packet_free(&packet);
        av_free(packet);
        packet = NULL;
    }
    return ret;
}

int LibVideoFFMPEG::sendAudioPacket(AVPacket *avPacket) {
    if (avPacket == NULL)
        return -1;
    return avcodec_send_packet(pAudioCodecCtx, avPacket);
}

int LibVideoFFMPEG::resampleAudioFrame(AVFrame *avFrame, uint8_t *resampleBuff) {
    int dataSize = 0;
    if (avcodec_receive_frame(pAudioCodecCtx, avFrame) == 0) {
        isCurrentResampleAudioFrameFinished = false;
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
                        data_nb_samples * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                double current_clock = avFrame->pts * av_q2d(audioTimeBase);
                if (current_clock < audioClock)
                    current_clock = audioClock;
                audioClock = current_clock;
            }
            swr_free(&swr_ctx);
            return dataSize;
        }
    } else {
        isCurrentResampleAudioFrameFinished = true;
    }
    return dataSize;
}

void LibVideoFFMPEG::release() {
    if (pFormatCtx != NULL) {
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx = NULL;
        LOGD("free pFormatCtx")
    }
    if (pAudioCodecCtx != NULL) {
//        avcodec_flush_buffers(pAudioCodecCtx);
        avcodec_close(pAudioCodecCtx);
        avcodec_free_context(&pAudioCodecCtx);
        pAudioCodecCtx = NULL;
        audioCodecpar = NULL;
        LOGD("free pCodecCtx")
    }
    if (pVideoCodecCtx != NULL) {
//        avcodec_flush_buffers(pVideoCodecCtx);
        avcodec_close(pVideoCodecCtx);
        avcodec_free_context(&pVideoCodecCtx);
        pVideoCodecCtx = NULL;
        videoCodecpar = NULL;
        LOGD("free pCodecCtx")
    }
    stream_index_audio = STREAM_NO_FOUND;
    stream_index_video = STREAM_NO_FOUND;
    audioDuration = 0;
    videoDuration = 0;
    audioClock = 0;
    videoClock = 0;
    isCurrentResampleAudioFrameFinished = true;
}


int
LibVideoFFMPEG::resampleVideoFrame(AVPacket *avPacket, AVFrame **avFrame_yuv, uint8_t *buffer,
                                   long *videoDelayTime) {
    if (avcodec_send_packet(pVideoCodecCtx, avPacket) != 0)
        return -1;
    AVFrame *avFrame = av_frame_alloc();
    if (avcodec_receive_frame(pVideoCodecCtx, avFrame) != 0) {
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        return -1;
    }
//    if (avFrame->format == AV_PIX_FMT_YUV420P) {
//        *avFrame_yuv = avFrame;
//        double diff = getVideo2AudioFrameDiffTime(avFrame);
//        *videoDelayTime = static_cast<int>(getFrameShouldDelayTimeByDiff(diff) * 1000000);
//        return 0;
//    }
    double diff = getVideo2AudioFrameDiffTime(avFrame);
    if (diff > 0) {
        LOGD("drop frame--");
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;
        return -1;
    }
    *videoDelayTime = static_cast<long>(getFrameShouldDelayTimeByDiff(diff) * 1000000);
    LOGD("videoDelayTime =%d", *videoDelayTime);
    AVFrame *pFrameYUV420P = av_frame_alloc();
    av_image_fill_arrays(
            pFrameYUV420P->data,
            pFrameYUV420P->linesize,
            buffer,
            AV_PIX_FMT_YUV420P,
            pVideoCodecCtx->width,
            pVideoCodecCtx->height,
            1);
    SwsContext *sws_ctx = sws_getContext(
            pVideoCodecCtx->width,
            pVideoCodecCtx->height,
            pVideoCodecCtx->pix_fmt,
            pVideoCodecCtx->width,
            pVideoCodecCtx->height,
            AV_PIX_FMT_YUV420P,
            SWS_BICUBIC, NULL, NULL, NULL);

    if (!sws_ctx) {
        av_frame_free(&avFrame);
        av_free(avFrame);
        avFrame = NULL;

        av_frame_free(&pFrameYUV420P);
        av_free(pFrameYUV420P);
        pFrameYUV420P = NULL;
        return -1;
    }
    sws_scale(
            sws_ctx,
            (const uint8_t *const *) (avFrame->data),
            avFrame->linesize,
            0,
            avFrame->height,
            pFrameYUV420P->data,
            pFrameYUV420P->linesize);
//    double diff = getVideo2AudioFrameDiffTime(avFrame);
//    *videoDelayTime = static_cast<int>(getFrameShouldDelayTimeByDiff(diff) * 1000000);

//    double timestamp;
//    if(avFrame->pts == AV_NOPTS_VALUE) {
//        timestamp = 0;
//    } else {
//        timestamp = av_frame_get_best_effort_timestamp(avFrame)*av_q2d(videoTimeBase);
//    }
//    double frameRate = av_q2d(pFormatCtx->streams[stream_index_video]->avg_frame_rate);
//    frameRate += avFrame->repeat_pict * (frameRate * 0.5);
//    if (timestamp == 0.0) {
//        usleep((unsigned long)(frameRate*1000));
//    }else {
//        if (fabs(timestamp - audioClock) > AV_SYNC_THRESHOLD_MIN &&
//            fabs(timestamp - audioClock) < AV_NOSYNC_THRESHOLD) {
//            if (timestamp > audioClock) {
//                usleep((unsigned long)((timestamp - audioClock)*1000000));
//            }
//        }
//    }
    av_frame_free(&avFrame);
    av_free(avFrame);
    avFrame = NULL;
    sws_freeContext(sws_ctx);
    *avFrame_yuv = pFrameYUV420P;
    return 0;
}

double LibVideoFFMPEG::getVideo2AudioFrameDiffTime(AVFrame *avFrame) {
    double pts = av_frame_get_best_effort_timestamp(avFrame);
    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }
    pts *= av_q2d(videoTimeBase);

    if (pts > 0) {
        videoClock = pts;
    }

    double diff = audioClock - videoClock;
    return diff;
}

double LibVideoFFMPEG::getFrameShouldDelayTimeByDiff(double diff) {
    if (diff > 0.003) {
        delayTime = delayTime * 2 / 3;
        if (delayTime < defaultVideoDelayTime / 2) {
            delayTime = defaultVideoDelayTime * 2 / 3;
        } else if (delayTime > defaultVideoDelayTime * 2) {
            delayTime = defaultVideoDelayTime * 2;
        }
    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < defaultVideoDelayTime / 2) {
            delayTime = defaultVideoDelayTime * 2 / 3;
        } else if (delayTime > defaultVideoDelayTime * 2) {
            delayTime = defaultVideoDelayTime * 2;
        }
    } else if (diff == 0.003) {

    }
    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultVideoDelayTime * 2;
    }

    if (fabs(diff) >= 10) {
        //无音频
        delayTime = defaultVideoDelayTime;
    }
    return delayTime;
}
