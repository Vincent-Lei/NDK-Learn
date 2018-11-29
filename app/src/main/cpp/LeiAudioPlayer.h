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

class LeiAudioPlayer {
public:
    AudioJavaCallBack *javaCallBack = NULL;
    AudioPlayStatus *audioPlayStatus = NULL;
    AudioSource *audioSource = NULL;
    LeiOpenSLES *openSLES = NULL;
    uint8_t *resampleBuff = NULL;
    pthread_mutex_t mutex_seek;

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

    int resampleAudioPacket();

    void seek(int64_t secTarget);


private:
    void resetToInit();
};


#endif //NDK_LEARN_LEIAUDIOPLAYER_H
