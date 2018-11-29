//
// Created by Android on 2018/11/21.
//
#include "jni.h"
#include "native-log.h"
#include "LeiAudioPlayer.h"
#include "stdlib.h"
#include "cstring"


extern "C"
JNIEXPORT jlong JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativeInit(JNIEnv *env, jobject instance) {
    LeiAudioPlayer *player = new LeiAudioPlayer(env, &instance);
    player->initFFMPEG();
    LOGD("native create player success ptr：%d", player);
    return (jlong) (player);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativePrepare(JNIEnv *env, jobject instance, jlong mNativePtr,
                                                    jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    LeiAudioPlayer *player = (LeiAudioPlayer *) (mNativePtr);
    if (player != NULL) {
        char *copy_data = (char *) (malloc(sizeof(char) * strlen(dataSource)));
        strcpy(copy_data, dataSource);
        LOGD("%s", copy_data);
        player->prepared(copy_data);
    }
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativeStart(JNIEnv *env, jobject instance, jlong mNativePtr) {
    LeiAudioPlayer *player = (LeiAudioPlayer *) (mNativePtr);
    if (player != NULL) {
        player->start();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativePause(JNIEnv *env, jobject instance, jlong mNativePtr) {
    LeiAudioPlayer *player = (LeiAudioPlayer *) (mNativePtr);
    if (player)
        player->onPause();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativeResume(JNIEnv *env, jobject instance,
                                                   jlong mNativePtr) {
    LeiAudioPlayer *player = (LeiAudioPlayer *) (mNativePtr);
    if (player)
        player->onResume();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativeDestroy(JNIEnv *env, jobject instance,
                                                    jlong mNativePtr) {

    LeiAudioPlayer *player = (LeiAudioPlayer *) (mNativePtr);
    if (player) {
        player->destroy();
        delete player;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativeSeek(JNIEnv *env, jobject instance, jlong mNativePtr,
                                                 jint second) {

    LeiAudioPlayer *player = (LeiAudioPlayer *) (mNativePtr);
    if (player) {
        player->seek(second);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_audio_LeiAudioPlayer_nativeSetVolume(JNIEnv *env, jobject instance,
                                                      jlong mNativePtr, jint percent) {

    LeiAudioPlayer *player = (LeiAudioPlayer *) (mNativePtr);
    if (player) {
        player->setVolume(percent);
    }

}