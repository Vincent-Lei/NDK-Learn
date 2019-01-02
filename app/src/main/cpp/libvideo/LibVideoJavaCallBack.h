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
    jmethodID jmid_checkHardwareCodecEnable = NULL;
    jmethodID jmid_initHardwareCodec = NULL;
    jmethodID jmid_harewareDecodeAVPacket = NULL;
    jmethodID jmid_prepare = NULL;
    jmethodID jmid_updateTimeClock = NULL;

    LibVideoJavaCallBack(JNIEnv *env, jobject *object);

    ~LibVideoJavaCallBack();

    void onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu, uint8_t *fv);

    bool callCheckIsHardwareCodecEnable(const char *codecName);

    void
    onCallInitHardwareCodec(const char *mime, int width, int height, int csd0_size, int csd1_size,
                            uint8_t *csd_0, uint8_t *csd_1);

    void onCallHarewareDecodeAVPacket(int datasize, uint8_t *data);

    void callPrepareFinished(bool success, int duration, int width, int height);

    void callUpdateTimeClock(int clock);

};


#endif //NDK_LEARN_LIBVIDEOJAVACALLBACK_H
