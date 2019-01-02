//
// Created by Android on 2018/12/24.
//

#ifndef NDK_LEARN_LIBVIDEOPLAYER_H
#define NDK_LEARN_LIBVIDEOPLAYER_H


#include <jni.h>
#include "LibVideoSource.h"
#include "LibVideoFFMPEG.h"
#include "LibVideoPlayState.h"
#include "LibAudioOpenSLES.h"
#include "LibVideoJavaCallBack.h"
#include <unistd.h>

class LibVideoPlayer {
public:
    LibVideoSource *videoSource = NULL;
    LibVideoPlayState *playState = NULL;
    LibVideoFFMPEG *FFMPEG = NULL;
    LibVideoJavaCallBack *javaCallBack = NULL;
    LibAudioOpenSLES *openSLES = NULL;
    pthread_mutex_t mutex_seek;
    uint8_t *audioResampleBuff = NULL;

    LibVideoPlayer(JNIEnv *env, jobject *object);

    ~LibVideoPlayer();

    void prepare(const char *sourceData);

    void resetForPrepare();

    void start();

    int resampleAudioPacket();

    void hardwareCodecVideo();

    void softwareCodecVideo();

    void stop();

    void destory();

    void seek(int second);

};


#endif //NDK_LEARN_LIBVIDEOPLAYER_H
