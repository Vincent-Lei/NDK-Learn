//
// Created by Android on 2018/11/21.
//
#include "jni.h"
#include "native-log.h"

extern "C"

JNIEXPORT void JNICALL
Java_com_lei_ndk_NDKObject_nativeHello(JNIEnv *env, jclass type) {
    LOGD("this is hello from native");
}


