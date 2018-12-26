//
// Created by Android on 2018/12/24.
//

#include "LibVideoJavaCallBack.h"

LibVideoJavaCallBack::LibVideoJavaCallBack(JNIEnv *env, jobject *object) {
    this->env = env;
    this->java_instance = env->NewGlobalRef(*object);
    this->java_class = env->GetObjectClass(java_instance);
    this->jmid_callSetYUV = env->GetMethodID(java_class, "nativeCallRenderYUV", "(II[B[B[B)V");
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
