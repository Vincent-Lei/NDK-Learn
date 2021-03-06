//
// Created by Android on 2018/12/10.
//

#include "LibJavaCallBack.h"

LibJavaCallBack::LibJavaCallBack(JNIEnv *env, jobject *object) {
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
    this->jmid_callAmplitude = env->GetMethodID(java_class, "onNativeCallAmplitude",
                                                "(I)V");
    this->jmid_callPCMRecord = env->GetMethodID(java_class, "onNativePCMRecord",
                                                "(II[B)V");
}

LibJavaCallBack::~LibJavaCallBack() {

}

void LibJavaCallBack::callJavaPrepared(int type) {
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

void LibJavaCallBack::callJavaError(int type, int code, char *msg) {
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

void LibJavaCallBack::callJavaDuration(int type, int current, int all) {
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

void LibJavaCallBack::callJavaFinished(int type) {
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

void LibJavaCallBack::callJavaAmplitude(int type, int amplitude) {
    if (type == MAIN_THREAD_CALL)
        env->CallVoidMethod(java_instance, jmid_callAmplitude, amplitude);
    else if (type == CHILD_THREAD_CALL) {
        extern JavaVM *m_thread_jvm;
        JNIEnv *jniEnv;
        if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
            jniEnv->CallVoidMethod(java_instance, jmid_callAmplitude, amplitude);
            m_thread_jvm->DetachCurrentThread();
        }
    }
}

void LibJavaCallBack::callJavaPCMRecord(int type, int sampleRate, int size, void *buffer) {
    if (type == MAIN_THREAD_CALL) {
        jbyteArray jbyteArray1 = env->NewByteArray(size);
        env->SetByteArrayRegion(jbyteArray1, 0, size, static_cast<const jbyte *>(buffer));
        env->CallVoidMethod(java_instance, jmid_callPCMRecord, sampleRate,
                            size, jbyteArray1);
    } else if (type == CHILD_THREAD_CALL) {
        extern JavaVM *m_thread_jvm;
        JNIEnv *jniEnv;
        if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
            jbyteArray jbyteArray1 = jniEnv->NewByteArray(size);
            jniEnv->SetByteArrayRegion(jbyteArray1, 0, size, static_cast<const jbyte *>(buffer));
            jniEnv->CallVoidMethod(java_instance, jmid_callPCMRecord, sampleRate,
                                   size, jbyteArray1);
            m_thread_jvm->DetachCurrentThread();
        }
    }
}

void LibJavaCallBack::callJavaDataOnLoad(int type, bool onLoad) {
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
