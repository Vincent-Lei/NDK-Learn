//
// Created by Android on 2018/12/10.
//

#include "LibAudioPlayState.h"

LibAudioPlayState::LibAudioPlayState() {
    init();
}

LibAudioPlayState::~LibAudioPlayState() {

}

void LibAudioPlayState::init() {
    isExit = false;
    isPrepared = false;
    isStarted = false;
    isErrorInPrepared = false;
    isNeedStartedAfterPrepared = false;
    isSeek = false;
    isDecodeFinished = false;
    isOnWaitingData = false;
    isRecordPCM = false;
    isCostFinished = false;
}
