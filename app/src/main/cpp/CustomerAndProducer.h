//
// Created by Android on 2018/11/21.
//

#ifndef NDK_LEARN_CUSTOMERANDPRODUCER_H
#define NDK_LEARN_CUSTOMERANDPRODUCER_H

#include "../native-log.h"
#include "pthread.h"
#include <unistd.h>

class CustomerAndProducer {
public:
    pthread_t pthread_customer;
    pthread_t pthread_producer;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool isExist = false;
    int dataSize = 0;
    int makeCount = 50;
public:
    CustomerAndProducer();

    ~CustomerAndProducer();

    void start();

    void release();
};


#endif //NDK_LEARN_CUSTOMERANDPRODUCER_H
