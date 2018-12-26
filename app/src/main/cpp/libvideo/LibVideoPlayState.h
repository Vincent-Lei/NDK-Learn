//
// Created by Android on 2018/12/24.
//

#ifndef NDK_LEARN_LIBVIDEOPLAYSTATE_H
#define NDK_LEARN_LIBVIDEOPLAYSTATE_H


class LibVideoPlayState {
public:
    bool isExit;
    bool isPrepared;
    bool isStarted;
    bool isNeedStartedAfterPrepared;
    bool isErrorInPrepared;
    bool isSeek;
    bool isOnWaitingData;
    LibVideoPlayState();

    ~LibVideoPlayState();

    void init();
};


#endif //NDK_LEARN_LIBVIDEOPLAYSTATE_H
