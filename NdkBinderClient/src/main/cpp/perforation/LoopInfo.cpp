//
// Created by janez on 27. 07. 21.
//

#include "LoopInfo.h"

int LoopInfo::getLoopId() const {
    return loopId;
}

void LoopInfo::setLoopId(int loopId) {
    LoopInfo::loopId = loopId;
}

int LoopInfo::getStoredFactor() const {
    return storedFactor;
}

void LoopInfo::setStoredFactor(int storedFactor) {
    LoopInfo::storedFactor = storedFactor;
}

int LoopInfo::getTime() const {
    return time;
}

void LoopInfo::setTime(int time) {
    LoopInfo::time = time;
}
