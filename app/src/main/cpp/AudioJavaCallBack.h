//
// Created by Android on 2018/11/21.
//

#ifndef NDK_LEARN_AUDIOJAVACALLBACK_H
#define NDK_LEARN_AUDIOJAVACALLBACK_H

#define MAIN_THREAD_CALL 1
#define CHILD_THREAD_CALL 2

#include "jni.h"
#include "jni-onLoad.h"

class AudioJavaCallBack {
public:
    JNIEnv *env = NULL;
    jobject java_instance;
    jclass java_class;
    jmethodID jmid_callPrepared = NULL;
    jmethodID jmid_callError = NULL;
    jmethodID jmid_callDataOnLoad = NULL;
    jmethodID jmid_callDuration = NULL;
    jmethodID jmid_callFinished = NULL;
    jmethodID jmid_callAmplitude = NULL;
    jmethodID jmid_callPCMRecord = NULL;
public:
    AudioJavaCallBack(JNIEnv *env, jobject *object);

    ~AudioJavaCallBack();

    void callJavaPrepared(int type);

    void callJavaError(int type, int code, char *msg);

    void callJavaDataOnLoad(int type, bool onLoad);

    void callJavaDuration(int type, int current, int all);

    void callJavaFinished(int type);

    void callJavaAmplitude(int type, int amplitude);

    void callJavaPCMRecord(int type, int sampleRate, int size, void *buffer);
};


#endif //NDK_LEARN_AUDIOJAVACALLBACK_H
