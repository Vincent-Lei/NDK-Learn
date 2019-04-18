//
// Created by Android on 2019/4/17.
//


#include <pty.h>
#include <pthread.h>
#include <unistd.h>
#include "jni.h"
#include "native-log.h"


pthread_mutex_t pmt_1;
pthread_cond_t pct_1;
int productCount = 0;
int makeCount = 10;
bool makeFinished = false;

void *mutex_text_add(void *data) {
    while (makeCount > 0) {
        pthread_mutex_lock(&pmt_1);
        productCount++;
        LOGD("mutex_text_add productCount = %d", productCount);
        makeCount--;
        pthread_cond_signal(&pct_1);
        pthread_mutex_unlock(&pmt_1);
        usleep(30000);
    }
    makeFinished = true;
    return 0;
}

void *mutex_text_minus(void *data) {

    while (!makeFinished || productCount > 0) {
        pthread_mutex_lock(&pmt_1);
        while (productCount == 0) {
            LOGD("mutex_text_minus wait");
            pthread_cond_wait(&pct_1, &pmt_1);
        }
        productCount--;
        LOGD("mutex_text_minus = %d", productCount);
        usleep(30000);
        pthread_mutex_unlock(&pmt_1);
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_pthread_PthreadTest_testMutex(JNIEnv *env, jclass type) {
    LOGD("Java_com_lei_ndk_pthread_PthreadTest_testMutex");
    makeFinished = false;
    makeCount = 10;
    productCount = 0;

    pthread_mutex_init(&pmt_1, NULL);
    pthread_cond_init(&pct_1, NULL);

    pthread_t pt1;
    pthread_t pt2;

    pthread_create(&pt2, NULL, &mutex_text_minus, NULL);
    pthread_create(&pt1, NULL, &mutex_text_add, NULL);

    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);

    pthread_mutex_destroy(&pmt_1);
    pthread_cond_destroy(&pct_1);

}


pthread_rwlock_t pthread_rwlock;

void *rwlock_read(void *data) {
    pthread_rwlock_rdlock(&pthread_rwlock);
    LOGD("rwlock_read");
    usleep(3000000);
    pthread_rwlock_unlock(&pthread_rwlock);
    return 0;
}

void *rwlock_write(void *data) {
    usleep(3000);
    pthread_rwlock_wrlock(&pthread_rwlock);
    LOGD("rwlock_write");
    pthread_rwlock_unlock(&pthread_rwlock);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_pthread_PthreadTest_testRWLock(JNIEnv *env, jclass type) {
    LOGD("Java_com_lei_ndk_pthread_PthreadTest_testRWLock");

    pthread_rwlock_init(&pthread_rwlock, NULL);

    pthread_t pt1;
    pthread_t pt2;
    pthread_t pt3;

    pthread_create(&pt1, NULL, &rwlock_read, NULL);
    pthread_create(&pt2, NULL, &rwlock_read, NULL);
    pthread_create(&pt3, NULL, &rwlock_write, NULL);

    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);
    pthread_join(pt3, NULL);
    LOGD("Java_com_lei_ndk_pthread_PthreadTest_testRWLock__");

    pthread_rwlock_destroy(&pthread_rwlock);

}


void callback(void *data) {
    LOGD("%s", data);
}

void *thread_call_back(void *data) {
    //pthread_cleanup_push   pthread_cleanup_pop必须成对出现
    pthread_cleanup_push(callback, (void *) "this is a queue thread, and was terminated.");
            pthread_cleanup_pop(1);
    pthread_exit(0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_pthread_PthreadTest_testThreadCallBack(JNIEnv *env, jclass type) {
    LOGD("Java_com_lei_ndk_pthread_PthreadTest_testThreadCallBack");
    pthread_t pt1;
    pthread_create(&pt1, NULL, &thread_call_back, NULL);
    pthread_join(pt1, NULL);
    LOGD("Java_com_lei_ndk_pthread_PthreadTest_testThreadCallBack___");
}

void *thread_release(void *data) {
    LOGD("%s", data);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_lei_ndk_pthread_PthreadTest_testThreadRelease(JNIEnv *env, jclass type) {

    LOGD("Java_com_lei_ndk_pthread_PthreadTest_testThreadRelease");
    pthread_t pt1;
    pthread_create(&pt1, NULL, &thread_release, (void *) "release by pthread_join");
    int result = pthread_join(pt1, NULL);
    LOGD("joinResult = %d", result)

    pthread_t pt2;
    pthread_attr_t pat;//线程属性
    pthread_attr_init(&pat);//初始化线程属性
    pthread_attr_setdetachstate(&pat, PTHREAD_CREATE_DETACHED);  //设置线程属性
    pthread_create(&pt2, &pat, &thread_release, (void *) "release by PTHREAD_CREATE_DETACHED");

    result = pthread_join(pt2, NULL);
    LOGD("joinResult = %d", result);

    LOGD("Java_com_lei_ndk_pthread_PthreadTest_testThreadRelease______");

}