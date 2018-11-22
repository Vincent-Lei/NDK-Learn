//
// Created by Android on 2018/11/21.
//
#include <unistd.h>
#include "jni.h"
#include "string"
#include "native-log.h"
#include "pthread.h"
#include "CustomerAndProducer.h"

extern "C"
{
#include <libavformat/avformat.h>
};


extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_nativetest_NativeTestObject_printFFMPEGConfigInfo(JNIEnv *env, jobject instance) {
    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL) {
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                LOGD("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGD("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGD("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_nativetest_NativeTestObject_nativeCallJavaMethod(JNIEnv *env, jobject instance) {

//    public void callByNative(java.lang.String);
//    descriptor: (Ljava/lang/String;)V
    jclass _jclass = env->GetObjectClass(instance);
    jmethodID _jmid = env->GetMethodID(_jclass, "callByNative", "(Ljava/lang/String;)V");
    const char *hello = "this is native call toast hello";
    env->CallVoidMethod(instance, _jmid, env->NewStringUTF(hello));
}


CustomerAndProducer *customerAndProducer = NULL;

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_nativetest_NativeTestObject_nativeCustomerAndProducer(JNIEnv *env,
                                                                       jobject instance) {
    if (customerAndProducer != NULL)
        customerAndProducer->release();
    customerAndProducer = new CustomerAndProducer();
    customerAndProducer->start();
}
