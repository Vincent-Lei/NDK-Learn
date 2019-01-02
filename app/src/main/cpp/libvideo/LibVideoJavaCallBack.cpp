//
// Created by Android on 2018/12/24.
//

#include "LibVideoJavaCallBack.h"

LibVideoJavaCallBack::LibVideoJavaCallBack(JNIEnv *env, jobject *object) {
    this->env = env;
    this->java_instance = env->NewGlobalRef(*object);
    this->java_class = env->GetObjectClass(java_instance);
    this->jmid_callSetYUV = env->GetMethodID(java_class, "nativeCallRenderYUV", "(II[B[B[B)V");
    this->jmid_checkHardwareCodecEnable = env->GetMethodID(java_class,
                                                           "nativeCheckIsHardwareCodecEnable",
                                                           "(Ljava/lang/String;)Z");
    this->jmid_initHardwareCodec = env->GetMethodID(java_class, "nativeCallInitHardwareCodec",
                                                    "(Ljava/lang/String;II[B[B)V");
    this->jmid_harewareDecodeAVPacket = env->GetMethodID(java_class,
                                                         "nativeCallHarewareDecodeAVPacket",
                                                         "(I[B)V");

    this->jmid_prepare = env->GetMethodID(java_class, "nativeCallPreparedFinished", "(ZIII)V");
    this->jmid_updateTimeClock = env->GetMethodID(java_class, "nativeCallUpdateTimeClock", "(I)V");
}

LibVideoJavaCallBack::~LibVideoJavaCallBack() {

}

void LibVideoJavaCallBack::onCallRenderYUV(int width, int height, uint8_t *fy, uint8_t *fu,
                                           uint8_t *fv) {
    extern JavaVM *m_thread_jvm;
    JNIEnv *jniEnv;
    if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
        jbyteArray y = jniEnv->NewByteArray(width * height);
        jniEnv->SetByteArrayRegion(y, 0, width * height, reinterpret_cast<const jbyte *>(fy));

        jbyteArray u = jniEnv->NewByteArray(width * height / 4);
        jniEnv->SetByteArrayRegion(u, 0, width * height / 4, reinterpret_cast<const jbyte *>(fu));

        jbyteArray v = jniEnv->NewByteArray(width * height / 4);
        jniEnv->SetByteArrayRegion(v, 0, width * height / 4, reinterpret_cast<const jbyte *>(fv));

        jniEnv->CallVoidMethod(java_instance, jmid_callSetYUV, width, height, y, u, v);

        jniEnv->DeleteLocalRef(y);
        jniEnv->DeleteLocalRef(u);
        jniEnv->DeleteLocalRef(v);

        m_thread_jvm->DetachCurrentThread();
    }
}

bool LibVideoJavaCallBack::callCheckIsHardwareCodecEnable(const char *codecName) {
    extern JavaVM *m_thread_jvm;
    JNIEnv *jniEnv;
    if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
        jstring jstr = jniEnv->NewStringUTF(codecName);
        bool enable = jniEnv->CallBooleanMethod(java_instance, jmid_checkHardwareCodecEnable, jstr);
        jniEnv->DeleteLocalRef(jstr);
        m_thread_jvm->DetachCurrentThread();
        return enable;
    }
    return false;
}

void LibVideoJavaCallBack::onCallInitHardwareCodec(const char *mime, int width, int height,
                                                   int csd0_size, int csd1_size, uint8_t *csd_0,
                                                   uint8_t *csd_1) {
    extern JavaVM *m_thread_jvm;
    JNIEnv *jniEnv;
    if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
        jstring type = jniEnv->NewStringUTF(mime);
        jbyteArray csd0 = jniEnv->NewByteArray(csd0_size);
        jniEnv->SetByteArrayRegion(csd0, 0, csd0_size, reinterpret_cast<const jbyte *>(csd_0));
        jbyteArray csd1 = jniEnv->NewByteArray(csd1_size);
        jniEnv->SetByteArrayRegion(csd1, 0, csd1_size, reinterpret_cast<const jbyte *>(csd_1));
        jniEnv->CallVoidMethod(java_instance, jmid_initHardwareCodec, type, width, height, csd0,
                               csd1);
        jniEnv->DeleteLocalRef(csd0);
        jniEnv->DeleteLocalRef(csd1);
        jniEnv->DeleteLocalRef(type);
        m_thread_jvm->DetachCurrentThread();
    }

}

void LibVideoJavaCallBack::onCallHarewareDecodeAVPacket(int dataSize, uint8_t *buff) {
    extern JavaVM *m_thread_jvm;
    JNIEnv *jniEnv;
    if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
        jbyteArray data = jniEnv->NewByteArray(dataSize);
        jniEnv->SetByteArrayRegion(data, 0, dataSize, reinterpret_cast<const jbyte *>(buff));
        jniEnv->CallVoidMethod(java_instance, jmid_harewareDecodeAVPacket, dataSize, data);
        jniEnv->DeleteLocalRef(data);
        m_thread_jvm->DetachCurrentThread();
    }
}

void LibVideoJavaCallBack::callPrepareFinished(bool success, int duration, int width, int height) {
    extern JavaVM *m_thread_jvm;
    JNIEnv *jniEnv;
    if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
        jniEnv->CallVoidMethod(java_instance, jmid_prepare, success, duration, width, height);
        m_thread_jvm->DetachCurrentThread();
    }
}

void LibVideoJavaCallBack::callUpdateTimeClock(int clock) {
    extern JavaVM *m_thread_jvm;
    JNIEnv *jniEnv;
    if (m_thread_jvm->AttachCurrentThread(&jniEnv, NULL) == JNI_OK) {
        jniEnv->CallVoidMethod(java_instance, jmid_updateTimeClock, clock);
        m_thread_jvm->DetachCurrentThread();
    }
}
