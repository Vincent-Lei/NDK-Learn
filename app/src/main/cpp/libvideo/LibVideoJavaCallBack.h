//
// Created by Android on 2018/12/24.
//

#ifndef NDK_LEARN_LIBVIDEOJAVACALLBACK_H
#define NDK_LEARN_LIBVIDEOJAVACALLBACK_H


#include <jni.h>
#include "../globle/jni-onLoad.h"

class LibVideoJavaCallBack {
public:
    JNIEnv *env = NULL;
    jobject java_instance;
    jclass java_class;
    jmethodID jmid_callSetYUV = NULL;

    LibVideoJavaCallBack(JNIEnv *env, jobject *object);

    ~LibVideoJavaCallBack();

    void onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv);

};


#endif //NDK_LEARN_LIBVIDEOJAVACALLBACK_H
