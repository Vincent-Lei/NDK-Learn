//
// Created by Android on 2018/11/22.
//

#include "AudioPlayStatus.h"

AudioPlayStatus::AudioPlayStatus() {
    init();
}

AudioPlayStatus::~AudioPlayStatus() {

}

void AudioPlayStatus::init() {
    isExist = false;
    isStarted = false;
    isPrepared = false;
    isDecodeFinished = false;
    isCostFinished = false;
    isCalledPrepare = false;
    isPreparedError = false;
    isNeedStartAfterPrepared = false;
}
