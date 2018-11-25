//
// Created by Android on 2018/11/21.
//

#include "AudioJavaCallBack.h"


AudioJavaCallBack::AudioJavaCallBack(JNIEnv *env, jobject *object) {
    this->env = env;
    this->java_instance = env->NewGlobalRef(*object);
    this->java_class = env->GetObjectClass(java_instance);
    this->jmid_callPrepared = env->GetMethodID(java_class, "onNativeCallPrepared", "()V");
    this->jmid_callFinished = env->GetMethodID(java_class, "onNativeCallFinished", "()V");
    this->jmid_callError = env->GetMethodID(java_class, "onNativeCallError",
                                            "(ILjava/lang/String;)V");
    this->jmid_callDataOnLoad = env->GetMethodID(java_class, "onNativeCallDataOnLoad",
                                                 "(Z)V");
    this->jmid_callDuration = env->GetMethodID(java_class, "onNativeCallDuration",
                                               "(II)V");
}

AudioJavaCallBack::~AudioJavaCallBack() {

}

void AudioJavaCallBack::callJavaPrepared(int type) {
    if (type == MAIN_THREAD_CALL)
        env->CallVoidMethod(java_instance, jmid_callPrepared);
    else if (type == CHILD_THREAD_CALL) {
        extern JavaVM *m_thread_jvm;
        JNIEnv *jniEnv;
        if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
            jniEnv->CallVoidMethod(java_instance, jmid_callPrepared);
            m_thread_jvm->DetachCurrentThread();
        }
    }
}

void AudioJavaCallBack::callJavaError(int type, int code, char *msg) {
    if (type == MAIN_THREAD_CALL)
        env->CallVoidMethod(java_instance, jmid_callError, code, msg);
    else if (type == CHILD_THREAD_CALL) {
        extern JavaVM *m_thread_jvm;
        JNIEnv *jniEnv;
        if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
            jniEnv->CallVoidMethod(java_instance, jmid_callError, code, msg);
            m_thread_jvm->DetachCurrentThread();
        }
    }
}

void AudioJavaCallBack::callJavaDataOnLoad(int type, bool onLoad) {
    if (type == MAIN_THREAD_CALL)
        env->CallVoidMethod(java_instance, jmid_callDataOnLoad, onLoad);
    else if (type == CHILD_THREAD_CALL) {
        extern JavaVM *m_thread_jvm;
        JNIEnv *jniEnv;
        if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
            jniEnv->CallVoidMethod(java_instance, jmid_callDataOnLoad, onLoad);
            m_thread_jvm->DetachCurrentThread();
        }
    }
}

void AudioJavaCallBack::callJavaDuration(int type, int current, int all) {
    if (type == MAIN_THREAD_CALL)
        env->CallVoidMethod(java_instance, jmid_callDuration, current, all);
    else if (type == CHILD_THREAD_CALL) {
        extern JavaVM *m_thread_jvm;
        JNIEnv *jniEnv;
        if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
            jniEnv->CallVoidMethod(java_instance, jmid_callDuration, current, all);
            m_thread_jvm->DetachCurrentThread();
        }
    }
}

void AudioJavaCallBack::callJavaFinished(int type) {
    if (type == MAIN_THREAD_CALL)
        env->CallVoidMethod(java_instance, jmid_callFinished);
    else if (type == CHILD_THREAD_CALL) {
        extern JavaVM *m_thread_jvm;
        JNIEnv *jniEnv;
        if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
            jniEnv->CallVoidMethod(java_instance, jmid_callFinished);
            m_thread_jvm->DetachCurrentThread();
        }
    }
}
