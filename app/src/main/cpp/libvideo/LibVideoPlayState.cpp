//
// Created by Android on 2018/12/24.
//

#include "LibVideoPlayState.h"

LibVideoPlayState::LibVideoPlayState() {

}

LibVideoPlayState::~LibVideoPlayState() {

}

void LibVideoPlayState::init() {
    isExit = false;
    isPrepared = false;
    isStarted = false;
    isNeedStartedAfterPrepared = false;
    isErrorInPrepared = false;
    isSeek = false;
    isOnWaitingData = false;
}
