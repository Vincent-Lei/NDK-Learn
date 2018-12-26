//
// Created by Android on 2018/12/24.
//
#include "jni.h"
#include "LibVideoPlayer.h"

extern "C"
JNIEXPORT jlong JNICALL
Java_com_sdk_video_VideoPlayer_nativeInit(JNIEnv *env, jobject instance) {

    return (jlong) (new LibVideoPlayer(env, &instance));
}


extern "C"
JNIEXPORT void JNICALL
Java_com_sdk_video_VideoPlayer_nativePrepare(JNIEnv *env, jobject instance, jlong ptr,
                                             jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);

    LibVideoPlayer *player = (LibVideoPlayer *) (ptr);
    if (player != NULL) {
        const int len = strlen(dataSource);
        char *copy_data = (char *) (malloc(sizeof(char) * (len + 1)));
        strcpy(copy_data, dataSource);
        copy_data[len] = '\0';
        LOGD("%s", copy_data);
        player->prepare(copy_data);
    }
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_sdk_video_VideoPlayer_nativeStart(JNIEnv *env, jobject instance, jlong ptr) {

    LibVideoPlayer *player = (LibVideoPlayer *) (ptr);
    if (player != NULL)
        player->start();
}