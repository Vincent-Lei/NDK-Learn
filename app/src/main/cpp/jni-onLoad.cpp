//
// Created by Android on 2018/11/21.
//
#include "jni-onLoad.h"
 JavaVM *m_thread_jvm;
 JNIEnv *m_thread_env;


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOGD("----JNI_OnLoad----")
    m_thread_jvm = vm;
    if (vm->GetEnv((void **) &m_thread_env, JNI_VERSION_1_4) == JNI_OK) {
        LOGD("----JNI_VERSION_1_4----")
        return JNI_VERSION_1_4;
    }
    if (vm->GetEnv((void **) &m_thread_env, JNI_VERSION_1_6) == JNI_OK) {
        LOGD("----JNI_VERSION_1_6----")
        return JNI_VERSION_1_6;
    }
    return -1;

}