//
// Created by Android on 2018/11/21.
//

#include "CustomerAndProducer.h"

void *makeData(void *data) {
    CustomerAndProducer *customerAndProducer = static_cast<CustomerAndProducer *>(data);
    while (!customerAndProducer->isExist && customerAndProducer->makeCount >= 0) {
        pthread_mutex_lock(&customerAndProducer->mutex);
        customerAndProducer->dataSize++;
        customerAndProducer->makeCount--;
        LOGD("make data and current size：%d", customerAndProducer->dataSize)
        pthread_cond_signal(&customerAndProducer->cond);
        pthread_mutex_unlock(&customerAndProducer->mutex);
        usleep(100 * 1000);
    }
    LOGD("makeData finished")
    return 0;
};

void *costData(void *data) {
    CustomerAndProducer *customerAndProducer = static_cast<CustomerAndProducer *>(data);
    while (!customerAndProducer->isExist &&
           (customerAndProducer->makeCount >= 0 || customerAndProducer->dataSize > 0)) {
        pthread_mutex_lock(&customerAndProducer->mutex);
        if (customerAndProducer->dataSize > 0) {
            customerAndProducer->dataSize--;
            LOGD("cost data and current size：%d", customerAndProducer->dataSize)
        } else
            pthread_cond_wait(&customerAndProducer->cond, &customerAndProducer->mutex);
        pthread_mutex_unlock(&customerAndProducer->mutex);
        usleep(120 * 1000);
    }
    LOGD("costData finished")
    return 0;
};


void CustomerAndProducer::start() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_create(&pthread_customer, NULL, makeData, this);
    pthread_create(&pthread_producer, NULL, costData, this);
}

CustomerAndProducer::CustomerAndProducer() {

}

CustomerAndProducer::~CustomerAndProducer() {
    LOGD("--release CustomerAndProducer--")
}

void CustomerAndProducer::release() {
    isExist = true;
    pthread_join(pthread_customer, NULL);
    pthread_join(pthread_producer, NULL);
    LOGD("--release--")
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    delete this;
}
