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
#include "LeiOpenSLES.h"
#include "SoundTouch.h"

using namespace soundtouch;

class LeiAudioPlayer {
public:
    AudioJavaCallBack *javaCallBack = NULL;
    AudioPlayStatus *audioPlayStatus = NULL;
    AudioSource *audioSource = NULL;
    LeiOpenSLES *openSLES = NULL;
    uint8_t *resampleBuff = NULL;
    pthread_mutex_t mutex_seek;

    bool isRecordPCM = false;
    //SoundTouch
    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    bool isSoundTouchFinished = false;
    uint8_t *out_soundTouch_buffer = NULL;
    int data_nb_samples = 0;
    int data_st_num = 0;
    float pitch = 1.0f;
    float speed = 1.0f;
    int amplitudeCount = 0;

public:
    LeiAudioPlayer(JNIEnv *env, jobject *object);

    ~ LeiAudioPlayer();


    void prepared(const char *dataSource);

    void onPreparedFinished();

    void start();

    void onPause();

    void onResume();

    void destroy();

    void initFFMPEG();

    int getSoundTouchData();

    int resampleAudioPacket(uint8_t **out_soundTouch_buffer);

    void seek(int64_t secTarget);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float pitch);

    int getPCMAmplitude(char *pcmcata, size_t pcmsize);

    void setRecordPCM(bool isRecordPCM);

private:
    void resetToInit();
};


#endif //NDK_LEARN_LEIAUDIOPLAYER_H
