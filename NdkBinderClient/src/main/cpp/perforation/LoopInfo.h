//
// Created by janez on 27. 07. 21.
//

#ifndef ANDROIDNDKBINDEREXAMPLES_LOOPINFO_H
#define ANDROIDNDKBINDEREXAMPLES_LOOPINFO_H

//Custom object for caching the perforation factors and their infomation
class LoopInfo {

    int loopId;
    int storedFactor;
    int timeSec;
    int timeUSec;
    int lifeSpan;

public:
    int getStoredFactor() const;

    void setStoredFactor(int storedFactor);

    int getTimeSec() const;

    void setTimeSec(int timeSec);

    int getTimeUSec() const;

    void setTimeUSec(int timeUSec);

    int getLoopId() const;

    void setLoopId(int loopId);

    int getLifeSpan() const;

    void setLifeSpan(int lifeSpan);

};


#endif //ANDROIDNDKBINDEREXAMPLES_LOOPINFO_H
