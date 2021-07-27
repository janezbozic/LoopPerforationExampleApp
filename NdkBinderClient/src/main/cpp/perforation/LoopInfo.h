//
// Created by janez on 27. 07. 21.
//

#ifndef ANDROIDNDKBINDEREXAMPLES_LOOPINFO_H
#define ANDROIDNDKBINDEREXAMPLES_LOOPINFO_H


class LoopInfo {

    int loopId;
    int storedFactor;
    int time;

public:
    int getStoredFactor() const;

    void setStoredFactor(int storedFactor);

    int getTime() const;

    void setTime(int time);

    int getLoopId() const;

    void setLoopId(int loopId);

};


#endif //ANDROIDNDKBINDEREXAMPLES_LOOPINFO_H
