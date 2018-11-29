//
// Created by Android on 2018/11/21.
//
#include <unistd.h>
#include <iostream>
#include "jni.h"
#include <string>
#include "native-log.h"
#include "pthread.h"
#include "CustomerAndProducer.h"
#include "cstring"
#include "sstream"

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

int count_child = 0;

void watchMainProcess(char *c_packageDir, int c_sdkVersion) {
    LOGD("watch file path %s", c_packageDir);
    count_child = 0;
    while (JNI_TRUE) {
        FILE *file = NULL;
        if ((file = fopen(c_packageDir, "rt")) == NULL) {
            // 应用被卸载了，通知系统打开用户反馈的网页
            LOGD("app uninstall,current sdkversion = %d", c_sdkVersion);
            if (c_sdkVersion >= 17) {
                // Android4.2系统之后支持多用户操作，所以得指定用户
                execlp("am", "am", "start", "--user", "0", "-a",
                       "android.intent.action.VIEW", "-d",
                       "http://www.baidu.com", (char *) NULL);
                break;
            } else {
                // Android4.2以前的版本无需指定用户
                execlp("am", "am", "start", "-a",
                       "android.intent.action.VIEW", "-d",
                       "http://www.baidu.com", (char *) NULL);
                break;
            }
        } else {
            fclose(file);
            // 应用没有被卸载
            LOGD("app run normal");
        }
        count_child++;
        if (count_child > 6) {
            exit(EXIT_SUCCESS);
        }
        sleep(3);
    }
}

void *checkChildProcessIsAlive(void *data) {
    int *id = static_cast<int *>(data);
    using namespace std;
    stringstream stringstream;
    stringstream << "/proc/" << (*id);
    const char *path = stringstream.str().c_str();
    LOGD("path = %s", path);
    FILE *file = NULL;
    while ((file = fopen(path, "rt")) != NULL) {
        fclose(file);
        LOGD("process pid = %d is alive", *id);
        sleep(3);
    }
    LOGE("process pid = %d is dead", *id);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_nativetest_NativeTestObject_watchUninstall(JNIEnv *env, jobject instance,
                                                            jstring packageDir_, jint sdkVersion) {
    const char *packageDir = env->GetStringUTFChars(packageDir_, 0);
    int c_sdkVersion = sdkVersion;
    char *c_packageDir = static_cast<char *>(malloc(sizeof(char) * strlen(packageDir)));
    strcpy(c_packageDir, packageDir);
    env->ReleaseStringUTFChars(packageDir_, packageDir);

    //创建当前进程的克隆进程
    pid_t pid = fork();
    if (pid < 0) {
        LOGE("fork process failed");
    } else if (pid > 0) {
        LOGD("fork process success pid = %d", pid);
        delete (c_packageDir);
        pthread_t pthread1;
        int *id = new int;
        *id = pid;
        pthread_create(&pthread1, NULL, checkChildProcessIsAlive, id);
    } else {
        LOGD("fork process success and current int child process");
        watchMainProcess(c_packageDir, c_sdkVersion);
    }
}
