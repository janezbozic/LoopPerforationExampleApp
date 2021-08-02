//
// Created by janez on 27. 07. 21.
//

#ifndef ANDROIDNDKBINDEREXAMPLES_BLACKSCHOLES_H
#define ANDROIDNDKBINDEREXAMPLES_BLACKSCHOLES_H

#ifdef __cplusplus
extern "C" {
#endif

//Main test function
int startBalckScholes (bool perf);

//Number of iterations in the test's main loop
int getNewCount();
void resetNewCount();

#ifdef __cplusplus
}
#endif

#endif //ANDROIDNDKBINDEREXAMPLES_BLACKSCHOLES_H
