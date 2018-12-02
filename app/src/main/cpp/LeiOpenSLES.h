//
// Created by Android on 2018/11/29.
//

#ifndef NDK_LEARN_LEIOPENSLES_H
#define NDK_LEARN_LEIOPENSLES_H
#define AUDIO_MUTE_LEFT 1
#define AUDIO_MUTE_RIFGT 0
#define AUDIO_MUTE_DOUBLE 2

#include <SLES/OpenSLES.h>
#include "SLES/OpenSLES_Android.h"
#include "native-log.h"


class LeiOpenSLES {
public:
    slAndroidSimpleBufferQueueCallback callBack;
    void *callBackContext;
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
    SLVolumeItf pcmVolumePlay = NULL;
    //声道
    SLMuteSoloItf pcmMutePlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    int volumePercent = 50;
    int audioMute = AUDIO_MUTE_DOUBLE ;
public:
    LeiOpenSLES();

    ~LeiOpenSLES();

    int getCurrentSampleRateForOpenSLES(int sample_rate);

    void prepare(int rate, slAndroidSimpleBufferQueueCallback callback, void *callBackContext);

    void play();

    void stop();

    void pause();

    void resume();

    void releasePlayer();

    void releaseSLES();

    void setVolume(int percent);

    void setMute(int mute);

private:
    void init();
};


#endif //NDK_LEARN_LEIOPENSLES_H
