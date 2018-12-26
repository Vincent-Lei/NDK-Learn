//
// Created by Android on 2018/11/21.
//

#ifndef NDK_LEARN_JNI_ONLOAD_H
#define NDK_LEARN_JNI_ONLOAD_H
#define MAIN_THREAD_CALL 1
#define CHILD_THREAD_CALL 2
#include "jni.h"
#include "native-log.h"

extern JavaVM *m_thread_jvm;
extern JNIEnv *m_thread_env;

#endif //NDK_LEARN_JNI_ONLOAD_H
