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
public:
    AudioJavaCallBack(JNIEnv *env, jobject *object);

    ~AudioJavaCallBack();

    void callJavaPrepared(int type);

    void callJavaError(int type, int code, char *msg);

    void callJavaDataOnLoad(int type, bool onLoad);
};


#endif //NDK_LEARN_AUDIOJAVACALLBACK_H
