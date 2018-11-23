//
// Created by Android on 2018/11/21.
//

#ifndef NDK_LEARN_LEIAUDIOPLAYER_H
#define NDK_LEARN_LEIAUDIOPLAYER_H

#include "AudioJavaCallBack.h"
#include "AudioPlayStatus.h"
#include "AudioSource.h"
#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"

class LeiAudioPlayer {
public:
    AudioJavaCallBack *javaCallBack = NULL;
    AudioPlayStatus *audioPlayStatus = NULL;
    AudioSource *audioSource = NULL;
    uint8_t *resampleBuff = NULL;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

public:
    LeiAudioPlayer(JNIEnv *env, jobject *object);

    ~ LeiAudioPlayer();

    void free();

    void initFFMPEG();

    void prepared(const char *dataSource);

    void onPreparedFinished();

    void start();

    void onPause();

    void onResume();

    int resampleAudioPacket();

    int getCurrentSampleRateForOpensles(int sample_rate);

    void initSLES();

    void freeSLES();
};


#endif //NDK_LEARN_LEIAUDIOPLAYER_H
