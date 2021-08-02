//
// Created by janez on 27. 07. 21.
//

#include "LoopInfo.h"

//Custom object for caching the perforation factors and their infomation

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

int LoopInfo::getLifeSpan() const {
    return lifeSpan;
}

void LoopInfo::setLifeSpan(int lifeSpan) {
    LoopInfo::lifeSpan = lifeSpan;
}

int LoopInfo::getTimeSec() const {
    return timeSec;
}

void LoopInfo::setTimeSec(int timeSec) {
    LoopInfo::timeSec = timeSec;
}

int LoopInfo::getTimeUSec() const {
    return timeUSec;
}

void LoopInfo::setTimeUSec(int timeUSec) {
    LoopInfo::timeUSec = timeUSec;
}
