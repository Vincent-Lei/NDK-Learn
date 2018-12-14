//
// Created by Android on 2018/12/10.
//

#ifndef NDK_LEARN_LIBAUDIOPLAYER_H
#define NDK_LEARN_LIBAUDIOPLAYER_H

#include "LibAudioPlayState.h"
#include "LibAudioFFMPEG.h"
#include "LibAudioSource.h"
#include "SoundTouch.h"
#include "LibAudioOpenSLES.h"
#include "LibJavaCallBack.h"
#define CALL_BACK_AMPLITUDE_COUNT 5

class LibAudioPlayer {
public:
    LibAudioPlayState *playState = NULL;
    LibAudioFFMPEG *FFMPEG = NULL;
    LibAudioSource *audioSource = NULL;
    LibAudioOpenSLES *openSLES = NULL;
    LibJavaCallBack *javaCallBack = NULL;
    uint8_t *resampleBuff = NULL;
    //SoundTouch
    soundtouch::SoundTouch *soundTouch = NULL;
    soundtouch::SAMPLETYPE *sampleBuffer = NULL;
    bool isSoundTouchFinished = false;
    uint8_t *out_soundTouch_buffer = NULL;
    int data_st_num = 0;

    float pitch = 1.0f;
    float speed = 1.0f;
    int amplitudeCount = 0;

    pthread_mutex_t mutex_seek;

    LibAudioPlayer(JNIEnv *env, jobject *object);

    ~LibAudioPlayer();

    void prepare(const char *audioDataSource);

    void start();

    int resampleAudioPacket(uint8_t **out_soundTouch_buffer);

    int getSoundTouchData();

    int getPCMAmplitude(char *pcmcata, size_t pcmsize);

    void resetForPrepare();

    void seek(int64_t secTarget);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    void setRecordPCM(bool isRecordPCM);

    void onPause();

    void onResume();

    void destroy();
};


#endif //NDK_LEARN_LIBAUDIOPLAYER_H
