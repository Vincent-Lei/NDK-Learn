//
// Created by Android on 2018/11/21.
//

#ifndef NDK_LEARN_NATIVE_LOG_H
#define NDK_LEARN_NATIVE_LOG_H
#include "android/log.h"
#define LOG_TAG "Vincent"
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,FORMAT,##__VA_ARGS__);
#endif //NDK_LEARN_NATIVE_LOG_H
