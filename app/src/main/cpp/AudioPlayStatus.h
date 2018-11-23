//
// Created by Android on 2018/11/22.
//

#ifndef NDK_LEARN_AUDIOPLAYSTATUS_H
#define NDK_LEARN_AUDIOPLAYSTATUS_H


class AudioPlayStatus {
public:
    bool isExist;
    bool isStarted;
    bool isPrepared;
    bool isDecodeFinished;
    bool isCostFinished;
    bool isCalledPrepare;
    bool isPreparedError;
    bool isNeedStartAfterPrepared;
    bool isDataOnLoad;
public:
    AudioPlayStatus();

    ~AudioPlayStatus();

    void init();
};


#endif //NDK_LEARN_AUDIOPLAYSTATUS_H
