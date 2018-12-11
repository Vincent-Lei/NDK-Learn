//
// Created by Android on 2018/11/21.
//

#ifndef NDK_LEARN_JNI_ONLOAD_H
#define NDK_LEARN_JNI_ONLOAD_H

#include "jni.h"
#include "native-log.h"

extern JavaVM *m_thread_jvm;
extern JNIEnv *m_thread_env;

#endif //NDK_LEARN_JNI_ONLOAD_H
