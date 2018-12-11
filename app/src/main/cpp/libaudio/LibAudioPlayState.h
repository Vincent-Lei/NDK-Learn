//
// Created by Android on 2018/12/10.
//

#ifndef NDK_LEARN_LIBAUDIOPLAYSTATE_H
#define NDK_LEARN_LIBAUDIOPLAYSTATE_H


class LibAudioPlayState {
public:
    bool isExit;
    bool isPrepared;
    bool isStarted;
    bool isNeedStartedAfterPrepared;
    bool isErrorInPrepared;
    bool isSeek;
    bool isDecodeFinished;
    bool isCostFinished;
    bool isOnWaitingData;
    bool isRecordPCM;

    LibAudioPlayState();

    ~LibAudioPlayState();

    void init();
};


#endif //NDK_LEARN_LIBAUDIOPLAYSTATE_H
